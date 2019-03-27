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
                case 0x00E0: // 00E0. Clears the screen
                    for(short i = 0; i < 2048; i++)
                        gfx[i] = 0;
                    pc += 2;
                    break;

                case 0x0EE: // 00EE. Returns from a subroutine
                    --sp;
                    pc = stack[sp];
                    pc += 2;
                    break;
            }
            break;

        case 0x1000: // 1NNN. Jumps to address NNN
            pc = opcode & 0x0FFF;
            break;

        case 0x2000: // 2NNN. Calls subroutine at NNN
            stack[sp] = pc;
            ++sp;
            pc = opcode & 0x0FFF;
            break;

        case 0x3000: // 3XNN. Skips the next instruction if VX equals NN
            if(V[(opcode & 0x0F00) >> 8] == (opcode & 0x00FF))
                pc += 4;
            else
                pc += 2;
            break;

        case 0x4000: // 4XNN. Skips the next instruction if VX doesn't equal NN
            if(V[(opcode & 0x0F00) >> 8] != (opcode & 0x00FF))
                pc += 4;
            else
                pc += 2;
            break;

        case 0x5000: // 5XY0. Skips the next instruction if VX equals VY
            if(V[(opcode & 0x0F00) >> 8] == V[(opcode & 0x00F0) >> 4])
                pc += 4;
            else
                pc += 2;
            break;

        case 0x6000: // 6XNN. Sets VX to NN
            V[(opcode & 0x0F00) >> 8] = opcode & 0x00FF;
            pc += 2;
            break;

        case 0x7000: // 7XNN. Adds NN to VX
            V[(opcode & 0x0F00) >> 8] += opcode & 0x00FF;
            pc += 2;
            break;

        case 0x8000:
            switch(opcode & 0x000F)
            {
                case 0x0000: // 8XY0. Sets VX to the value of VY
                    V[(opcode & 0x0F00) >> 8] = V[(opcode & 0x00F0) >> 4];
                    pc += 2;
                    break;

                case 0x0001: // 8XY1. Sets VX to VX or VY (Bitwise OR)
                    V[(opcode & 0x0F00) >> 8] |= V[(opcode & 0x00F0) >> 4];
                    pc += 2;
                    break;

                case 0x0002: // 8XY2. Sets VX to VX and VY. (Bitwise AND)
                    V[(opcode & 0x0F00) >> 8] &= V[(opcode & 0x00F0) >> 4];
                    pc += 2;
                    break;

                case 0x0003: // 8XY3. Sets VX to VX xor VY
                    V[(opcode & 0x0F00) >> 8] ^= V[(opcode & 0x00F0) >> 4];
                    pc += 2;
                    break;

                case 0x0004: // 8XY4. Adds VY to VX. VF is set to 1 when there's a carry, and to 0 when there isn't
                    if (V[(opcode & 0x00F0) >> 4] > (0xFF - V[(opcode & 0x0F00) >> 8]))
                        V[0xF] = 1; // carry
                    else
                        V[0xF] = 0;
                    V[(opcode & 0x0F00) >> 8] += V[(opcode & 0x00F0) >> 4];
                    pc += 2;
                    break;
            }


    }
}
