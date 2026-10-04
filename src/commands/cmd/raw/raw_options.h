/**
 * @file raw_options.h
 * @brief Header file for raw subcommand options
 * 
 * The code provided defines the structures and function prototypes necessary for parsing and handling the options related to the raw subcommand.
 * The `raw_options` structure contains various fields that represent the options for the raw subcommand,
 * such as the serial port, baud rate, device address, and security key for encrypting data if necessary as well as the parameters specific to send a raw OSDP message.
 * The function prototypes include `raw_parse` for parsing the command-line options specific to the raw subcommand and `raw_run` for executing the raw subcommand based on the parsed options and the CLI context.
 * This header file is designed to be used in osdp-cli application where the raw subcommand can be executed with various options that need to be parsed and handled appropriately.
 */
#ifndef RAW_OPTIONS_H
#define RAW_OPTIONS_H


// This structure is used to store the various options that can be provided for the raw subcommand in the osdp-cli application
// It includes fields for the port, baud rate, device address, security key for encrypting data and all parameters specific to send a raw OSDP message
struct raw_options
{
    const char *port;          // Serial port to use for sniffing, e.g., "/dev/ttyUSB0" on Linux or "COM3" on Windows
    const char *baud;          // Baud rate for the serial communication, e.g., "9600", "115200", etc
    int addr;                  // Device address for the OSDP device to query

    int scbk_d;                // Flag to indicate whether to use the default SCBK key for secure channel establishment, if applicable
    unsigned char scbk[16];    // Security Control Block Key (SCBK) for decrypting sniffed data, if applicable
    int scbk_set;              // Flag to indicate if the SCBK key has been set by the user

    const unsigned char *cmd;   // Raw OSDP message to send, represented as a hex string (e.g., "6F 00 00 00 00 00 00 00")
    int cmd_len;                // Length of the raw OSDP message
    const unsigned char *data;  // Optional data payload for the OSDP message, represented as a hex string (e.g., "01 02 03 04")
    int data_len;               // Length of the data payload
};

struct cli_context;         // forward-declare opaque command context

// Function for parsing the "raw" subcommand based on the provided command line options and possible options.
int raw_parse(int argc, char **argv, struct raw_options *opts);

// Function for executing the raw subcommand based on the parsed options and the CLI context.
int raw_run(struct cli_context *ctx);
// Display the parsed options for the raw subcommand (for testing purposes)
int raw_test(struct cli_context *ctx);

#endif // RAW_OPTIONS_H