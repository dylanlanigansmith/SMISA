

#include "emu.hpp"
#define DEBUG 1

enum OPERATIONS : uint8_t
{
    NOP = 0u,
    ADD,
    SUB,
    LD,
    MOV,
    JMP,
    JNE,
    HLT,
    LADR, 
};


enum REG_ARG_IDS : uint8_t {
    REG_A = 0xA,
    REG_B,
    REG_C
};

struct opcode_t  
{
    
    uint8_t op : 4;
    uint8_t is_addr : 1;
    uint8_t is_byte : 1;
    uint8_t is_RA : 1;
    uint8_t is_RB : 1;
    uint8_t args;

    operator uint16_t () const{
        return *(uint16_t*)(this); 
    }
} __attribute__((packed));

static_assert(sizeof(opcode_t) == 2);

void print_binary(uint16_t num) {
    for (int i = 15; i >= 0; --i) {
        printf("%d", (num >> i) & 1);
        if (i % 4 == 0) printf(" "); 
    }
}
void print_binary(uint8_t num) {
    for (int i = 7; i >= 0; --i) {
        printf("%d", (num >> i) & 1);
        if (i % 4 == 0) printf(" "); 
    }
}


const char* get_opcode_name(uint8_t o){
    return magic_enum::enum_name<OPERATIONS>((OPERATIONS)o).data();
}

void print_opcode(const opcode_t& code, bool brief = false){
    uint16_t raw = *(uint16_t*)(&code);
    uint8_t* raw_b = (uint8_t*)(&raw);
    if(brief){
        printf(" 0x%hx [%hhx]{%hhx} %s \n", raw, raw_b[0], raw_b[1], get_opcode_name(code.op)); return;
    }

    printf("Decoding 0x%hx [%hhx]{%hhx} \n", raw, raw_b[0], raw_b[1]);
    printf("  - op = %hhx = %s \n", code.op, magic_enum::enum_name<OPERATIONS>((OPERATIONS)code.op).data());
    printf("  - is_addr = %hhx \n", code.is_addr);
    printf("  - is_byte = %hhx \n", code.is_byte);
    printf("  - is_rel_addr = %hhx \n", code.is_addr && code.is_byte);
    printf("  - is_RA = %hhx \n", code.is_RA);
    printf("  - is_RB = %hhx \n", code.is_RB);
    printf("  - is_RC = %hhx \n", code.is_RA && code.is_RB);
    printf("  - args = %hhx \n  ", code.args);
    print_binary(raw);
    puts("\n\n");
}


#define MEM_SIZE 0x1000
struct cpu_t
{
    uint16_t RIP, RX;
    uint8_t RA, RB, RC;
    uint8_t mem[MEM_SIZE]; //4k memory


    void reset() { memset(this, 0u, sizeof(cpu_t)); printf("smisma-emu reloaded...  [mem_size=%lx, debug=%d] \n", MEM_SIZE, DEBUG);  }
    void setmem(uint16_t addr, uint8_t val){
        if(!valid_address(addr)) return;
        mem[addr] = val;
    }
    auto getmem(uint16_t addr = 0xffff){ //with default arg returns address in RX
        if(addr == 0xffff) return mem[(size_t)RX];
        if(!valid_address(addr)) return (uint8_t)0xAA;
        return mem[addr];
    }
    auto getmem16(uint16_t addr = 0xffff){ //with default arg returns address in RX
        if(addr == 0xffff) return *(uint16_t*)(&mem[RX]);
        if(!valid_address(addr)) return (uint16_t)0xDEAD;
        return *(uint16_t*)(&mem[addr]);
    }

    opcode_t* get_instruction(){
        return (opcode_t*)(&mem[(size_t)RIP]);
    }

    bool valid_address(uint16_t addr){
        if(addr > MEM_SIZE){
            ERROR("Invalid Address %hhx", addr); return false;
        }
        return addr < MEM_SIZE;
    }
    cpu_t() { this->reset(); }


    bool load_prog(const uint8_t* prog, size_t len = MEM_SIZE){
       
        memcpy(mem, prog, len);

        return true;
    }

    uint8_t* get_reg_from_arg(uint8_t ar){
    switch(ar){
        case REG_A: return &RA;
        case REG_B: return &RB;
        case REG_C: return &RC;
        default: ERROR("get_reg_from_arg(): unknown reg type %hhx", ar); return &RC; //seg fault we aint checkin
    }
}

    void dump(const char* msg = "CPU Dump"){
        printf("== %s == \n", msg);
        printf("  Registers: \n");
        printf("   - RA = 0x%hhx / %hhu \n", RA, RA);
        printf("   - RB = 0x%hhx / %hhu\n", RB, RB);
        printf("   - RC = 0x%hhx / %hhu\n", RC, RC);
        printf("   - RX = 0x%hx / %hu\n", RX, RX);
        printf("   - RIP = 0x%hx / %hu\n", RIP, RIP);
        printf("  Stats: \n");
        printf("   - MEM = 0x%lx bytes [%lu kb] \n", MEM_SIZE, MEM_SIZE / 1024);

    }

    void dump_mem(bool all = false, size_t max = MEM_SIZE){
        printf("-- memory dump -- \n");
        for(size_t i = 0; i < max; ++i){
            if(mem[i] == 0x0 && !all) continue;

            
            printf(" [0x%04lx] | %02hhx %02hhx \n", i, mem[i], mem[i + 1] );
            i += 1;
        }
        printf("-- -- -- --\n");
    }
};


std::vector<std::string> split(const std::string& str) {
    std::vector<std::string> tokens;
    std::stringstream ss(str);
    std::string token;

    while (std::getline(ss, token, ' ')) {
        size_t start = 0, end = 0;
        while ((end = token.find(',', start)) != std::string::npos) {
            if (end != start) {
                tokens.push_back(token.substr(start, end - start));
            }
            start = end + 1;
        }
        if (start < token.size()) {
            tokens.push_back(token.substr(start));
        }
    }

    return tokens;
}

struct program_t
{
    uint8_t* data;
    size_t size;
    bool ok = false;
};

template <typename T>
T hexstr_to(const std::string& hex_str){
    T addr;
    std::stringstream ss;
    ss << std::hex << hex_str;
    ss >> addr;
    return addr; 
}

bool is_hexstr(const std::string& str) {
    if (str.size() > 2 && str[0] == '0' && str[1] == 'x') {
        return std::all_of(str.begin() + 2, str.end(), [](unsigned char c) {
            return std::isxdigit(c);
        });
    }
    return false;
}



std::vector<const char*> symbols_reg = {
    "RA", "RB","RC"
};

bool symbol_exists(const char* to_find, const std::vector<const char*>& symbols){
    
            auto it = std::find_if(symbols.begin(), symbols.end(), [to_find](const char *element)
                                   { return strcmp(element, to_find) == 0; });
            return it != symbols.end();
}
std::optional<std::pair<std::string, uint16_t>> label_exists(const std::string& to_find, const std::vector<std::pair<std::string, uint16_t>>& labels){
    
            auto it = std::find_if(labels.begin(), labels.end(), [to_find](const std::pair<std::string, uint16_t>& element)
                                   { return to_find == element.first; });
            if ( it != labels.end())
                return std::optional<std::pair<std::string, uint16_t>>(*it);
            return std::optional<std::pair<std::string, uint16_t>>();
}

program_t assemble(const char* filename) {
    std::ifstream infile(filename);
    std::string line;

    auto prog = program_t{0,0};

    if (!infile.is_open()) {
        std::cerr << "Failed to open file. " << filename << std::endl;
        return prog;
    }
    std::vector<uint16_t> assembly;
    bool add_next_label = false;
    std::vector<std::pair<std::string, uint16_t>> labels;
    while (std::getline(infile, line)) {
        std::vector<std::string> raw_tokens = split(line);
        std::vector<std::string> tokens;
        
        if(line.empty()) continue;
        std::cout << "Line: " << line << std::endl;
        
        for (const auto& token : raw_tokens) {
            //std::cout << "[" << token << "] ";
            if (token.find(';') != std::string::npos){
                break; // rest is a comment
            }
            if (token.find(':') != std::string::npos){
                add_next_label = true;
                labels.push_back({token.substr(0, token.length() - 1), 0xffff});
                break; // rest is a comment
            }
            tokens.push_back(token);
        }

        if(tokens.empty()){ std::cout << std::endl; continue; }
          std::cout << "Tokens: ";
        for (const auto& token : tokens) {
            std::cout << "[" << token << "] " ;
            if(*(tokens.at(0).end() - 1) == ':'){
            std::cout << "found label" << tokens.at(0) << std::endl;
            continue;
        }
        }

        

        std::cout <<std::endl;
        auto op = magic_enum::enum_cast<OPERATIONS>(tokens.front());
        if(!op.has_value()){
            std::cerr << "Unknown Operation " << tokens.front() << std::endl;
            return prog;
        }
        std::cout << "assembling " << get_opcode_name(op.value()) << " " << std::endl;

        switch (op.value())
        {
        case NOP: {
            opcode_t oc = { .op = NOP, .is_addr = 0, .is_byte = 0, .is_RA = 0, .is_RB = 0 };
            assembly.push_back(oc);
            break;
        }
         
        case HLT:{
            opcode_t oc = { .op = HLT, .is_addr = 0, .is_byte = 0, .is_RA = 0, .is_RB = 0, .args = 0x77 };
            assembly.push_back(oc);
            break;
        } 
        case LADR:{
            //Syntax means next token should always be 0xffff, 0x24 etc.
            auto label = label_exists(tokens.at(1), labels);
            
            uint16_t address = label.has_value() ? label.value().second : hexstr_to<uint16_t>(tokens.at(1)); 
            opcode_t oc = { .op = LADR, .is_addr = 0, .is_byte = 0, .is_RA = 0, .is_RB = 0 };
            assembly.push_back(oc);
            assembly.push_back(address);
            break;
        }
        case LD:
        {
            /*
            LD REG, BYTE
            LD REG, REG
            //LD REG ADDR
            LD ADDR, REG
            LD ADDR. BYTE
            */
            opcode_t oc = { .op = LD, .is_addr = 0, .is_byte = 0, .is_RA = 0, .is_RB = 0, .args = 0 };
          //  if(tokens.size() < 3)  std::cout << "Too few arguments for instruction" << tokens.size() << std::endl; return prog;

            bool dest_isreg = symbol_exists(tokens.at(1).c_str(), symbols_reg); //overkill for what could be first char = R 
            bool src_isreg =  symbol_exists(tokens.at(2).c_str(), symbols_reg);

            bool dest_isaddr = (dest_isreg) ? false : is_hexstr(tokens.at(1));
            bool src_isbyte = (src_isreg) ? false : is_hexstr(tokens.at(2));
            //todo addr


            if(dest_isreg){
                std::string reg = tokens.at(1);
                for (auto & c: reg) c = toupper(c);

                if(reg == "RA"){
                    oc.is_RA = 1;
                } 
                else if (reg == "RB"){
                    oc.is_RB = 1;
                }
                else if(reg == "RC") {
                    oc.is_RA = oc.is_RB = 1;
                } 
            }

            if(src_isreg){
                std::string reg = tokens.at(2);
                for (auto & c: reg) c = toupper(c);
                if(reg == "RA"){
                    oc.args = REG_A;
                } 
                else if (reg == "RB"){
                    oc.args = REG_B;
                }
                else if(reg == "RC") {
                    oc.args = REG_C;
                } 
            }
            
            if(src_isbyte){
                oc.is_byte = 1;
                uint16_t u16 = hexstr_to<uint16_t>(tokens.at(2)); //dont ask
                oc.args = (uint8_t)(u16 & 0xff);
            }


            oc.is_addr = dest_isaddr;
            
            assembly.push_back(oc);
            if(dest_isaddr){
                assembly.push_back(hexstr_to<uint16_t>(tokens.at(1))); 
            }
            //printf("srcr %u destr %u b %u ad %u args %x \n",src_isreg, dest_isreg, src_isbyte, dest_isaddr, oc.args);
            //print_opcode(oc);
            break;
        }
        case SUB:
        case ADD: {
            /*
            ADD REG
            ADD BYTE
            ADD ADDR
            SUB ^^ **
            */
           //only 1 arg for now
            opcode_t oc = { .op = (op.value() == ADD) ? ADD : SUB, .is_addr = 0, .is_byte = 0, .is_RA = 0, .is_RB = 0, .args = 0 };   
            bool src_isreg =  symbol_exists(tokens.at(1).c_str(), symbols_reg);

            bool src_isaddr = false;// (src_isreg) ? false : is_hexstr(tokens.at(1));
            bool src_isbyte = (src_isreg) ? false : is_hexstr(tokens.at(1));
            //todo addr



            if(src_isreg){
                std::string reg = tokens.at(1);
                for (auto & c: reg) c = toupper(c);

                if(reg == "RA"){
                    oc.is_RA = 1;
                } 
                else if (reg == "RB"){
                    oc.is_RB = 1;
                }
                else if(reg == "RC") {
                    oc.is_RA = oc.is_RB = 1;
                } 
            }            
            if(src_isbyte){
                oc.is_byte = 1;
                uint16_t u16 = hexstr_to<uint16_t>(tokens.at(1)); //dont ask
                oc.args = (uint8_t)(u16 & 0xff);
            }


            oc.is_addr = src_isaddr;
            
            assembly.push_back(oc);
            
            //printf("srcr %u destr %u b %u ad %u args %x \n",src_isreg, dest_isreg, src_isbyte, dest_isaddr, oc.args);
            print_opcode(oc);
            break;
        }
        case JNE: {
            opcode_t oc = { .op = JNE, .is_addr = 0, .is_byte = 0, .is_RA = 0, .is_RB = 0, .args = 0 };   
            bool src_isreg =  symbol_exists(tokens.at(1).c_str(), symbols_reg);
            assert(src_isreg);
             if(src_isreg){
                std::string reg = tokens.at(1);
                for (auto & c: reg) c = toupper(c);

                if(reg == "RA"){
                    oc.is_RA = 1;
                } 
                else if (reg == "RB"){
                    oc.is_RB = 1;
                }
                else if(reg == "RC") {
                    oc.is_RA = oc.is_RB = 1;
                } 
            }            
            assembly.push_back(oc);
            break;

        }
        case JMP:{
            opcode_t oc = { .op = JMP, .is_addr = 0, .is_byte = 0, .is_RA = 0, .is_RB = 0, .args = 0 };   
            assembly.push_back(oc);  
            break;
        }
        default:
            std::cerr << "Unknown Operation Handler for " << tokens.front() << std::endl; return prog;
            break;
        }

        if(add_next_label){
            add_next_label = false;
            labels.back().second = assembly.size() * 2;
        }
    }

    puts("-LABELS-");
    for(auto& label : labels){
        std::cout << label.first << " = " << label.second << std::endl;
    }


    infile.close();
    prog.size = assembly.size() * 2; //size in bytes
    prog.data = new uint8_t[prog.size];
    memcpy(prog.data, assembly.data(), prog.size);
    prog.ok = true;
    return prog;
}



cpu_t cpu;

const uint8_t program2[] =
{
    0b01000010, 0xff, //ADD RA
    0b00010100, 0x10, //ADD 0x10
    0b00110011, 0x00, //LD RC 
    0b00000111, 0x69 // UKN / HLT
};

const uint8_t program[] =
{   
    0b00001000, 0x00, // LDADR 0x6 // RX = 0x6
    0x8, 0x00,
    0b10000110, 0x00, //JNE RB
    0b00000111, 0x77, // UKN / HLT
    0b00100001, 0xFF, //ADD BYTE FF // RC = RC + FF
    0b00100010, 0xFE, //SUB BYTE FE // RC = RC - FE
    0b10100011, 0x42, //LD RB BYTE FF // 
    0b01000011, 0x0B, //LD RA RB
    0b11000011, 0x0A, //LD RC RA
    0b00100001, 0x01, //ADD BYTE FF // RC = RC + FF
    0b11000011, 0x0A, //LD RC RA
    0b00001000, 0x00, // LDADR 0x700 // RX = 0x700
    0x0, 0x0,
    0b00100010, 0x42, //SUB BYTE from RC
    0b11000110, 0x00, //JNE RC
    0b10100011, 0x11, //LD RB BYTE FF // 
    0b00000111, 0x77 // UKN / HLT
};



int main(int argc, const char * argv[]) {
    // insert code here...
    const opcode_t o = *(opcode_t*)(program);
   // print_opcode(o);
   // print_opcode(*(opcode_t*)(program + 2));
   // print_opcode(*(opcode_t*)(program + 4));
   // print_opcode(*(opcode_t*)(program + 6));
    auto p = assemble("/Users/dylan/Code/SMISA/emu/asm/test.sma");

    if(p.size){
        cpu.load_prog(p.data, p.size);
        cpu.dump_mem(1, 0x40);
    }
 // return 0 ;
  //  if(cpu.load_prog(program, sizeof(program)))
      printf("loaded program...\n");
 //   cpu.dump_mem();
 if(!p.ok) return 0;

    for(;;)
    {
        
        auto instruction = cpu.get_instruction();
        printf("$%04hx | %5s = ", cpu.RIP, get_opcode_name(instruction->op));

        bool verbose = true;
        if(instruction->op != NOP && verbose) 
            print_opcode(*instruction, 1);
        else putc('\n', stdout);

        uint16_t instruction_size = 2;
        if(instruction->op == HLT) break; 
        switch (instruction->op)
        {
        case ADD:
            if(instruction->is_byte){
                cpu.RC += instruction->args; break;
            }
            else if (instruction->is_addr){
                cpu.RC += cpu.getmem(); break;
                //no rel yet
            } 

            if(instruction->is_RA && instruction->is_RB){
                cpu.RC += cpu.RC;
            } else if (instruction->is_RA) {
                
                cpu.RC += cpu.RA;
            } else if (instruction->is_RB) { 
                cpu.RC += cpu.RB;
            } 
            break;
        case SUB:
            if(instruction->is_byte){
                cpu.RC -= instruction->args; break;
            }
            else if (instruction->is_addr){
                cpu.RC -= cpu.getmem(); break;
                //no rel yet
            } 

            if(instruction->is_RA && instruction->is_RB){
                cpu.RC -= cpu.RC;
            } else if (instruction->is_RA) {
                
                cpu.RC -= cpu.RA;
            } else if (instruction->is_RB) { 
                cpu.RC -= cpu.RB;
            } 
            break;
        case LD:
            if(instruction->is_byte){
                if(instruction->is_RA && instruction->is_RB){
                    cpu.RC = instruction->args;
                } else if (instruction->is_RA) {
                    cpu.RA = instruction->args;
                } else if (instruction->is_RB) { 
                    cpu.RB = instruction->args;
                } 
                if(instruction->is_addr){
                    cpu.setmem(cpu.RX, instruction->args);
                }
            }
            else{
                uint8_t* r = (instruction->is_RA && instruction->is_RB) ? ( &cpu.RC) : ( ( instruction->is_RA ) ? &cpu.RA : &cpu.RB ); //sorry

                if (instruction->is_addr){
                    if(!instruction->is_RA && !instruction->is_RB) {
                        r = cpu.get_reg_from_arg(instruction->args); //so is_addr flag with no reg flags = set reg to value at addr
                        cpu.setmem(cpu.RX, *r);

                     
                    } else{
                        *r = cpu.getmem(); 
                    }
                    

                    //printf("%lx %lx %lx v %lx", &cpu.RA, &cpu.RB, &cpu.RC, r);
                } else {
                    *r = *cpu.get_reg_from_arg(instruction->args);
                }
            } 
            break;
        case MOV:
            break; //probs irrel
        case JNE:
            if(instruction->is_RA && instruction->is_RB){
                if(cpu.RC) break;
            } else if (instruction->is_RA) {
                if(cpu.RA) break;
            } else if (instruction->is_RB) { 
                 if(cpu.RB) break;
            } 
         
           if ( (instruction->is_addr && cpu.RX) ) break;
            
            cpu.RIP = cpu.RX;
            instruction_size = 0;
            break;
        case JMP:
            cpu.RIP = cpu.RX;
             instruction_size = 0;
            break;
        case LADR:
            //todo: use RA/RB as MSB/LSB
            cpu.RX = cpu.getmem16(cpu.RIP + 2); 
            instruction_size = 4;
            break;
        case NOP: break;
        default:
            ERROR("Invalid Instruction 0x%hhx at $RIP= %hx", instruction->op, cpu.RIP);
            break;
        }

        cpu.RIP += instruction_size;

        //cpu.dump();
    }

    cpu.dump();
    cpu.dump_mem();
    return 0;
}
