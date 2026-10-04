/**
 * @file buzzer_options_parser.c
 * @brief Implementation for parsing buzzer subcommand options
 * 
 * The code provided implements the parsing of command-line options for the "buzzer" subcommand in the osdp-cli application.
 * It defines a function `buzzer_parse` that takes command-line arguments and a pointer to a `buzzer_options` structure, which is used to store the parsed options for the buzzer subcommand. 
 * The function uses a reference array of `cli_option` structures to define all possible options for the buzzer subcommand, includes their names, types, value pointers, and help descriptions.
 * The `parse_command_line options` function is called to perform the actual parsing of the command-line arguments based on the referenced options.
 * The function handles help requests, parsing errors, and updates the `buzzer_options` structure with the parsed values for use in executing the buzzer subcommand.
 * 
 * The `buzzer_options` structure includes fields for the serial port, baud rate, security keys and all parameters specific to buzzer management such as mode, on and off times as well 
 * as the number of cycles for which the buzzer should be controlled.
 * The `buzzer_mode_values` array defines the possible modes for the buzzer, which can be <off|on|beep|pattern>. The parsing function checks for the presence of each option in the 
 * command-line arguments and updates the corresponding fields in the `buzzer_options` structure accordingly. 
 */
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "buzzer_options.h"
#include "../../../cli/parser_engine.h"


// Reference array of buzzer modes for the buzzer command options, 
// used for parsing the mode option for the buzzer command.
static const struct cli_enum buzzer_mode_values[] = {
    { "off",     BUZZER_MODE_OFF },
    { "on",      BUZZER_MODE_ON },
    { "beep",    BUZZER_MODE_BEEP },
    { "pattern", BUZZER_MODE_PATTERN },

    { NULL, 0 }
};


/**
 * @brief Parse command line options for the buzzer subcommand
 * 
 * The function is called to perform the actual parsing of the command-line arguments based on the predefined options. It takes the command-line arguments and a pointer to a `buzzer_options` structure, 
 * which is used to store the parsed options for the buzzer subcommand. 
 * The function checks for the presence of each option in the command-line arguments and updates the corresponding fields in the `buzzer_options` structure accordingly.
 * The function handles help requests.
 * 
 * @param[in] argc `argc` is the argument count, which represents the number of arguments passed to the buzzer subcommand from the command line, including the name of the subcommand itself.
 * @param[in] argv represents the command-line arguments passed to the buzzer subcommand.
 * @param[out] opts a pointer to an `buzzer_options` structure, which will be populated with the parsed options.
 * 
 * @return 0 on success, 1 if help was requested, or a negative value on error.
 */
int buzzer_parse(int argc, char **argv, struct buzzer_options *opts)
{

    memset(opts, 0, sizeof(*opts));
    //opts->addr = -1;       // Initialize to an invalid address to check later if it was set.
    opts->addr = 0;          // Initialize to the first available address.
    opts->baud = "9600";       // Initialize to normal baud rate to use if not specified by the user.
    opts->buzzer_mode = BUZZER_MODE_NONE; // Initialize to an invalid mode to check later if it was set.
    opts->onTime = -1;   // Initialize to an invalid on time to check later if it was set.
    opts->offTime = -1;  // Initialize to an invalid off time to check later if it was set.
    opts->count = -1; // Initialize to an invalid duration to check later if it was set.

    // Define the available command-line options for the buzzer subcommand.
    // Each `struct cli_option` defines a specific command-line option.
    // The `cli_option` structure includes fields for the option name, type, value pointer, size (for hex options), presence indicator, enumeration values (for enum options), and help text needed for parsing and handling that option.
    // It's the reference options, like a grammar for the command line arguments of the buzzer subcommand, that the parsing function will use to understand and process the user's input correctly.
    struct cli_option options[] = {

        { "--port",     CLI_OPT_STRING, &opts->port,                  0,            NULL,                NULL, "Serial port to use (e.g. COM3 or /dev/ttyUSB0)" },
        { "--baud",     CLI_OPT_STRING, &opts->baud,                  0,            NULL,                NULL, "Baud rate to use" },
        { "--addr",     CLI_OPT_INT,    &opts->addr,                  0,            NULL,                NULL, "Device address for the OSDP device to query" },        
        { "--scbk-d",   CLI_OPT_FLAG,   &opts->scbk_d,                0,            NULL,                NULL, "Use default SCBK key for secure channel establishment" },        
        { "--scbk",     CLI_OPT_HEX,    &opts->scbk, sizeof(opts->scbk), &opts->scbk_set,                NULL, "Use this Base Key for Secure Channel if possible" },
        { "--mode",     CLI_OPT_ENUM,   &opts->buzzer_mode,           0,            NULL,  buzzer_mode_values, "Mode to set the buzzer to" },
        { "--on",       CLI_OPT_INT,    &opts->onTime,                0,            NULL,                NULL, "Time in milliseconds for which the buzzer should be turned on" },
        { "--off",      CLI_OPT_INT,    &opts->offTime,               0,            NULL,                NULL, "Time in milliseconds for which the buzzer should be turned off" },
        { "--count",    CLI_OPT_INT,    &opts->count,                 0,            NULL,                NULL, "Number of times to buzz" }
    };
    
    // buzzer command line options parsing
    int ret = parse_command_line_options(argc, argv, options, sizeof(options) / sizeof(options[0])) ;

    if (ret == -1) { // Help requested, handle help option case.
        printf("\nUsage: cmd buzzer [options]\n");
        print_command_line_options(options, sizeof(options) / sizeof(options[0]));
        return 1;
    }
    else if (ret < 0) { // Parsing format error, handle error case.
        return ret;
    }
    
    return 0;
}    
