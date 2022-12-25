#include "util.h"

#include <string.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>

void free_char_ppbuf(char **ppbuf)
{
    for (size_t i = 0; ppbuf[i]; i++) {
        free(ppbuf[i]);
    }
    free(ppbuf);
}

char **read_file(char const *filepath)
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

bool find_string(char * const strings[], char * const target, size_t *index)
{
    for (*index = 0; strings[*index]; (*index)++) {
        if (strcmp(strings[*index], target) == 0) {
            return true;
        }
    }
    return false;
}
