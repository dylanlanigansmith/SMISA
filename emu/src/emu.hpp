#pragma once

#include "shared.hpp"
#include "assembler.hpp"

#define MEM_SIZE 0x1000



enum OPERATIONS : uint8_t
{
    NOP = 0u,
    ADD,
    SUB,
    LD,
    INT,
    JMP,
    JNE,
    HLT,
    LADR, 
    INC,

    DUMMY = 16, //start of "fake ops"
    JE,
    DB,
};





enum REG_ARG_IDS : uint8_t {
    REG_A = 0xA,
    REG_B,
    REG_C,
    REG_XL = 0xe,
    REG_XH = 0xf
};


const std::unordered_map<std::string, uint8_t> reg_lookup =
{
    {"RA", REG_A}, {"RB", REG_B}, {"RC", REG_C}, {"RXL", REG_XL}, {"RXH", REG_XH}, 
};

uint8_t str_to_reg(const std::string& s){
    return reg_lookup.at(s);
}
uint8_t str_to_reg_flags(const std::string& reg, uint8_t& is_RA, uint8_t& is_RB){
    is_RA = (reg == "RA" || reg == "RC");
    is_RB = (reg == "RB" || reg == "RC");

    return (is_RB || is_RA);
}


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

 struct cpu_rflags_t {
        uint8_t interupt : 1;
        uint8_t _res : 7;
} __attribute__((packed));
 static_assert(sizeof(cpu_rflags_t) == 1);


struct cpu_t
{
    uint16_t RIP, RX;
    uint8_t RA, RB, RC;

   
   

    uint8_t* RX_H, *RX_L; 
    uint8_t mem[MEM_SIZE]; //4k memory
    

    struct {
        std::string strout;

    } io;
    cpu_t() { this->reset(); }
    cpu_t(const cpu_t&) = delete;

    void reset() {
        memset(this, 0u, sizeof(cpu_t)); 

        uint8_t* rx = (uint8_t*)(&this->RX);

        RX_L = rx; 
        RX_H = rx + 1;

        printf("smisma-emu reloaded...  [mem_size=%x, debug=%d] \n", MEM_SIZE, DEBUG);  


    }
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
            ERROR("Invalid Address %04hx", addr); return false;
        }
        return addr < MEM_SIZE;
    }
   


    bool load_prog(const uint8_t* prog, size_t len = MEM_SIZE){
       
        memcpy(mem, prog, len);

        return true;
    }

    uint8_t* get_reg_from_arg(uint8_t ar){
    switch(ar){
        case REG_A: return &RA;
        case REG_B: return &RB;
        case REG_C: return &RC;
        case REG_XH: return RX_H;
        case REG_XL: return RX_L;
        default: ERROR("get_reg_from_arg(): unknown reg type %hhx", ar); return &RC; //throw exception once we have interupts
    }
}
    void dump_registers(bool one_line = false){
        printf("   - RA  = 0x%02hhx   / %5hhu \n", RA, RA);
        printf("   - RB  = 0x%02hhx   / %5hhu\n", RB, RB);
        printf("   - RC  = 0x%02hhx   / %5hhu\n", RC, RC);
        printf("   - RX  = 0x%04hx / %5hu\n", RX, RX);
        printf("   - RIP = 0x%04hx / %5hu\n", RIP, RIP);
    }
    void dump(const char* msg = "CPU Dump"){
        set_logclr(LOG_CYAN); printf("== == %s == ==\n", msg); reset_logclr();
        printf("  Registers: \n");
         set_logclr(LOG_MAGENTA);
        dump_registers();
        reset_logclr();
        printf("  Stats: \n");
        printf("   - MEM = 0x%lx bytes [%lu kb] \n", (size_t) MEM_SIZE, (size_t) MEM_SIZE / 1024);

    }

    void dump_mem(bool all = false, size_t max = MEM_SIZE){
        set_logclr(LOG_CYAN);
        printf("-- -- memory dump -- -- \n"); reset_logclr();
        for(size_t i = 0; i < max; ++i){
            if(mem[i] == 0x0 && !all) continue;

            
            printf(" [0x%04lx] | %02hhx %02hhx | %c%c \n", i, mem[i], mem[i + 1], mem[i], mem[i + 1] );
            i += 1;
        }
        set_logclr(LOG_CYAN); printf("-- -- -- -- -- -- -- -- \n");reset_logclr();
    }
};


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
        printf(" 0x%hx [%hhx]{%hhx} %s", raw, raw_b[0], raw_b[1], get_opcode_name(code.op)); return;
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