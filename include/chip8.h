#include "configuration.h"
#include <stdbool.h>
#include <string.h>
#include <stddef.h>

typedef struct chip8 chip8;

//------------------------------------------------------------------------------------------
// CHIP8 MEMORY
//------------------------------------------------------------------------------------------
typedef struct chip8Mem {
    unsigned char RAM[CHIP8_MEM_SIZE];
} chip8Mem;

// Memory functions
void chip8_CheckMemIndexBound(int index);
unsigned char chip8_GetMem (chip8Mem *memory, int index);
void chip8_SetMem (chip8Mem *memory, int index, unsigned char val);
unsigned short chip8_FetchInstructionMem (chip8Mem *memory, int index);

//------------------------------------------------------------------------------------------
// CHIP8 REGISTERS
//------------------------------------------------------------------------------------------
typedef struct chip8Registers {
    //8-bit registers
    unsigned char V_Registers[V_REGISTER_COUNT];
    unsigned char delay_timer;
    unsigned char sound_timer;
    unsigned char stackPointer;
    //16-bit registers
    unsigned short I_Register;
    unsigned short PC;
    
} chip8Registers;

//------------------------------------------------------------------------------------------
// CHIP8 STACK
//------------------------------------------------------------------------------------------
typedef struct chip8Stack {
    unsigned short stack[CHIP8_STACK_SIZE];
} chip8Stack;

// Push operation
void chip8_StackPush ( chip8 *chip8, unsigned short val);
// Pop operation
unsigned short chip8_StackPop ( chip8 *chip8);
// Check stack bound
void chip8_CheckStackBound ( int index );

//------------------------------------------------------------------------------------------
// CHIP8 KEYBOARD
//------------------------------------------------------------------------------------------
typedef struct chip8Keyboard {
    bool v_keyboard[CHIP8_KEYBOARD_SIZE];
    const char *map;
} chip8Keyboard;

char chip8_waitForKeyPress (chip8 *chip);
void chip8_KeyboardSetKeyboardMap ( chip8Keyboard *keyboard, const char *map);
int  chip8_MapKey    ( chip8Keyboard *keyboard, char key);
void chip8_PutKeyDown( chip8Keyboard *keyboard, int key);
void chip8_PutKeyUp  ( chip8Keyboard *keyboard, int key);
bool chip8_IsKeyDown ( chip8Keyboard *keyboard, int key);

//------------------------------------------------------------------------------------------
// CHIP8 SCREEN
//------------------------------------------------------------------------------------------
typedef struct chip8Screen {
    bool pixle[WIN_HEIGHT][WIN_WIDTH];
} chip8Screen;

void chip8_ScreenCheckBound (int x, int y);
void chip8_SetScreenPixle( chip8Screen *screen, int x, int y);
bool chip8_IsScreenPixleSet ( chip8Screen *screen, int x, int y);
bool chip8_ScreenDrawSprite ( chip8Screen *screen, int x, int y, const char *sprite, int num);
void chip8_ScreenClear (chip8Screen *screen);

//------------------------------------------------------------------------------------------
// CHIP8
//------------------------------------------------------------------------------------------
struct chip8 {
    chip8Mem memory;
    chip8Registers registers;
    chip8Stack stack;
    chip8Keyboard keyboard;
    chip8Screen screen;
};

void chip8_init( chip8 *chip );
const char * chip8_ReadProgram( char *filename, long *filesize );
void chip8_LoadProgram ( chip8 *chip, const char *buff, size_t size);
void chip8_ExecuteInstruction ( chip8 *chip, unsigned short opcode);
void chip8_ExecuteInstructionExtended(chip8 *chip, unsigned short opcode);
void chip8_ExecuteInstructionClassEight( chip8 *chip, unsigned short opcode);
void chip8_ExecuteInstructionClassF_instruction( chip8 *chip, unsigned short opcode);