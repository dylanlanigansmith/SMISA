; Smisa Architecture Assembly Language (SMISASM) Overview and Example Program

; Intro:
; - 3 8-bit program registers, 16 bit addressing, minimal instruction set and assembly language
; We have 3 registers to use in our program, RA, RB, and RC
; RA and RB are full general purpose, and RC is the dedicated register for the result of arithmatic operations 
; RX is our 16 bit address register, and is loaded via a dedicated instruction covered further operation
; It is safe to assume address space up to 0x1000 is always available, program execution starts at address 0x0 on boot

;Labels are defined like this! They can be referenced with the LADR instruction
START:
    NOP ;no operation
;LADR {address/label}
; $ may be used to represent current value in RX (16 bit address register) for the LD instruction
; LADR sets RX to given address / address of label 
; ex: LADR 0xB1F3, LADR LABEL_NAME
    LADR 0x500
;LD {reg, address_reg}, {hex byte literal, reg, address_reg}
; examples:
; LADR 0x100
; LD $, RC ; sets memory at 0x100 to value in RC
; LD RA, $ ; sets RA to value in memory at 0x100 
; LD RB, 5 ; sets RB to 5 
    LD $, 0x40

;ADD {reg, hex byte literal} / SUB {reg, hex byte literal}
; Adds or subtracts specified register or hex byte literal to/from RC
; examples:
; ADD 5 ; adds 5 to RC
; ADD RB ; adds RB to RC
; SUB 7 ; subtracts 7 from RC
    ADD 0x41

; JMP 
; sets instruction ptr to address in RX set via LADR
; ex: 
; LADR 0x2 ; set RX to 16 bit address 0x2
; JMP ; resumes execution at address in RX 
    LADR END
    JMP
; JNE { reg }
; if specified register is zero, performs a JMP instruction to address in RX
; example:
; 
; LD RC, 5
; LOOP:
;    SUB 1
;    LADR EXIT
;    JNE RC ; jmp to EXIT label if RC == 0
;    LADR LOOP
;    JMP LOOP ; jmp to beginning of our loop
; EXIT:
;   LD RA, 1 ; set RA to 1 to signal we reached our JNE target
;   HLT ; halt CPU to end program
;

    SUB 1 
    LD RB, RA
END:
    LD RB, 3 ; example of how to add 3 to a register and save the result
    LD RC, RB
    ADD 3
    LD RB, RC
    HLT ; programs always must end with a HLT to avoid the CPU executing random memory 