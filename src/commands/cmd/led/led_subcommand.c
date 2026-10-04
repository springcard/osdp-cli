/**
 * @file led_subcommand.c
 * @brief Implementation of the "led" subcommand
 * 
 * The code provided implements the "led" subcommand for the osdp-cli application, which is used for controlling OSDP LED indicators. 
 * The implementation includes the command handler function, which is responsible for parsing the command-line options specific to the led subcommand and executing the command based on those options. 
 * The implementation also includes the registration of the led subcommand in the CLI command registry, allowing it to be recognized and executed when the user enters the "led" subcommand in the CLI.
  */
#include <stddef.h>

#include "led_options.h"
#include "../../../cli/commands_registry.h"
#include "../../../cli/global_options_parser.h"

/** 
 * @brief Handler for the led command
 * 
 * This function handles the "led" subcommand, parsing the command-line options and processing the led subcommand operation.
 * The command handler function first calls the `led_parse` function to parse the command-line options and populate a `led_options` structure. 
 * If the parsing is successful, it then calls the `led_run` function to execute the led subcommand based on the parsed options and the CLI context. 
 * 
 * @param[in] ctx The CLI context
 * @param[in] argc The number of command-line arguments
 * @param[in] argv The array of command-line arguments
 * 
 * @return An integer status code indicating the success or failure of the command execution
 */
int led_handler(struct cli_context *ctx, int argc, char **argv)
{

    int ret = led_parse(argc, argv, &ctx->led_opts);
    if (ret != 0)
        return ret;

    return led_run(ctx);//, &ctx->led_opts);
    //return led_test(ctx);//, &opts);

}

// Define the led subcommand and register it in the CLI command registry. 
// The `cli_command` structure includes the command name, help description, handler function, and information about subcommands (if any).
static struct cli_command led_cmd = {
    .name = "led",
    .help = "Control OSDP LED indicators",
    .handler = led_handler,
    .parent = "cmd",
    .subcommands = NULL,
    .subcommands_count = 0
};

// Register the led command in the CLI command registry, allowing it to be recognized and executed when the user enters the "led" subcommand in the CLI.
REGISTER_COMMAND(led_cmd);