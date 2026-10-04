/**
 * @file led_options.h
 * @brief Header file for led subcommand options
 * 
 * The code provided defines the structures and function prototypes necessary for parsing and handling the options related to the led subcommand.
 * The `led_options` structure contains various fields that represent the options for the led subcommand,
 * such as the serial port, baud rate, device address, and security key for encrypting data if necessary as well as the parameters specific to LED management.
 * The function prototypes include `led_parse` for parsing the command-line options specific to the led subcommand and `led_run` for executing the led subcommand based on the parsed options and the CLI context.
 * This header file is designed to be used in osdp-cli application where the led subcommand can be executed with various options that need to be parsed and handled appropriately.
 */
#ifndef LED_OPTIONS_H
#define LED_OPTIONS_H

enum led_colors 
{ 
    LED_OFF=0,
    LED_RED, 
    LED_GREEN, 
    LED_AMBER, 
    LED_BLUE, 
    LED_WHITE=7, 
    LED_COLOR_NONE = -1
}; // Color to set the LED to, if applicable


enum led_modes 
{ 
    LED_MODE_OFF=0, 
    LED_MODE_ON, 
    LED_MODE_BLINK, 
    LED_MODE_TEMP,
    LED_MODE_NONE = -1
} ; // Mode to set the LED to (e.g., on, off, blink,...), if applicable


// This structure is used to store the various options that can be provided for the led subcommand in the osdp-cli application
// It includes fields for the port, baud rate, device address, security key for encrypting data and all parameters specific to LED management
struct led_options
{
    const char *port;          // Serial port to use for sniffing, e.g., "/dev/ttyUSB0" on Linux or "COM3" on Windows
    const char *baud;          // Baud rate for the serial communication, e.g., "9600", "115200", etc
    int addr;                  // Device address for the OSDP device to query

    int scbk_d;                // Flag to indicate whether to use the default SCBK key for secure channel establishment, if applicable
    unsigned char scbk[16];    // Security Control Block Key (SCBK) for decrypting sniffed data, if applicable
    int scbk_set;              // Flag to indicate if the SCBK key has been set by the user

    int led_number;           // LED ID to control
    enum led_colors led_color; // Color to set the LED to
    enum led_modes led_mode;   // Mode to set the LED to (e.g., on, off, blink,...)

    int onTime;      // Time in milliseconds for which the LED should be turned on.
    int offTime;     // Time in milliseconds for which the LED should be turned off
    int duration;    // Total duration in seconds for which the LED should be controlled, if applicable
};

struct cli_context;         // forward-declare opaque command context

// Function for parsing the "led" subcommand based on the provided command line options and possible options.
int led_parse(int argc, char **argv, struct led_options *opts);

// Function for executing the led subcommand based on the parsed options and the CLI context.
int led_run(struct cli_context *ctx);
// Display the parsed options for the led subcommand (for testing purposes)
int led_test(struct cli_context *ctx);

#endif // LED_OPTIONS_H