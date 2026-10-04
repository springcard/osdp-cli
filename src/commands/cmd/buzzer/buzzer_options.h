/**
 * @file buzzer_options.h
 * @brief Header file for buzzer subcommand options
 * 
 * The code provided defines the structures and function prototypes necessary for parsing and handling the options related to the buzzer subcommand.
 * The `buzzer_options` structure contains various fields that represent the options for the buzzer subcommand,
 * such as the serial port, baud rate, device address, and security key for encrypting data if necessary as well as the parameters specific to buzzer management.
 * The function prototypes include `buzzer_parse` for parsing the command-line options specific to the buzzer subcommand and `buzzer_run` for executing the buzzer subcommand based on 
 * the parsed options and the CLI context.
 * This header file is designed to be used in osdp-cli application where the buzzer subcommand can be executed with various options that need to be parsed and handled appropriately.
 */
#ifndef BUZZER_OPTIONS_H
#define BUZZER_OPTIONS_H


enum buzzer_modes 
{ 
    BUZZER_MODE_OFF=0, 
    BUZZER_MODE_ON, 
    BUZZER_MODE_BEEP,
    BUZZER_MODE_PATTERN,
    BUZZER_MODE_NONE = -1
} ; // Mode to set the buzzer to (e.g., on, off, beep,...), if applicable


// This structure is used to store the various options that can be provided for the buzzer subcommand in the osdp-cli application
// It includes fields for the port, baud rate, device address, security key for encrypting data and all parameters specific to buzzer management
struct buzzer_options
{
    const char *port;          // Serial port to use for sniffing, e.g., "/dev/ttyUSB0" on Linux or "COM3" on Windows
    const char *baud;          // Baud rate for the serial communication, e.g., "9600", "115200", etc
    int addr;                  // Device address for the OSDP device to query

    int scbk_d;                // Flag to indicate whether to use the default SCBK key for secure channel establishment, if applicable
    unsigned char scbk[16];    // Security Control Block Key (SCBK) for decrypting sniffed data, if applicable
    int scbk_set;              // Flag to indicate if the SCBK key has been set by the user

    enum buzzer_modes buzzer_mode;   // Mode to set the buzzer to (e.g., on, off, blink,...)

    int onTime;      // Time in milliseconds for which the buzzer should be turned on.
    int offTime;     // Time in milliseconds for which the buzzer should be turned off
    int count;       // Total number of cycles for which the buzzer should be controlled
};

struct cli_context;         // forward-declare opaque command context

// Function for parsing the "buzzer" subcommand based on the provided command line options and possible options.
int buzzer_parse(int argc, char **argv, struct buzzer_options *opts);

// Function for executing the buzzer subcommand based on the parsed options and the CLI context.
int buzzer_run(struct cli_context *ctx);
// Display the parsed options for the buzzer subcommand (for testing purposes)
int buzzer_test(struct cli_context *ctx);

#endif // BUZZER_OPTIONS_H