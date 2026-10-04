/*
 * Author: Samriddhi
 * Course: CS2102
 * Description: IITPSx Two-Pass Assembler
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#define MAX_LABELS 200
#define MAX_LINE_LEN 256

typedef struct {
    const char *mnemonic;
    int opcode;
    int has_operand; /* 0 = no operand, 1 = requires operand */
    int is_branch;   /* 1 = PC-relative displacement, 0 = absolute */
} Instruction;

/* Complete IITPSx Instruction Lookup Table */
static const Instruction INSTRUCTION_TABLE[] = {
    {"data",   -1, 1, 0},
    {"ldc",     0, 1, 0},
    {"adc",     1, 1, 0},
    {"ldl",     2, 1, 0},
    {"stl",     3, 1, 0},
    {"ldnl",    4, 1, 0},
    {"stnl",    5, 1, 0},
    {"add",     6, 0, 0},
    {"sub",     7, 0, 0},
    {"shl",     8, 0, 0},
    {"shr",     9, 0, 0},
    {"adj",    10, 1, 0},
    {"a2sp",   11, 0, 0},
    {"sp2a",   12, 0, 0},
    {"call",   13, 1, 1},
    {"return", 14, 0, 0},
    {"brz",    15, 1, 1},
    {"brlz",   16, 1, 1},
    {"br",     17, 1, 1},
    {"HALT",   18, 0, 0},
    {"mul",    19, 0, 0},
    {"div",    20, 0, 0},
    {"SET",    -2, 1, 0}
};

static const int NUM_INSTRUCTIONS = sizeof(INSTRUCTION_TABLE) / sizeof(INSTRUCTION_TABLE[0]);

typedef struct {
    char name[64];
    int address;
    int is_set;
} Label;

static Label label_table[MAX_LABELS];
static int label_count = 0;

static const Instruction* find_instruction(const char *mnemonic) {
    int i;
    for (i = 0; i < NUM_INSTRUCTIONS; i++) {
        if (strcmp(INSTRUCTION_TABLE[i].mnemonic, mnemonic) == 0) {
            return &INSTRUCTION_TABLE[i];
        }
    }
    return NULL;
}

static int find_label(const char *name) {
    int i;
    for (i = 0; i < label_count; i++) {
        if (strcmp(label_table[i].name, name) == 0) {
            return label_table[i].address;
        }
    }
    return -1;
}

static char* trim_whitespace(char *str) {
    char *end;
    while (isspace((unsigned char)*str)) str++;
    if (*str == 0) return str;
    end = str + strlen(str) - 1;
    while (end > str && isspace((unsigned char)*end)) end--;
    end[1] = '\0';
    return str;
}

static int pass_one(FILE *fp) {
    char line[MAX_LINE_LEN];
    int pc = 0;
    int line_num = 0;
    int errors = 0;
    char *semicolon;
    char *cleaned;
    char *colon;
    char mnemonic[64];
    char label_name[64];
    const Instruction *inst;

    while (fgets(line, sizeof(line), fp)) {
        line_num++;

        semicolon = strchr(line, ';');
        if (semicolon) *semicolon = '\0';

        cleaned = trim_whitespace(line);
        if (strlen(cleaned) == 0) continue;

        colon = strchr(cleaned, ':');
        if (colon) {
            *colon = '\0';
            sscanf(cleaned, "%s", label_name);

            if (find_label(label_name) != -1) {
                fprintf(stderr, "Error line %d: Duplicate label '%s'\n", line_num, label_name);
                errors++;
            } else {
                strcpy(label_table[label_count].name, label_name);
                label_table[label_count].address = pc;
                label_table[label_count].is_set = 0;
                label_count++;
            }

            cleaned = trim_whitespace(colon + 1);
            if (strlen(cleaned) == 0) continue;
        }

        if (sscanf(cleaned, "%s", mnemonic) == 1) {
            inst = find_instruction(mnemonic);
            if (inst == NULL) {
                fprintf(stderr, "Error line %d: Unknown instruction '%s'\n", line_num, mnemonic);
                errors++;
            } else {
                pc++;
            }
        }
    }

    return errors;
}

static int pass_two(FILE *fp, FILE *out_obj, FILE *out_lst) {
    char line[MAX_LINE_LEN];
    int pc = 0;
    int line_num = 0;
    int errors = 0;

    char *semicolon;
    char *cleaned;
    char *colon;
    char *endptr;
    char mnemonic[64];
    char operand_str[64];
    char label_name[64];
    int num_tokens;
    const Instruction *inst;
    int operand_val;
    int machine_code;
    int target_addr;

    while (fgets(line, sizeof(line), fp)) {
        line_num++;
        operand_val = 0;
        machine_code = 0;

        semicolon = strchr(line, ';');
        if (semicolon) *semicolon = '\0';

        cleaned = trim_whitespace(line);
        if (strlen(cleaned) == 0) continue;

        colon = strchr(cleaned, ':');
        if (colon) {
            sscanf(cleaned, "%s", label_name);
            fprintf(out_lst, "%08X          %s:\n", pc, label_name);

            cleaned = trim_whitespace(colon + 1);
            if (strlen(cleaned) == 0) continue;
        }

        num_tokens = sscanf(cleaned, "%s %s", mnemonic, operand_str);
        if (num_tokens < 1) continue;

        inst = find_instruction(mnemonic);
        if (inst == NULL) {
            errors++;
            continue;
        }

        if (inst->has_operand) {
            if (num_tokens < 2) {
                fprintf(stderr, "Error line %d: Instruction '%s' requires an operand\n", line_num, mnemonic);
                errors++;
                continue;
            }

            operand_val = (int)strtol(operand_str, &endptr, 0);

            if (*endptr != '\0') {
                target_addr = find_label(operand_str);
                if (target_addr == -1) {
                    fprintf(stderr, "Error line %d: Undefined label '%s'\n", line_num, operand_str);
                    errors++;
                    continue;
                }

                if (inst->is_branch) {
                    operand_val = target_addr - (pc + 1);
                } else {
                    operand_val = target_addr;
                }
            }
        }

        if (inst->opcode == -1) {
            machine_code = operand_val;
        } else {
            machine_code = ((operand_val & 0xFFFFFF) << 8) | (inst->opcode & 0xFF);
        }

        fwrite(&machine_code, sizeof(int), 1, out_obj);

        if (inst->has_operand) {
            fprintf(out_lst, "%08X %08X %s %s\n", pc, machine_code, mnemonic, operand_str);
        } else {
            fprintf(out_lst, "%08X %08X %s\n", pc, machine_code, mnemonic);
        }

        pc++;
    }

    return errors;
}

int main(int argc, char *argv[]) {
    FILE *fp;
    FILE *out_obj;
    FILE *out_lst;
    char obj_filename[256];
    char lst_filename[256];
    char *dot;
    int pass1_errors, pass2_errors;

    if (argc < 2) {
        printf("Usage: %s <source_file.asm>\n", argv[0]);
        return 1;
    }

    strcpy(obj_filename, argv[1]);
    strcpy(lst_filename, argv[1]);

    dot = strrchr(obj_filename, '.');
    if (dot) *dot = '\0';
    strcat(obj_filename, ".o");

    dot = strrchr(lst_filename, '.');
    if (dot) *dot = '\0';
    strcat(lst_filename, ".lst");

    /* PASS 1 */
    fp = fopen(argv[1], "r");
    if (!fp) {
        perror("Error opening source file");
        return 1;
    }

    printf("Starting Pass 1 for %s...\n", argv[1]);
    pass1_errors = pass_one(fp);
    fclose(fp);

    if (pass1_errors > 0) {
        printf("Pass 1 failed with %d error(s).\n", pass1_errors);
        return 1;
    }

    /* PASS 2 */
    fp = fopen(argv[1], "r");
    if (!fp) {
        perror("Error reopening source file for Pass 2");
        return 1;
    }

    out_obj = fopen(obj_filename, "wb");
    out_lst = fopen(lst_filename, "w");

    if (!out_obj || !out_lst) {
        perror("Error creating output files");
        if (fp) fclose(fp);
        return 1;
    }

    printf("Starting Pass 2 for %s...\n", argv[1]);
    pass2_errors = pass_two(fp, out_obj, out_lst);

    fclose(fp);
    fclose(out_obj);
    fclose(out_lst);

    if (pass2_errors > 0) {
        printf("Pass 2 failed with %d error(s).\n", pass2_errors);
        return 1;
    }

    printf("Assembly complete! Generated:\n");
    printf("  Binary object file : %s\n", obj_filename);
    printf("  Listing file       : %s\n", lst_filename);

    return 0;
}
