// Referenced resources:
// - https://www.learncpp.com/cpp-tutorial/header-files/
// - https://www.learncpp.com/cpp-tutorial/header-guards/
// - https://www.learncpp.com/cpp-tutorial/how-to-design-your-first-programs/
// - https://en.cppreference.com/w/c/language/storage_duration
// - https://c-faq.com/decl/spiral.anderson.html

#ifndef ICHARD26_CLIKIT_H
#define ICHARD26_CLIKIT_H

#include <stdbool.h>
#include <stddef.h>

#define SETUP_CLI(argv, desc, args, opts)       \
    setup_cli(argv[0], desc,                    \
        args, (sizeof(args) / sizeof(args[0])), \
        opts, (sizeof(opts) / sizeof(opts[0])))
#define PARSE_CLI_AND_MAYBE_RETURN(cli, argv) \
    ParseStatus s = parse_cli(cli, argv);     \
    if (s) {                                  \
        free_cli(cli);                        \
        return s == PARSE_HELP ? 0 : 2;       \
    }

typedef struct {
    char const * const id;
    bool optional;
} CLIArg;

typedef struct {
    char const * const id;
    char const *name;
    bool is_flag;
} CLIOpt;

typedef struct {
    char const *name;
    char const *desc;
    CLIArg *args;
    CLIOpt *opts;
    char const **opt_names;
    char const **ids;
    size_t arg_count;
    size_t opt_count;
    size_t parameter_count;
    char const **parsed_argv;
} CLI;

typedef enum {
    PARSE_OK = 0, PARSE_HELP,
    PARSE_INVALID_PARAMETER, PARSE_MISSING_PARAMETER,
    PARSE_UNKNOWN_OPT, PARSE_TOO_MANY_ARGS,
} ParseStatus;

CLI *setup_cli(char const *name, char const *desc,
                   CLIArg args[], size_t arg_count, CLIOpt opts[], size_t opt_count);
ParseStatus parse_cli(CLI *cli, char *argv[]);
void free_cli(CLI *cli);

char const *cli_get_string(CLI *cli, char const * const id);
bool cli_get_bool(CLI *cli, char const * const id);

void print_cli_usage(CLI *cli);
void print_cli_full_help(CLI *cli);
void print_cli_parse_error(CLI *cli, char const *format, ...);

#endif
