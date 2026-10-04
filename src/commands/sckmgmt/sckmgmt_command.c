/**
 * @file sckmgmt_command.c
 * @brief Implementation of the sckmgmt command for managing secure channel keys
 * 
 * The code provided implements the "sckmgmt" command for the osdp-cli application, which is used for managing secure channel keys for OSDP devices communication. 
 * The implementation includes the command handler function, which is responsible for parsing the command-line options specific to the sckmgmt command and executing the command based on those options. 
 * The implementation also includes the registration of the sckmgmt command in the CLI command registry, allowing it to be recognized and executed when the user enters the "sckmgmt" command in the CLI.
  */
#include <stddef.h>

#include "sckmgmt_options.h"
#include "../../cli/commands_registry.h"
#include "../../cli/global_options_parser.h"

/** 
 * @brief Handler for the sckmgmt command
 * 
 * This function handles the "sckmgmt" command, parsing the command-line options and processing the sckmgmt command operation.
 * The command handler function first calls the `sckmgmt_parse` function to parse the command-line options and populate a `sckmgmt_options` structure. 
 * If the parsing is successful, it then calls the `sckmgmt_run` function to execute the sckmgmt command based on the parsed options and the CLI context. 
 * 
 * @param[in] ctx The CLI context
 * @param[in] argc The number of command-line arguments
 * @param[in] argv The array of command-line arguments
 * 
 * @return An integer status code indicating the success or failure of the command execution
 */
int sckmgmt_handler(struct cli_context *ctx, int argc, char **argv)
{

    int ret = sckmgmt_parse(argc, argv, &ctx->sckmgmt_opts);
    if (ret != 0) {
        sckmgmt_clear_options(&ctx->sckmgmt_opts);
        return ret;
    }

    return sckmgmt_run(ctx);

}

// Define the sckmgmt command and register it in the CLI command registry. 
// The `cli_command` structure includes the command name, help description, handler function, and information about subcommands (if any).
static struct cli_command sckmgmt_cmd = {
    .name = "sckmgmt",
    .help = "Manage OSDP secure channel keys (change SCBK)",
    .handler = sckmgmt_handler,
    .parent = NULL,
    .subcommands = NULL,
    .subcommands_count = 0
};

// Register the sckmgmt command in the CLI command registry, allowing it to be recognized and executed when the user enters the "sckmgmt" command in the CLI.
REGISTER_COMMAND(sckmgmt_cmd);
