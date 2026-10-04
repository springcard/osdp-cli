/**
 * @file raw_options_parser.c
 * @brief Implementation for parsing raw subcommand options
 * 
 * The code provided implements the parsing of command-line options for the "raw" subcommand in the osdp-cli application.
 * It defines a function `raw_parse` that takes command-line arguments and a pointer to a `raw_options` structure, which is used to store the parsed options for the raw subcommand. 
 * The function uses a reference array of `cli_option` structures to define all possible options for the raw subcommand, includes their names, types, value pointers, and help descriptions.
 * The `parse_command_line options` function is called to perform the actual parsing of the command-line arguments based on the referenced options.
 * The function handles help requests, parsing errors, and updates the `raw_options` structure with the parsed values for use in executing the raw subcommand.
 * 
 * The `raw_options` structure includes fields for the serial port, baud rate, security keys and all parameters specific to raw command management such as command and data.
 */
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "raw_options.h"
#include "../../../cli/parser_engine.h"


/**
 * @brief Parse command line options for the raw subcommand
 * 
 * The function is called to perform the actual parsing of the command-line arguments based on the predefined options. It takes the command-line arguments and a pointer to a `raw_options` structure, 
 * which is used to store the parsed options for the raw subcommand. 
 * The function checks for the presence of each option in the command-line arguments and updates the corresponding fields in the `raw_options` structure accordingly.
 * The function handles help requests.
 * 
 * @param[in] argc `argc` is the argument count, which represents the number of arguments passed to the raw subcommand from the command line, including the name of the subcommand itself.
 * @param[in] argv represents the command-line arguments passed to the raw subcommand.
 * @param[out] opts a pointer to an `raw_parse` structure, which will be populated with the parsed options.
 * 
 * @return 0 on success, 1 if help was requested, or a negative value on error.
 */
int raw_parse(int argc, char **argv, struct raw_options *opts)
{

    memset(opts, 0, sizeof(*opts));
    opts->addr = 0;          // Initialize to the first available address.
    opts->baud = "9600";       // Initialize to normal baud rate to use if not specified by the user.

    opts->cmd = NULL;   // Initialize to an invalid command to check later if it was set
    opts->data = NULL;  // Initialize to an invalid data to check later if it was set


    // Define the available command-line options for the raw subcommand.
    // Each `struct cli_option` defines a specific command-line option.
    // The `cli_option` structure includes fields for the option name, type, value pointer, size (for hex options), presence indicator, enumeration values (for enum options), and help text needed for parsing and handling that option.
    // It's the reference options, like a grammar for the command line arguments of the raw subcommand, that the parsing function will use to understand and process the user's input correctly.
    struct cli_option options[] = {

        { "--port",     CLI_OPT_STRING, &opts->port,                  0,            NULL,             NULL, "Serial port to use (e.g. COM3 or /dev/ttyUSB0)" },
        { "--baud",     CLI_OPT_STRING, &opts->baud,                  0,            NULL,             NULL, "Baud rate to use" },
        { "--addr",     CLI_OPT_INT,    &opts->addr,                  0,            NULL,             NULL, "Device address for the OSDP device to query" },        
        { "--scbk-d",   CLI_OPT_FLAG,   &opts->scbk_d,                0,            NULL,             NULL, "Use default SCBK key for secure channel establishment" },        
        { "--scbk",     CLI_OPT_HEX,    &opts->scbk, sizeof(opts->scbk), &opts->scbk_set,             NULL, "Use this Base Key for Secure Channel if possible" },
        { "--cmd",      CLI_OPT_HEX,    &opts->cmd,                   0,  &opts->cmd_len,             NULL, "Command to send" },
        { "--data",     CLI_OPT_HEX,    &opts->data,                  0, &opts->data_len,             NULL, "Data to send" }
    };
    
    // raw command line options parsing
    int ret = parse_command_line_options(argc, argv, options, sizeof(options) / sizeof(options[0])) ;

    if (ret == -1) { // Help requested, handle help option case.
        printf("\nUsage: cmd raw [options]\n");
        print_command_line_options(options, sizeof(options) / sizeof(options[0]));
        return 1;
    }
    else if (ret < 0) { // Parsing format error, handle error case.
        return ret;
    }
    
    return 0;
}    
