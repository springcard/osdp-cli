/**
 * @file put_options.h
 * @brief Header file for put subcommand options
 * 
 * The code provided defines the structures and function prototypes necessary for parsing and handling the options related to the put subcommand.
 * The `put_options` structure contains various fields that represent the options for the put subcommand,
 * such as the serial port, baud rate, device address, and security key for encrypting data if necessary as well as the parameters specific to put file management.
 * The function prototypes include `put_parse` for parsing the command-line options specific to the put subcommand and `put_run` for executing the put subcommand based on the parsed options and the CLI context.
 * This header file is designed to be used in osdp-cli application where the put subcommand can be executed with various options that need to be parsed and handled appropriately.
 */
#ifndef PUT_OPTIONS_H
#define PUT_OPTIONS_H

enum put_types 
{ 
    PUT_TYPE_CONFIG = 0,
    PUT_TYPE_FIRMWARE,
    PUT_TYPE_BLOB, 
    PUT_TYPE_NONE = -1 
}; // Types of put operations


enum put_hash_algorithms
{
    PUT_HASH_NONE = 0,
    PUT_HASH_MD5,
    PUT_HASH_SHA1,
    PUT_HASH_SHA256
}; // Hash algorithms to use for integrity verification purposes, if applicable 


// This structure is used to store the various options that can be provided for the put subcommand in the osdp-cli application
// It includes fields for the port, baud rate, device address, security key for encrypting data and all parameters specific to put file management
struct put_options
{
    const char *port;          // Serial port to use for sniffing, e.g., "/dev/ttyUSB0" on Linux or "COM3" on Windows
    const char *baud;          // Baud rate for the serial communication, e.g., "9600", "115200", etc
    int addr;                  // Device address for the OSDP device to query

    int scbk_d;                // Flag to indicate whether to use the default SCBK key for secure channel establishment, if applicable
    unsigned char scbk[16];    // Security Control Block Key (SCBK) for decrypting sniffed data, if applicable
    int scbk_set;              // Flag to indicate if the SCBK key has been set by the user

    enum put_types type;       // Type of put operation to perform (e.g., put configuration, put blob,...)
    
    const char *input_file;    // Path to the input file containing data to be sent
    int input_file_set;        // Flag to indicate if the input_file field has been set

    //todo demande precisions enum put_hash_algorithms hash;  // Hash algorithm to use for integrity verification purposes (if applicable)

    int chunk;     // Size in bytes of each chunk of data to retrieve in each request

    int resume;    // Flag to indicate whether to resume an interrupted get operation by checking the existing output file and retrieving only the remaining data (if applicable)
    int reboot;    // Flag to indicate whether to reboot the device after the get operation is completed (if applicable)
    int progress;  // Flag to indicate whether to display a progress bar in the console during the retrieval process (if the process is expected to take a significant amount of time)    
    int pause;     // Delay in milliseconds between each frame 
};

struct cli_context;         // forward-declare opaque command context

// Function for parsing the "put" subcommand based on the provided command line options and possible options.
int put_parse(int argc, char **argv, struct put_options *opts);

// Function for executing the put subcommand based on the parsed options and the CLI context.
int put_run(struct cli_context *ctx);
// Display the parsed options for the put subcommand (for testing purposes)
int put_test(struct cli_context *ctx);

#endif // PUT_OPTIONS_H