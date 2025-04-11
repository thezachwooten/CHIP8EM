#ifndef CHIP8_H
#define CHIP8_H

// Include necessary libraries

// Class definition
class Chip8 {
public:
    Chip8();            // Constructor
    ~Chip8();           // Destructor

    void initialize();  // Method to initialize the emulator
    void loadProgram(const char* filename);
    void emulateCycle();
    
    bool drawFlag; // boolean to indicate whether to draw to screen or not

private:
    unsigned short opcode;
    unsigned char memory[4096];
    unsigned char V[16];         // CPU registers V0 to VF
    unsigned short I;            // Index register
    unsigned short pc;           // Program counter
    unsigned char gfx[64 * 32];  // Graphics (black and white)
    unsigned char delay_timer;
    unsigned char sound_timer;
    unsigned short stack[16];    // Stack
    unsigned short sp;           // Stack pointer
    unsigned char key[16];       // HEX-based keypad (0x0-0xF)
};

#endif // CHIP8_H
