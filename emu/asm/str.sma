START:
    LADR 0x500   ; Set RX to the starting address 0x500
    
    LD $, 0x48   ; Set memory at 0x500 to 'H' (ASCII 0x48)
    LADR 0x501
    LD $, 0x65   ; Set memory at 0x501 to 'e' (ASCII 0x65)
    LADR 0x502
    LD $, 0x6C   ; Set memory at 0x502 to 'l' (ASCII 0x6C)
    LADR 0x503
    LD $, 0x6C   ; Set memory at 0x503 to 'l' (ASCII 0x6C)
    LADR 0x504
    LD $, 0x6F   ; Set memory at 0x504 to 'o' (ASCII 0x6F)
    LADR 0x505
    LD $, 0x00   ; Set memory at 0x505 to null terminator (0x00)
    
    HLT          ; Halt the program