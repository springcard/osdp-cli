/**
 * @file file_command.c
 * @brief Implementation of the "file" command allowing file transfer and firmware update procedure.
 * 
 * The code provided implements the "file" command for the osdp-cli application, which is used for transferring a file to the PD for advanced configuration or performing a 
 * firmware update procedure. It also used for getting a file from the PD.
 * The `cli_command` structure is defined to represent the "file" command, including its name, help description, handler function, and information about subcommands.
 * The implementation also includes the registration of the file command in the CLI command registry, allowing it to be recognized and executed when the user enters the "file" command in the CLI.
  */
#include <stddef.h>

#include "../../cli/commands_registry.h"



// Define the file command 
// The `cli_command` structure includes the command name, help description, handler function, and information about subcommands
static struct cli_command file_cmd = {
    .name = "file",
    .help = "Perform file transfer operation with OSDP device",
    .handler = NULL,
    .parent = NULL,
    .subcommands = NULL,
    .subcommands_count = 0
};

// Register the file command in the CLI command registry, allowing it to be recognized and executed when the user enters the "file" command in the CLI.
REGISTER_COMMAND(file_cmd);