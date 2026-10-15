/**
 * @file relay_subcommand.c
 * @brief Implementation of the "relay" subcommand
 * 
 * The code provided implements the "relay" subcommand for the osdp-cli application, which is used for controlling relay outputs on a OSDP devices.
 * The implementation includes the command handler function, which is responsible for parsing the command-line options specific to the relay subcommand and executing the command based on those options. 
 * The implementation also includes the registration of the relay subcommand in the CLI command registry, allowing it to be recognized and executed when the user enters the "relay" subcommand in the CLI.
  */
#include <stddef.h>

#include "relay_options.h"
#include "../../../cli/commands_registry.h"
#include "../../../cli/global_options_parser.h"

/** 
 * @brief Handler for the relay command
 * 
 * This function handles the "relay" subcommand, parsing the command-line options and processing the relay subcommand operation.
 * The command handler function first calls the `relay_parse` function to parse the command-line options and populate a `relay_options` structure. 
 * If the parsing is successful, it then calls the `relay_run` function to execute the relay subcommand based on the parsed options and the CLI context. 
 * 
 * @param[in] ctx The CLI context
 * @param[in] argc The number of command-line arguments
 * @param[in] argv The array of command-line arguments
 * 
 * @return An integer status code indicating the success or failure of the command execution
 */
int relay_handler(struct cli_context *ctx, int argc, char **argv)
{

    int ret = relay_parse(argc, argv, &ctx->relay_opts);
    if (ret != 0)
        return ret == 1 ? 0 : ret;

    return relay_run(ctx);
    //return relay_test(ctx);

}

// Define the relay subcommand and register it in the CLI command registry. 
// The `cli_command` structure includes the command name, help description, handler function, and information about subcommands (if any).
static struct cli_command relay_cmd = {
    .name = "relay",
    .help = "Control the relay outputs of an OSDP device",
    .handler = relay_handler,
    .parent = "cmd",
    .subcommands = NULL,
    .subcommands_count = 0
};

// Register the relay command in the CLI command registry, allowing it to be recognized and executed when the user enters the "relay" subcommand in the CLI.
REGISTER_COMMAND(relay_cmd);