/**
 * @file relay_options_parser.c
 * @brief Implementation for parsing relay subcommand options
 * 
 * The code provided implements the parsing of command-line options for the "relay" subcommand in the osdp-cli application.
 * It defines a function `relay_parse` that takes command-line arguments and a pointer to a `relay_options` structure, which is used to store the parsed options for the relay subcommand. 
 * The function uses a reference array of `cli_option` structures to define all possible options for the relay subcommand, includes their names, types, value pointers, and help descriptions.
 * The `parse_command_line options` function is called to perform the actual parsing of the command-line arguments based on the referenced options.
 * The function handles help requests, parsing errors, and updates the `relay_options` structure with the parsed values for use in executing the relay subcommand.
 * 
 * The `relay_options` structure includes fields for the serial port, baud rate, security keys and all parameters specific to relay management such as relay number, mode and duration of relay operation.
 * The `relay_mode_values` array defines the possible modes for the relay, which can be <off|on|pulse>. The parsing function checks for the presence of each option in the command-line arguments and 
 * updates the corresponding fields in the `relay_options` structure accordingly. 
 */
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "relay_options.h"
#include "../../../cli/parser_engine.h"


// Reference array of relay modes for the relay command options, 
// used for parsing the mode option for the relay command.
static const struct cli_enum relay_mode_values[] = {
    { "off",   RELAY_MODE_OFF },
    { "on",    RELAY_MODE_ON },

    { NULL, 0 }/*,
    { "pulse", RELAY_MODE_PULSE }*/
};


/**
 * @brief Parse command line options for the relay subcommand
 * 
 * The function is called to perform the actual parsing of the command-line arguments based on the predefined options. It takes the command-line arguments and a pointer to a `relay_options` structure, 
 * which is used to store the parsed options for the relay subcommand. 
 * The function checks for the presence of each option in the command-line arguments and updates the corresponding fields in the `relay_options` structure accordingly.
 * The function handles help requests.
 * 
 * @param[in] argc `argc` is the argument count, which represents the number of arguments passed to the relay subcommand from the command line, including the name of the subcommand itself.
 * @param[in] argv represents the command-line arguments passed to the relay subcommand.
 * @param[out] opts a pointer to an `relay_parse` structure, which will be populated with the parsed options.
 * 
 * @return 0 on success, 1 if help was requested, or a negative value on error.
 */
int relay_parse(int argc, char **argv, struct relay_options *opts)
{

    memset(opts, 0, sizeof(*opts));

    opts->addr = 0;           // Initialize to the first available address
    opts->baud = "9600";      // Initialize to normal baud rate to use if not specified by the user
    opts->output_number = -1; // Initialize to an invalid relay output number to check later if it was set
    opts->duration = -1;      // Initialize to an invalid duration to check later if it was set
    opts->relay_mode = RELAY_MODE_NONE;   // Initialize to an invalid mode to check later if it was set

    // Define the available command-line options for the relay subcommand.
    // Each `struct cli_option` defines a specific command-line option.
    // The `cli_option` structure includes fields for the option name, type, value pointer, size (for hex options), presence indicator, enumeration values (for enum options), and help text needed for parsing and handling that option.
    // It's the reference options, like a grammar for the command line arguments of the relay subcommand, that the parsing function will use to understand and process the user's input correctly.
    struct cli_option options[] = {

        { "--port",     CLI_OPT_STRING, &opts->port,                  0,            NULL,               NULL, "Serial port to use (e.g. COM3 or /dev/ttyUSB0)" },
        { "--baud",     CLI_OPT_STRING, &opts->baud,                  0,            NULL,               NULL, "Baud rate to use" },
        { "--addr",     CLI_OPT_INT,    &opts->addr,                  0,            NULL,               NULL, "Device address for the OSDP device to query" },        
        { "--scbk-d",   CLI_OPT_FLAG,   &opts->scbk_d,                0,            NULL,               NULL, "Use default SCBK key for secure channel establishment" },        
        { "--scbk",     CLI_OPT_HEX,    &opts->scbk, sizeof(opts->scbk), &opts->scbk_set,               NULL, "Use this Base Key for Secure Channel if possible" },
        { "--output",   CLI_OPT_INT,    &opts->output_number,         0,            NULL,               NULL, "Relay output ID to control" },
        { "--mode",     CLI_OPT_ENUM,   &opts->relay_mode,            0,            NULL,  relay_mode_values, "Mode to set the relay to" },
        { "--duration", CLI_OPT_INT,    &opts->duration,              0,            NULL,               NULL, "Total duration in ms for which the relay should be controlled" }
    };
    
    // relay command line options parsing
    int ret = parse_command_line_options(argc, argv, options, sizeof(options) / sizeof(options[0])) ;

    if (ret == -1) { // Help requested, handle help option case.
        printf("\nUsage: cmd relay [options]\n");
        print_command_line_options(options, sizeof(options) / sizeof(options[0]));
        return 1;
    }
    else if (ret < 0) { // Parsing format error, handle error case.
        return ret;
    }
    
    return 0;
}    
