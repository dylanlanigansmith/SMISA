#  SMISA Instruction Set 
this doc is out of date in a few important ways!! see ./emu/asm/ASM.sma for a better guide

#### Instructions
| Mnemonic | Opcode | Usage | Pseudo | Note |
| ---      | ---    | ---   | ---    | ---  | 
| NOP | 0 | NOP | ; | no operation
| ADD | 1 | ADD RA / ADD 5 | RC = RC + RA;  | adds second arg (reg or const int) to RC |
| SUB | 2 | SUB RB / SUB 1 | RC = RC - RB | RC = second arg - third arg  |
| LD  | 3 | LD RC, 4 / LD RC, RA | RC = RA; | sets first register to second arg |
| MOV | 4 | MOV 0x10, RB | *(uintptr_t*)(0x10) = (uintptr_t*)RB | stores val of second arg at address/register in first |
| JMP | 5 | JMP 0x10 | goto 0x10 / RIP = 0x10 | unconditional jumps to instruction at address in first|
| JNE | 6 | JNE 0x10, RB | if(!RB) goto 0x10 | jumps to inst. at addr. in first arg if reg in second is zero |
| HLT | 7 | HLT | int 3 / exit | halts CPU | 
| LADR| 8 | LADR 0x10 | LADR = 0x10 | loads RX with 16 bit instruction |  

[8/15 Slots left for additional operations]



---
#### Registers and Memory

- RIP - 16bits
- RA - 8 bit, gp
- RB - 8 bit, gp
- RC - 8 bit, result / carry 
- RX - 16 bits, jmp address

```
RA = a
RB = b
RC = c

```

8-bit data - 16 bit addressing

Little Endian
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
                         | - > if both set arg is rel. addr from RIP (maybe add case for negative or do signed)
2 - arg is const. byte  -

4 - arg is RA  -
                | - > is RA && is RB == arg is RC
8 - arg is RB  -


for two reg arg situations, second byte will have reg correspond

````

Second Byte: Arguments

if arg is const. byte or rel addr this will be that value

## TODO UPDATE WITH SYNTAX FOR SMISASM
