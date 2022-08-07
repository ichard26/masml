// Referenced resources:
// - https://www.learncpp.com/cpp-tutorial/header-files/
// - https://www.learncpp.com/cpp-tutorial/header-guards/
// - https://www.learncpp.com/cpp-tutorial/how-to-design-your-first-programs/
// - https://en.cppreference.com/w/c/language/storage_duration
// - https://c-faq.com/decl/spiral.anderson.html

#ifndef CLIKIT_H
#define CLIKIT_H

#include <stdbool.h>
#include <stddef.h>

typedef struct {
    char const * const id;
} CLIArg;

typedef struct {
    char const * const id;
    char const * const name;
    bool is_flag;
} CLIOpt;

typedef struct {
    CLIArg *args;
    CLIOpt *opts;
    char const **opt_names;
    char const **ids;
    size_t arg_count;
    size_t opt_count;
    size_t parameter_count;
    char const **parsed_argv;
} CLI;

CLI *setup_cli(CLIArg args[], size_t arg_count, CLIOpt opts[], size_t opt_count);
bool parse_cli(CLI *cli, char *argv[]);
void free_cli(CLI *cli);

char const *cli_get_string(CLI *cli, char const * const id);
bool cli_get_bool(CLI *cli, char const * const id);

#endif
