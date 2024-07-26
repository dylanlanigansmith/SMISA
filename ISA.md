#  SMISA Instruction Set 
see ./emu/asm/ASM.sma for a better and more applied guide to the assembly language

ISA is turing complete and functional, but a bit awful to use without macros or a formal stack 


#### Instructions
| Mnemonic | Opcode | Usage | Pseudo | Note |
| ---      | ---    | ---   | ---    | ---  | 
| NOP | 0 | NOP | ; | no operation
| ADD | 1 | ADD RA / ADD 5 | RC = RC + RA;  | adds second arg (reg or byte literal) to RC |
| SUB | 2 | SUB RB / SUB 1 | RC = RC - RB | RC = second arg - third arg (args reg or byte literal)  |
| LD  | 3 | LD RC, 4 / LD RC, RA | RC = RA; | sets first arg (reg or address) to second arg (reg, addr, or byte) |
| INT | 4 | INT 3;| asm("int 3") //x86 equiv  | calls interupt at index in first arg (reg or byte) |
| JMP | 5 | JMP | goto RX / RIP = RX | unconditional jumps to instruction at address in first|
| JNE | 6 | JNE RB | if(!RB) goto RX | jumps to inst. at addr. in first arg if reg in second is zero |
| JE | "  | JE RB | if(RB) goto RX | jumps to inst. at addr. in first arg if reg in second is not zero (shares opcode with JNE) |
| HLT | 7 | HLT | exit | halts CPU | 
| LADR| 8 | LADR 0x1000 | RX = (uint16_t)0x1000 | loads RX with 16 bit instruction |  
| INC | 9 | INC RA / INC $ | RA++; RX++ | adds one to arg (reg, addr) |


[6 slots left for additional operations]

```$``` is used to represent the address in ```RX``` within  instructions that can access memory (ex. ```LD```)


---
#### Registers and Memory

- RIP - 16bits
- RA - 8 bit, gp
- RB - 8 bit, gp
- RC - 8 bit, result / carry
- RX - 16 bits, address
    - Can be refered in ```ADD/SUB/LD/INC``` operations as ```RXH/RXL``` for the high and low bits respectively


###### Memory Map

```
0x00 - RIP value at boot, program entry point
-- 
todo: finish this 

//mmap a frame buffer? or do we do selector regs and switch to ROM/vidmem/etc being selectable address spaces sorta thing 

0xE00 - Stack Bottom (always at end of mem - stack size ? )
0x1000 - Stack Top
//stack size == 0x200 (512) bytes
//do we want HW or SW stack.. I prefer SW, HW makes sense for what this is (ex 6502)
```

Little Endian, 8-bit data - 16 bit addressing

Architecture assumes minimum of 4kb of shared memory. 

It is not ideal to steal x86-64 register names for IP, stack, etc. I just am used to them and doing so was both comfy and natural, sorry for any confusion. 

----
#### Encoding

Instructions are encoded as two byte pairs.

First Byte: Opcode and Addr. Mode

````
MSB 128 64 32 16 8 4 2 1 LSB
    [ opcode  ] [ addr. mode]
````

Addressing Modes

````
1 - arg is address      -
                         | - > if(!is_RB) reg arg refers to RX MSB/LSB (msb if is_RA = 1)
2 - arg is const. byte  -

4 - arg is RA  -
                | - > is RA && is RB == arg is RC
8 - arg is RB  -



for two reg arg situations, second reg will be in actual arguement byte of opcode 

````

Second Byte: Arguments

if arg is const. byte or rel addr this will be that value



not all addressing mode cases described here, check structs in emu.hpp 

----
#### Interupts

16 hardware interupts, 16 software 

| Name | Num. | Description |
| ---  | ---  | ---         |
| udf  | 0    | not impl |
| udf  | 1    | not impl |
| udf  | 2    | not impl |
| Breakpoint / Trap to debugger  | 3    | debug/dev interupt for use in emu, currently emulator dumps all registers and resumes execution |
| """  | 4    | same as above but pauses execution  |


Software Interupt Table: **To Be Added!!!** 

16 Interupts - each contains 16 bit address of handler, with idx 0 being (int 16), as interupts 0-15 are for hardware use

example: 

0xDE0 - Software Interupt Table: First Entry (int 16)
    - 0xDE4 - third (int 17)
    - etc..
    - 0xDFE - last entry (int 31)

use LIDT 0xDE0 to load and activate the software interupts or something, probably shouldn't ignore hardware side (PIC equiv.) 

