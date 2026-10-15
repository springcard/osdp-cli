#include "../../cli/command_result.h"
/**
 * @file sckmgmt_options_parser.c
 * @brief Implementation for parsing sckmgmt command options
 * 
 * The code provided implements the parsing of command-line options for the "sckmgmt" command in the osdp-cli application.
 * It defines a function `sckmgmt_parse` that takes command-line arguments and a pointer to an `sckmgmt_options` structure, which is used to store the parsed options for the sckmgmt command. 
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "sckmgmt_options.h"
#include "../../cli/parser_engine.h"


/**
 * @brief Parse command line options for the sckmgmt command
 * 
 * This function parses command line options for a command called `sckmgmt`. 
 * The function is called to perform the actual parsing of the command-line arguments based on the predefined options. It takes the command-line arguments and a pointer to an `sckmgmt_options` structure, 
 * which is used to store the parsed options for the sckmgmt command. The function checks for the presence of each option in the command-line arguments and updates the corresponding fields in the `sckmgmt_options`
 * structure accordingly. The function handles help requests and parsing errors, returning appropriate status codes based on the outcome of the parsing process. 
 * A reference array of `cli_option` structures to define all possible options for the sckmgmt command is used, includes their names, types, value pointers, and help descriptions.
 * The `sckmgmt_options` structure includes fields for the serial port, baud rate, device address, and security keys (SCBK, SCSK and new SCBK) for encrypting data if necessary.
 * The parsing function checks for the presence of each option in the command-line arguments and updates the corresponding fields in the `sckmgmt_options` structure accordingly. 
 * 
 * @param[in] argc is the argument count, which represents the number of options passed to the sckmgmt command from the command line, including the name of the sckmgmt command itself.
 * @param[in] argv represents the command-line arguments passed to the sckmgmt command.
 * @param[out] opts A pointer to an `sckmgmt_options` structure, which will be populated with the parsed options for the sckmgmt command.
 * @return 0 on success, 1 if help was requested, or a negative value on error.
 */
int sckmgmt_parse(int argc, char **argv, struct sckmgmt_options *opts)
{

    memset(opts, 0, sizeof(*opts)); 
    //opts->addr = -1; // Set default value for addr to -1 to indicate it's not set
    opts->addr = 0;          // Initialize to the first available address.
    opts->baud = "9600";       // Initialize to normal baud rate to use if not specified by the user.

    // Define the available command-line options for the sckmgmt command.
    // Each `struct cli_option` defines a specific command-line option.
    // The `cli_option` structure includes fields for the option name, type, value pointer, size (for hex options), presence indicator, enumeration values (for enum options), and help text needed for parsing and handling that option.
    // It's the reference options, like a grammar for the command line arguments of the sckmgmt command, that the parsing function will use to understand and process the user's input correctly.
    struct cli_option options[] = {

        { "--port", CLI_OPT_STRING, &opts->port, 0, NULL, NULL, "Serial port to use (e.g. COM3 or /dev/ttyUSB0)" },
        { "--baud", CLI_OPT_STRING, &opts->baud, 0, NULL, NULL, "Baud rate to use" },
        { "--addr", CLI_OPT_INT, &opts->addr, 0, NULL, NULL, "Device address for the OSDP device to query" },
        { "--scbk", CLI_OPT_HEX, &opts->scbk, sizeof(opts->scbk), &opts->scbk_set, NULL, "Decrypt Secure Channel if possible using this Base Key (16 bytes in hex)" },
        { "--scbk-d", CLI_OPT_FLAG  , &opts->scbk_d, 0                 , NULL           , NULL, "Use default SCBK key for secure channel establishment" },
        { "--scsk", CLI_OPT_HEX, &opts->scsk, sizeof(opts->scsk), &opts->scsk_set, NULL, "Unsupported session key option (reserved for compatibility)" },
        { "--set-key", CLI_OPT_HEX, &opts->new_scbk, sizeof(opts->new_scbk), &opts->new_scbk_set, NULL, "New SCBK to update the device with (16 bytes in hex)" },
        { "--verify", CLI_OPT_FLAG, &opts->verify, 0, NULL, NULL, "Compatibility flag: the new SCBK is always verified after update" }
    };
    
    // sckmgmt command line options parsing
    int ret = parse_command_line_options(argc, argv, options, sizeof(options) / sizeof(options[0])) ;

    if (ret == -1) { // Help requested, handle help option case.
        printf("\nUsage: sckmgmt [options]\n");
        print_command_line_options(options, sizeof(options) / sizeof(options[0]));
        sckmgmt_clear_options(opts);
        return 1;
    }
    else if (ret < 0) { // Parsing format error, handle error case.
        sckmgmt_clear_options(opts);
        return ret;
    }
    
    if (ret != argc || !opts->new_scbk_set) {
        cli_diagnostic(stderr, "sckmgmt: --set-key required; unexpected positional arguments are rejected\n");
        sckmgmt_clear_options(opts);
        return -3;
    }
    return 0;
}
