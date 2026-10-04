/**
 * @file sniff_command.c
 * @brief Implementation of the sniff command for OSDP traffic sniffing
 * 
 * The code provided implements the "sniff" command for the osdp-cli application, which is used for sniffing OSDP traffic. The implementation includes the command handler function, 
 * which is responsible for parsing the command-line options specific to the sniff command and executing the command based on those options. 
 * The implementation also includes the registration of the sniff command in the CLI command registry, allowing it to be recognized and executed when the user enters the "sniff" command in the CLI.
  */
#include <stddef.h>

#include "sniff_options.h"
#include "../../cli/commands_registry.h"
#include "../../cli/global_options_parser.h"

/** @brief Handler for the sniff command
 * 
 * This function handles the "sniff" command, parsing the command-line options and executing the sniffing operation.
 * The command handler function first calls the `sniff_parse` function to parse the command-line options and populate a `sniff_options` structure. If the parsing is successful, it then calls the `sniff_run` function 
 * to execute the sniff command based on the parsed options and the CLI context. The `sniff_run` function checks for the required options such as the serial port and baud rate, and it prints out the configuration for 
 * the sniffing operation.
 * 
 * @param[in] ctx The CLI context
 * @param[in] argc The number of command-line arguments
 * @param[in] argv The array of command-line arguments
 * 
 * @return An integer status code indicating the success or failure of the command execution
 */
int sniff_handler(struct cli_context *ctx, int argc, char **argv)
{

    int ret = sniff_parse(argc, argv, &ctx->sniff_opts);
    if (ret != 0)
        return ret;

    return sniff_run(ctx);
    //return sniff_test(ctx);

}

// Define the sniff command and register it in the CLI command registry. 
// The `cli_command` structure includes the command name, help description, handler function, and information about subcommands (if any).
static struct cli_command sniff_cmd = {
    .name = "sniff",
    .help = "Sniff OSDP traffic",
    .handler = sniff_handler,
    .parent = NULL,
    .subcommands = NULL,
    .subcommands_count = 0
};

// Register the sniff command in the CLI command registry, allowing it to be recognized and executed when the user enters the "sniff" command in the CLI.
REGISTER_COMMAND(sniff_cmd);