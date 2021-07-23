#include <stdio.h>
#include <windows.h>
#include "SDL2/SDL.h"
#include "chip8.h"

const char char_map[CHIP8_KEYBOARD_SIZE] = {
    SDLK_0, SDLK_1, SDLK_2, SDLK_3,
    SDLK_4, SDLK_5, SDLK_6, SDLK_7,
    SDLK_8, SDLK_9, SDLK_a, SDLK_b,
    SDLK_c, SDLK_d, SDLK_e, SDLK_f

};

int main(int argc, char **argv)
{
    if (argc < 2) {
        printf( "[ERROR] Please provide Program file");
        return -1;
    }

    //--------------------------------------------------------------------------
    // Reading Program file
    //--------------------------------------------------------------------------
    char *filename = argv[1];
    long filesize;

    printf("[!] File to be loaded : %s \n",filename);
    const char *buff = chip8_ReadProgram(filename,&filesize);
    
    if (!buff) {
        printf ("[ERROR] failed to read %s \n",filename);
        return -1;
    }
    printf("------------ buff = %s size = %d \n",buff,filesize);

//------------------------------------------------------------------------
// CHIP8 INSTANCE CREATION
//------------------------------------------------------------------------
    struct chip8 chip8;
    chip8_init(&chip8);
    chip8_KeyboardSetKeyboardMap (&chip8.keyboard, char_map);
    chip8_LoadProgram (&chip8, buff, filesize);

//-------------------------------------------------------------------------
// Initialising SDL
//-------------------------------------------------------------------------
    if (SDL_Init(SDL_INIT_EVERYTHING) < 0) {
        printf("[!ERROR} error from SDL_Init : %s \n",SDL_GetError());
    }
    
//-------------------------------------------------------------------------
// Creating SDL Window
//-------------------------------------------------------------------------
    SDL_Window *window = SDL_CreateWindow(
        WIN_TITLE,
        SDL_WINDOWPOS_UNDEFINED,
        SDL_WINDOWPOS_UNDEFINED,
        WIN_WIDTH * WIN_MULTIPLIER,
        WIN_HEIGHT * WIN_MULTIPLIER,
        SDL_WINDOW_SHOWN);

    if (window == NULL) {
        printf("[!ERROR} error from window creation : %s \n",SDL_GetError());
    }

//-------------------------------------------------------------------------
// Creating SDL renderer
//-------------------------------------------------------------------------
    SDL_Renderer *renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_SOFTWARE);
    if (!renderer) {
        printf("[!ERROR] error from renderer creation : %s \n", SDL_GetError());
    }

//-------------------------------------------------------------------------
// MAIN LOOP 
//-------------------------------------------------------------------------
    while (1)
    {
        //Polling for SDL events
        SDL_Event event;
        while (SDL_PollEvent(&event))
        {
            switch (event.type)
            {
            case SDL_QUIT:
                printf("exiting \n");
                goto out;
                break;
            case SDL_KEYDOWN:
            {
                char key = event.key.keysym.sym;
                int vir_key = chip8_MapKey(&chip8.keyboard, key);
                if (vir_key > 0) {
                    chip8_PutKeyDown(&chip8.keyboard, vir_key);
                }
                break;
            }
            case SDL_KEYUP:
            {
                char key = event.key.keysym.sym;
                int vir_key = chip8_MapKey(&chip8.keyboard, key);
                if (vir_key > 0) {
                    chip8_PutKeyUp(&chip8.keyboard, vir_key);
                }
                break;
            }
            default:
                break;
            }
        }
        // Setting up Renderer
        SDL_SetRenderDrawColor(renderer, 0, 0, 0, 0);
        SDL_RenderClear(renderer);
        SDL_SetRenderDrawColor(renderer, 255, 255, 255, 0);

        // Displaying pixels 
        for (int x =0 ; x < WIN_WIDTH; x++) {
            for (int y = 0; y < WIN_HEIGHT; y++) {
                if (chip8_IsScreenPixleSet(&chip8.screen, x, y)) {
                    SDL_Rect r;
                    r.x = x*WIN_MULTIPLIER;
                    r.y = y*WIN_MULTIPLIER;
                    r.w = WIN_MULTIPLIER;
                    r.h = WIN_MULTIPLIER;
                    SDL_RenderFillRect(renderer, &r);
                }
            }
        }
        SDL_RenderPresent(renderer);
        
        // Delay timer
        if (chip8.registers.delay_timer > 0) {
        /*TODO: Right now need to change timer value for each game separately,
           need to figure out a standard value for each game*/
            Sleep(70);
            chip8.registers.delay_timer -= 1;
        }

        // Sound timer
        if (chip8.registers.sound_timer > 0) {
            // TODO: Beep not working 
            //Beep(15000, 100 );
            chip8.registers.sound_timer -= 1;
            
        }
        // Fetching Instruction
        unsigned short opcode = chip8_FetchInstructionMem(&chip8.memory, chip8.registers.PC);
        chip8.registers.PC += 2;
        chip8_ExecuteInstruction( &chip8, opcode);
    }



out:
    SDL_DestroyWindow(window);
    return 0;
}