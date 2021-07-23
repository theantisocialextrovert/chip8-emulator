#include "../include/chip8.h"
#include "assert.h"
#include <stdio.h>
#include <stdlib.h>
#include "../include/SDL2/SDL.h"
#include <time.h>

//-----------------------------------------------------------------
// CHIP8 MEMORY FUNCTIONS
//-----------------------------------------------------------------
void chip8_CheckMemIndexBound(int index) {
    // checks if the index provided in less then CHIP8_MEM_SIZE
    assert( index >= 0 && index < CHIP8_MEM_SIZE );
}

unsigned char chip8_GetMem (chip8Mem *memory, int index) {
    // fetches the value at index passed in chip8Mem->RAM
    chip8_CheckMemIndexBound(index);
    return memory->RAM[index];

}
void chip8_SetMem (chip8Mem *memory, int index, unsigned char val) {
    // sets the value at index passed in chip8Mem->RAM
    chip8_CheckMemIndexBound(index);
    memory->RAM[index] = val;
}

unsigned short chip8_FetchInstructionMem (chip8Mem *memory, int index) {
    unsigned char byte1 = chip8_GetMem (memory, index);
    unsigned char byte2 = chip8_GetMem (memory, index + 1);
    return (byte1 << 8) | byte2; 
}

//-----------------------------------------------------------------
// CHIP8 STACK FUNCTIONS
//-----------------------------------------------------------------
void chip8_CheckStackBound(int index) {
    assert( index < CHIP8_STACK_SIZE);
}

// Push operation
void chip8_StackPush ( chip8 *chip8, unsigned short val) {

    chip8->registers.stackPointer += 1;
    chip8_CheckStackBound(chip8->registers.stackPointer);
    chip8->stack.stack[chip8->registers.stackPointer] = val;
    
    
}
// Pop operation
unsigned short chip8_StackPop ( chip8 *chip8) {
    chip8_CheckStackBound(chip8->registers.stackPointer);
    unsigned short popped = chip8->stack.stack[chip8->registers.stackPointer];
    chip8->registers.stackPointer -= 1;
    return popped;
}

//------------------------------------------------------------------
// CHIP8 KEYBOARD FUNCTIONS
//------------------------------------------------------------------
char chip8_waitForKeyPress (chip8 *chip) {
    SDL_Event event;
    while ( SDL_WaitEvent(&event)) {

        if (event.type != SDL_KEYDOWN ) 
            continue;
        char pressedkey = event.key.keysym.sym;
        char chip8_key = chip8_MapKey( &chip->keyboard, pressedkey );
        if (chip8_key != -1 )
            return chip8_key;
    }
    return -1;
}

void chip8_KeyboardSetKeyboardMap ( chip8Keyboard *keyboard, const char *map) {
    keyboard->map = map;
}

int  chip8_MapKey    ( chip8Keyboard *keyboard, char key) {

    for (int index = 0 ; index < CHIP8_KEYBOARD_SIZE; index++){
        if ( keyboard->map[index] == key )
        return index;
    }
    return -1;
}

void chip8_PutKeyDown( chip8Keyboard *keyboard, int key) {
    keyboard->v_keyboard[key] = true;
}

void chip8_PutKeyUp  ( chip8Keyboard *keyboard, int key) {
    keyboard->v_keyboard[key] = false;
}
bool chip8_IsKeyDown ( chip8Keyboard *keyboard, int key) {
    return keyboard->v_keyboard[key];
}

//-------------------------------------------------------------------
// CHIP8 SCREEN FUNCTIONS
//-------------------------------------------------------------------
void chip8_ScreenCheckBound ( int x, int y) {
    assert (x >= 0 && x < WIN_WIDTH && y >=0 && y < WIN_HEIGHT);
}
void chip8_SetScreenPixle( chip8Screen *screen, int x, int y) {
    chip8_ScreenCheckBound( x, y);
    screen->pixle[y][x] = true;
}
bool chip8_IsScreenPixleSet ( chip8Screen *screen, int x, int y) {
    chip8_ScreenCheckBound( x, y);
    return screen->pixle[y][x];
}

bool chip8_ScreenDrawSprite ( chip8Screen *screen, int x, int y, const char *sprites, int num) {

    bool pixle_collision = false;
    for (int ly = 0; ly < num; ly++){
        char sprite = sprites[ly];
        for (int lx = 0; lx < 8; lx++) {
            if ((sprite & (0b10000000 >> lx)) == 0 )
                continue;

            if (screen->pixle[(ly + y)%WIN_HEIGHT][(lx + x)%WIN_WIDTH])
                pixle_collision = true;
            screen->pixle[(ly + y)%WIN_HEIGHT][(lx + x)%WIN_WIDTH]^= true;

        }
    }
    return pixle_collision;
}

void chip8_ScreenClear( chip8Screen *screen) {
    memset (screen->pixle, 0 , sizeof(screen->pixle));
}
//-------------------------------------------------------------------
// CHIP8 FUNCTIONS
//-------------------------------------------------------------------
//  CHIP8 DEFAULT CHAR SET
const char chip8_default_character_set[] = {
    0xf0, 0x90, 0x90, 0x90, 0xf0,
    0x20, 0x60, 0x20, 0x20, 0x70,
    0xf0, 0x10, 0xf0, 0x80, 0xf0,
    0xf0, 0x10, 0xf0, 0x10, 0xf0,
    0x90, 0x90, 0xf0, 0x10, 0x10,
    0xf0, 0x80, 0xf0, 0x10, 0xf0,
    0xf0, 0x80, 0xf0, 0x90, 0xf0,
    0xf0, 0x10, 0x20, 0x40, 0x40,
    0xf0, 0x90, 0xf0, 0x90, 0xf0,
    0xf0, 0x90, 0xf0, 0x10, 0xf0,
    0xf0, 0x90, 0xf0, 0x90, 0x90,
    0xe0, 0x90, 0xe0, 0x90, 0xe0,
    0xf0, 0x80, 0x80, 0x80, 0xf0,
    0xe0, 0x90, 0x90, 0x90, 0xe0,
    0xf0, 0x80, 0xf0, 0x80, 0xf0, 
    0xf0, 0x80, 0xf0, 0x80, 0x80
};

// CHIP8 INIT
void chip8_init( chip8 *chip) {
    memset(chip, 0 , sizeof(chip8));
    memcpy(&chip->memory.RAM, chip8_default_character_set, sizeof(chip8_default_character_set));
}

const char * chip8_ReadProgram( char *filename, long *filesize ) {

    FILE* file = fopen( filename, "rb");
    if (!file ) {
        printf("[ERROR] failed to open program file: %s \n",filename);
        return NULL;
    }

    fseek (file, 0, SEEK_END);
    long size = ftell(file);
    fseek (file, 0, SEEK_SET);
    char *buff = (char*) malloc(size);
    
    int res = fread(buff, size, 1, file);
    if (res != 1) {
        printf("[ERROR] fialed to read program file \n");
        return NULL;
    }
    *filesize = size;
    fclose(file);
    return buff;
}

void chip8_LoadProgram ( chip8 *chip, const char *buff, size_t size) {
    assert( CHIP8_PROGRAM_LOAD_ADDR + size < CHIP8_MEM_SIZE);
    memcpy( &chip->memory.RAM[CHIP8_PROGRAM_LOAD_ADDR], buff, size);
    chip->registers.PC = CHIP8_PROGRAM_LOAD_ADDR;
}

// Function for OPCODE starting from 8: eg - 8xy1, 8xy2
void chip8_ExecuteInstructionClassEight( chip8 *chip, unsigned short opcode){
    unsigned short nnn = opcode & 0x0fff;
    char x = (opcode >> 8) & 0x000f;
    char y = (opcode >> 4) & 0x000f;
    char kk = opcode & 0x00ff;
    char last_four_bits = opcode & 0x000f;
    unsigned short temp = 0;

    switch (last_four_bits) {
        //8xy0 - LD Vx, Vy, Set Vx = Vy
        case 0x00 :
            chip->registers.V_Registers[x] = chip->registers.V_Registers[y];
            break;
        //8xy1 - OR Vx, Vy, Set Vx = Vx OR Vy
        case 0x01 :
            chip->registers.V_Registers[x] |= chip->registers.V_Registers[y];
            break;
        //8xy2 - AND Vx, Vy, Set Vx = Vx AND Vy
        case 0x02 :
            chip->registers.V_Registers[x] &= chip->registers.V_Registers[y];
            break; 
        //8xy3 - XOR Vx, Vy, Set Vx = Vx XOR Vy.
        case 0x03 :
            chip->registers.V_Registers[x] ^= chip->registers.V_Registers[y];
            break;
        //8xy4 - ADD Vx, Vy, Set Vx = Vx + Vy, set VF = carry.
        case 0x04 :
        {
            temp = chip->registers.V_Registers[x] + chip->registers.V_Registers[y];
            chip->registers.V_Registers[0x0f] = false;
            if ( temp > 0xff ) {
                chip->registers.V_Registers[0x0f] = true;
            }
            chip->registers.V_Registers[x] = temp;
        }
            break;
        //8xy5 - SUB Vx, Vy, Set Vx = Vx - Vy, set VF = NOT borrow.
        case 0x05 :
        {
            chip->registers.V_Registers[0x0f] = false;
            if (  chip->registers.V_Registers[x] > chip->registers.V_Registers[y] ) {
                chip->registers.V_Registers[0x0f] = true;
            }
            chip->registers.V_Registers[x] =  chip->registers.V_Registers[x] - chip->registers.V_Registers[y];
        }
            break;
        //8xy6 - SHR Vx {, Vy}, Set Vx = Vx SHR 1.
        case 0x06 : 
        {
            chip->registers.V_Registers[0x0f] = chip->registers.V_Registers[x] & 0x01 ;
            chip->registers.V_Registers[x] /= 2;
        }       
            break;
        //8xy7 - SUBN Vx, Vy, Set Vx = Vy - Vx, set VF = NOT borrow.
        case 0x07 :
        { 
            chip->registers.V_Registers[0x0f] = chip->registers.V_Registers[y] > chip->registers.V_Registers[x];
            chip->registers.V_Registers[x] =  chip->registers.V_Registers[y] - chip->registers.V_Registers[x];
        }
            break;
        //8xyE - SHL Vx {, Vy}, Set Vx = Vx SHL 1.
        case 0x0E :
        { 
            chip->registers.V_Registers[0x0f] = chip->registers.V_Registers[x] & 0b10000000 ;
            chip->registers.V_Registers[x] *= 2;
        }
            break;


    }
}

// Execution function for opcode starting with F
void chip8_ExecuteInstructionClassF_instruction( chip8 *chip, unsigned short opcode) {
    unsigned short nnn = opcode & 0x0fff;
    char x = (nnn >> 8) & 0x000f;

    switch ( opcode & 0x00ff ) {
        //Fx07 - LD Vx, DT, Set Vx = delay timer value
        case 0x07 : 
            chip->registers.V_Registers[x] = chip->registers.delay_timer;
        break;
        //Fx0A - LD Vx, K, Wait for a key press, store the value of the key in Vx.
        case 0x0A :
        {
            char pressed_key               = chip8_waitForKeyPress(chip);
            chip->registers.V_Registers[x] = pressed_key;
            break;
        }
        //Fx15 - LD DT, Vx, Set delay timer = Vx.
        case 0x15 :
            chip->registers.delay_timer = chip->registers.V_Registers[x];
            break;
        //Fx18 - LD ST, Vx, Set sound timer = Vx
        case 0x18 :
            chip->registers.sound_timer = chip->registers.V_Registers[x];
            break;
        //Fx1E - ADD I, Vx,Set I = I + Vx.
        case 0x1E :
            chip->registers.I_Register += chip->registers.V_Registers[x];
            break;
        //Fx29 - LD F, Vx, Set I = location of sprite for digit Vx.
        case 0x29 :
            chip->registers.I_Register = chip->registers.V_Registers[x] * CHIP8_DEFAULT_SPRITE_HEIGHT;
            break;
        //Fx33 - LD B, Vx, Store BCD representation of Vx in memory locations I, I+1, and I+2.
        case 0x33 :
        {
            unsigned char hundered = chip->registers.V_Registers[x] / 100;
            unsigned char tenth    = chip->registers.V_Registers[x] / 10 % 10;
            unsigned char ones     = chip->registers.V_Registers[x] % 10;
            chip8_SetMem ( &chip->memory, chip->registers.I_Register, hundered);
            chip8_SetMem ( &chip->memory, chip->registers.I_Register + 1, tenth);
            chip8_SetMem ( &chip->memory, chip->registers.I_Register + 2, ones);
        }
            break;
        //Fx55 - LD [I], Vx, Store registers V0 through Vx in memory starting at location I
        case 0x55 :
        {
            for(int i=0; i<=x; i++){
                chip8_SetMem( &chip->memory, chip->registers.I_Register +i, chip->registers.V_Registers[i]);
            }
        }
            break;
        //Fx65 - LD Vx, [I], Read registers V0 through Vx from memory starting at location I
        case 0x65 :
        {
            for (int i =0; i<=x; i++) {
                chip->registers.V_Registers[i] = chip8_GetMem( &chip->memory, chip->registers.I_Register + i);
            }
        }
        break;
    }

}

void chip8_ExecuteInstructionExtended(chip8 *chip, unsigned short opcode) {
    
    unsigned short nnn = opcode & 0x0fff;
    char x = (opcode >> 8) & 0x000f;
    char y = (opcode >> 4) & 0x000f;
    char kk = opcode & 0x00ff;
    char n  = opcode & 0x000f;

    switch (opcode & 0xf000) {
        // JP: jump to nnn addr 
        case 0x1000 : 
            chip->registers.PC = nnn;
            break;
        case 0x2000 :
        // CALL: call a subroutine
            chip8_StackPush(chip, chip->registers.PC);
            chip->registers.PC = nnn;
            break;
        //3xkk - SE: skip next instruction
        case 0x3000 :
            if (chip->registers.V_Registers[x] == kk) {
                chip->registers.PC += 2;
            }
            break;
        //4xkk - SNE Vx, byte Skip next instruction if Vx != kk, increments the program counter by 2.
        case 0x4000 : 
            if (chip->registers.V_Registers[x] != kk) {
                chip->registers.PC += 2;
            }
            break;
        //5xy0 - SE Vx, Vy, Skip next instruction if Vx = Vy.
        case 0x5000 : 
            if (chip->registers.V_Registers[x] == chip->registers.V_Registers[y]) {
                chip->registers.PC += 2;
            }
            break;
        //6xkk - LD Vx, byte, Set Vx = kk.
        case 0x6000 :
            chip->registers.V_Registers[x] = kk;
            break;
        //7xkk - ADD Vx, byte, Set Vx = Vx + kk
        case 0x7000 : 
            chip->registers.V_Registers[x] += kk;
            break;
        //Instctrion starting from 8:
        case 0x8000 :
            chip8_ExecuteInstructionClassEight( chip, opcode);
            break;
        //9xy0 - SNE Vx, Vy, Skip next instruction if Vx != Vy.
        case 0x9000 :
            if ( chip->registers.V_Registers[x] != chip->registers.V_Registers[y] ) {
                chip->registers.PC += 2;
            }
            break;
        //Annn - LD I, addr, Set I = nnn
        case 0xA000 :
            chip->registers.I_Register = nnn;
            break;
        //Bnnn - JP V0, addr, Jump to location nnn + V0
        case 0xB000 :
            chip->registers.PC = nnn + chip->registers.V_Registers[0x00];
            break;
        //Cxkk - RND Vx, byte, Set Vx = random byte AND kk.
        case 0xC000 :
            srand( clock() );
            chip->registers.V_Registers[x] = (rand() % 255) & kk;
            break;
        //Dxyn - DRW Vx, Vy, nibble, Display n-byte sprite starting at memory location I at (Vx, Vy), set VF = collision.
        case 0xD000 :
            {
            const char *sprites = (const char*) &chip->memory.RAM[chip->registers.I_Register];
            chip->registers.V_Registers[0x0f] = chip8_ScreenDrawSprite(&chip->screen, chip->registers.V_Registers[x], chip->registers.V_Registers[y],sprites,n);
            }
            break;
        //Ex__ - Instructions starting with E : Keyboard operations
        case 0xE000 :
            {
            switch( opcode & 0x00ff) {
                //Ex9E - SKP Vx, Skip next instruction if key with the value of Vx is pressed
                case 0x9e :
                    if ( chip8_IsKeyDown( &chip->keyboard, chip->registers.V_Registers[x])) {
                        chip->registers.PC += 2;
                    }
                    break;
                //Ex9E - SKP Vx, Skip next instruction if key with the value of Vx is not pressed
                case 0xa1 :
                    if ( !chip8_IsKeyDown( &chip->keyboard, chip->registers.V_Registers[x])) {
                        chip->registers.PC += 2;
                    }
                    break;
                }

            }
            break;
        case 0xF000 :
            chip8_ExecuteInstructionClassF_instruction( chip, opcode);
            break;

    }
}

void chip8_ExecuteInstruction ( chip8 *chip, unsigned short opcode) {

    switch (opcode) {
        // CLS : clears screen
        case 0x00E0 :
            chip8_ScreenClear(&chip->screen);
            break;
        // RED : returns from subroutine
        case 0x00EE : 
            chip->registers.PC = chip8_StackPop(chip);
            break;
        default:
            chip8_ExecuteInstructionExtended(chip, opcode);
            
    }    
}
