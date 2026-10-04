/**
 * @file commands_dispatch.h
 * @brief Header file for CLI command dispatcher
 * 
 * The code provided is a header file for a CLI command dispatcher. It defines the function prototype for dispatching a command in the CLI application. The dispatcher is responsible for routing the 
 * command execution to the appropriate handler function based on the command entered by the user.
 * The function `cli_dispatch_command` takes the CLI context, an array of available commands, the number of available commands, and the command-line arguments for the command to be executed. It returns an 
 * integer status code indicating success or error.
 */
#ifndef COMMANDS_DISPATCH_H
#define COMMANDS_DISPATCH_H

#include "global_options_parser.h"
//struct cli_context;
struct cli_command; // forward-declare opaque command context

// Function prototype for dispatching a command in the CLI application. 
int cli_dispatch_command(struct cli_context *ctx, struct cli_command **commands, int command_count, int argc, char **argv);

#endif // COMMANDS_DISPATCH_H