/**
 * @file commands_registry.c
 * @brief Implementation of the CLI command registry
 * @details The code provided a command registry system for a Command Line Interface (CLI). 
 * The goal is to define a structure to manage the available commands in the CLI interface.
 * 
 * The code defines functions to register commands in a CLI command registry, retrieve available
 * commands, and get the count of registered commands.
 * 
 * The structure of type `cli_command` contains information about a specific command, such as its name, help text, handler function,
 * and any child commands it may have.
 */

#include <string.h>
#include <stdlib.h>

#include "commands_registry.h"

#define MAX_COMMANDS 64


/* array for hold pointers to data struct of available commands CLI */
static struct cli_command *command_tree[MAX_COMMANDS];

/* counter of available commands */
static int command_tree_count = 0;


/**
 * @brief Register a command in the CLI command registry
 * 
 * This function registers a command in an array of CLI commands if there is space available.
 * 
 * @param[in] cmd The parameter `cmd` is a pointer to a structure of type `cli_command`.
 * 
 * @return and does nothing if the MAX_COMMAND number is reached.
 */
void cli_register_command(struct cli_command *cmd)
{
    if (command_tree_count >= MAX_COMMANDS)
        return;

    command_tree[command_tree_count++] = cmd;
}


/**
 * @brief Returns the array of available commands in the CLI. 
 * 
 * This function provides access to the root commands that have been registered in the command registry. It returns a pointer to an array of pointers to `struct cli_command`, 
 * which represents the commands available in the CLI. Each element in the returned array points to a `struct cli_command` that contains information about a specific command,
 * such as its name, help text, handler function, and any child commands it may have.
 * 
 * @return A pointer to a pointer to a struct cli_command, which represents all commands in the command-line interface.
 */
struct cli_command **cli_get_command_tree(void)
{
    return command_tree;
}


/**
 * @brief Return the count of available commands in the CLI.
 *  
 * This function is returning the number of commands that have been registered in the command registry. 
 * It simply returns the value of the variable `command_tree_count`, which keeps track of the number of commands that have been registered using the `cli_register_command` function. 
 * This count can be used to determine how many commands are available in the CLI and to iterate through the array of commands returned by `cli_get_command_tree`.
 * 
 * @return Nombre de commandes
 */
int cli_get_command_count(void)
{
    return command_tree_count;
}


/**
 * @brief Build the command tree for the CLI application. 
 * 
 * This function is responsible for constructing the hierarchical structure of commands and subcommands in the CLI, allowing for organized command management and execution. 
 * It involves defining the commands, their relationships (parent-child), and registering them in the command registry.
 */
void cli_build_command_tree(void)
{
    for (int i = 0; i < command_tree_count; i++) {

        // First find a subcommand (child), in registered commands, that has a parent defined
        struct cli_command *subcommand = command_tree[i];

         if (!subcommand->parent)
            continue;   

        // Then find the parent command and add the child to the parent's children array.
        for (int j = 0; j < command_tree_count; j++) {

            struct cli_command *parent = command_tree[j];

            if (strcmp(parent->name, subcommand->parent) == 0) {

                parent->subcommands = realloc(
                    parent->subcommands,
                    sizeof(struct cli_command*) *
                    (parent->subcommands_count + 1));

                parent->subcommands[parent->subcommands_count++] = subcommand;
            }
        }
    }    
}