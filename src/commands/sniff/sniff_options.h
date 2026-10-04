/**
 * @file sniff_options.h
 * @brief Header file for sniff command options
 * 
 * The code provided defines the structures and function prototypes necessary for parsing and handling the options related to the sniff command.
 * The `sniff_options` structure contains various fields that represent the options for the sniff command, such as the port, baud rate, security keys, output format, and more. 
 * The function prototypes include `sniff_parse` for parsing the command-line options specific to the sniff command and `sniff_run` for executing the sniff command based on the parsed options and the CLI context. 
 * This header file is designed to be used in osdp-cli application where the sniff command can be executed with various options that need to be parsed and handled appropriately.
 */
#ifndef SNIFF_OPTIONS_H
#define SNIFF_OPTIONS_H

#include <stdint.h>
#include <stddef.h>

// Enumeration for output format options for the sniff command logging. 
// It defines different output formats that can be used for logging the sniffed data such as text, JSON, and binary. 
// The choice of output format can affect how the logged data is structured and presented.
enum sniff_out_format
{
    SNIFF_OUT_TEXT = 0,
    SNIFF_OUT_JSON,
    SNIFF_OUT_BIN
};


// This structure is used to store the various options that can be provided for the sniff command in the osdp-cli application.
// It includes fields for the port, baud rate, security keys (SCBK and SCSK), display options for raw, decoded, and error data, output file name, output format, and log rotation settings.
struct sniff_options
{
    const char *port;       // Serial port to use for sniffing, e.g., "/dev/ttyUSB0" on Linux or "COM3" on Windows.
    const char *baud;       // Baud rate for the serial communication, e.g., 9600, 115200, etc.
    
    unsigned char scbk[16]; // Security Control Block Key (SCBK) for decrypting sniffed data, if applicable.
    int scbk_set;           // Flag to indicate if the SCBK key has been set by the user

    unsigned char scsk[16]; // Security Control Session Key (SCSK) for decrypting sniffed data, if applicable.
    int scsk_set;           // Flag to indicate if the SCSK key has been set by the user

    int show_raw;           // Flag to indicate whether to display raw sniffed data in the output.
    int show_decoded;       // Flag to indicate whether to display decoded sniffed data in the output.
    int show_errors;        // Flag to indicate whether to display error information in the output.

    const char *out_file;   // File name for logging the sniffed data, if specified by the user. If not provided, logging may be done to standard output or a default file.
    enum sniff_out_format out_format;// The output format for logging the sniffed data, which can be text, JSON, or binary, as defined by the `sniff_out_format` enumeration.
    int out_format_set;     // Flag to indicate if the output format has been set by the user.
    int rotate;             // Flag to indicate whether to enable log rotation for the output file, if applicable. 

    int no_color;   // Disable ANSI colors in the output, which can be useful when logging to a file or when the terminal does not support colors.
};

struct cli_context;         // forward-declare opaque command context

#define MAX_FRAME_SIZE   100

/**
 * @brief Represents one captured OSDP frame.
 */
typedef struct
{
    size_t len;

    uint8_t data[MAX_FRAME_SIZE];

} sniff_frame_t;


// Function for parsing the "sniff" command based on the provided command line options and possible options.
int sniff_parse(int argc, char **argv, struct sniff_options *opts);

// Function for executing the sniff command based on the parsed options and the CLI context.
int sniff_run(struct cli_context *ctx);
// Display the parsed options for the sniff command (for testing purposes)
int sniff_test(struct cli_context *ctx);

#endif // SNIFF_OPTIONS_H