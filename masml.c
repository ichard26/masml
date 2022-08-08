// Referenced resources:
// - https://stackoverflow.com/questions/940087/whats-the-correct-way-to-use-printf-to-print-a-size-t
// - https://stackoverflow.com/questions/164194/why-do-i-get-a-segmentation-fault-when-writing-to-a-char-s-initialized-with-a
// - https://stackoverflow.com/questions/9907160/how-to-convert-enum-names-to-string-in-c
// - https://linux.die.net/man/3/strtok_r
// - https://www.open-std.org/jtc1/sc22/wg14/www/docs/n1256.pdf
// - https://en.cppreference.com/w/c/language/_Static_assert
// - https://stackoverflow.com/questions/7228438/convert-double-float-to-string
// - https://www.tutorialspoint.com/c_standard_library/c_function_atof.htm
// - https://makefiletutorial.com/
// - https://vimawesome.com/
// - https://stackoverflow.com/questions/9549729/vim-insert-the-same-characters-across-multiple-lines
// - https://www.databasesandlife.com/position-of-the-star-when-declaring-c-pointers/
// - https://stackoverflow.com/a/64339233/
// - https://cplusplus.com/reference/cstring/memset/
// - https://stackoverflow.com/questions/47202557/what-is-a-designated-initializer-in-c
// - https://stackoverflow.com/questions/24263291/define-a-makefile-variable-using-a-env-variable-or-a-default-value
// - https://stackoverflow.com/questions/14562845/why-does-passing-char-as-const-char-generate-a-warning
// - https://stackoverflow.com/questions/9642732/parsing-command-line-arguments-in-c
// - https://stackoverflow.com/questions/4647665/why-cast-an-unused-function-parameter-value-to-void

#include "clikit.h"

#include <assert.h>
#include <math.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define NELEMS(x) (sizeof(x) / sizeof(x[0]))

#define RAM_SIZE 1000

typedef enum {
    LOAD, STORE,
    SET_REG, SWAP,
    ADD, SUB, MUL, DIV, MOD,
    EQUAL, NOT,
    GOTO, GOTO_IF, GOTO_IF_NOT, EXIT,
    PRINT
} InstructionType;

// TODO: find a better way of creating string arrays for enum members.
const char * const instruction_type_names[] = {
    [LOAD] = "LOAD", [STORE] = "STORE",
    [SET_REG] = "SET-REGISTER", [SWAP] = "SWAP",
    [ADD] = "ADD", [SUB] = "SUBTRACT", [MUL] = "MULTIPLY", [DIV] = "DIVIDE", [MOD] = "MODULO",
    [EQUAL] = "EQUAL", [NOT] = "NOT",
    [GOTO] = "GOTO", [GOTO_IF] = "GOTO-IF", [GOTO_IF_NOT] = "GOTO-IF-NOT", [EXIT] = "EXIT",
    [PRINT] = "PRINT",
    NULL
};
static_assert((sizeof(instruction_type_names) / sizeof(instruction_type_names[0]) == PRINT + 2),
    "instruction_type_names is misssing a InstructionType name!"
);

typedef enum { REG_NONE, REG_A, REG_B } RegisterID;

typedef struct {
    InstructionType type;
    RegisterID reg;
    double *arg;
} Instruction;

typedef struct {
    Instruction *instrs;
    size_t instr_count;
} Program;


static void free_char_ppbuf(char **ppbuf)
{
    for (size_t i = 0; ppbuf[i]; i++) {
        free(ppbuf[i]);
    }
    free(ppbuf);
}

static char **read_file(char const *filepath)
{
    FILE *fp = fopen(filepath, "r");
    if (fp == NULL) {
        printf("[FATAL] can't open file: %s\n", filepath);
        return NULL;
    }

    size_t allocated_lines = 100;
    size_t line = 0;
    char str[64];
    char **ppbuf = calloc(sizeof(char *), allocated_lines);
    if (ppbuf == NULL) {
        printf("[FATAL] failed to malloc program ppbuf!\n");
        goto BAIL;
    }

    while (!feof(fp)) {
        if (fgets(str, sizeof(str), fp)) {
            ppbuf[line] = malloc(sizeof(char) * strlen(str) + 1);
            if (ppbuf[line] == NULL) {
                printf("[FATAL] failed to malloc program ppbuf[%zu]\n", line);
                goto BAIL;
            }
            strcpy(ppbuf[line], str);
            line++;
            if (line >= allocated_lines) {
                allocated_lines += 100;
                char **new_ppbuf = realloc(ppbuf, sizeof(ppbuf[0]) * allocated_lines);
                if (new_ppbuf == NULL) {
                    printf("[FATAL] failed to realloc program ppbuf[%zu]\n", line);
                    goto BAIL;
                }
                ppbuf = new_ppbuf;
            }
        }
    }
    ppbuf[line] = NULL;
    fclose(fp);
    return ppbuf;

BAIL:
    if (ppbuf != NULL) {
        free_char_ppbuf(ppbuf);
    }
    fclose(fp);
    return NULL;
}

static bool find_string(char * const strings[], char * const target, size_t *index)
{
    for (*index = 0; strings[*index]; (*index)++) {
        if (strcmp(strings[*index], target) == 0) {
            return true;
        }
    }
    return false;
}

void free_program(Program *program)
{
    if (program->instr_count > 0) {
        for (size_t i = 0; i < program->instr_count; i++) {
            free(program->instrs[i].arg);
        }
    }
    free(program->instrs);
    free(program);
}

Program *parse(char *ppbuf[], bool debug)
{
    Program *prog = malloc(sizeof(*prog));
    size_t variables_size = 15;
    size_t instrs_size = 100;
    char **variables = calloc(sizeof(char *), variables_size);
    prog->instr_count = 0;
    prog->instrs = calloc(sizeof(Instruction), instrs_size);
    size_t i = 1;
    // Since I want to print the line currently being parsed on error, a copy of `line`
    // needs to be kept as strtok() will modify `line`.
    //
    // NOTE: `line_copy` is only used for the BAIL printf far below. This is
    // because `variables` (and probably other variables too) store char pointers that
    // point into `line` which comes from `ppbuf` until the **whole** program has been
    // parsed. `line_copy` is meant to be freed after each line is parsed, so it won't
    // work for the aforementioned variables. The alternative would be to duplicate
    // `ppbuf` in its entirety, but that seems overkill (for now).
    char *line = NULL, *line_copy = NULL;

    for (; (line = ppbuf[i - 1]); i++) {
        line_copy = malloc(sizeof(char) * strlen(line) + 1);
        strcpy(line_copy, line);
        if (line[0] == '\n' || line[0] == '#') {
            goto SKIP_LINE;
        }

        // Core tokenization logic follows below:
        char *tokens[3] = {0};
        tokens[0] = strtok(line, " ");
        tokens[1] = strtok(NULL, " ");
        tokens[2] = strtok(NULL, " ");
        size_t last = (bool)tokens[0] + (bool)tokens[1] + (bool)tokens[2] - 1;
        size_t last_length = strlen(tokens[last]);
        if (last == 2 && tokens[last][last_length - 1] != '\n') {
            printf("[FATAL] too many tokens on line %zu\n", i);
            goto BAIL;
        }
        tokens[last][last_length - 1] = '\0';
        char *stype = tokens[0], *reg = tokens[1], *arg = tokens[2];

        // OK, now time to do additional processing needed to set up the instruction:
        if (stype[0] == '\0') {
            // Line has *only* whitespace, skip it.
            goto SKIP_LINE;
        }
        // If a register was specified but `reg` doesn't start with a dollarsign,
        // then it's actually considered as an argument.
        if (reg && reg[0] != '$') {
            arg = reg;
            reg = NULL;
        }
        // Time to verify this instruction makes sense, reject it otherwise.
        size_t instr_n;
        if (!find_string((char **)instruction_type_names, stype, &instr_n)) {
            printf("[FATAL] unknown instruction at line %zu: %s\n", i, stype);
            goto BAIL;
        }
        InstructionType type = (InstructionType)instr_n;
        if (reg != NULL && reg[1] != '1' && reg[1] != '2') {
            printf("[FATAL] unknown register at line %zu: %s\n", i, reg);
            goto BAIL;
        }
        if (type == SWAP || type == GOTO || type == EXIT || type == PRINT) {
            if (reg && type != PRINT) {
                printf("[FATAL] %s at line %zu doesn't need a register\n", stype, i);
                goto BAIL;
            }
        } else if (reg == NULL) {
            printf("[FATAL] %s at line %zu requires a register\n", stype, i);
            goto BAIL;
        }
        if (type == LOAD || type == STORE || type == PRINT) {
            if (arg && arg[0] != '&') {
                printf("[FATAL] a constant is an unsupported argument for %s, line %zu\n",
                    stype, i);
                goto BAIL;
            }
        } else {
            if (arg && arg[0] == '&') {
                printf("[FATAL] a variable is an unsupported argument for %s, line %zu\n",
                    stype, i);
                goto BAIL;
            }
        }
        if ((arg && arg[0] != '&')
                && (atof(arg) == 0.0 && strcmp(arg, "0") && strcmp(arg, "0.0"))) {
            printf("[FATAL] invalid numerical constant on line %zu\n", i);
            goto BAIL;
        }
        // We need to give each unique variable their own RAM index as I'm not
        // implementing a hash table so string keys would work >.<
        size_t var_index = 20220723;
        if (arg && arg[0] == '&') {
            if (!find_string(variables, arg, &var_index)) {
                // This variable hasn't been allocated an index yet, let's change that!
                // By design, if find_string() doesn't find `arg` in the array of
                // strings (`variables`), `var_index` will be set to the next empty
                // index, soooo allocating a new variable is quite simple, haha.
                variables[var_index] = arg;
                if (var_index + 1 >= variables_size) {
                    assert(var_index + 1 == variables_size);
                    char **new_variables = realloc(variables, sizeof(char *) * (variables_size + 50));
                    if (new_variables == NULL) {
                        printf("[FATAL] failed to realloc `variables`\n");
                        goto BAIL;
                    }
                    // NOTE: the extra space realloc provides probably won't be NULLed so
                    // we have to do calloc's job ourselves >.<
                    memset(new_variables + 1, 0, sizeof(char *) * 50);
                    variables = new_variables;
                    variables_size += 50;
                }
            }
        }
        // We can *finally* prepare the final Instruction struct 🎉
        if (debug) {
            if (arg && arg[0] == '&') {
                printf("[LINE %-3zu] %-13s %-7s %s -> ram[%zu]\n",
                    i, stype, reg, arg, var_index);
            } else {
                printf("[LINE %-3zu] %-13s %-7s %s\n", i, stype, reg, arg);
            }
        }
        Instruction instr = { .type = type };
        if (reg == NULL) {
            instr.reg = REG_NONE;
        } else {
            instr.reg = (reg[1] == '1' ? REG_A : REG_B);
        }
        if (arg == NULL) {
            instr.arg = NULL;
        } else {
            instr.arg = malloc(sizeof(double));
            if (arg[0] == '&') {
                *(instr.arg) = (double)var_index;
            } else {
                *(instr.arg) = atof(arg);
            }
        }
        prog->instrs[prog->instr_count] = instr;
        prog->instr_count++;
        if (prog->instr_count >= instrs_size) {
            Instruction *new_instrs = realloc(prog->instrs, sizeof(Instruction) * (instrs_size + 100));
            if (new_instrs == NULL) {
                printf("[FATAL] failed to realloc `prog->instrs`\n");
                goto BAIL;
            }
            prog->instrs = new_instrs;
            instrs_size += 100;
        }
SKIP_LINE:
        free(line_copy);
    }

    free(variables);
    return prog;

BAIL:
    printf("[LINE %zu] %s", i, line_copy);
    free(line_copy);
    free(variables);
    free_program(prog);
    return NULL;
}

double execute(Program program, bool debug)
{
    double reg = 0, reg_b = 0;
    double *target_reg = NULL;
    double swap_temp;
    double ram[RAM_SIZE] = {0};
    for (size_t i = 0; i < program.instr_count; i++) {
        Instruction instr = program.instrs[i];
        if (debug) {
            printf("[DEBUG] executing %s (index %zu) using register %d with argument %f\n",
                instruction_type_names[instr.type], i, instr.reg, instr.arg ? *instr.arg: -1.0);
            printf("[DEBUG]   registerA: %f, registerB: %f\n", reg, reg_b);
        }
        if (instr.reg == REG_NONE) {
            target_reg = NULL;
        } else if (instr.reg == REG_A) {
            target_reg = &reg;
        } else if (instr.reg == REG_B) {
            target_reg = &reg_b;
        }
        double *arg = instr.arg;
        switch (instr.type) {
            case LOAD:
                *target_reg = ram[(size_t)*arg];
                break;
            case STORE:
                ram[(size_t)*arg] = *target_reg;
                break;
            case SET_REG:
                *target_reg = *arg;
                break;
            case SWAP:
                swap_temp = reg;
                reg = reg_b;
                reg_b = swap_temp;
                break;
            case ADD:
                *target_reg = (arg == NULL ? reg + reg_b : *target_reg + *arg);
                break;
            case SUB:
                *target_reg = (arg == NULL ? reg - reg_b : *target_reg - *arg);
                break;
            case MUL:
                *target_reg = (arg == NULL ? reg * reg_b : *target_reg * *arg);
                break;
            case DIV:
                *target_reg = (arg == NULL ? reg / reg_b : *target_reg / *arg);
                break;
            case MOD:
                *target_reg = (arg == NULL ? fmod(reg, reg_b) : fmod(*target_reg, *arg));
                break;
            case EQUAL:
                *target_reg = (arg == NULL ? reg == reg_b : *target_reg == *arg);
                break;
            case NOT:
                *target_reg = (*target_reg == 0.0);
                break;
            case GOTO:
                i = ((size_t)*arg) - 1;
                assert(i <= program.instr_count);
                break;
            case GOTO_IF:
                if (*target_reg != 0.0) {
                    i = ((size_t)*arg) -1;
                    assert(i <= program.instr_count);
                }
                break;
            case GOTO_IF_NOT:
                if (*target_reg == 0.0) {
                    i = ((size_t)*arg) - 1;
                    assert(i <= program.instr_count);
                }
                break;
            case EXIT:
                return reg;
            case PRINT:
                if (arg == NULL) {
                    printf("[OUTPUT] %f\n", *target_reg);
                } else {
                    printf("[OUTPUT] %f\n", ram[(size_t)*arg]);
                }
                break;
            default:
                printf("[FATAL] unimplemented instruction: %s\n",
                    instruction_type_names[instr.type]);
                break;
        }
    }
    return reg;
}

int main(int argc, char *argv[])
{
    (void)argc;

    char const * const desc = "Richard's silly ASM-like language.";
    CLIArg cli_args[] = { { .id = "program" } };
    CLIOpt cli_opts[] = {
        { .id = "result", .name = "show-result", .is_flag = true },
        { .id = "debug-parser", .name = "debug-parser", .is_flag = true },
        { .id = "debug-vm", .name = "debug-vm", .is_flag = true },
    };
    CLI *cli = setup_cli(argv[0], desc, cli_args, NELEMS(cli_args), cli_opts, NELEMS(cli_opts));
    ParseStatus s = parse_cli(cli, argv);
    if (s) {
        free_cli(cli);
        return s == PARSE_HELP ? 0 : 2;
    }
    char const *filepath = cli_get_string(cli, "program");
    bool show_result = cli_get_bool(cli, "result");
    bool debug_parser = cli_get_bool(cli, "debug-parser");
    bool debug_vm = cli_get_bool(cli, "debug-vm");
    free_cli(cli);

    if (filepath == NULL) {
        printf("[FATAL] please pass a .masml file, masml.c: [program.masml]\n");
        return 2;
    }
    char **ppbuf = read_file(filepath);
    if (ppbuf == NULL) {
        return 1;
    }

    // NOTE: `ppbuf` is ruined after parsing, we can and should only free it afterwards.
    Program *prog = parse(ppbuf, debug_parser);
    free_char_ppbuf(ppbuf);
    if (prog == NULL) {
        return 1;
    }

    double result = execute(*prog, debug_vm);
    if (show_result) {
        printf("[RESULT] %f\n", result);
    }

    free_program(prog);
    return 0;
}
