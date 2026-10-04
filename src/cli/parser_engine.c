/**
 * @file parser_engine.c
 * @brief Implements a command-line options parser for CLI commands
 * @details The code provided contains several functions related to parsing command-line options,
 * handling different types of options, and converting hexadecimal strings to bytes. 
 * The parser can handle various types of options and their values, including flags, integers, strings, hexadecimal values, and enumerations, with
 * error handling and help message printing capabilities.
 * 
 * A reference array of `struct cli_option` containing all available options for a command is provided.
 * Each `struct cli_option` defines a specific command-line possible option, including its name, type, value pointer, and
 * other relevant information needed for parsing and handling that option.
 * Each name of the option, in user command-line arguments, being searched for and parsed from reference array of `cli_option` structures.
 * 
 * The code provided contains several functions related to parsing command-line options,
 * handling different types of options, and converting hexadecimal strings to bytes.
 */
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <ctype.h>
#include <errno.h>
#include <limits.h>

#include "parser_engine.h"

static int hex_to_bytes(const char *hex, unsigned char *out, int out_len);
static int parse_enum(const char *str, const struct cli_enum *table, int *value);
static const char *option_value_hint(const struct cli_option *opt);
static void print_enum_values(const struct cli_enum *table);


/**
 * @brief Search for an option by name in an array of cli_option structures
 * 
 * This function searches for a specific option by name within an array of `cli_option` structures. 
 * It iterates through the array of options and compares the provided name with the name field of each option. 
 * If a match is found, it returns a pointer to the corresponding `cli_option` structure.
 * 
 * @param[in] name is a pointer to a character array representing the name of the option being searched for in the array of `cli_option` structures.
 * @param[in] opts is a pointer to an array of `cli_option` structures. Each `cli_option` structure
 * contains a `name` field which is a string representing the name of the option.
 * @param[in] count represents the number of elements in the `opts` array, which is the total number of `cli_option` structures in the array that we want
 * to search through.
 * 
 * @return If a match is found, it returns a pointer to the corresponding `cli_option` structure. 
 * If no match is found after iterating through the array, it returns `NULL` to indicate that the option was not found.
 */
static struct cli_option *find_option(const char *name, struct cli_option *opts, int count)
{
    for (int i = 0; i < count; i++) {

        if (strcmp(name, opts[i].name) == 0)
            return &opts[i];
    }

    return NULL;
}


/**
 * @brief Parse command-line options based on a given set of options and their types
 * 
 * This function parses command line options based on a given set of options and their types. 
 * It iterates through the command-line arguments, matches them against the provided options, and sets the corresponding values in the output variables. 
 * It handles different types of options such as flags, integers, strings, hexadecimal values, and enumerations.
 * It also provides error handling for unknown options, missing values, and invalid formats. 
 * If the help option is encountered, it returns a specific value to indicate that help information should be displayed.
 * 
 * @param argc `argc` is the argument count, which represents the number of arguments passed to the program from the command line, 
 * excluding the name of the program itself the globales options.
 * @param argv `argv` is a pointer to an array of strings representing the arguments
 * passed to the command. Each element of `argv` is a null-terminated C string. The first element
 * (`argv[0]`) is the name of command itself, and subsequent elements contain the arguments
 * provided by the user for that command, including any options and their values.
 * @param opts is a pointer to an array of `struct cli_option` elements this provided all available options for that command. 
 * The args provided by user are parse and match with each element of array. 
 * Each `struct cli_option` defines a specific command-line option, including its name, type, value pointer, 
 * and other relevant information needed for parsing and handling that option.
 * @param opt_count represents the total number of command options that are available for parsing. 
 * This value is used to search through when processing the command-line arguments provided to the command.
 * 
 * @return -1 if the help option `--help` or `-h` is encountered, indicating that the user requested help information.
 * It return -2 if option `--version` or `-V` is encountered, indicating that the user requested the osdp-cli version.
 * It returns -3 if there is an error such as an unknown option, missing value for an option, invalid
 * value for an option, or an unknown option type. If all options are successfully parsed without errors, it returns a positive 
 * value corresponding to the index of the last element of the command line processed.
 */
int parse_command_line_options(int argc, char **argv, struct cli_option *opts, int opt_count)
{
    int i;

    for (i = 0; i < argc; i++) {

        const char *arg = argv[i];

        if (arg[0] != '-') // First command encountered
            return i;

        // Handle option help for command, immediate return
        if (strcmp(arg, "--help") == 0 || strcmp(arg, "-h") == 0)
            return -1;

        // Handle option version for osdp-cli, immediate return
        if (strcmp(arg, "--version") == 0 || strcmp(arg, "-V") == 0)
            return -2;            

        struct cli_option *opt = find_option(arg, opts, opt_count);

        if (!opt) {
            fprintf(stderr, "Unknown option\n");
            return -3;
        }

        switch (opt->type) {

            case CLI_OPT_FLAG:

                *(int *)opt->value = 1;
                break;

            case CLI_OPT_INT:

                if (i + 1 >= argc) {
                    fprintf(stderr, "Missing value for %s\n", arg);
                    return -3;
                }

                const char *integer = argv[++i];
                char *end;
                errno = 0;
                long parsed = strtol(integer, &end, 10);
                if (errno || integer == end || *end || parsed < INT_MIN || parsed > INT_MAX) {
                    fprintf(stderr, "Invalid integer for %s\n", arg);
                    return -3;
                }
                *(int *)opt->value = (int)parsed;

                if (opt->present)
                    *opt->present = 1;
                break;

            case CLI_OPT_STRING:

                if (i + 1 >= argc) {
                    fprintf(stderr, "Missing value for %s\n", arg);
                    return -3;
                }

                *(const char **)opt->value = argv[++i];
                break;

            case CLI_OPT_HEX:
            
                if (i + 1 >= argc) {
                    fprintf(stderr, "Missing value for %s\n", arg);
                    return -3;
                }

                const char *hex = argv[++i];

                if(opt->size > 0)
                {
                    if (hex_to_bytes(hex, (unsigned char *)opt->value, opt->size) != 0) {

                        fprintf(stderr, "Invalid hex value for %s (expected %d bytes)\n", arg, opt->size);

                        return -3;
                    }
                    
                    if (opt->present)
                        *opt->present = 1;
                }
                else
                {
                    int hex_len = strlen(hex)/2; // Each byte is represented by 2 hex characters
                    if (opt->present)
                        *opt->present = hex_len ; // Set the presence to the number of bytes parsed from the hex string, which is half the length of the hex string

                    unsigned char *hex_value = (unsigned char *)malloc(hex_len); // Buffer to store the parsed hex value

                    if(!hex_value) {
                        fprintf(stderr, "Memory allocation failed for hex value of option %s\n", arg);
                        return -3;
                    }

                    // Convert the hex string to bytes and store it in the provided value pointer
                    if (hex_to_bytes(hex, hex_value, hex_len ) != 0) {

                        fprintf(stderr, "Invalid hex value for option %s (expected %d bytes)\n", arg, opt->size);
                        return -3;
                    }

                    *(const unsigned char **)opt->value = hex_value; // Assign the parsed hex value to the option's value pointer

                }

                break;

            case CLI_OPT_ENUM:

                if (i + 1 >= argc) {
                    fprintf(stderr, "Missing value for %s\n", arg);
                    return -3;
                }

                const char *val = argv[++i];

                if (parse_enum(val, opt->enum_values, (int *)opt->value) != 0) {

                    fprintf(stderr, "Invalid value for %s\n", arg);
                    return -3;
                }

                if (opt->present)
                    *(opt->present) = 1;
                break;

            default:
                fprintf(stderr, "Unknown option type for %s\n", arg);
                return -3;                
        }
    }

    return i;
}


/**
 * @brief Print command-line options for help messages
 * 
 * This function is responsible for printing the available command-line options along with their descriptions.
 * It takes an array of `cli_option` structures and the count of options as parameters.
 * 
 * @param[in] opts Array of cli_option structures to print contains the options to print
 * @param[in] opt_count Number of options in the array
 */
void print_command_line_options(struct cli_option *opts, int opt_count)
{
    printf("\nOptions:\n");
    printf("  %-24s %s\n", "-h, --help", "Display this help.");

    for (int i = 0; i < opt_count; i++) {
        char label[64];
        snprintf(label, sizeof(label), "%s%s%s",
                 opts[i].name,
                 option_value_hint(&opts[i])[0] != '\0' ? " " : "",
                 option_value_hint(&opts[i]));

        printf("  %-24s", label);

        if (opts[i].help || (opts[i].type == CLI_OPT_ENUM && opts[i].enum_values != NULL)) {
            printf("  ");
        }

        printf("%s",
               opts[i].help ? opts[i].help : "");

        if (opts[i].type == CLI_OPT_ENUM && opts[i].enum_values != NULL) {
            printf(" Values: ");
            print_enum_values(opts[i].enum_values);
            printf(".");
        }

        printf("\n");
    }
    printf("\n");
}

static const char *option_value_hint(const struct cli_option *opt)
{
    switch (opt->type) {
        case CLI_OPT_FLAG:
            return "";
        case CLI_OPT_INT:
            return "<int>";
        case CLI_OPT_STRING:
            if (strcmp(opt->name, "--port") == 0) {
                return "<port>";
            }
            if (strcmp(opt->name, "--baud") == 0) {
                return "<baud>";
            }
            if (strcmp(opt->name, "--log") == 0 ||
                strcmp(opt->name, "--out") == 0 ||
                strcmp(opt->name, "--in") == 0) {
                return "<file>";
            }
            return "<value>";
        case CLI_OPT_HEX:
            return opt->size > 0 ? "<hex16>" : "<hex>";
        case CLI_OPT_ENUM:
            return "<value>";
        default:
            return "";
    }
}

static void print_enum_values(const struct cli_enum *table)
{
    for (int i = 0; table[i].name != NULL; i++) {
        if (i > 0) {
            printf("|");
        }
        printf("%s", table[i].name);
    }
}


/**
 * @brief Convert a hexadecimal string to bytes
 * 
 * This function takes a hexadecimal string as input and converts it into its corresponding byte representation.
 * It first checks if the length of the hexadecimal string matches the expected length based on the output buffer size. 
 * If the lengths do not match, it returns an error.
 * Then, it iterates through the hexadecimal string in pairs of characters, converts each pair into a byte using `strtol`, 
 * and stores the result in the output buffer.
 * 
 * @param[in] hex The hexadecimal string
 * @param[out] out The output buffer for the bytes
 * @param[in] out_len The expected length of the hexadecimal value also length of the output buffer
 * @return 0 on success, -1 on failure
 */
static int hex_to_bytes(const char *hex, unsigned char *out, int out_len)
{
    int hex_len = strlen(hex);

    // A byte is represented by 2 hex characters -> the expected length of the hex string should be twice the output buffer length
    if (hex_len != out_len * 2)
        return -1;

    for (int i = 0; i < hex_len; i++) {
        if (!isxdigit((unsigned char)hex[i])) return -1;
    }

    for (int i = 0; i < out_len; i++) {

        char tmp[3];
        tmp[0] = hex[i * 2];
        tmp[1] = hex[i * 2 + 1];
        tmp[2] = 0;

        out[i] = (unsigned char)strtol(tmp, NULL, 16);
    }

    return 0;
}


/**
 * @brief Parse an enum value from a string using a lookup table
 * 
 * This function parses a string input to match with enum values in a lookup table and updates the value pointer accordingly.
 * It iterates through the provided `cli_enum` table, comparing the input string with the names in the table. 
 * If a match is found, it sets the corresponding value. 
 * 
 * @param[in] str The input string to parse
 * @param[in] table The lookup table of enum values, terminated by {NULL, 0}
 * @param[out] value The output integer where the parsed enum value will be stored
 * 
 * @return 0 if the input string matches an enum value in the table and the value is set successfully.
 *         If no match is found after iterating through the table, it returns -1 to indicate an error.
 */
static int parse_enum(const char *str, const struct cli_enum *table, int *value)
{
    for (int i = 0; table[i].name; i++) {

        if (strcmp(str, table[i].name) == 0) {

            *value = table[i].value;
            return 0;
        }
    }

    return -1;
}
