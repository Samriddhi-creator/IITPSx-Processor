/*
 * Author: Samriddhi
 * Course: CS2102
 * Description: IITPSx Processor Emulator
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define MEMORY_SIZE 65536

static int32_t memory[MEMORY_SIZE];

/* Signs-extends a 24-bit integer to a 32-bit signed integer */
static int32_t sign_extend24(int32_t val) {
    if (val & 0x00800000) {
        return val | 0xFF000000;
    }
    return val & 0x00FFFFFF;
}

int main(int argc, char *argv[]) {
    FILE *fp;
    int32_t pc = 0;
    int32_t sp = 0;
    int32_t a = 0;
    int32_t b = 0;
    int32_t instruction_count = 0;
    int words_read;
    int running = 1;

    if (argc < 2) {
        printf("Usage: %s <object_file.o>\n", argv[0]);
        return 1;
    }

    fp = fopen(argv[1], "rb");
    if (!fp) {
        perror("Error opening binary object file");
        return 1;
    }

    memset(memory, 0, sizeof(memory));
    words_read = (int)fread(memory, sizeof(int32_t), MEMORY_SIZE, fp);
    fclose(fp);

    printf("Loaded %d words from %s into memory.\n", words_read, argv[1]);
    printf("Executing program...\n");

    while (running) {
        int32_t instr;
        int opcode;
        int32_t operand;

        if (pc < 0 || pc >= MEMORY_SIZE) {
            fprintf(stderr, "Runtime Error: PC 0x%08X out of memory bounds\n", pc);
            return 1;
        }

        instr = memory[pc];
        opcode = instr & 0xFF;
        operand = sign_extend24((instr >> 8) & 0xFFFFFF);

        /* Increment PC before executing instruction as required by specification */
        pc++;
        instruction_count++;

        switch (opcode) {
            case 0:  /* ldc */  b = a; a = operand; break;
            case 1:  /* adc */  a = a + operand; break;
            case 2:  /* ldl */  b = a; a = memory[sp + operand]; break;
            case 3:  /* stl */  memory[sp + operand] = a; a = b; break;
            case 4:  /* ldnl */ a = memory[a + operand]; break;
            case 5:  /* stnl */ memory[a + operand] = b; break;
            case 6:  /* add */  a = b + a; break;
            case 7:  /* sub */  a = b - a; break;
            case 8:  /* shl */  a = b << a; break;
            case 9:  /* shr */  a = b >> a; break;
            case 10: /* adj */  sp = sp + operand; break;
            case 11: /* a2sp */ sp = a; a = b; break;
            case 12: /* sp2a */ b = a; a = sp; break;
            case 13: /* call */ b = a; a = pc; pc = pc + operand; break;
            case 14: /* return */ pc = a; a = b; break;
            case 15: /* brz */  if (a == 0) pc = pc + operand; break;
            case 16: /* brlz */ if (a < 0) pc = pc + operand; break;
            case 17: /* br */   pc = pc + operand; break;
            case 18: /* HALT */ running = 0; break;
            case 19: /* mul */  a = b * a; break;
            case 20: /* div */
                if (a == 0) {
                    fprintf(stderr, "Runtime Error: Division by zero at PC=0x%08X\n", pc - 1);
                    return 1;
                }
                a = b / a;
                break;
            default:
                fprintf(stderr, "Runtime Error: Unknown opcode %d at PC=0x%08X\n", opcode, pc - 1);
                return 1;
        }
    }

    printf("\n--- Execution Finished --- \n");
    printf("Instructions executed : %d\n", instruction_count);
    printf("Final PC              : 0x%08X (%d)\n", pc, pc);
    printf("Register A            : %d (0x%08X)\n", a, a);
    printf("Register B            : %d (0x%08X)\n", b, b);
    printf("Stack Pointer (SP)    : 0x%08X (%d)\n", sp, sp);

    return 0;
}
