/**
 * @file info_options.h
 * @brief Header file for info command options
 * 
 * The code provided defines the structures and function prototypes necessary for parsing and handling the options related to the info command.
 * The `info_options` structure contains various fields that represent the options for the info command,
 * such as the serial port, baud rate, device address, and security keys (SCBK and SCSK) for encrypting data if necessary.
 * The function prototypes include `info_parse` for parsing the command-line options specific to the info command and `info_run` for executing the info command based on the parsed options and the CLI context.
 * This header file is designed to be used in osdp-cli application where the info command can be executed with various options that need to be parsed and handled appropriately.
 */
#ifndef INFO_OPTIONS_H
#define INFO_OPTIONS_H

// This structure is used to store the various options that can be provided for the info command in the osdp-cli application.
// It includes fields for the port, baud rate, device address, and security keys (SCBK and SCSK) for encrypting data if necessary.
struct info_options
{
    const char *port;       // Serial port to use for sniffing, e.g., "/dev/ttyUSB0" on Linux or "COM3" on Windows.
    const char *baud;       // Baud rate for the serial communication, e.g., "9600", "115200", etc.
    int addr;       // Device address for the OSDP device to query.

    int scbk_d;             // Flag to indicate whether to use the default SCBK key for secure channel establishment, if applicable.
    unsigned char scbk[16]; // Security Control Block Key (SCBK) for decrypting sniffed data, if applicable.
    int scbk_set;           // Flag to indicate if the SCBK key has been set by the user

    unsigned char scsk[16]; // Security Control Session Key (SCSK) for decrypting sniffed data, if applicable.
    int scsk_set;           // Flag to indicate if the SCSK key has been set by the user
};

struct cli_context;         // forward-declare opaque command context

// Function for parsing the "sniff" command based on the provided command line options and possible options.
int info_parse(int argc, char **argv, struct info_options *opts);

// Function for executing the sniff command based on the parsed options and the CLI context.
int info_run(struct cli_context *ctx);
// Display the parsed options for the info command (for testing purposes)
int info_test(struct cli_context *ctx);

#endif // INFO_OPTIONS_H