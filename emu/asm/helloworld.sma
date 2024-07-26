;Prints Hello World! twice using hardware print char interupt

LOAD_STR: ; loads "Hello World" 
    LADR 0x500   
    LD $, 0x48   
    INC $
    LD $, 0x65   
    INC $
    LD $, 0x6C  
    INC $
    LD $, 0x6C   
    INC $
    LD $, 0x6F   
    INC $
    LD $, 0x20
    INC $ 
    LD $, 0x57
    INC $ 
    LD $, 0x6f
    INC $ 
    LD $, 0x72
    INC $ 
    LD $, 0x6c
    INC $ 
    LD $, 0x64
    INC $
    LD $, 0x20
    INC $
    LD $, 0x00   ; Set memory at 0x505 to null terminator (0x00)
    LADR 0x500

LADR 0x400
LD $, 0x0

LADR 0x401
LD $, 0x1

LADR 0x500
LD RB, RXL
LD RC, RXH
  
PRINT_STR: 
    LD RB, RB ; bug in label code - stupid one too
    LD RXL, RB
    LD RXH, RC
    LD RA, $
    LD RB, RXL
    LD RC, RXH
    LADR STRING_END
    JNE RA
    INT 2 ; print RA
    LD RXL, RB
    LD RXH, RC
    INC $ 
    LD RB, RXL
    LD RC, RXH
    LADR PRINT_STR
    JMP
STRING_END:
    LD RXL, RXL
    
    LADR 0x400
    LD RA, $
    
    LADR END
    JE RA
    LADR 0x400
    INC RA
    LD $, RA

    LADR 0x500
    LD RB, RXL
    LD RC, RXH
    LADR PRINT_STR
    JMP 

END:
    LD RA, 0x21
    INT 2
    LD RA, 0x20
    INT 2
    LADR 0xcafe
    HLT