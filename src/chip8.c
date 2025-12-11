#include "chip8.h"


uint8_t memory[MEM_SIZE];
uint8_t V[16];
uint16_t PC;
uint16_t I;
uint8_t SP;
uint16_t stack[STACK_SIZE];
uint8_t DT;
uint8_t ST;
bool graphics[GFX_ROWS][GFX_COLS];

void initialize_emu() {
    PC = 0x200;
    I = 0;
    SP = 0;
    DT = 0;
    ST = 0;

    memset(memory, 0, sizeof(uint8_t) * MEM_SIZE);
    memset(V, 0, sizeof(uint8_t) * 16);
    memset(stack, 0, sizeof(uint16_t) * STACK_SIZE);
    memset(graphics, 0, sizeof(bool) * GFX_ROWS * GFX_COLS);

    for(int i = 0; i < 80; i++)
        memory[FONT_ADDR + i] = font[i];
}

int load_rom(const char *location) {
    FILE *rom;
    if(!(rom = fopen(location, "r"))) {
        fprintf(stderr, "Rom doesn't exists.\n");
        return 1;
    }

    if(fread(&memory[0x200], 1, MEM_SIZE - 0x200, rom) == 0) {
        fprintf(stderr, "Failed to load rom to memory");
        return 1;
    }

    fclose(rom);

    return 0;
}

void execute() {
    uint8_t x, y, n;
    uint16_t instr = memory[PC] << 8 | memory[PC + 1];
    uint8_t opcode = (instr & 0xF000) >> 12;
    switch(opcode) {
        case 0x0:
            if((instr & 0x0F00) == 0 && (instr & 0x00E0) == 0x00E0) {
                if((instr & 0x000F) == 0x0) {
                    memset(graphics, 0, sizeof(bool) * GFX_ROWS * GFX_COLS);
                    PC += 2;
                } else {
                    PC = stack[SP];
                    SP--;
                }
            }
            break;
        case 0x1:
            PC = instr & 0x0FFF;
            break;
        case 0x6:
            x = (instr & 0x0F00) >> 8;
            V[x] = instr & 0x00FF;
            PC += 2;
            break;
        case 0x7:
            x = (instr & 0x0F00) >> 8;
            V[x] += instr & 0x00FF;
            PC += 2;
            break;
        case 0xA:
            I = instr & 0x0FFF;
            PC += 2;
            break;
        case 0xD:
            n = instr & 0x000F;
            x = V[(instr & 0x0F00) >> 8] & 63;
            y = V[(instr & 0x00F0) >> 4] & 31;
            V[0xF] = 0;
            for(uint16_t i = 0; i < n; i++) {
                uint8_t sprite_data = memory[I + i];

                for(uint8_t j = 0; j < 8; j++) {
                    uint8_t pixel = (sprite_data >> (7 - j)) & 1;

                    if(pixel) {
                        uint8_t dx = (x + j) & 63;
                        uint8_t dy = (y + i) & 31;

                        if(graphics[dy][dx] == 1)
                            V[0xF] = 1;

                        graphics[dy][dx] ^= 1;
                    }
                }
            }
            PC += 2;
            break;

        default:
            fprintf(stderr, "%d not implemented yet!", instr & 0xF000);
            break;
    }
}
