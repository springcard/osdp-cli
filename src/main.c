/**
 * @file main.c
 * @brief Main entry point for the CLI application
 * 
 * The code provided is the main entry point for the Command Line Interface (CLI) osdp-cli application. It initializes the CLI context, builds the command tree before parsing global command-line options, 
 * and dispatches commands based on user input.
 * First builds command tree, then initializes a `cli_context` structure to store global options by calls the `cli_parse_global_options` function to parse global command-line options and update the context accordingly. 
 * If the parsing is successful, it proceeds to dispatch the command using the `cli_dispatch_command` function, which routes the command execution to the appropriate handler function based on the command 
 * entered by the user.
 * 
 * The main function returns an integer status code indicating the success or failure of the command execution.
 */
#include <stdio.h>
#include <string.h>
#include "commands/commissioning/commissioning.h"

#include "cli/commands_registry.h"
#include "cli/commands_dispatcher.h"
#include "cli/global_options_parser.h"

int main(int argc, char **argv)
{
    for (int i = 1; i < argc; ++i)
        if (strcmp(argv[i], "--unattended") == 0 || strcmp(argv[i], "-u") == 0)
            return commissioning_main(argc - 1, argv + 1);
    struct cli_context ctx = {0};
    int cmd_index = 0;

    cli_build_command_tree(); // Build the command tree before parsing global options, as some global options may depend on the command structure.

    /* Parse global CLI options */
    int ret = cli_parse_global_options(argc-1, &argv[1], &ctx, &cmd_index);// argc-1 and &argv[1] -> ignore the first argument, the application name
    
    if(ret == 1) { // Help requested and already displayed -> exit
        return ret;
    }

    if (ret != 0) {
        fprintf(stderr, "Invalid global options\n");
        return ret;
    }

    /* Dispatch command using command tree */
    ret = cli_dispatch_command(&ctx,
                       cli_get_command_tree(),         // get the root commands
                       cli_get_command_count(),    // get the number of defined root commands
                       argc - cmd_index,            // adjust `argc` to match the command and its arguments in the input line.
                       &argv[cmd_index]);           // pointer to the command and its arguments

    //cli_global_options_test(&ctx);

    return ret;
}
