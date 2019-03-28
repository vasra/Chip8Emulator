#include "chip8.h"
#include <stdio.h>
#include <stdlib.h>

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

                case 0x0005: // 8XY5. VY is subtracted from VX. VF is set to 0 when there's a borrow, and 1 when there isn't
                    if (V[(opcode & 0x0F00) >> 8] < V[(opcode & 0x00F0) >> 4])
                        V[0xF] = 0; // borrow
                    else
                        V[0xF] = 1;
                    V[(opcode & 0x0F00) >> 8] -= V[(opcode & 0x00F0) >> 4];
                    pc += 2;
                    break;
                case 0x0006: // 8XY6. Stores the least significant bit of VX in VF and then shifts VX to the right by 1
                    V[0xF] = V[(opcode & 0x0F00) >> 8] & 0x1;
                    V[(opcode & 0x0F00) >> 8] >>= 1;
                    pc += 2;
                    break;

                case 0x0007: // 8XY7. Sets VX to VY minus VX. VF is set to 0 when there's a borrow, and 1 when there isn't
                    if (V[(opcode & 0x0F00) >> 8] > V[(opcode & 0x00F0) >> 4])
                        V[0xF] = 0; //borrow
                    else
                        V[0xF] = 1;
                    V[(opcode & 0x0F00) >> 8] = V[(opcode & 0x00F0) >> 4] - V[(opcode & 0x0F00) >> 8];
                    pc += 2;
                    break;

                case 0x000E: // 8XYE. Stores the most significant bit of VX in VF and then shifts VX to the left by 1
                    V[0xF] = V[(opcode & 0x0F00) >> 8] >> 7;
                    V[(opcode & 0x0F00) >> 8] <<= 1;
                    pc += 2;
                    break;

                default:
                    printf("Unknown opcode: 0x%X\n", opcode);
            }
            break;

        case 0x9000: // 9XY0. Skips the next instruction if VX doesn't equal VY
            if (V[(opcode & 0x0F00) >> 8] != V[(opcode & 0x00F0) >> 4])
                pc += 4;
            else
                pc += 2;
            break;

        case 0xA000: // ANNN. Sets I to the address NNN
            I = opcode & 0x0FFF;
            pc += 2;
            break;

        case 0xB000: // BNNN. Jumps to the address NNN plus V0
            pc = (opcode & 0x0FFF) + V[0];
            break;

        case 0xC000: // CXNN. Sets VX to the result of a bitwise AND operation on a random number (Typically: 0 to 255) AND NN
            V[(opcode & 0x0F00) >> 8] = (rand() % 0xFF) & (opcode & 0x00FF);
            pc += 2;
            break;

        case 0xD000: // DXYN. Draws a sprite at coordinate (VX, VY) that has a width of 8 pixels and a height of N pixels
                     // Each row of 8 pixels is read as bit-coded starting from memory location I
                     // I value doesn’t change after the execution of this instruction.
                     // As described above, VF is set to 1 if any screen pixels are flipped from set to unset when the sprite is drawn, and to 0 if that doesn’t happen
    }
}
