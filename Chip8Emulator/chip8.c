#include "chip8.h"

void init()
{
    // set the program counter to address 0x200 (512 in Dec) because the first 512 bytes are occupied by the interpreter itself
    pc = 0x200;
    opcode = 0;
    sp = 0;
    I = 0;

    // Clear the screen;
    for(short i = 0; i < 2048; i++)
        gfx[i] = 0;

    // Clear the stack, registers and keys
    for(short i = 0; i < 16; i++)
        key [i] = V[i] = stack[i] = 0;

    // Clear the memory
    for(short i = 0; i < 4096; i++)
        memory[i] = 0;

    // Load the fontset in the first 80 slots
    for(short i = 0; i < 80; i++)
        memory[i] = fontset[i];

    delay_timer = 0;
    sound_timer = 0;
}

void emulateCPUCycle()
{
    opcode = (memory[pc] << 8) | memory[pc + 1];

    switch(opcode & 0xF000)
    {
        case 0x0000:
            switch(opcode & 0x00FF)
            {
                case 0x00E0: // Clears the screen
                    for(short i = 0; i < 2048; i++)
                        gfx[i] = 0;
                    pc += 2;
                    break;

                case 0x0EE: // Returns from a subroutine
                    --sp;
                    pc = stack[sp];
                    pc += 2;
                    break;
            }
            break;

        case 0x1000: pc = opcode & 0x0FFF; break;

        case 0x2000:
    }
}
