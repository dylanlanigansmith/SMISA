# SMISA: an 8-bit CPU architecture 

I made up a CPU and instruction set without googling anything for fun last night.

sometimes doing is better than designing



---

### Features
- Emulator
- Assembler
- **PLANNED:**
    - FPGA Core
    - GPU
    - macros for assembler and some nicer instructions 
    - physical hardware

### TODO
- Add a stack (push/pop, bp/sp regs)
- Add some DMA areas or interupts for doing stuff
- Basically get enough going that making this in verilog is worth it (ex. add GPIO dma area to make bad microcontroller)
- addressing mode cleanups 



---

### see ./emu/asm/ASM.sma and ISA.md for more details!

ps. this acts as proof if society collapsed I could make a computer from scratch, which is a pretty entertaining notion 