/**
 * @file text_subcommand.c
 * @brief Implementation of the "text" subcommand
 * 
 * The code provided implements the "text" subcommand for the osdp-cli application, which is used for displaying text on OSDP device's screen. 
 * The implementation includes the command handler function, which is responsible for parsing the command-line options specific to the text subcommand and executing the 
 * command based on those options. 
 * The implementation also includes the registration of the text subcommand in the CLI command registry, allowing it to be recognized and executed when the user enters 
 * the "text" subcommand in the CLI.
  */
#include <stddef.h>

#include "text_options.h"
#include "../../../cli/commands_registry.h"
#include "../../../cli/global_options_parser.h"

/** 
 * @brief Handler for the text command
 * 
 * This function handles the "text" subcommand, parsing the command-line options and processing the text subcommand operation.
 * The command handler function first calls the `text_parse` function to parse the command-line options and populate a `text_options` structure. 
 * If the parsing is successful, it then calls the `text_run` function to execute the text subcommand based on the parsed options and the CLI context. 
 * 
 * @param[in] ctx The CLI context
 * @param[in] argc The number of command-line arguments
 * @param[in] argv The array of command-line arguments
 * 
 * @return An integer status code indicating the success or failure of the command execution
 */
int text_handler(struct cli_context *ctx, int argc, char **argv)
{

    int ret = text_parse(argc, argv, &ctx->text_opts);
    if (ret != 0)
        return ret;

    return text_run(ctx);
    //return text_test(ctx);

}

// Define the text subcommand and register it in the CLI command registry. 
// The `cli_command` structure includes the command name, help description, handler function, and information about subcommands (if any).
static struct cli_command text_cmd = {
    .name = "text",
    .help = "Send text to displaying on OSDP device's screen",
    .handler = text_handler,
    .parent = "cmd",
    .subcommands = NULL,
    .subcommands_count = 0
};

// Register the text command in the CLI command registry, allowing it to be recognized and executed when the user enters the "text" subcommand in the CLI.
REGISTER_COMMAND(text_cmd);