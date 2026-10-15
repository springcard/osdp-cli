/**
 * @file config_command.c
 * @brief Implementation of the "config" command allowing change PD address and serial baud rate
 * 
 * The code provided implements the "config" command for the osdp-cli application, which is used for changing addresses and serial baud rates of OSDP devices. 
 * The implementation includes the command handler function, which is responsible for parsing the command-line options specific to the config command and executing the command based on those options. 
 * The implementation also includes the registration of the config command in the CLI command registry, allowing it to be recognized and executed when the user enters the "config" command in the CLI.
  */
#include <stddef.h>

#include "config_options.h"
#include "../../cli/commands_registry.h"
#include "../../cli/global_options_parser.h"

/** 
 * @brief Handler for the config command
 * 
 * This function handles the "config" command, parsing the command-line options and processing the config command operation.
 * The command handler function first calls the `config_parse` function to parse the command-line options and populate a `config_options` structure. 
 * If the parsing is successful, it then calls the `config_run` function to execute the config command based on the parsed options and the CLI context. 
 * 
 * @param[in] ctx The CLI context
 * @param[in] argc The number of command-line arguments
 * @param[in] argv The array of command-line arguments
 * 
 * @return An integer status code indicating the success or failure of the command execution
 */
int config_handler(struct cli_context *ctx, int argc, char **argv)
{

    int ret = config_parse(argc, argv, &ctx->config_opts);
    if (ret != 0)
        return ret == 1 ? 0 : ret;

    return config_run(ctx);
    //return config_test(ctx);

}

// Define the config command and register it in the CLI command registry. 
// The `cli_command` structure includes the command name, help description, handler function, and information about subcommands (if any).
static struct cli_command config_cmd = {
    .name = "config",
    .help = "Display OSDP device configrmation",
    .handler = config_handler,
    .parent = NULL,
    .subcommands = NULL,
    .subcommands_count = 0
};

// Register the config command in the CLI command registry, allowing it to be recognized and executed when the user enters the "config" command in the CLI.
REGISTER_COMMAND(config_cmd);