/**
 * @file relay_options.h
 * @brief Header file for relay subcommand options
 * 
 * The code provided defines the structures and function prototypes necessary for parsing and handling the options related to the relay subcommand.
 * The `relay_options` structure contains various fields that represent the options for the relay subcommand,
 * such as the serial port, baud rate, device address, and security key for encrypting data if necessary as well as the parameters specific to relay outputs management.
 * The function prototypes include `relay_parse` for parsing the command-line options specific to the relay subcommand and `relay_run` for executing the relay subcommand based on the parsed options and the CLI context.
 * This header file is designed to be used in osdp-cli application where the relay subcommand can be executed with various options that need to be parsed and handled appropriately.
 */
#ifndef RELAY_OPTIONS_H
#define RELAY_OPTIONS_H

enum relay_modes
{ 
    RELAY_MODE_OFF=0,
    RELAY_MODE_ON,
//    RELAY_MODE_PULSE,
    RELAY_MODE_NONE = -1
}; // State to set the relay to, if applicable


// This structure is used to store the various options that can be provided for the relay subcommand in the osdp-cli application
// It includes fields for the port, baud rate, device address, security key for encrypting data and all parameters specific to relay outputs management
struct relay_options
{
    const char *port;          // Serial port to use for sniffing, e.g., "/dev/ttyUSB0" on Linux or "COM3" on Windows
    const char *baud;          // Baud rate for the serial communication, e.g., "9600", "115200", etc
    int addr;                  // Device address for the OSDP device to query

    int scbk_d;                // Flag to indicate whether to use the default SCBK key for secure channel establishment, if applicable
    unsigned char scbk[16];    // Security Control Block Key (SCBK) for decrypting sniffed data, if applicable
    int scbk_set;              // Flag to indicate if the SCBK key has been set by the user

    int output_number;             // Relay output ID to control
    enum relay_modes relay_mode;   // Mode to set the relay to (e.g., on, off, pulse,...)

    int duration;    // Total duration in seconds for which the relay should be controlled, if applicable
};

struct cli_context;         // forward-declare opaque command context

// Function for parsing the "relay" subcommand based on the provided command line options and possible options.
int relay_parse(int argc, char **argv, struct relay_options *opts);

// Function for executing the relay subcommand based on the parsed options and the CLI context.
int relay_run(struct cli_context *ctx);
// Display the parsed options for the relay subcommand (for testing purposes)
int relay_test(struct cli_context *ctx);

#endif // RELAY_OPTIONS_H