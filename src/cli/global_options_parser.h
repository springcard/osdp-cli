/**
 * @file global_options_parser.c
 * @brief Implements the CLI parser for global options
 * 
 * The code provided implements a parser for global command line options for the CLI application.
 * The parser is responsible for parsing global command-line options, handling help and version requests, and preparing a context for the command to be executed. 
 * It processes various global options such as verbosity, output format, timeout, retries, logging, and more.
 * The parser updates a `cli_context` structure based on the parsed options and returns the index of the command in the input arguments for further processing by the command dispatcher.
 */

#ifndef GLOBAL_OPTIONS_PARSER_H
#define GLOBAL_OPTIONS_PARSER_H

#include "../commands/cmd/buzzer/buzzer_options.h"
#include "../commands/cmd/led/led_options.h"
#include "../commands/cmd/raw/raw_options.h"
#include "../commands/cmd/relay/relay_options.h"
#include "../commands/cmd/text/text_options.h"
#include "../commands/config/config_options.h"
#include "../commands/cp/cp_options.h"
#include "../commands/file/put/put_options.h"
#include "../commands/info/info_options.h"
#include "../commands/probe/probe_options.h"
#include "../commands/sckmgmt/sckmgmt_options.h"
#include "../commands/sniff/sniff_options.h"
#include "parser_engine.h"

// This structure is used to define the log format for logging in the CLI application. 
// It includes different log formats such as text, JSON, and PCAPNG.
//enum global_log_format
enum log_formats
{
    LOG_TEXT = 0,
    LOG_JSON,
    LOG_PCAPNG
};


// The `global_options` structure is used to store the various parsed global options provided by the user through the command line. 
// It includes fields for verbosity, output format, timeout, retries, logging options, and more. 
// This structure allows the CLI application to maintain a centralized context that can be accessed by different parts of the application as needed.
struct global_options
{
    int version;    // 
    int verbose;    // Increase verbosity (repeatable)
    int quiet;      // Reduce output

    int json;       // Output machine-readable JSON (when applicable)
    int no_color;   // Disable ANSI colors

    int timeout;    // Default command timeout (ms). Default: 1000
    int retries;    // Default command retries. Default: 3    

    const char *log_file_name;     // File name for logging
    enum log_formats log_format;   // The log format (text, json, pcapng)
    int timestamp;                 // Include timestamp in beginning of logs
    int sc_params;                 // Display and log sc params session

};


// The `cli_context` structure is used to store the various parsed globale options provided by the user through the command line. 
// It includes fields for verbosity, output format, timeout, retries, logging options, and more. 
// This structure allows the CLI application to maintain a centralized context that can be accessed by different parts of the application as needed.
struct cli_context
{
    struct global_options glob_options;
    union {
         struct buzzer_options buzzer_opts;
         struct led_options led_opts;
         struct raw_options raw_opts;
         struct relay_options relay_opts;
         struct text_options text_opts;                
         struct config_options config_opts;    
         struct cp_options cp_opts;    
         struct put_options put_opts;
         struct info_options info_opts;
         struct probe_options probe_opts;
         struct sckmgmt_options sckmgmt_opts;
         struct sniff_options sniff_opts;
    } ;    
};


// Function for parsing global command line options and updating the CLI context.
int cli_parse_global_options(int argc, char **argv, struct cli_context *ctx, int *cmd_index);
// Function inspects the CLI context structure and prints the current state of all global options that were parsed from the command line.
int cli_global_options_test(struct cli_context *ctx);
// Function to display help messages for global command-line options.
void print_globale_line_options(struct cli_option *opts, int opt_count);

#endif // GLOBAL_OPTIONS_PARSER_H
