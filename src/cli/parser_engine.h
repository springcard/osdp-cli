/**
 * @file parser_engine.h
 * @brief Header file for command-line options parser for CLI commands
 * 
 * The code provided is a header file for a command-line options parser used in CLI commands. It defines the structures and function prototypes necessary for parsing command-line options, handling 
 * different types of options, and printing help messages for available options.
 * The `cli_option` structure defines the properties of a command-line option, including its name, type, value pointer, size, presence indicator, enumeration values (if applicable), and help text.
 * The function prototypes include `parse_command_line_options` for parsing the command-line options based on the provided options and their types, and `print_command_line_options` for printing the 
 * available options and their descriptions for help messages.
 * The code is designed to be used in a CLI application where commands can have various options that need to be parsed and handled appropriately.
 */
#ifndef PARSER_ENGINE_H
#define PARSER_ENGINE_H


enum cli_opt_type
{
    CLI_OPT_FLAG,
    CLI_OPT_INT,
    CLI_OPT_STRING,
    CLI_OPT_HEX,
    CLI_OPT_ENUM
};


// Structure to define a command-line option of enum type with a name and corresponding integer value.
struct cli_enum
{
    const char *name;
    int value;
};


// The `struct cli_option` is defining the properties of a command-line option. 
// - `name`: A string representing the name of the command-line option (e.g., "--help", "-v").
// - `type`: An enumeration value of type `cli_opt_type` that indicates the type of the option (e.g., flag, integer, string, hexadecimal, or enumeration).
// - `value`: A pointer to a variable where the parsed value of the option will be stored. The type of the variable pointed to depends on the `type` of the option.
// - `size`: An integer that is optional and used for options of type `CLI_OPT_HEX` to specify the expected size of the hexadecimal value. If not applicable, it can be set to 0.
// - `present`: An optional pointer to an integer that will be set to 1 if the option is present in the command-line arguments, allowing the program to distinguish between default values and user-provided values.
// - `enum_values`: For options of type `CLI_OPT_ENUM`, this is a pointer to an array of `cli_enum` structures that define the possible enumeration values for the option. If not applicable, it can be set to NULL.
// - `help`: A string containing the help text or description for the option, which can be used when printing help messages to the user.
struct cli_option
{
    const char *name;
    enum cli_opt_type type;

    void *value;
    int size;       
    int *present;   

    const struct cli_enum *enum_values; // For CLI_OPT_ENUM type, array of possible enum values -> find and parse the enum value based on the provided string, otherwise ignored (NULL).

    const char *help;
};

// Function for command-line option parsing
int parse_command_line_options(int argc, char **argv, struct cli_option *opts, int opt_count);
/* Let command parsers recognize global verbosity without duplicating options. */
void cli_set_verbose_target(int *verbose);

// Function for command-line option printing (for help messages)
void print_command_line_options( struct cli_option *opts, int opt_count);
        
#endif // PARSER_ENGINE_H
