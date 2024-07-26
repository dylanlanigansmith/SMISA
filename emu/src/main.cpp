/*
Emulator / Assembler Core for SMISA CPU Arch
DLS 7.22.24

*/

#include "emu.hpp"






int assemble_line(std::string line, std::vector<uint16_t>& assembly, label_map_t& labels)
{       
        int log_level = 0;
        
        line = remove_comments(line);
        if(line.empty()) return 0;
        std::vector<std::string> raw_tokens = split(line);
        std::vector<std::string> tokens;
        
        if(log_level > 2) std::cout << "Line: " << line << std::endl;
      
        for (const auto& token : raw_tokens) {
            //std::cout << "[" << token << "] ";
            if (token.find(ASM_COMMENT) != std::string::npos){
                break; // rest is a comment
            }
            if (token.find(ASM_LABEL) != std::string::npos){
                break; // label we already handled, eventually do this in preproc and only run assemble lines on preproc line vector
            }
            tokens.push_back(token);
        }

        if(tokens.empty()){ std::cout << std::endl; return 0; }
          if(log_level > 4) std::cout << "Tokens: ";
            for (const auto& token : tokens) {
                if(log_level > 4) std::cout << "[" << token << "] " ;
          ///      if(*(tokens.at(0).end() - 1) == ASM_LABEL){
            //        std::cout << "found label" << tokens.at(0) << std::endl;
            //        continue;
             //   }
        }

        

         if(log_level > 3) std::cout << std::endl;
        std::string op_str = tokens.front();
        auto op = magic_enum::enum_cast<OPERATIONS>(tokens.front());
        if(!op.has_value()){
            return assembly_error(ASM_FATAL, "Unknown Operation '%s' %02hhx", tokens.front().c_str(), std::atoi(tokens.front().c_str()));
        }
        if(log_level > 2) std::cout << "assembling " << get_opcode_name(op.value()) << " " << std::endl;

        auto it = labels.find(line);

        if(it != labels.end()){
            it->second.first.second = (size_t)(assembly.size() * sizeof(uint16_t)); 
            
            if(log_level > 2) std::cout << " -> found label " << it->second.first.first << " for address " << it->second.first.second << std::endl;
        }


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
            
            uint16_t address = label.has_value() ? label.value().second.first.second: hexstr_to<uint16_t>(tokens.at(1)); 
            
            opcode_t oc = { .op = LADR, .is_addr = 0, .is_byte = 0, .is_RA = 0, .is_RB = 0 };
            assembly.push_back(oc);
            if(label.has_value()){
                auto& lbl = labels.at( (*label).first ); //dont ask
            //     printf("adding reference to %s '%s' at 0x%4lx$ \n", lbl.first.first.c_str(), line.c_str(), (assembly.size() - 2) * sizeof(uint8_t) );
                assembly.push_back(address);
                lbl.second.push_back((assembly.size() - 2) * sizeof(uint8_t) ); //add ptr to this address that needs to get filled in
            }else 
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

            bool dest_isaddr = (dest_isreg) ? false : is_hexstr(tokens.at(1)) || tokens.at(1).at(0) == ASM_ADDR ;
            bool src_isbyte = (src_isreg) ? false : is_hexstr(tokens.at(2));

            if(tokens.at(2).length() == 1 && tokens.at(2).at(0) == ASM_ADDR){
               oc.is_addr = true;
            }
            
            //todo addr
            bool addr_reg = is_address_reg(tokens.at(1).c_str()) || is_address_reg(tokens.at(2).c_str());

            if(dest_isreg){
                std::string reg = tokens.at(1);
                for (auto & c: reg) c = toupper(c);

                if(is_address_reg(reg.c_str())){
                    oc.is_addr = 1; oc.is_byte = 1; 
                    oc.is_RA = (reg == "RXH");
                }
                else{
                    oc.is_RA = (reg == "RA" || reg == "RC");
                    oc.is_RB = (reg == "RB" || reg == "RC");
                }
            }

            if(src_isreg){
                std::string reg = tokens.at(2);
                for (auto & c: reg) c = toupper(c);
                oc.args = str_to_reg(reg);
            }
            
            if(src_isbyte){
                oc.is_byte = 1;
                uint16_t u16 = hexstr_to<uint16_t>(tokens.at(2)); //dont ask
                oc.args = (uint8_t)(u16 & 0xff);
            }

            if(!oc.is_addr)
                oc.is_addr = dest_isaddr;
            
            if(dest_isaddr && addr_reg) oc.is_RB = 1; 

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

            bool address_reg = false;

            if(src_isreg){
                std::string reg = tokens.at(1);
                for (auto & c: reg) c = toupper(c);

                if(address_reg = is_address_reg(reg.c_str()); address_reg ){
                    oc.is_addr = 1; oc.is_byte = 1; 
                    oc.is_RA = (reg == "RXH");
                }
                else{
                    oc.is_RA = (reg == "RA" || reg == "RC");
                    oc.is_RB = (reg == "RB" || reg == "RC");
                }
            }            
            if(src_isbyte){
                oc.is_byte = 1;
                uint16_t u16 = hexstr_to<uint16_t>(tokens.at(1)); //dont ask
                oc.args = (uint8_t)(u16 & 0xff);
            }

            if(!address_reg)
                oc.is_addr = src_isaddr;
            
            assembly.push_back(oc);
            
            //printf("srcr %u destr %u b %u ad %u args %x \n",src_isreg, dest_isreg, src_isbyte, dest_isaddr, oc.args);
           
            break;
        }
        case JE:
        case JNE: {
            bool is_jne = op_str.c_str()[1] == 'N'; 
            opcode_t oc = { .op = JNE, .is_addr = 0, .is_byte = 0, .is_RA = 0, .is_RB = 0, .args = (is_jne) ? (uint8_t)0 : (uint8_t)0xff };   
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
        case INC:{
            opcode_t oc = { .op = INC, .is_addr = 0, .is_byte = 0, .is_RA = 0, .is_RB = 0, .args = 0 };   
            bool dest_isreg = symbol_exists(tokens.at(1).c_str(), symbols_reg); //overkill for what could be first char = R 
            bool dest_isaddr = (dest_isreg) ? false : is_hexstr(tokens.at(1)) || tokens.at(1).at(0) == ASM_ADDR ;
            if(dest_isreg){
                 std::string reg = tokens.at(1);
                for (auto & c: reg) c = toupper(c);
                oc.args = str_to_reg(reg);
            }
            else if(dest_isaddr){
                oc.is_addr = 1;
            }
            assembly.push_back(oc);  
            
            break;
        }
        case INT:{
            opcode_t oc = { .op = INT, .is_addr = 0, .is_byte = 0, .is_RA = 0, .is_RB = 0, .args = 0 };   
            bool src_isreg =  symbol_exists(tokens.at(1).c_str(), symbols_reg);
            bool src_isbyte = (src_isreg) ? false : is_hexstr(tokens.at(1));
            if(src_isreg){
                 std::string reg = tokens.at(2);
                for (auto & c: reg) c = toupper(c);
                if(is_address_reg(reg.c_str())){
                    oc.is_addr = oc.is_byte = 1; 
                    oc.args =  (reg == "RXL") ? REG_XL : REG_XH;
                }
                else{
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
            }
            if(src_isbyte){
                oc.is_byte = 1;
                uint16_t u16 = hexstr_to<uint16_t>(tokens.at(1)); //dont ask
                oc.args = (uint8_t)(u16 & 0xff);
            }
            assembly.push_back(oc);  
            break;
        }
        default:
            std::cout << "Unknown Operation Handler for " << tokens.front() << std::endl; return 1;
            break;
        }

        
        

        return 0;
}





program_t assemble(const char* filename) {
    printf("Assembling %s \n", filename);
    std::ifstream infile(filename);
    std::string line;
    int log_level = 0;
    auto prog = program_t{0,0};

    if (!infile.is_open()) {
        assembly_error(ASM_FATAL, "Failed to open source file: '%s' ", filename);
        return prog;
    }
    std::vector<std::string> lines;

    label_map_t labels;
    puts("Running preprocessor");
    while(std::getline(infile, line)){ //get line vec first and use it from here on so u can do shit like add line nums to label references! 
        if(line.empty()) continue;
        std::vector<std::string> raw_tokens = split(line);
        if(raw_tokens.empty()) continue;

        
         if(log_level > 5) std::cout << "Line: " << line << std::endl;
        
        for (const auto& token : raw_tokens) {
            //std::cout << "[" << token << "] ";
            if (token.find(ASM_COMMENT) != std::string::npos){
                break; // rest is a comment
            }
            if (token.find(ASM_LABEL) != std::string::npos){
                std::streampos currentPos = infile.tellg();
                std::string next_line;
                // Read the next line without advancing the loop
                if (std::getline(infile, next_line)) {
                    // Process nextLine as needed
                    std::string label_name = token.substr(0, token.length() - 1);
                    std::string label_line = remove_comments(next_line);
                    label_entry_t entry = {{label_name, LABEL_INVALID}, std::vector<size_t>()} ;
                  
                  //  set_logclr(LOG_MAGENTA); printf("adding label %s '%s' \n", label_name.c_str(), label_line.c_str()); reset_logclr();
                    labels.insert({label_line, entry});
                }
                 // Reset the file position to the stored position
                infile.clear(); // Clear any EOF flags
                infile.seekg(currentPos);
       
                
                break; // rest is a comment
            }
        }
        lines.push_back(line);
    }
    infile.close();

    set_logclr(LOG_GREEN); printf("Preprocessing Complete with %li labels", labels.size()); reset_logclr();
    //print_labels(labels, 1);

    infile.open(filename); //stoopid
  
    std::vector<uint16_t> assembly;
    bool add_next_label = false;
  
    while (std::getline(infile, line)) {
        
        if(assemble_line(line, assembly, labels)){
            ERROR("Error Assembling %s", line.c_str()); return prog;
        }
      
    }
    set_logclr(LOG_GREEN); puts("Assembly Complete"); reset_logclr();
    
    
   //  print_labels(labels);
    puts("resolving symbols");
    int resolved = 0;
   for(auto& entry : labels){
        auto& resolutions = entry.second.second;
        auto& label_name = entry.second.first.first;
        if(entry.second.first.second == LABEL_INVALID){
            assembly_error(ASM_INTERNAL, "label %s address never deduced", label_name.c_str());
            continue;//return prog;
        }
        if(entry.second.second.empty()) {
            assembly_warning("no references to label %s ", label_name.c_str()); continue;
        }
        for(auto& res : resolutions){
            assembly.at(res + 1) = entry.second.first.second;
            resolved++;
        }
        //std::cout << label_str(entry, true) << std::endl;
   }
   printf("resolved %d symbols\n", resolved);
    if(log_level || 1) print_labels(labels);
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

int emulate(program_t& p)
{
    if(p.size){
        
        cpu.load_prog(p.data, p.size);
        //cpu.dump_mem(1, p.size + 4);
    }

    set_logclr(LOG_GREEN); puts("Starting Loaded Program...\n"); reset_logclr();

    puts("Addr. |  Instruction                | ");
    puts("------------------------------------");
    for(;;)
    {
        
        auto instruction = cpu.get_instruction();
        printf("$%04hx | %5s = ", cpu.RIP, get_opcode_name(instruction->op));

        bool verbose = true;
        if(instruction->op != NOP && verbose) {
            print_opcode(*instruction, 1);
           // printf(" |");
        }
       putc('\n', stdout);

        uint16_t instruction_size = 2;
        if(instruction->op == HLT) break; 
        switch (instruction->op)
        {
        case ADD:
            if( instruction->is_addr && instruction->is_byte && !(instruction->is_RB)){
                //RXH RXL 
                //if(!is_RB) reg arg refers to RX MSB/LSB (msb if is_RA = 1)
                uint8_t reg = (instruction->is_RA) ? *cpu.RX_H : *cpu.RX_L;
                cpu.RC += reg; break;
                
            }
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
             if( instruction->is_addr && instruction->is_byte && !(instruction->is_RB)){
                //RXH RXL 
                //if(!is_RB) reg arg refers to RX MSB/LSB (msb if is_RA = 1)
                uint8_t reg = (instruction->is_RA) ? *cpu.RX_H : *cpu.RX_L;
                cpu.RC -= reg; break;
                
            }
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
            if( instruction->is_addr && instruction->is_byte && !(instruction->is_RB)){
                    //RXH RXL 
                    //if(!is_RB) reg arg refers to RX MSB/LSB (msb if is_RA = 1)
                    uint8_t* dest_reg = (instruction->is_RA) ? cpu.RX_H : cpu.RX_L;
                    
                    uint8_t* src_reg = cpu.get_reg_from_arg(instruction->args); 
                    *dest_reg = *src_reg;
                    //!(instruction->is_RB) use for arg = byte or reg
            }
            else if(instruction->is_byte){
        
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
        //    print_opcode(*instruction);
            break;
        case INT:{
            uint8_t n = 0xff;
            if(instruction->is_byte)
                n = instruction->args;
            else n = *cpu.get_reg_from_arg(instruction->args);


            switch(n)
            {
                case 0x2:{
                    //putc RA
                    char c = (char)cpu.RA;
                    char cc[2] = {c,0};
                    cpu.io.strout.append(cc);
                    printf(">%s \n", cc);
                    break;
                } 
                case 0x4:
                case 0x3:{
                    //trap to debugger
                    set_logclr(LOG_YELLOW);
                    puts("Reached breakpoint, dumping and continuing... "); 
                    cpu.dump_registers();

                    reset_logclr();

                    if(n == 0x4){
                        puts("** PRESS ENTER TO CONTINUE **");
                        std::string idc;
                        std::getline(std::cin, idc);
                    }
                    break;
                }
                
                case 0xff:
                default:{
                     ERROR("Unknown or unhandled interupt 0x%hhx at $RIP= %hx", n, cpu.RIP); break;
                }
                    
            }
            break;
        }
        case INC:{
            if(instruction->is_addr){
                cpu.RX++;
            } else{
                auto r = cpu.get_reg_from_arg(instruction->args);
                *r += 1; 
            }
            break;
        }
        
        
        case JNE:{
            if(instruction->args == 0xff) { //je
                if(instruction->is_RA && instruction->is_RB){
                if(!cpu.RC) break;
            } else if (instruction->is_RA) {
                if(!cpu.RA) break;
            } else if (instruction->is_RB) { 
                 if(!cpu.RB) break;
            } 
         
                if ( (instruction->is_addr && !cpu.getmem()) ) break;
            } 
            else{
            if(instruction->is_RA && instruction->is_RB){
                if(cpu.RC) break;
            } else if (instruction->is_RA) {
                if(cpu.RA) break;
            } else if (instruction->is_RB) { 
                if(cpu.RB) break;
            } 
            
            if ( (instruction->is_addr && cpu.getmem()) ) break;
            }
 
            
            cpu.RIP = cpu.RX;
            instruction_size = 0;
            break;
        }
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
    
   
    set_logclr(LOG_GREEN); puts("[reached HLT instruction, execution complete]\n"); reset_logclr(); 
    if(cpu.io.strout.size()){
         set_logclr(LOG_CYAN);
        printf("Program Output: ");
        reset_logclr();
        puts(cpu.io.strout.c_str());
    }
    
    cpu.dump("Final CPU State");
    cpu.dump_mem();
   
    return 0;
}

int main(int argc, const char * argv[]) {
    // insert code here...
    const opcode_t o = *(opcode_t*)(program);
   // print_opcode(o);
   // print_opcode(*(opcode_t*)(program + 2));
   // print_opcode(*(opcode_t*)(program + 4));
   // print_opcode(*(opcode_t*)(program + 6));
     const char* filepath = "/Users/dylan/Code/SMISA/emu/asm/%s";
    char path_buf[512];

    bool assem = false;
    bool save_bin = false;
    bool load_bin = false;
    bool execute = false;
   if(argc > 1){
        if(!strcmp(argv[1], "make") && argc >= 2){
            strncpy(path_buf, argv[2], 512);
            assem = execute = true;
        }
        else if(!strcmp(argv[1], "asm") && argc >= 2){
            strncpy(path_buf, argv[2], 512);
            assem = save_bin = true;
        }
        else if(!strcmp(argv[1], "exec") && argc >= 2){
            strncpy(path_buf, argv[2], 512);
            load_bin = execute = true;
        }
        else{
            set_logclr(LOG_CYAN);
            puts("== SMISA assembler and emulator - DLS 2024 =="); reset_logclr();
            puts("Usage: \n [cmd] make [asm file path] assembles and runs a file");
            puts(" [cmd] asm [asm file path] assembles and saves a file");
            puts(" [cmd] exec [binary file path] loads and runs an assembled file");

        }
   } else {
        puts("no args, assuming dev mode");
        snprintf(path_buf, 512, filepath, "test.sma");
        assem = execute = true;
   }
   
    auto p = program_t{0,0,0};



    if(assem){
        if(std::string(path_buf).substr(strlen(path_buf) - ASM_EXT_LEN) != ASM_SRC_EXT) {
            set_logclr(LOG_RED); puts("Input file is not a SMIASM src file (.sma)\n"); reset_logclr(); return 1;
        }
        p = assemble(path_buf);
        if(save_bin && p.ok && p.size){
            std::string save_file = std::string(path_buf).substr(0, strlen(path_buf) - ASM_EXT_LEN);
            save_file.append(ASM_BIN_EXT);
            writeprog_bin(save_file.c_str(), p);
        } 
    } else if(load_bin){
        if(std::string(path_buf).substr(strlen(path_buf) - ASM_EXT_LEN) != ASM_BIN_EXT) {
            set_logclr(LOG_RED); puts("Input file is not binary (.bin)\n"); reset_logclr(); return 1;
        }
        p = readprog_bin(path_buf);
    }
   
  



    if(!p.ok){set_logclr(LOG_RED); puts("Error with loaded program \n"); reset_logclr(); return 1;}


    if(!execute){
            set_logclr(LOG_GREEN); puts("[Complete]\n"); reset_logclr(); return 0;
    }

    return emulate(p);
}
