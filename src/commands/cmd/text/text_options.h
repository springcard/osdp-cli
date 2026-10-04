/**
 * @file text_options.h
 * @brief Header file for text subcommand options
 * 
 * The code provided defines the structures and function prototypes necessary for parsing and handling the options related to the text subcommand.
 * The `text_options` structure contains various fields that represent the options for the text subcommand,
 * such as the serial port, baud rate, device address, and security key for encrypting data if necessary as well as the parameters specific to text management.
 * The function prototypes include `text_parse` for parsing the command-line options specific to the text subcommand and `text_run` for executing the text subcommand based on the parsed options and the CLI context.
 * This header file is designed to be used in osdp-cli application where the text subcommand can be executed with various options that need to be parsed and handled appropriately.
 */
#ifndef TEXT_OPTIONS_H
#define TEXT_OPTIONS_H


// This structure is used to store the various options that can be provided for the text subcommand in the osdp-cli application
// It includes fields for the port, baud rate, device address, security key for encrypting data and all parameters specific to text management
struct text_options
{
    const char *port;          // Serial port to use for sniffing, e.g., "/dev/ttyUSB0" on Linux or "COM3" on Windows
    const char *baud;          // Baud rate for the serial communication, e.g., "9600", "115200", etc
    int addr;                  // Device address for the OSDP device to query

    int scbk_d;                // Flag to indicate whether to use the default SCBK key for secure channel establishment, if applicable
    unsigned char scbk[16];    // Security Control Block Key (SCBK) for decrypting sniffed data, if applicable
    int scbk_set;              // Flag to indicate if the SCBK key has been set by the user

    int row;           // The row where the first character will be displayed
    int column;        // The column where the first character will be displayed
    const char *msg;   // Text to display on the PD's screen, if applicable
    int msg_set;       // Flag to indicate if the msg field has been set by the user
    int clear;         // Flag to indicate whether to clear the screen before displaying the message
};

struct cli_context;         // forward-declare opaque command context

// Function for parsing the "text" subcommand based on the provided command line options and possible options.
int text_parse(int argc, char **argv, struct text_options *opts);

// Function for executing the text subcommand based on the parsed options and the CLI context.
int text_run(struct cli_context *ctx);
// Display the parsed options for the text subcommand (for testing purposes)
int text_test(struct cli_context *ctx);

#endif // TEXT_OPTIONS_H