/**
 * @file info_command.c
 * @brief Implementation of the info command for displaying device informations
 * 
 * The code provided implements the "info" command for the osdp-cli application, which is used for displaying information about OSDP devices. The implementation includes the command handler function, 
 * which is responsible for parsing the command-line options specific to the info command and executing the command based on those options. 
 * The implementation also includes the registration of the info command in the CLI command registry, allowing it to be recognized and executed when the user enters the "info" command in the CLI.
  */
#include <stddef.h>

#include "info_options.h"
#include "../../cli/commands_registry.h"
#include "../../cli/global_options_parser.h"

/** 
 * @brief Handler for the info command
 * 
 * This function handles the "info" command, parsing the command-line options and processing the info command operation.
 * The command handler function first calls the `info_parse` function to parse the command-line options and populate a `info_options` structure. If the parsing is successful, it then calls the `info_run` function 
 * to execute the info command based on the parsed options and the CLI context. 
 * 
 * @param[in] ctx The CLI context
 * @param[in] argc The number of command-line arguments
 * @param[in] argv The array of command-line arguments
 * 
 * @return An integer status code indicating the success or failure of the command execution
 */
int info_handler(struct cli_context *ctx, int argc, char **argv)
{

    int ret = info_parse(argc, argv, &ctx->info_opts);
    if (ret != 0)
        return ret;

    return info_run(ctx);
    //return info_test(ctx);

}

// Define the info command and register it in the CLI command registry. 
// The `cli_command` structure includes the command name, help description, handler function, and information about subcommands (if any).
static struct cli_command info_cmd = {
    .name = "info",
    .help = "Display OSDP device information",
    .handler = info_handler,
    .parent = NULL,
    .subcommands = NULL,
    .subcommands_count = 0
};

// Register the info command in the CLI command registry, allowing it to be recognized and executed when the user enters the "info" command in the CLI.
REGISTER_COMMAND(info_cmd);