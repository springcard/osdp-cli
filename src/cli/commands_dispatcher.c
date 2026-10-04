/**
 * @file commands_dispatcher.c
 * @brief Implements the CLI command dispatcher
 * @details The code provided implements a command dispatcher for the CLI application.
 * The dispatcher is responsible for routing the command execution to the appropriate handler function based on the command entered by the user.
 * It receives the CLI context, the command tree, the number of available commands, and the command-line arguments for the command to be executed.
 * The dispatcher traverses the command tree to find the matching command and executes its handler function if found. If the command has subcommands, it continues to traverse the tree until it reaches a leaf command with a handler.
 * If the command is not found or if there is an issue with the command configuration, it returns an error message.
*/
#include <stdio.h>
#include <string.h>

#include "commands_registry.h"
#include "commands_dispatcher.h"

static int is_help_arg(const char *arg)
{
    return (strcmp(arg, "--help") == 0) || (strcmp(arg, "-h") == 0);
}

static void print_subcommand_help(const struct cli_command *cmd)
{
    printf("\nUsage: osdp-cli %s <subcommand> [options]\n\n", cmd->name);
    printf("%s\n\n", cmd->help ? cmd->help : "Command group");
    printf("Subcommands:\n");

    for (int i = 0; i < cmd->subcommands_count; i++) {
        const struct cli_command *subcommand = cmd->subcommands[i];
        printf("  %-16s %s\n",
               subcommand->name,
               subcommand->help ? subcommand->help : "");
    }

    printf("\nUse `osdp-cli %s <subcommand> --help` to show subcommand options.\n\n", cmd->name);
}

/**
 * @brief Dispatch a command in the CLI
 * 
 * This function is responsible for dispatching a command in the CLI. It takes the CLI context, an array of available commands, the number of available commands, and the command-line arguments for 
 * the command to be executed.
 * It searches for the command in the array of available commands, and if found, it checks if the command has subcommands. If it does, it continues to dispatch the subcommand. If it doesn't, 
 * it executes the command's handler function.
 * If the command is not found or if there is an issue with the command arguments, it returns an error message. 
 * 
 * @param[in] ctx The CLI context
 * @param[in] commands The array of available commands
 * @param[in] command_count The number of available commands
 * @param[in] argc The argument count for the command to be executed
 * @param[in] argv The argument values for the command to be executed
 * 
 * @return 0 on success, 1 on error
 */
int cli_dispatch_command( struct cli_context *ctx, struct cli_command **commands, int command_count, int argc, char **argv)
{

    if (argc == 0) {
        fprintf(stderr, "Missing command. Type `osdp-cli --help` for more information.\n");
        return 1;
    }

    const char *name = argv[0];

    for (int i = 0; i < command_count; i++) {

        struct cli_command *cmd = commands[i];

        if (strcmp(name, cmd->name) != 0)
            continue;

        // if the command has subcommands, continue dispatching the subcommand
        if (cmd->subcommands && cmd->subcommands_count > 0) {

            if (argc == 1 || is_help_arg(argv[1])) {
                if (argc > 1) {
                    print_subcommand_help(cmd);
                    return 0;
                }
                fprintf(stderr, "Missing subcommand for '%s'\n", cmd->name);
                fprintf(stderr, "Type `osdp-cli %s --help` for the list of subcommands.\n", cmd->name);
                return 1;
            }

            return cli_dispatch_command(
                    ctx,
                    cmd->subcommands,
                    cmd->subcommands_count,
                    argc - 1,
                    &argv[1]);
        }

        /* Handler final */
        if (cmd->handler)
            return cmd->handler(ctx, argc - 1, &argv[1]);

        fprintf(stderr, "Invalid command configuration\n");
        return 1;
    }

    fprintf(stderr, "Unknown command: %s\n", name);
    return 1;
}
