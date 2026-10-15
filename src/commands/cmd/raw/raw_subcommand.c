/**
 * @file raw_subcommand.c
 * @brief Implementation of the "raw" subcommand
 * 
 * The code provided implements the "raw" subcommand for the osdp-cli application, which is used for sending a raw OSDP command payload. 
 * The implementation includes the command handler function, which is responsible for parsing the command-line options specific to the raw subcommand and executing the 
 * command based on those options. 
 * The implementation also includes the registration of the raw subcommand in the CLI command registry, allowing it to be recognized and executed when the user enters 
 * the "raw" subcommand in the CLI.
  */
#include <stddef.h>

#include "raw_options.h"
#include "../../../cli/commands_registry.h"
#include "../../../cli/global_options_parser.h"

/** 
 * @brief Handler for the raw command
 * 
 * This function handles the "raw" subcommand, parsing the command-line options and processing the raw subcommand operation.
 * The command handler function first calls the `raw_parse` function to parse the command-line options and populate a `raw_options` structure. 
 * If the parsing is successful, it then calls the `raw_run` function to execute the raw subcommand based on the parsed options and the CLI context. 
 * 
 * @param[in] ctx The CLI context
 * @param[in] argc The number of command-line arguments
 * @param[in] argv The array of command-line arguments
 * 
 * @return An integer status code indicating the success or failure of the command execution
 */
int raw_handler(struct cli_context *ctx, int argc, char **argv)
{

    int ret = raw_parse(argc, argv, &ctx->raw_opts);
    if (ret != 0)
        return ret == 1 ? 0 : ret;

    return raw_run(ctx);
    //return raw_test(ctx);

}

// Define the raw subcommand and register it in the CLI command registry. 
// The `cli_command` structure includes the command name, help description, handler function, and information about subcommands (if any).
static struct cli_command raw_cmd = {
    .name = "raw",
    .help = "Send a raw OSDP command payload (advanced / debugging)",
    .handler = raw_handler,
    .parent = "cmd",
    .subcommands = NULL,
    .subcommands_count = 0
};

// Register the raw command in the CLI command registry, allowing it to be recognized and executed when the user enters the "raw" subcommand in the CLI.
REGISTER_COMMAND(raw_cmd);