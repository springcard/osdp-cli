/**
 * @file led_options_parser.c
 * @brief Implementation for parsing led subcommand options
 * 
 * The code provided implements the parsing of command-line options for the "led" subcommand in the osdp-cli application.
 * It defines a function `led_parse` that takes command-line arguments and a pointer to a `led_options` structure, which is used to store the parsed options for the led subcommand. 
 * The function uses a reference array of `cli_option` structures to define all possible options for the led subcommand, includes their names, types, value pointers, and help descriptions.
 * The `parse_command_line options` function is called to perform the actual parsing of the command-line arguments based on the referenced options.
 * The function handles help requests, parsing errors, and updates the `led_options` structure with the parsed values for use in executing the led subcommand.
 * 
 * The `led_options` structure includes fields for the serial port, baud rate, security keys and all parameters specific to LED management such as LED number, color, mode, on and off times as well as the 
 * total duration for which the LED should be controlled.
 * The `led_color_value` array defines the possible colors for the LED, which can be red, green, amber, blue, white, or off. The `led_mode_values` array defines the possible modes for the LED, 
 * which can be off, on, blink, or temporary. The parsing function checks for the presence of each option in the command-line arguments and updates the corresponding fields in the `led_options` structure accordingly. 
 */
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "led_options.h"
#include "../../../cli/parser_engine.h"


// Reference array of LED colors for the led command options, 
// used for parsing the color option for the led command.   
static const struct cli_enum led_color_values[] = {
    { "red",   LED_RED },
    { "green", LED_GREEN },
    { "amber", LED_AMBER },
    { "blue",  LED_BLUE },
    { "white", LED_WHITE },
    { "off",   LED_OFF },

    { NULL, 0 }
};

// Reference array of LED modes for the led command options, 
// used for parsing the mode option for the led command.
static const struct cli_enum led_mode_values[] = {
    { "off",   LED_MODE_OFF },
    { "on",    LED_MODE_ON },
    { "blink", LED_MODE_BLINK },
    { "temp",  LED_MODE_TEMP },

    { NULL, 0 }
};


/**
 * @brief Parse command line options for the led subcommand
 * 
 * The function is called to perform the actual parsing of the command-line arguments based on the predefined options. It takes the command-line arguments and a pointer to a `led_options` structure, 
 * which is used to store the parsed options for the led subcommand. 
 * The function checks for the presence of each option in the command-line arguments and updates the corresponding fields in the `led_options` structure accordingly.
 * The function handles help requests.
 * 
 * @param[in] argc `argc` is the argument count, which represents the number of arguments passed to the led subcommand from the command line, including the name of the subcommand itself.
 * @param[in] argv represents the command-line arguments passed to the led subcommand.
 * @param[out] opts a pointer to an `led_parse` structure, which will be populated with the parsed options.
 * 
 * @return 0 on success, 1 if help was requested, or a negative value on error.
 */
int led_parse(int argc, char **argv, struct led_options *opts)
{

    memset(opts, 0, sizeof(*opts));
    //opts->addr = -1;       // Initialize to an invalid address to check later if it was set.
    opts->addr = 0;          // Initialize to the first available address.
    opts->baud = "9600";       // Initialize to normal baud rate to use if not specified by the user.    
    opts->led_number = -1; // Initialize to an invalid LED number to check later if it was set.

    opts->onTime = -1;   // Initialize to an invalid on time to check later if it was set.
    opts->offTime = -1;  // Initialize to an invalid off time to check later if it was set.
    opts->duration = -1; // Initialize to an invalid duration to check later if it was set.

    opts->led_color = LED_COLOR_NONE; // Initialize to an invalid color to check later if it was set.
    opts->led_mode = LED_MODE_NONE;   // Initialize to an invalid mode to check later if it was set.

    // Define the available command-line options for the led subcommand.
    // Each `struct cli_option` defines a specific command-line option.
    // The `cli_option` structure includes fields for the option name, type, value pointer, size (for hex options), presence indicator, enumeration values (for enum options), and help text needed for parsing and handling that option.
    // It's the reference options, like a grammar for the command line arguments of the led subcommand, that the parsing function will use to understand and process the user's input correctly.
    struct cli_option options[] = {

        { "--port",     CLI_OPT_STRING, &opts->port,                  0,            NULL,             NULL, "Serial port to use (e.g. COM3 or /dev/ttyUSB0)" },
        { "--baud",     CLI_OPT_STRING, &opts->baud,                  0,            NULL,             NULL, "Baud rate to use" },
        { "--addr",     CLI_OPT_INT,    &opts->addr,                  0,            NULL,             NULL, "Device address for the OSDP device to query" },        
        { "--scbk-d",   CLI_OPT_FLAG,   &opts->scbk_d,                0,            NULL,             NULL, "Use default SCBK key for secure channel establishment" },        
        { "--scbk",     CLI_OPT_HEX,    &opts->scbk, sizeof(opts->scbk), &opts->scbk_set,             NULL, "Use this Base Key for Secure Channel if possible" },
        { "--led",      CLI_OPT_INT,    &opts->led_number,            0,            NULL,             NULL, "LED ID to control" },
        { "--color",    CLI_OPT_ENUM,   &opts->led_color,             0,            NULL, led_color_values, "Color to set the LED to" },
        { "--mode",     CLI_OPT_ENUM,   &opts->led_mode,              0,            NULL,  led_mode_values, "Mode to set the LED to" },
        { "--on",       CLI_OPT_INT,    &opts->onTime,                0,            NULL,             NULL, "Time in milliseconds for which the LED should be turned on" },
        { "--off",      CLI_OPT_INT,    &opts->offTime,               0,            NULL,             NULL, "Time in milliseconds for which the LED should be turned off" },
        { "--duration", CLI_OPT_INT,    &opts->duration,              0,            NULL,             NULL, "Total duration in ms for which the LED should be controlled" }
    };
    
    // led command line options parsing
    int ret = parse_command_line_options(argc, argv, options, sizeof(options) / sizeof(options[0])) ;

    if (ret == -1) { // Help requested, handle help option case.
        printf("\nUsage: cmd led [options]\n");
        print_command_line_options(options, sizeof(options) / sizeof(options[0]));
        return 1;
    }
    else if (ret < 0) { // Parsing format error, handle error case.
        return ret;
    }
    
    return 0;
}    
