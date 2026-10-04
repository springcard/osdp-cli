/**
 * @file sniff_options_parser.c
 * @brief Implementation for parsing sniff command options
 * 
 * The code provided implements the parsing of command-line options for the "sniff" command in the osdp-cli application.
 * It defines a function `sniff_parse` that takes command-line arguments and a pointer  to a `sniff_options` structure, which is used to store the parsed options for the sniff command. 
 * The function uses a reference array of `cli_option` structures to define all possible options for the sniff command, includes their names, types, value pointers, and help descriptions.
 * The `parse_command_line options` function is called to perform the actual parsing of the command-line arguments based on the referenced options.
 * The function handles help requests, parsing errors, and updates the `sniff_options` structure with the parsed values for use in executing the sniff command.
 * 
 * The `sniff_options` structure includes fields for the serial port, baud rate, security keys (SCBK and SCSK), display options for raw, decoded, and error data, output file name, output format, 
 * and log rotation settings.
 * The `sniff_out_format` enumeration defines the possible output formats for logging the sniffed data, which can be text, JSON, or binary. The parsing function checks for the presence of each option in the 
 * command-line arguments and updates the corresponding fields in the `sniff_options` structure accordingly.
 */
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "sniff_options.h"
#include "../../cli/parser_engine.h"


// Reference array of log format for the sniff command  logging options, 
// used for parsing the output format option for the sniff command.
static const struct cli_enum sniff_out_format_enum[] = {

    { "text",  SNIFF_OUT_TEXT },
    { "json", SNIFF_OUT_JSON },
    { "bin",   SNIFF_OUT_BIN },

    { NULL, 0 }
};


/**
 * @brief Parse command line options for the sniff command
 * 
 * This function parses command line options for a command called `sniff`. 
 * The function is called to perform the actual parsing of the command-line arguments based on the predefined options. It takes the command-line arguments and a pointer to a `sniff_options` structure, 
 * which is used to store the parsed options for the sniff command. 
 * The function checks for the presence of each option in the command-line arguments and updates the corresponding fields in the `sniff_options` structure accordingly.
 * The function handles help requests.
 * 
 * @param[in] argc `argc` is the argument count, which represents the number of arguments passed to the
 * program from the command line, including the name of the program itself.
 * @param[in] argv argv is a pointer to an array of strings representing the command-line arguments passed
 * to the program. Each element in the array is a null-terminated string, with the first element
 * (argv[0]) typically being the name of the program itself. The subsequent elements contain the
 * arguments provided by the user when
 * @param[out] opts The `opts` parameter in the `sniff_parse` function is a pointer to a `struct
 * sniff_options`. This structure is used to store various options and settings for the sniffing
 * functionality.
 * 
 * @return The `sniff_parse` function returns an integer value. If the function encounters a parsing
 * format error during command line options parsing, it returns the error code. If the user requests
 * help by providing the appropriate option, the function prints out the usage information and
 * available options, then returns 1. Otherwise, if the parsing is successful without any errors, the
 * function returns 0.
 */
int sniff_parse(int argc, char **argv, struct sniff_options *opts)
{

    memset(opts, 0, sizeof(*opts));
    opts->baud = "9600";       // Initialize to normal baud rate to use if not specified by the user.

    opts->out_file = NULL;
    opts->out_format = SNIFF_OUT_TEXT;
    opts->rotate = 0;    

    // Define the available command-line options for the sniff command.
    // Each `struct cli_option` defines a specific command-line option.
    // The `cli_option` structure includes fields for the option name, type, value pointer, size (for hex options), presence indicator, enumeration values (for enum options), and help text needed for parsing and handling that option.
    // It's the reference options, like a grammar for the command line arguments of the sniff command, that the parsing function will use to understand and process the user's input correctly.
    struct cli_option options[] = {

        { "--port", CLI_OPT_STRING, &opts->port, 0, NULL, NULL, "Serial port to use (e.g. COM3 or /dev/ttyUSB0)" },
        { "--baud", CLI_OPT_STRING, &opts->baud, 0, NULL, NULL, "Baud rate to use" },
        { "--scbk", CLI_OPT_HEX, &opts->scbk, sizeof(opts->scbk), &opts->scbk_set, NULL, "Decrypt Secure Channel if possible using this Base Key (16 bytes in hex)" },
        { "--scsk", CLI_OPT_HEX, &opts->scsk, sizeof(opts->scsk), &opts->scsk_set, NULL, "Decrypt Secure Channel if possible using this Session Key (16 bytes in hex)" },
        { "--show-raw", CLI_OPT_FLAG, &opts->show_raw, 0, NULL, NULL, "Print raw frame bytes" },
        { "--show-decoded", CLI_OPT_FLAG, &opts->show_decoded, 0, NULL, NULL, "Print decoded command/reply fields (default)" },
        { "--show-errors", CLI_OPT_FLAG, &opts->show_errors, 0, NULL, NULL, "Print CRC/checksum and parsing errors" },
        { "--out", CLI_OPT_STRING, &opts->out_file, 0, NULL, NULL, "Write capture to file" },
        { "--out-format", CLI_OPT_ENUM, &opts->out_format,0, &opts->out_format_set, sniff_out_format_enum, "Output format (decoded text, JSON, raw binary)" },
        { "--rotate", CLI_OPT_INT, &opts->rotate, 0, NULL, NULL, "Log rotation enabled, size in MB (0 = disabled)" }

    };
    
    // Sniff command line options parsing
    int ret = parse_command_line_options(argc, argv, options, sizeof(options) / sizeof(options[0])) ;

    if (ret == -1) { // Help requested, handle help option case.
        printf("\nUsage: sniff [options]\n");
        print_command_line_options(options, sizeof(options) / sizeof(options[0]));
        return 1;
    }
    else if (ret < 0) { // Parsing format error, handle error case.
        return ret;
    }
    
    return 0;
}    
