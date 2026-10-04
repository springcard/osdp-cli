/**
 * @file cp_options.h
 * @brief Header file for cp command options
 * 
 * The code provided defines the structures and function prototypes necessary for parsing and handling the options related to the cp command.
 * The `cp_options` structure contains various fields that represent the options for the cp command,
 * such as the serial port, baud rate, device address, and security keys (SCBK and SCSK) for encrypting data if necessary.
 * The function prototypes include `cp_parse` for parsing the command-line options specific to the cp command and `cp_run` for executing the cp command based on the parsed options and the CLI context.
 * This header file is designed to be used in osdp-cli application where the cp command can be executed with various options that need to be parsed and handled appropriately.
 */
#ifndef CP_OPTIONS_H
#define CP_OPTIONS_H

// This structure is used to store the various options that can be provided for the cp command in the osdp-cli application
// It includes fields for the port, baud rate, device address, and security keys (SCBK and SCSK) for encrypting data if necessary
struct cp_options
{
    const char *port;       // Serial port to use for sniffing, e.g., "/dev/ttyUSB0" on Linux or "COM3" on Windows
    const char *baud;       // Baud rate for the serial communication, e.g., "9600", "115200", etc
    int addr;       // Device address for the OSDP device to query

    int scbk_d;             // Flag to indicate whether to use the default SCBK key for secure channel establishment, if applicable
    unsigned char scbk[16]; // Security Control Block Key (SCBK) for decrypting sniffed data, if applicable
    int scbk_set;           // Flag to indicate if the SCBK key has been set by the user

    int single;   // Flag to indicate a query a device and exit after the first successful query (instead of continuously querying device until a timer interrupts the process)
    int addr_all; // Flag to indicate whether to query all addresses from 0 to 126 (if not set, only the address specified in the addr field will be queried)
    int delay;    // Delay in milliseconds between queries when querying all addresses (if addr_all is set)
};

struct cli_context;         // forward-declare opaque command context

// Function for parsing the "cp" command based on the provided command line options and possible options.
int cp_parse(int argc, char **argv, struct cp_options *opts);

// Function for executing the cp command based on the parsed options and the CLI context.
int cp_run(struct cli_context *ctx);
// Display the parsed options for the cp command (for testing purposes)
int cp_test(struct cli_context *ctx);

#endif // CP_OPTIONS_H