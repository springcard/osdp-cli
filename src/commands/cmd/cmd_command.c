/**
 * @file cmd_command.c
 * @brief Implementation of the "cmd" command allowing send a single control command as CP
 * 
 * The code provided implements the "cmd" command for the osdp-cli application, which is used for sending a single control command like operational subcommands led, buzzer, relay, text, etc.
 * The implementation also includes the registration of the cmd command in the CLI command registry, allowing it to be recognized and executed when the user enters the "cmd" command in the CLI.
  */
#include <stddef.h>

#include "../../cli/commands_registry.h"



// Define the cmd command 
// The `cli_command` structure includes the command name, help description, handler function, and information about subcommands
static struct cli_command cmd_cmd = {
    .name = "cmd",
    .help = "Send operational commands to OSDP devices",
    .handler = NULL,
    .parent = NULL,
    .subcommands = NULL,
    .subcommands_count = 0
};

// Register the cmd command in the CLI command registry, allowing it to be recognized and executed when the user enters the "cmd" command in the CLI.
REGISTER_COMMAND(cmd_cmd);