/**
 * @file text_options_parser.c
 * @brief Implementation for parsing text subcommand options
 * 
 * The code provided implements the parsing of command-line options for the "text" subcommand in the osdp-cli application.
 * It defines a function `text_parse` that takes command-line arguments and a pointer to a `text_options` structure, which is used to store the parsed options for the text subcommand. 
 * The function uses a reference array of `cli_option` structures to define all possible options for the text subcommand, includes their names, types, value pointers, and help descriptions.
 * The `parse_command_line options` function is called to perform the actual parsing of the command-line arguments based on the referenced options.
 * The function handles help requests, parsing errors, and updates the `text_options` structure with the parsed values for use in executing the text subcommand.
 * 
 * The `text_options` structure includes fields for the serial port, baud rate, security keys and all parameters specific to text management such as text content and display settings.
 */
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "text_options.h"
#include "../../../cli/parser_engine.h"


/**
 * @brief Parse command line options for the text subcommand
 * 
 * The function is called to perform the actual parsing of the command-line arguments based on the predefined options. It takes the command-line arguments and a pointer to a `text_options` structure, 
 * which is used to store the parsed options for the text subcommand. 
 * The function checks for the presence of each option in the command-line arguments and updates the corresponding fields in the `text_options` structure accordingly.
 * The function handles help requests.
 * 
 * @param[in] argc `argc` is the argument count, which represents the number of arguments passed to the text subcommand from the command line, including the name of the subcommand itself.
 * @param[in] argv represents the command-line arguments passed to the text subcommand.
 * @param[out] opts a pointer to an `text_parse` structure, which will be populated with the parsed options.
 * 
 * @return 0 on success, 1 if help was requested, or a negative value on error.
 */
int text_parse(int argc, char **argv, struct text_options *opts)
{

    memset(opts, 0, sizeof(*opts));
    
    opts->addr = 0;          // Initialize to the first available address.
    opts->baud = "9600";       // Initialize to normal baud rate to use if not specified by the user.    
    opts->row = -1;    // Initialize to an invalid row number to check later if it was set.
    opts->column = -1; // Initialize to an invalid column number to check later if it was set.
    opts->msg = NULL;  // Initialize to NULL to check later if it was set.

    // Define the available command-line options for the text subcommand.
    // Each `struct cli_option` defines a specific command-line option.
    // The `cli_option` structure includes fields for the option name, type, value pointer, size (for hex options), presence indicator, enumeration values (for enum options), and help text needed for parsing and handling that option.
    // It's the reference options, like a grammar for the command line arguments of the text subcommand, that the parsing function will use to understand and process the user's input correctly.
    struct cli_option options[] = {

        { "--port",   CLI_OPT_STRING, &opts->port,   0                 , NULL           , NULL, "Serial port to use (e.g. COM3 or /dev/ttyUSB0)" },
        { "--baud",   CLI_OPT_STRING, &opts->baud,   0                 , NULL           , NULL, "Baud rate to use" },
        { "--addr",   CLI_OPT_INT,    &opts->addr,   0                 , NULL           , NULL, "Device address for the OSDP device to query" },        
        { "--scbk-d", CLI_OPT_FLAG,   &opts->scbk_d, 0                 , NULL           , NULL, "Use default SCBK key for secure channel establishment" },        
        { "--scbk",   CLI_OPT_HEX,    &opts->scbk,   sizeof(opts->scbk), &opts->scbk_set, NULL, "Use this Base Key for Secure Channel if possible" },
        { "--row",    CLI_OPT_INT,    &opts->row,    0                 , NULL           , NULL, "Row number for the text display" },
        { "--col",    CLI_OPT_INT,    &opts->column, 0                 , NULL           , NULL, "Column number for the text display" },
        { "--clear",  CLI_OPT_FLAG,   &opts->clear,  0                 , NULL           , NULL, "Clear the text display" },
        { "--msg",    CLI_OPT_STRING, &opts->msg,    0                 , NULL           , NULL, "Text to display on the PD's screen" },
    };
    
    // text command line options parsing
    int ret = parse_command_line_options(argc, argv, options, sizeof(options) / sizeof(options[0])) ;

    if (ret == -1) { // Help requested, handle help option case.
        printf("\nUsage: cmd text [options]\n");
        print_command_line_options(options, sizeof(options) / sizeof(options[0]));
        return 1;
    }
    else if (ret < 0) { // Parsing format error, handle error case.
        return ret;
    }
    
    return 0;
}    
