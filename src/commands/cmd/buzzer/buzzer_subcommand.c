/**
 * @file buzzer_subcommand.c
 * @brief Implementation of the "buzzer" subcommand
 * 
 * The code provided implements the "buzzer" subcommand for the osdp-cli application, which is used for controlling the OSDP device's buzzer indicator. 
 * The implementation includes the command handler function, which is responsible for parsing the command-line options specific to the buzzer subcommand
 * and executing the command based on those options. 
 * The implementation also includes the registration of the buzzer subcommand in the CLI command registry, allowing it to be recognized and executed when 
 * the user enters the "buzzer" subcommand in the CLI.
  */
#include <stddef.h>

#include "buzzer_options.h"
#include "../../../cli/commands_registry.h"
#include "../../../cli/global_options_parser.h"

/** 
 * @brief Handler for the buzzer command
 * 
 * This function handles the "buzzer" subcommand, parsing the command-line options and processing the buzzer subcommand operation.
 * The command handler function first calls the `buzzer_parse` function to parse the command-line options and populate a `buzzer_options` structure. 
 * If the parsing is successful, it then calls the `buzzer_run` function to execute the buzzer subcommand based on the parsed options and the CLI context. 
 * 
 * @param[in] ctx The CLI context
 * @param[in] argc The number of command-line arguments
 * @param[in] argv The array of command-line arguments
 * 
 * @return An integer status code indicating the success or failure of the command execution
 */
int buzzer_handler(struct cli_context *ctx, int argc, char **argv)
{

    int ret = buzzer_parse(argc, argv, &ctx->buzzer_opts);
    if (ret != 0)
        return ret;

    return buzzer_run(ctx);
    //return buzzer_test(ctx);

}


// Define the buzzer subcommand and register it in the CLI command registry. 
// The `cli_command` structure includes the command name, help description, handler function, and information about subcommands (if any).
static struct cli_command buzzer_cmd = {
    .name = "buzzer",
    .help = "Control OSDP buzzer indicator",
    .handler = buzzer_handler,
    .parent = "cmd",
    .subcommands = NULL,
    .subcommands_count = 0
};

// Register the buzzer command in the CLI command registry, allowing it to be recognized and executed when the user enters the "buzzer" subcommand in the CLI.
REGISTER_COMMAND(buzzer_cmd);