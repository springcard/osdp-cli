/**
 * @file sckmgmt_options.h
 * @brief Header file for sckmgmt command options
 * 
 * The code provided defines the structures and function prototypes necessary for parsing and handling the options related to the sckmgmt command.
 * The `sckmgmt_options` structure contains various fields that represent the options for the sckmgmt command,
 * such as the serial port, baud rate, device address, and security keys (SCBK and SCSK, new SCBK) for encrypting data if necessary.
 * The function prototypes include `sckmgmt_parse` for parsing the command-line options specific to the sckmgmt command and `sckmgmt_run` for executing the sckmgmt command based on the parsed options and the CLI context.
 * This header file is designed to be used in osdp-cli application where the sckmgmt command can be executed with various options that need to be parsed and handled appropriately.
 */
#ifndef SCKMGT_OPTIONS_H
#define SCKMGT_OPTIONS_H

// This structure is used to store the various options that can be provided for the sckmgmt command in the osdp-cli application.
// It includes fields for the port, baud rate, device address, and security keys (SCBK and SCSK) for encrypting data if necessary.
struct sckmgmt_options
{
    const char *port;       // Serial port to use for sniffing, e.g., "/dev/ttyUSB0" on Linux or "COM3" on Windows.
    const char *baud;       // Baud rate for the serial communication, e.g., "9600", "115200", etc.
    int addr;               // Device address for the OSDP device to query.
    
    int scbk_d;             // Flag to indicate whether to use the default SCBK key for secure channel establishment, if applicable.
    unsigned char scbk[16]; // Security Control Block Key (SCBK) for decrypting sniffed data, if applicable.
    int scbk_set;           // Flag to indicate if the SCBK key has been set by the user

    unsigned char scsk[16]; // Security Control Session Key (SCSK) for decrypting sniffed data, if applicable.
    int scsk_set;           // Flag to indicate if the SCSK key has been set by the user

    unsigned char new_scbk[16]; // New Security Control Block Key (SCBK) for updating the device, if applicable.
    int new_scbk_set;       // Flag to indicate if the new SCBK key has been set by the user

    int verify;            // Flag to indicate if the new SCBK should be verified after update
};

struct cli_context;         // forward-declare opaque command context

// Function for parsing the "sniff" command based on the provided command line options and possible options.
int sckmgmt_parse(int argc, char **argv, struct sckmgmt_options *opts);

// Function for executing the sniff command based on the parsed options and the CLI context.
int sckmgmt_run(struct cli_context *ctx);
// Display the parsed options for the sckmgmt command (for testing purposes)
void sckmgmt_clear_options(struct sckmgmt_options *opts);

#endif // SCKMGT_OPTIONS_H
