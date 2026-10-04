/**
 * @file config_options.h
 * @brief Header file for config command options
 * 
 * The code provided defines the structures and function prototypes necessary for parsing and handling the options related to the config command.
 * The `config_options` structure contains various fields that represent the options for the config command,
 * such as the serial port, baud rate, device address, and security key for encrypting data if necessary.
 * The function prototypes include `config_parse` for parsing the command-line options specific to the info command and `config_run` for executing the info command based on the parsed options and the CLI context.
 * This header file is designed to be used in osdp-cli application where the info command can be executed with various options that need to be parsed and handled appropriately.
 */
#ifndef CONFIG_OPTIONS_H
#define CONFIG_OPTIONS_H

// This structure is used to store the various options that can be provided for the config command in the osdp-cli application.
// It includes fields for the port, baud rate, device address, and security key for encrypting data if necessary.
struct config_options
{
    const char *port;       // Serial port to use for sniffing, e.g., "/dev/ttyUSB0" on Linux or "COM3" on Windows.
    const char *baud;       // Baud rate for the serial communication, e.g., "9600", "115200", etc.
    int addr;               // Device address for the OSDP device to query.

    int scbk_d;             // Flag to indicate whether to use the default SCBK key for secure channel establishment, if applicable.
    unsigned char scbk[16]; // Security Control Block Key (SCBK) for decrypting sniffed data, if applicable.
    int scbk_set;           // Flag to indicate if the SCBK key has been set by the user

    int new_addr;           // New device address for the OSDP device, if applicable.
    int new_addr_set;       // Flag to indicate if the new device address has been set by the user

    unsigned int new_baud;           // New baud rate for the serial communication, if applicable.
    int new_baud_set;       // Flag to indicate if the new baud rate has been set by the user

    int reconnect;          // Flag to indicate if the tool should attempt to reconnect to the device after changing the address or baud rate, if applicable.
};

struct cli_context;         // forward-declare opaque command context

// Function for parsing the "config" command based on the provided command line options and possible options.
int config_parse(int argc, char **argv, struct config_options *opts);

// Function for executing the config command based on the parsed options and the CLI context.
int config_run(struct cli_context *ctx);
// Display the parsed options for the config command (for testing purposes)
int config_test(struct cli_context *ctx);

#endif // CONFIG_OPTIONS_H