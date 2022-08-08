// Referenced resources:
// - https://stackoverflow.com/questions/53579909/dot-initialization-struct-after-malloc
// - https://stackoverflow.com/questions/2963394/finding-character-in-string-c-language
// - https://stackoverflow.com/questions/1079832/how-can-i-configure-my-makefile-for-debug-and-release-builds

#include "clikit.h"

#include <stdarg.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define MAX_PARAMETERS 100

static long find_string(char const * const strings[], size_t n, char const * const target)
{
    for (size_t i = 0; i < n; i++) {
        if (!strcmp(strings[i], target)) {
            return (long)i;
        }
    }
    return -1;
}

static size_t max_string_length(char const *strings[], size_t n)
{
    size_t highest = 0;
    for (size_t i = 0; i < n; i++) {
        size_t length = strlen(strings[i]);
        if (length > highest) {
            highest = length;
        }
    }
    return highest;
}

static char *create_repeated_string(char const *part, size_t n)
{
    size_t part_length = strlen(part);
    char *s = malloc(sizeof(char) * part_length * n + 1);
    char *s_start = s;
    for (size_t i = 0; i < n; i++) {
        strncpy(s, part, part_length);
        s += part_length;
    }
    *s = '\0';
    return s_start;
}

CLI *setup_cli(char const *name, char const *desc,
                   CLIArg args[], size_t arg_count, CLIOpt opts[], size_t opt_count)
{
    CLI *cli = malloc(sizeof(*cli));
    // NOTE: this compound literal assignment *must* come before all other
    // member assignments because members that aren't specified in the literal will be
    // set to their NUL equivalent value.
    *cli = (CLI){
        .name = name, .desc = desc,
        .args = args, .opts = opts,
        .arg_count = arg_count, .opt_count = opt_count,
        .parameter_count = (arg_count + opt_count)
    };
    cli->ids = malloc(sizeof(char *) * cli->parameter_count);
    cli->opt_names = malloc(sizeof(char *) * opt_count);
    for (size_t i = 0; i < arg_count; i++) {
        cli->ids[i] = args[i].id;
    }
    for (size_t i = 0; i < opt_count; i++) {
        cli->ids[arg_count + i] = opts[i].id;
        cli->opt_names[i] = opts[i].name;
    }
    cli->parsed_argv = calloc(sizeof(char *), MAX_PARAMETERS);
    if (cli->parsed_argv == NULL) {
        return false;
    }
    return cli;
}

void free_cli(CLI *cli)
{
    free(cli->ids);
    free(cli->opt_names);
    free(cli->parsed_argv);
    free(cli);
}

ParseStatus parse_cli(CLI *cli, char *argv[])
{
    // NOTE: `cli->parsed_argv` is implemented as a string array whose values correspond
    // with the parameter ID at the same index. So if --flag is the second CLI parameter,
    // `cli->ids[1]` and `cli->parsed_argv[1]` belong to it.
    size_t arg_count = cli->arg_count, opt_count = cli->opt_count;
    size_t arg_i = 0, opt_i = 0;
    bool arguments_left = true;
    bool arguments_only = false;
    bool awaiting_option_value = false;
    char extra_args[2048] = {0};
    char *extra_args_head = extra_args;
    for (size_t argv_i = 1; argv[argv_i]; argv_i++) {
        // printf("[CLIKIT] argv[%zu]='%s'\n", argv_i, argv[argv_i]);
        char const *value = argv[argv_i];
        if (awaiting_option_value) {
            // Basically everything after an option's flag should be interpreted as its
            // value so this check comes first.
            cli->parsed_argv[arg_count + (size_t)opt_i] = value;
            awaiting_option_value = false;
        } else if (!strcmp(value, "--") && !arguments_only) {
            // The first standalone double dash is usually interpreted as a signal
            // everything after are arguments, let's support that.
            arguments_only = true;
        } else if (value[0] == '-' && value[1] == '-' && value[2] != '\0' && !arguments_only) {
            // Remove the leading double dash so we can look up the option by name.
            char const *option_name = value + 2;
            if (!strcmp(option_name, "help")) {
                print_cli_full_help(cli);
                return PARSE_HELP;
            }
            // Remember that with `cli->opts` and `cli->opt_names` their items correspond
            // one-on-one just like `cli->parsed_argv` and its friends.
            long index = find_string(cli->opt_names, opt_count, option_name);
            if (index == -1) {
                print_cli_parse_error(cli, "unknown option: %s", value);
                return PARSE_UNKNOWN_OPT;
            }
            opt_i = (size_t)index;
            if (cli->opts[opt_i].is_flag) {
                // Boolean options are marked as true (ie. given) by simply replacing the
                // NULL pointer with something that isn't NULL. Could assign "deadbeef"
                // but using `value` seems like the least surprising.
                //
                // NOTE: arguments precede options so we need to offset the option index.
                cli->parsed_argv[arg_count + (size_t)opt_i] = value;
            } else {
                // We'll have to process another argv string to get the option's value.
                awaiting_option_value = true;
            }
        } else if (arguments_left) {
            cli->parsed_argv[arg_i] = value;
            arguments_left = (++arg_i < arg_count);
        } else {
            // We only error out *after* parsing so we can note *all* extra arguments
            // instead of just the first one. It means more work, but it leads to a nicer
            // UX.
            strcpy(extra_args_head, value);
            extra_args_head += strlen(value);
            strcpy(extra_args_head, " ");
            extra_args_head++;
        }
    }
    // for (size_t i = 0; i < cli->parameter_count; i++) {
    //     printf("[CLIKIT] parsed[%zu, %s]='%s'\n", i, cli->ids[i], cli->parsed_argv[i]);
    // }
    if (extra_args[0] != '\0') {
        print_cli_parse_error(cli, "got unexpected extra arguments: %s", extra_args);
        return PARSE_TOO_MANY_ARGS;
    }
    return PARSE_OK;
}

char const *cli_get_string(CLI *cli, char const * const id)
{
    long i = find_string(cli->ids, cli->parameter_count, id);
    if (i == -1) {
        return NULL;
    }
    return cli->parsed_argv[i];
}

bool cli_get_bool(CLI *cli, char const * const id)
{
    long i = find_string(cli->ids, cli->parameter_count, id);
    if (i == -1) {
        return NULL;
    }
    return cli->parsed_argv[i] != NULL;
}

void print_cli_usage(CLI *cli)
{
    if (cli->opt_count) {
        printf("Usage: %s [OPTIONS]", cli->name);
    } else {
        printf("Usage: %s", cli->name);
    }
    for (size_t i = 0; i < cli->arg_count; i++) {
        printf(" $%s", cli->args[i].id);
    }
    printf("\n");
}

void print_cli_full_help(CLI *cli)
{
    print_cli_usage(cli);
    if (cli->desc) {
        printf("\n%s\n", cli->desc);
    }
    printf("\nOptions:\n");
    size_t opt_name_col_width = max_string_length(cli->opt_names, cli->opt_count);
    for (size_t i = 0; i < cli->opt_count; i++) {
        CLIOpt opt = cli->opts[i];
        // NOTE: this is a bit messy, I'll probably write str_rjust / str_ljust helper
        // functions to make this more manageable. Would only work well if I reimplement
        // (v)asprintf though.
        size_t name_margin_len = opt_name_col_width - strlen(opt.name);
        char *name_margin = create_repeated_string(" ", name_margin_len);
        if (opt.is_flag) {
            printf("  --%s\n", opt.name);
        } else {
            printf("  --%s%s  TEXT\n", opt.name, name_margin);
        }
        free(name_margin);
    }
}

void print_cli_parse_error(CLI *cli, char const *format, ...)
{
    print_cli_usage(cli);
    printf("Try '%s --help' for help.\n", cli->name);
    printf("\nError: ");
    va_list ap;
    va_start(ap, format);
    vprintf(format, ap);
    va_end(ap);
    printf("\n");
}
