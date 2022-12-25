#include "masml.h"
#include "util.h"
#include "clikit.h"

#include <stdbool.h>
#include <stdio.h>

int main(int argc, char *argv[])
{
    (void)argc;

    CLIArg cli_args[] = { { .id = "program" } };
    CLIOpt cli_opts[] = {
        { .id = "result", .name = "show-result", .is_flag = true },
        { .id = "debug-parser", .is_flag = true },
        { .id = "debug-vm", .is_flag = true },
    };
    CLI *cli = SETUP_CLI(argv, "Richard's silly ASM-like language.", cli_args, cli_opts);
    PARSE_CLI_AND_MAYBE_RETURN(cli, argv);
    char const *filepath = cli_get_string(cli, "program");
    bool show_result = cli_get_bool(cli, "result");
    bool debug_parser = cli_get_bool(cli, "debug-parser");
    bool debug_vm = cli_get_bool(cli, "debug-vm");
    free_cli(cli);

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
