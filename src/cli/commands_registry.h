/**
 * @file commands_registry.h
 * @brief Header file for CLI command registry  
 * 
 * The code provided is a header file for a CLI command registry. It defines the structures and function prototypes necessary for registering commands in CLI application, accessing the root commands, 
 * and counting the number of registered commands. The `cli_command` structure represents a command in the CLI, including its name, help text, handler function, and any child commands it may have. 
 * The `REGISTER_COMMAND` macro is used to simplify the registration of commands by automatically calling the registration function at program initialization. 
 */
#ifndef COMMANDS_REGISTRY_H
#define COMMANDS_REGISTRY_H

struct cli_context; // forward-declare opaque command context

// The `cli_handler` type is a function pointer that defines the signature of command handler functions in the CLI application. 
// It specifies that a command handler function should take a pointer to a `cli_context` structure, an integer representing the argument count (`argc`), 
// and an array of strings representing the argument values (options command) (`argv`). The function should return an integer status code, where typically `0` 
// indicates success and non-zero values indicate different types of errors or statuses.
typedef int (*cli_handler)(
        struct cli_context *ctx,
        int argc,
        char **argv);


// The `cli_command` structure represents a command in the CLI application. 
// This structure allows for a hierarchical organization of commands in the CLI, where a command can have multiple subcommands
// and each command/subcommand can have its own handler function to execute specific logic when the command is invoked. 
struct cli_command
{
    const char *name;                 // Name of the command or subcommand, used to identify the command in the CLI
    const char *help;                 // Help string that provides a description of the command, which can be displayed in the CLI when users request help or information about the command

    cli_handler handler;              // Function pointer for the command handler, which is called when the command is executed

    const char *parent;               // Pointer to the parent command, if any (subcommands)

    struct cli_command **subcommands; // Pointer to an array of pointers to subcommands that belong to this command, if any
    int subcommands_count;            // Number of sub-commands
};


// The function is used to register a command in the command registry of a CLI application. 
// It takes a pointer to a `cli_command` structure as its argument, which represents the command to be registered.
void cli_register_command(struct cli_command *cmd);

// Function to get access of the command registry of a CLI application.
struct cli_command **cli_get_command_tree(void);

// Function to get the count of commands contained in the command registry CLI application.
int cli_get_command_count(void);

// Function to build the command tree for the CLI application. This function is responsible for constructing the hierarchical structure of commands and subcommands in the CLI, 
// allowing for organized command management and execution. It typically involves defining the commands, their relationships (parent-child), and registering them in the command registry.
void cli_build_command_tree(void);


// Macro used to simplify the registration of commands in the CLI application. It defines a static function that is automatically called at program initialization (before main()) to register the command 
// in the CLI command registry. The macro takes a command structure as an argument and generates a function that calls `cli_register_command` with the provided command structure. 
// This allows for easy registration of commands without needing to manually call the registration function in the main program. 
#ifdef _MSC_VER
#pragma section(".CRT$XCU", read)
#if defined(__clang__)
#define CLI_REGISTRY_USED __attribute__((used))
#else
#define CLI_REGISTRY_USED
#endif
#define REGISTER_COMMAND(cmd) \
    static void __register_##cmd(void); \
    __declspec(allocate(".CRT$XCU")) static void (*__register_##cmd##_initializer)(void) CLI_REGISTRY_USED = __register_##cmd; \
    static void __register_##cmd(void) \
    { \
        cli_register_command(&cmd); \
    }
#else
#define REGISTER_COMMAND(cmd) \
    static void __register_##cmd(void) __attribute__((constructor)); \
    static void __register_##cmd(void) \
    { \
        cli_register_command(&cmd); \
    }
#endif

#endif // COMMANDS_REGISTRY_H
