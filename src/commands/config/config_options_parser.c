/**
 * @file config_options_parser.c
 * @brief Implementation for parsing config command options
 * 
 * The code provided implements the parsing of command-line options for the "config" command in the osdp-cli application.
 * It defines a function `config_parse` that takes command-line arguments and a pointer to an `config_options` structure, which is used to store the parsed options for the config command. 
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "config_options.h"
#include "../../cli/parser_engine.h"


/**
 * @brief Parse command line options for the config command
 * 
 * The function is called to perform the actual parsing of the command-line arguments based on the predefined options. It takes the command-line arguments and a pointer to an `config_options` structure, 
 * which is used to store the parsed options for the config command. The function checks for the presence of each option in the command-line arguments and updates the corresponding fields in the `config_options`
 * structure accordingly. The function handles help requests and parsing errors, returning appropriate status codes based on the outcome of the parsing process. 
 * A reference array of `cli_option` structures to define all possible options for the config command is used, includes their names, types, value pointers, and help descriptions.
 * The `config_options` structure includes fields for the serial port, baud rate, device address, and security key for encrypting data if necessary.
 * The parsing function checks for the presence of each option in the command-line arguments and updates the corresponding fields in the `config_options` structure accordingly. 
 * 
 * @param[in] argc is the argument count, which represents the number of options passed to the config command from the command line, including the name of the config command itself.
 * @param[in] argv represents the command-line arguments passed to the config command.
 * @param[out] opts A pointer to an `config_options` structure, which will be populated with the parsed options for the config command.
 * @return 0 on success, 1 if help was requested, or a negative value on error.
 */
int config_parse(int argc, char **argv, struct config_options *opts)
{

    memset(opts, 0, sizeof(*opts)); 
    
    opts->addr = 0;          // Initialize to the first available address.
    opts->baud = "9600";       // Initialize to normal baud rate to use if not specified by the user.    

    // Define the available command-line options for the config command.
    // Each `struct cli_option` defines a specific command-line option.
    // The `cli_option` structure includes fields for the option name, type, value pointer, size (for hex options), presence indicator, enumeration values (for enum options), 
    // and help text needed for parsing and handling that option.
    // It's the reference options, like a grammar for the command line arguments of the config command, that the parsing function will use to understand and process the user's input correctly.
    struct cli_option options[] = {

        { "--port"  , CLI_OPT_STRING, &opts->port  , 0                 , NULL           , NULL, "Serial port to use (e.g. COM3 or /dev/ttyUSB0)" },
        { "--baud"  , CLI_OPT_STRING, &opts->baud  , 0                 , NULL           , NULL, "Baud rate to use" },
        { "--addr"  , CLI_OPT_INT   , &opts->addr  , 0                 , NULL           , NULL, "Device address for the OSDP device to query" },
        { "--scbk"  , CLI_OPT_HEX   , &opts->scbk  , sizeof(opts->scbk), &opts->scbk_set, NULL, "Decrypt Secure Channel if possible using this Base Key (16 bytes in hex)" },
        { "--scbk-d", CLI_OPT_FLAG  , &opts->scbk_d, 0                 , NULL           , NULL, "Use default SCBK key for secure channel establishment" },
        { "--new-addr"  , CLI_OPT_INT   , &opts->new_addr  , 0                 , &opts->new_addr_set, NULL, "New device address for the PD device" },
        { "--new-baud"  , CLI_OPT_INT   , &opts->new_baud  , 0                 , &opts->new_baud_set, NULL, "New baud rate for the serial communication" },
        { "--reconnect" , CLI_OPT_FLAG  , &opts->reconnect , 0                 , NULL           , NULL, "Attempt to reconnect to the device after changing configuration" }
    };

    int ret = parse_command_line_options(argc, argv, options, sizeof(options) / sizeof(options[0]));
    
    if (ret == -1) 
    { // Help requested, handle help option case.
        printf("\nUsage: config [options]\n");
        print_command_line_options(options, sizeof(options) / sizeof(options[0]));
        return 1;
    }
    else if (ret < 0) { // Parsing format error, handle error case.
        return ret;
    }
    
    return 0;
}  