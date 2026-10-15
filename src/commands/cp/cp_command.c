/**
 * @file cp_command.c
 * @brief Implementation of the cp command for running as a basic control panel.
 * 
 * The code provided implements the "cp" command for the osdp-cli application, which is used for running a minimal Control Panel that looks for PD and queries them, 
 * useful for enumerating the PD on the bus and validating the PD behaviors.
 * The implementation includes the command handler function, which is responsible for parsing the command-line options specific to the cp command and executing the command based on those options. 
 * The implementation also includes the registration of the cp command in the CLI command registry, allowing it to be recognized and executed when the user enters the "cp" command in the CLI.
  */
#include <stddef.h>

#include "cp_options.h"
#include "../../cli/commands_registry.h"
#include "../../cli/global_options_parser.h"

/** 
 * @brief Handler for the cp command
 * 
 * This function handles the "cp" command, parsing the command-line options and processing the cp command operation.
 * The command handler function first calls the `cp_parse` function to parse the command-line options and populate a `cp_options` structure. If the parsing is successful, it then calls the `cp_run` function 
 * to execute the cp command based on the parsed options and the CLI context. 
 * 
 * @param[in] ctx The CLI context
 * @param[in] argc The number of command-line arguments
 * @param[in] argv The array of command-line arguments
 * 
 * @return An integer status code indicating the success or failure of the command execution
 */
int cp_handler(struct cli_context *ctx, int argc, char **argv)
{

    int ret = cp_parse(argc, argv, &ctx->cp_opts);
    if (ret != 0)
        return ret == 1 ? 0 : ret;

    return cp_run(ctx);
    //return cp_test(ctx);

}

// Define the cp command and register it in the CLI command registry. 
// The `cli_command` structure includes the command name, help description, handler function, and information about subcommands (if any).
static struct cli_command cp_cmd = {
    .name = "cp",
    .help = "Run as a basic control panel",
    .handler = cp_handler,
    .parent = NULL,
    .subcommands = NULL,
    .subcommands_count = 0
};

// Register the cp command in the CLI command registry, allowing it to be recognized and executed when the user enters the "cp" command in the CLI.
REGISTER_COMMAND(cp_cmd);