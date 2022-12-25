#ifndef ICHARD26_MASML_UTIL_H
#define ICHARD26_MASML_UTIL_H

#include <stdbool.h>
#include <stddef.h>

char **read_file(char const *filepath);
bool find_string(char * const strings[], char * const target, size_t *index);
void free_char_ppbuf(char **ppbuf);

#endif
