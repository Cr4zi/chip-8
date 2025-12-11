#include <stdio.h>

#include "raylib.h"

#include "chip8.h"

#define RESCALE_FACTOR 20 

int main(int argc, char *argv[]) {
    if(argc != 2) {
        fprintf(stderr, "Usage ./chip-8 /path/to-rom\n");
        return 1;
    }

    initialize_emu();

    if(load_rom(argv[1])) {
        return 1;
    }

    InitWindow(GFX_COLS * RESCALE_FACTOR, GFX_ROWS * RESCALE_FACTOR, "chip-8 emulator");
    SetTargetFPS(FPS);
    
    while(!WindowShouldClose()) {
        execute();
        
        BeginDrawing();
        for(int i = 0; i < GFX_ROWS; i++) {
            for(int j = 0; j < GFX_COLS; j++) {
                if(graphics[i][j] == 0)
                    DrawRectangle(j * RESCALE_FACTOR, i * RESCALE_FACTOR, RESCALE_FACTOR, RESCALE_FACTOR, BLACK);
                else if(graphics[i][j] == 1)
                    DrawRectangle(j * RESCALE_FACTOR, i * RESCALE_FACTOR, RESCALE_FACTOR, RESCALE_FACTOR, RAYWHITE);
                else
                    fprintf(stderr, "Unrecognized value for graphics at (%d, %d)", i, j);
            }
        }
        EndDrawing();
    }

    CloseWindow();
    
    return 0;
}
