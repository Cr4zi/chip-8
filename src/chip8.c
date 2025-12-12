#include "chip8.h"

uint8_t memory[MEM_SIZE];
uint8_t V[16];
uint16_t PC;
uint16_t I;
uint8_t SP;
uint16_t stack[STACK_SIZE];
uint8_t DT;
uint8_t ST;
uint8_t keyboard[NUM_OF_KEYS];
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
    memset(keyboard, 0, sizeof(uint8_t) * 16);
    memset(graphics, 0, sizeof(bool) * GFX_ROWS * GFX_COLS);

    for(int i = 0; i < FONTS_SIZE; i++)
        memory[FONT_ADDR + i] = font[i];

    srand(time(NULL));
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

static void XY_operations(uint16_t instr) {
    uint8_t nibble = instr & 0x000F;
    uint8_t x = (instr & 0x0F00) >> 8; // hold which register
    uint8_t y = V[(instr & 0x00F0) >> 4]; // hold value
    switch(nibble) {
        case 0x0: // Assign
            V[x] = y;
            break;
        case 0x1: // OR
            V[x] |= y;
            break;
        case 0x2: // AND
            V[x] &= y;
            break;
        case 0x3: // XOR
            V[x] ^= y;
            break;
        case 0x4: // ADD
            V[0xF] = (uint16_t)V[x] + (uint16_t)y > 255;
            V[x] += y;
            break;
        case 0x5: // SUB
            V[0xF] = V[x] >= y;
            V[x] -= y;
            break;
        case 0x6: // SHR
            V[0xF] = V[x] & 1;
            V[x] >>= 1;
            break;
        case 0x7:
            V[0xF] = y >= V[x];
            V[x] = y - V[x];
            break;
        case 0xE: // SHL
            V[0xF] = (V[x] & (1 << 7));
            V[x] <<= 1;
            break;
        default:
            fprintf(stderr, "No such instructions: %d.\n", instr);
            break;
    }

    PC += INSTRUCTION_SIZE;
}

static void handle_opcode_f(uint16_t instr) {
    uint8_t x = (instr & 0x0F00) >> 8, n;
    switch(instr & 0x00FF) {
        case 0x07: // LD Vx, DT
            V[x] = DT;
            PC += INSTRUCTION_SIZE;
            break;
        case 0x0A: // LD Vx, K
            for(int i = 0; i < NUM_OF_KEYS; i++) {
                if(keyboard[i]) {
                    V[x] = i;
                    PC += INSTRUCTION_SIZE;
                    break;
                }
            }
            break;
        case 0x15: // LD DT, Vx
            DT = V[x];
            PC += INSTRUCTION_SIZE;
            break;
        case 0x18: // LD ST, Vx
            ST = V[x];
            PC += INSTRUCTION_SIZE;
            break;
        case 0x1E: // ADD I, Vx
            I += V[x];
            PC += INSTRUCTION_SIZE;
            break;
        case 0x29: // LD F, Vx
            I = FONT_ADDR + 5 * V[x];
            PC += INSTRUCTION_SIZE;
            break;
        case 0x33: // LD B, Vx
            n = V[x];
            memory[I + 2] = n % 10;
            memory[I + 1] = (n / 10) % 10;
            memory[I] = n / 100;
            PC += INSTRUCTION_SIZE;
            break;
        case 0x55:
            for(int i = 0; i <= x; i++)
                memory[I + i] = V[i];
            PC += INSTRUCTION_SIZE;
            break;
        case 0x65:
            for(int i = 0; i <= x; i++)
                V[i] = memory[I + i];
            PC += INSTRUCTION_SIZE;
            break;
        default:
            fprintf(stderr, "No such instruction: %d\n", instr);
            break;
    }
}

static uint8_t random_byte() {
    return rand() & 255;
}

void execute() {
    uint8_t x, y, n;
    uint16_t instr = (memory[PC] << 8) | (memory[PC + 1]);
    uint8_t opcode = (instr & 0xF000) >> 12;
    switch(opcode) {
        case 0x0:
            if((instr & 0x0F00) == 0 && (instr & 0x00E0) == 0x00E0) {
                if((instr & 0x000F) == 0x0) { // CLS
                    memset(graphics, 0, sizeof(bool) * GFX_ROWS * GFX_COLS);
                    PC += INSTRUCTION_SIZE;
                } else { // RET
                    PC = stack[SP];
                    SP--;
                }
            }
            break;
        case 0x1: // JP addr
            PC = instr & 0x0FFF;
            break;
        case 0x2: // CALL addr
            SP++;
            stack[SP] = PC + INSTRUCTION_SIZE;
            PC = instr & 0x0FFF;
            break;
        case 0x3: // SE Vx, byte
            x = V[(instr & 0x0F00) >> 8];
            n = instr & 0x00FF;
            if(x == n)
                PC += 2 * INSTRUCTION_SIZE;
            else
                PC += INSTRUCTION_SIZE;
            break;
        case 0x4: // SNE Vx, byte
            x = V[(instr & 0x0F00) >> 8];
            n = instr & 0x00FF;
            if(x != n)
                PC += 2 * INSTRUCTION_SIZE;
            else
                PC += INSTRUCTION_SIZE;
            break;
        case 0x5: // SE Vx, Vy
            x = V[(instr & 0x0F00) >> 8];
            y = V[(instr & 0x00F0) >> 4];
            if(x == y)
                PC += 2 * INSTRUCTION_SIZE;
            else
                PC += INSTRUCTION_SIZE;
            break;
        case 0x6: // LD Vx, byte
            x = (instr & 0x0F00) >> 8;
            V[x] = instr & 0x00FF;
            PC += INSTRUCTION_SIZE;
            break;
        case 0x7: // ADD Vx, byte
            x = (instr & 0x0F00) >> 8;
            V[x] += instr & 0x00FF;
            PC += INSTRUCTION_SIZE;
            break;
        case 0x8:
            XY_operations(instr);
            break;
        case 0x9:
            x = V[(instr & 0x0F00) >> 8];
            y = V[(instr & 0x00F0) >> 4];
            if(x != y)
                PC += 2 * INSTRUCTION_SIZE;
            else
                PC += INSTRUCTION_SIZE;
            break;
        case 0xA: // LD I, addr
            I = instr & 0x0FFF;
            PC += INSTRUCTION_SIZE;
            break;
        case 0xB:
            PC = (instr & 0x0FFF) + V[0];
            break;
        case 0xC:
            V[(instr & 0x0F00) >> 8] = random_byte() & (instr & 0x00FF);
            PC += INSTRUCTION_SIZE;
            break;
        case 0xD: // DRW Vx, Vy, nibble
            n = instr & 0x000F;
            x = V[(instr & 0x0F00) >> 8] & 63;
            y = V[(instr & 0x00F0) >> 4] & 31;
            V[0xF] = 0;
            for(uint16_t i = 0; i < n; i++) {
                uint8_t sprite_data = memory[I + i];
                
                for(uint8_t j = 0; j < 8; j++) {
                    if((sprite_data & (0x80 >> j)) != 0) {
                        uint8_t dx = (x + j) % 64;
                        uint8_t dy = (y + i) % 32;

                        if(graphics[dy][dx] == 1)
                            V[0xF] = 1;
                        graphics[dy][dx] ^= 1;
                    }

                }
            }
            PC += INSTRUCTION_SIZE;
            break;
        case 0xE:
            x = (instr & 0x0F00) >> 8;
            if((instr & 0x00FF) == 0x009E) {// SKP Vx
                PC += keyboard[V[x]] * INSTRUCTION_SIZE + INSTRUCTION_SIZE;
            }
            else if((instr & 0x00FF) == 0x00A1) { // SKNP Vx
                PC += (!keyboard[V[x]]) * INSTRUCTION_SIZE + INSTRUCTION_SIZE;
            } 
            else 
                fprintf(stderr, "No such instruction: %d\n", instr);
            break;
        case 0xF:
            handle_opcode_f(instr);
            break;
        default:
            fprintf(stderr, "%d not implemented yet!\n", instr);
            break;
    }
}
