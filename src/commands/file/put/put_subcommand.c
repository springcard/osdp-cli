/**
 * @file put_subcommand.c
 * @brief Implementation of the "put" subcommand
 * 
 * The code provided implements the "put" subcommand for the osdp-cli application, which is used for pushing a file to OSDP device.  
 * The implementation includes the command handler function, which is responsible for parsing the command-line options specific to the put subcommand and executing the command based on those options. 
 * The implementation also includes the registration of the put subcommand in the CLI command registry, allowing it to be recognized and executed when the user enters the "put" subcommand in the CLI.
  */
#include <stddef.h>

#include "put_options.h"
#include "../../../cli/commands_registry.h"
#include "../../../cli/global_options_parser.h"

/** 
 * @brief Handler for the put command
 * 
 * This function handles the "put" subcommand, parsing the command-line options and processing the put subcommand operation.
 * The command handler function first calls the `put_parse` function to parse the command-line options and populate a `put_options` structure. 
 * If the parsing is successful, it then calls the `put_run` function to execute the put subcommand based on the parsed options and the CLI context. 
 * 
 * @param[in] ctx The CLI context
 * @param[in] argc The number of command-line arguments
 * @param[in] argv The array of command-line arguments
 * 
 * @return An integer status code indicating the success or failure of the command execution
 */
int put_handler(struct cli_context *ctx, int argc, char **argv)
{

    int ret = put_parse(argc, argv, &ctx->put_opts);
    if (ret != 0)
        return ret == 1 ? 0 : 1;

    return put_run(ctx);
    //return put_test(ctx);

}

// Define the put subcommand and register it in the CLI command registry. 
// The `cli_command` structure includes the command name, help description, handler function, and information about subcommands (if any).
static struct cli_command put_cmd = {
    .name = "put",
    .help = "Push a file to OSDP device",
    .handler = put_handler,
    .parent = "file",
    .subcommands = NULL,
    .subcommands_count = 0
};

// Register the put command in the CLI command registry, allowing it to be recognized and executed when the user enters the "put" subcommand in the CLI.
REGISTER_COMMAND(put_cmd);
