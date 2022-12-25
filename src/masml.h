// Referenced resources:
// - https://stackoverflow.com/questions/42056160/static-functions-declared-in-c-header-files
// - https://softwareengineering.stackexchange.com/questions/285811/c-module-where-to-put-prototypes-and-definitions-that-do-not-belong-to-the-pub

#ifndef ICHARD26_MASML_MASML_H
#define ICHARD26_MASML_MASML_H

#include <stdbool.h>
#include <stddef.h>

typedef enum {
    LOAD, STORE,
    SET_REG, SWAP,
    ADD, SUB, MUL, DIV, MOD,
    EQUAL, NOT,
    GOTO, GOTO_IF, GOTO_IF_NOT, EXIT,
    PRINT
} InstructionType;

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

void free_program(Program *program);
Program *parse(char *ppbuf[], bool debug);
double execute(Program program, bool debug);

#endif
