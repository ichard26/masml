// Referenced resources:
// - https://stackoverflow.com/questions/53579909/dot-initialization-struct-after-malloc

#include "clikit.h"

#include <stdbool.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define MAX_PARAMETERS 100

static long find_string(char const * const strings[], size_t length, char const * const target)
{
    for (size_t i = 0; i < length; i++) {
        if (!strcmp(strings[i], target)) {
            return (long)i;
        }
    }
    return -1;
}

CLI *setup_cli(CLIArg args[], size_t arg_count, CLIOpt opts[], size_t opt_count)
{
    size_t parameter_count = arg_count + opt_count;
    CLI *cli = malloc(sizeof(*cli));
    // NOTE: this compound literal assignment *must* come before all other
    // member assignments because members that aren't specified in the literal will be
    // set to their NUL equivalent value.
    *cli = (CLI){
        .args = args, .opts = opts,
        .arg_count = arg_count, .opt_count = opt_count,
        .parameter_count = parameter_count
    };
    cli->ids = malloc(sizeof(char *) * parameter_count);
    cli->opt_names = malloc(sizeof(char *) * opt_count);
    for (size_t i = 0; i < arg_count; i++) {
        cli->ids[i] = args[i].id;
    }
    for (size_t i = 0; i < opt_count; i++) {
        cli->ids[arg_count + i] = opts[i].id;
        cli->opt_names[i] = opts[i].name;
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

bool parse_cli(CLI *cli, char *argv[])
{
    cli->parsed_argv = calloc(sizeof(char *), MAX_PARAMETERS);
    if (cli->parsed_argv == NULL) {
        return false;
    }

    size_t arg_count = cli->arg_count, opt_count = cli->opt_count;
    size_t arg_i = 0, opt_i = 0;
    bool arguments_left = true;
    bool arguments_only = false;
    bool awaiting_option_value = false;
    for (size_t argv_i = 1; argv[argv_i]; argv_i++) {
        // printf("[CLIKIT] argv[%zu]='%s'\n", argv_i, argv[argv_i]);
        char const *value = argv[argv_i];
        if (awaiting_option_value) {
            // Basically everything after an option's flag should be interpreted as its
            // value so this check comes first.
            cli->parsed_argv[arg_count + (size_t)opt_i] = value;
            awaiting_option_value = false;
        } else if (!strcmp(value, "--") && !arguments_only && !awaiting_option_value) {
            // The first standalone double dash is usually interpreted as a signal
            // everything after are arguments, let's support that.
            arguments_only = true;
        } else if (value[0] == '-' && value[1] == '-' && value[2] != '\0' && !arguments_only) {
            // Remove the leading double dash so we can look up the option by name.
            char const *option_name = value + 2;
            // `cli->parsed_argv` is implemented as a string array whose values correspond
            // with the parameter ID at the same index. So if --flag is the second
            // CLI parameter, `cli->ids[1]` and `cli->parsed_argv[1]` belong to it.
            long index = find_string(cli->opt_names, opt_count, option_name);
            if (index == -1) {
                printf("[CLIKIT:FATAL] unknown option: %s\n", value);
                return false;
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
            printf("[CLIKIT:WARNING] unused argument: %s\n", value);
        }
    }
    // for (size_t i = 0; i < cli->parameter_count; i++) {
    //     printf("[CLIKIT] parsed[%zu, %s]='%s'\n", i, cli->ids[i], cli->parsed_argv[i]);
    // }
    return true;
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
