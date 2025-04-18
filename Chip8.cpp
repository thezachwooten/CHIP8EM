// g++ -o myproject myproject.cpp `sdl2-config --cflags --libs`
// LIBS
#include "Chip8.h"
#include <cstring>
#include <fstream>
#include <iostream>
#include <cstdlib>
#include <SDL2/SDL.h> // Graphics and input library


Chip8::Chip8() {
    bool drawFlag = 0; // turn off drawFlag
    initialize();
}

Chip8::~Chip8() {}

void Chip8::initialize(){
    pc     = 0x200;  // Program counter starts at 0x200
    opcode = 0;      // Reset current opcode	
    I      = 0;      // Reset index register
    sp     = 0;      // Reset stack pointer
 
    // Clear display
    for (int i = 0; i < 64 * 32; i++){
        gfx[i] = 0; // set the i-th pixel to 0 aka off
        }
    // Clear stack
    for (int i = 0; i < 16; i++){
        stack[i] = 0; // clear stack[i]
    }
    // Clear registers V0-VF
    for (int i = 0; i < 16; i++){
        V[i] = 0; // clear V[i]
    }
    // Clear memory
    for (int i = 0; i < 4096; i++){
        memory[i] = 0; // clear memory[i]
    }
    
    // Load fontset
    //   for(int i = 0; i < 80; ++i){
    //     // memory[i] = chip8_fontset[i];	
        
    //   }	
    
    // Reset timers
}

void Chip8::loadProgram(const char* filename) {
    // Open file in binary read mode
    FILE* file = fopen(filename, "rb");
    if (file == nullptr) {
        std::cerr << "Failed to open file: " << filename << std::endl;
        return;
    }

    // Seek to end to find file size
    fseek(file, 0, SEEK_END);
    long fileSize = ftell(file);
    rewind(file);

    // Allocate temporary buffer
    char* buffer = new char[fileSize];
    fread(buffer, 1, fileSize, file);
    fclose(file);

    // Load program into memory starting at 0x200 (512)
    for (int i = 0; i < fileSize; ++i) {
        memory[512 + i] = buffer[i];
    }

    delete[] buffer;
}

void Chip8::emulateCycle()
{
  // Fetch opcode; 2 bytes connected with an |
  opcode = memory[pc] << 8 | memory[pc + 1];
 
  // Decode opcode
  switch(opcode & 0xF000)
  {    
    case 0x0000:{
    // Some opcodes //
        switch(opcode & 0x000F){
            case 0x0000: // 0NNN: calls machine code at NNN; will NOP instead
                // No operation — legacy opcode
                pc += 2;
            break;

            case 0x00E0: // 0x00E0: clears the screen
                for (int i = 0; i < 64 * 32; i++){
                    gfx[i] = 0; // set the i-th pixel to 0 aka off
                }
                pc += 2; // move the pc
            break;

            case 0x00EE: // 0x00EE: returns from subroutine
                sp--;            // Decrease the stack pointer to pop the return address
                pc = stack[sp];  // Set the program counter to the return address
                pc += 2;         // Move the program counter to the next instruction after the call
            break;
        }
        break;
    }

    case 0x1000: // 1NNN: jump to NNN
        pc = opcode & 0x0FFF; // set pc to NNN (mask to get low order 12 bits)
    break;
    
    case 0x2000: // 0x2NNN: call subroutine at NNN
        stack[sp] = pc;  // Push the current program counter onto the stack
        sp++;            // Increment the stack pointer
        pc = opcode & 0x0FFF;  // Set the program counter to NNN (mask to get the low-order 12 bits of the opcode)
    break;
    
    case 0x3000:{ // 0x3XNN: Skips the next instruction if VX equals NN
        unsigned char X = (opcode & 0x0F00) >> 8;  // Extract the register index (X)
        unsigned char NN = opcode & 0x00FF;        // Extract the immediate value (NN)

        if (V[X] == NN) {  // Compare VX with NN
            pc += 4;  // Skip the next instruction (2 bytes)
        } else {
            pc += 2;  // Move to the next instruction
        }
    break;
    }

    case 0x4000: {// 0x4XNN: Skips the next instruction if VX does not equal NN (usually the next instruction is a jump to skip a code block)
        unsigned char X = (opcode & 0x0F00) >> 8;  // Extract the register index (X)
        unsigned char NN = opcode & 0x00FF;        // Extract the immediate value (NN)

        if (V[X] != NN) {  // Compare VX with NN
            pc += 4;  // Skip the next instruction (2 bytes)
        } else {
            pc += 2;  // Move to the next instruction
        }
    break;
    }

    case 0x5000: {// 5XY0:Skips the next instruction if VX equals VY (usually the next instruction is a jump to skip a code block)
        unsigned char X = (opcode & 0x0F00) >> 8;  // Extract the register index (X)
        unsigned char Y = (opcode & 0x00F0) >> 4;  // Extract the register index (Y)

        if (V[X] == V[Y]) {  // Compare VX with NN
            pc += 4;  // Skip the next instruction (2 bytes)
        } else {
            pc += 2;  // Move to the next instruction
        }

    break;
    }

    case 0x6000: {// 6XNN: Sets VX to NN.
        unsigned char X = (opcode & 0x0F00) >> 8;  // Extract the register index (X)
        unsigned char NN = opcode & 0x00FF;        // Extract the immediate value (NN)

        V[X] = NN; // VX = NN
        pc += 2; // Move to next instruction
       
    break;
    }

    case 0x7000: { // 7XNN:	Const	Vx += NN	Adds NN to VX (carry flag is not changed)
        unsigned char X = (opcode & 0x0F00) >> 8;  // Extract the register index (X)
        unsigned char NN = opcode & 0x00FF;        // Extract the immediate value (NN)

        V[X] += NN;
        pc += 2;

    break;
    }

    case 0x8000: { // 8XY0	Assig	Vx = Vy	Sets VX to the value of VY.[24]
        unsigned char X = (opcode & 0x0F00) >> 8;  // Extract the register index (X)
        unsigned char Y = (opcode & 0x00F0) >> 4;        // Extract the register index (Y)

        V[X] = V[Y];
        pc += 2;

    break;
    }

    case 0x8001: {// 8XY1	BitOp	Vx |= Vy	Sets VX to VX or VY. (bitwise OR operation)
        unsigned char X = (opcode & 0x0F00) >> 8;  // Extract the register index (X)
        unsigned char Y = (opcode & 0x00F0) >> 4;        // Extract the register index (Y)

        V[X] = V[X] | V[Y]; // | is a bitwise OR
        pc += 2;
    break;
    }

    case 0x8002: { // 8XY2	BitOp	Vx &= Vy	Sets VX to VX and VY. (bitwise AND operation)
        unsigned char X = (opcode & 0x0F00) >> 8;  // Extract the register index (X)
        unsigned char Y = (opcode & 0x00F0) >> 4;        // Extract the register index (Y)

        V[X] = V[X] & V[Y]; // & is a bitwise AND
        pc += 2;

    break;
    }

    case 0x8003: { // 8XY3[a]	BitOp	Vx ^= Vy	Sets VX to VX xor VY
        unsigned char X = (opcode & 0x0F00) >> 8;  // Extract the register index (X)
        unsigned char Y = (opcode & 0x00F0) >> 4;        // Extract the register index (Y)

        V[X] = V[X] ^= V[Y]; // ^= is a bitwise XOR
        pc += 2;
        
    break;
    } 

    case 0x8004: { // 8XY4	Math	Vx += Vy	Adds VY to VX. VF is set to 1 when there's an overflow, and to 0 when there is not
        unsigned char X = (opcode & 0x0F00) >> 8;  // Extract the register index (X)
        unsigned char Y = (opcode & 0x00F0) >> 4;        // Extract the register index (Y)

        // Check if there will be a carry
        if (V[Y] > (0xFF - V[X])) {
            V[0xF] = 1;  // Carry occurred
        } else {
            V[0xF] = 0;  // No carry
        }

        V[X] += V[Y];
        pc += 2;

    break;
    }

    case 0x8005: { // 8XY5: Vx = Vx - Vy; VF = NOT borrow
        unsigned char X = (opcode & 0x0F00) >> 8;
        unsigned char Y = (opcode & 0x00F0) >> 4;
    
        if (V[X] >= V[Y]) {
            V[0xF] = 1; // No borrow
        } else {
            V[0xF] = 0; // Borrow occurred
        }
    
        V[X] -= V[Y];
        pc += 2;
        break;
    }

    case 0x8006: { // 8XY6[a]	BitOp	Vx >>= 1	Shifts VX to the right by 1, then stores the least significant bit of VX prior to the shift into VF
        unsigned char X = (opcode & 0x0F00) >> 8;
        // unsigned char Y = (opcode & 0x00F0) >> 4; // THIS ISN'T USED BY THE MODERN INTERPRETATION

        V[15] = V[X] & 0x01; // mask for the LSB
        V[X] >>= 1;
        pc += 2; 

        break;
    }

    case 0x8007: {// 8XY7[a]	Math	Vx = Vy - Vx	Sets VX to VY minus VX. VF is set to 0 when there's an underflow, and 1 when there is not. (i.e. VF set to 1 if VY >= VX)
        unsigned char X = (opcode & 0x0F00) >> 8;
        unsigned char Y = (opcode & 0x00F0) >> 4;

        if (V[Y] >= V[X]) {
            V[15] = 1;
        } else {
            V[15] = 0;
        }

        V[X] = V[Y] - V[X];
        pc += 2;

        break;
    }

    case 0x800E: {// 8XYE[a]	BitOp	Vx <<= 1	Shifts VX to the left by 1, then sets VF to 1 if the most significant bit of VX prior to that shift was set, or to 0 if it was unset.
        unsigned char X = (opcode & 0x0F00) >> 8;
        // unsigned char Y = (opcode & 0x00F0) >> 4; // Not needed in modern interpretation

        V[15] = (V[X] & 0x80) >> 7; // Store MSB before shift in VF [0x80 = 0b10000000 for hsb mask]
        V[X] <<= 1;
        pc += 2;

        break;
    }

    case 0x9000: {// 9XY0	Cond	if (Vx != Vy)	Skips the next instruction if VX does not equal VY. (Usually the next instruction is a jump to skip a code block)
        unsigned char X = (opcode & 0x0F00) >> 8;
        unsigned char Y = (opcode & 0x00F0) >> 4;

        // check if VX equals VY
        if (V[X] != V[Y]) {
            pc += 4; // skip next instruction
        } else {
            pc += 2; // go to next instruction
        }
        break;
    }

    // EXAMPLE OPCODE DECODE //
    case 0xA000: // ANNN: Sets I to the address NNN
        // Execute opcode
        I = opcode & 0x0FFF;
        pc += 2;
    break;
 
    case 0xB000: {// BNNN	Flow	PC = V0 + NNN	Jumps to the address NNN plus V0
        unsigned short NNN = (opcode & 0x0FFF);

        pc = V[0] + NNN;
        break;
    }

    case 0xC000: { // CXNN	Rand	Vx = rand() & NN	Sets VX to the result of a bitwise and operation on a random number (Typically: 0 to 255) and NN
        unsigned char X = (opcode & 0x0F00) >> 8;
        unsigned short NN = (opcode & 0x00FF);
        unsigned char randByte = rand() % 256;

        V[X] = randByte & NN;
        pc += 2;

        break;
    }

    case 0xD000: { // DXYN	Display	draw(Vx, Vy, N)
        unsigned char X = V[(opcode & 0x0F00) >> 8];
        unsigned char Y = V[(opcode & 0x00F0) >> 4];
        unsigned char height = opcode & 0x000F;

        V[0xF] = 0; // Reset collision flag

        for (int yLine = 0; yLine < height; yLine++) {
            unsigned char spriteByte = memory[I + yLine];
            for (int xLine = 0; xLine < 8; xLine++) {
                // Check if the bit in the sprite is set
                if ((spriteByte & (0x80 >> xLine)) != 0) {
                    int xCoord = (X + xLine) % 64;
                    int yCoord = (Y + yLine) % 32;
                    int index = xCoord + (yCoord * 64);

                    // Check if the pixel is already set
                    if (gfx[index] == 1) {
                        V[0xF] = 1; // Collision occurred
                    }
                    gfx[index] ^= 1; // XOR pixel
                }
            }
        }

        drawFlag = true; // Trigger screen redraw
        pc += 2;
        break;
    }


    default:
      printf ("Unknown opcode: 0x%X\n", opcode);
  }  
 
  // Update timers
  if(delay_timer > 0)
    --delay_timer;
 
  if(sound_timer > 0)
  {
    if(sound_timer == 1)
      printf("BEEP!\n");
    --sound_timer;
  }  
}
