/**
 * @file put_options_parser.c
 * @brief Implementation for parsing put subcommand options
 * 
 * The code provided implements the parsing of command-line options for the "put" subcommand in the osdp-cli application.
 * It defines a function `put_parse` that takes command-line arguments and a pointer to a `put_options` structure, which is used to store the parsed options for the put subcommand. 
 * The function uses a reference array of `cli_option` structures to define all possible options for the put subcommand, includes their names, types, value pointers, and help descriptions.
 * The `parse_command_line options` function is called to perform the actual parsing of the command-line arguments based on the referenced options.
 * The function handles help requests, parsing errors, and updates the `put_options` structure with the parsed values for use in executing the put subcommand.
 * 
 * The `put_options` structure includes fields for the serial port, baud rate, security keys and all parameters specific to put file management such as type, input file, and chunk size.
 * The `put_type_values` array defines the possible types for the PUT operation, which can be config, firmware, or blob.
 */
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "put_options.h"
#include "../../../cli/parser_engine.h"


// Reference array of PUT types for the put command options, 
// used for parsing the type option for the put command.   
static const struct cli_enum put_type_values[] = {
    { "config",   PUT_TYPE_CONFIG },
    { "firmware", PUT_TYPE_FIRMWARE },
    { "blob",     PUT_TYPE_BLOB },

    { NULL, 0 }
};


/**
 * @brief Parse command line options for the put subcommand
 * 
 * The function is called to perform the actual parsing of the command-line arguments based on the predefined options. It takes the command-line arguments and a pointer to a `put_options` structure, 
 * which is used to store the parsed options for the put subcommand. 
 * The function checks for the presence of each option in the command-line arguments and updates the corresponding fields in the `put_options` structure accordingly.
 * The function handles help requests.
 * 
 * @param[in] argc `argc` is the argument count, which represents the number of arguments passed to the put subcommand from the command line, including the name of the subcommand itself.
 * @param[in] argv represents the command-line arguments passed to the put subcommand.
 * @param[out] opts a pointer to an `put_parse` structure, which will be populated with the parsed options.
 * 
 * @return 0 on success, 1 if help was requested, or a negative value on error.
 */
int put_parse(int argc, char **argv, struct put_options *opts)
{

    memset(opts, 0, sizeof(*opts));
    
    opts->addr = 0;          // Initialize to the first available address.
    opts->baud = "9600";       // Initialize to normal baud rate to use if not specified by the user.    
    opts->type = PUT_TYPE_NONE; // Initialize to an invalid type to check later if it was set
    opts->chunk = 1024;         // Default chunk size to 1024 bytes

    // Define the available command-line options for the put subcommand.
    // Each `struct cli_option` defines a specific command-line option.
    // The `cli_option` structure includes fields for the option name, type, value pointer, size (for hex options), presence indicator, enumeration values (for enum options), and help text needed for parsing and handling that option.
    // It's the reference options, like a grammar for the command line arguments of the put subcommand, that the parsing function will use to understand and process the user's input correctly.
    struct cli_option options[] = {

        { "--port",     CLI_OPT_STRING, &opts->port,                  0,            NULL,             NULL, "Serial port to use (e.g. COM3 or /dev/ttyUSB0)" },
        { "--baud",     CLI_OPT_STRING, &opts->baud,                  0,            NULL,             NULL, "Baud rate to use" },
        { "--addr",     CLI_OPT_INT,    &opts->addr,                  0,            NULL,             NULL, "Device address for the OSDP device to query" },        
        { "--scbk-d",   CLI_OPT_FLAG,   &opts->scbk_d,                0,            NULL,             NULL, "Use default SCBK key for secure channel establishment" },        
        { "--scbk",     CLI_OPT_HEX,    &opts->scbk, sizeof(opts->scbk), &opts->scbk_set,             NULL, "Use this Base Key for Secure Channel if possible" },
        { "--type",     CLI_OPT_ENUM,   &opts->type,                  0,            NULL,  put_type_values, "File type for operation to perform" },
        { "--in",       CLI_OPT_STRING, &opts->input_file,            0,            NULL,             NULL, "Input file containing data to put" },
        { "--chunk",    CLI_OPT_INT,    &opts->chunk,                 0,            NULL,             NULL, "Size of each chunk of data to send" },
        { "--resume",   CLI_OPT_FLAG,   &opts->resume,                0,            NULL,             NULL, "Resume an interrupted operation" },
        { "--reboot",   CLI_OPT_FLAG,   &opts->reboot,                0,            NULL,             NULL, "Reboot the device after operation is completed" },
        { "--progress", CLI_OPT_FLAG,   &opts->progress,              0,            NULL,             NULL, "Display a progress bar during the operation" },
        { "--pause",    CLI_OPT_INT,    &opts->pause,                 0,            NULL,             NULL, "Delay in milliseconds between queries" }    
    };
    
    // put command line options parsing
    int ret = parse_command_line_options(argc, argv, options, sizeof(options) / sizeof(options[0])) ;

    if (ret == -1) { // Help requested, handle help option case.
        printf("\nUsage: cmd put [options]\n");
        print_command_line_options(options, sizeof(options) / sizeof(options[0]));
        return 1;
    }
    else if (ret < 0) { // Parsing format error, handle error case.
        return ret;
    }
    
    return 0;
}    
