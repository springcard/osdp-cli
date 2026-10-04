/**
 * @file global_options_parser.c
 * @brief Implements the CLI parser for global options
 * 
 * The code provided implements a parser for global command line options for the CLI application.
 * The parser is responsible for parsing global command-line options, handling help and version requests, and preparing a context for the command to be executed. 
 * It processes various global options such as verbosity, output format, timeout, retries, logging, and more.
 * The parser updates a `cli_context` structure based on the parsed options and returns the index of the command in the input arguments for further processing by the command dispatcher.
 */

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "global_options_parser.h"
#include "commands_registry.h"
#include "version.h"
#include "parser_engine.h"

// Reference array of log format for the sniff command  logging options, 
// used for parsing the output format option for the sniff command.
static const struct cli_enum log_format_values[] = {
    { "text",        LOG_TEXT },
    { "json",        LOG_JSON },
    { "pcapng-like", LOG_PCAPNG },

    { NULL, 0 }
};

static const char *global_option_value_hint(const struct cli_option *opt);
static void print_global_enum_values(const struct cli_enum *table);


/**
 * @brief Parse global command line options and update the CLI context
 * 
 * This function parses global command line options and updates the context accordingly, while also handling help and version requests.
 * 
 * @param[in] argc `argc` is the argument count, which represents the number of arguments passed to the
 * program from the command line, including the program's name itself.
 * @param[in] argv The `argv` parameter in the `cli_parse_global_options` function is an array of strings that
 * represents the command-line arguments passed to the program. Each element of the array `argv` is a
 * null-terminated C string, where `argv[0]` is the name of the program and ` argv[1]` to `argv[argc-1]` are the actual command-line arguments provided by the user.
 * @param[out] ctx The `ctx` parameter in the `cli_parse_global_options` function is a pointer to a `struct
 * cli_context`. This structure is used to store various configuration options and settings for the
 * command-line interface (CLI) parser. The function parses the command-line arguments provided to the
 * program and updates the `ctx` structure with the parsed values.
 * @param[out] cmd_index The `cmd_index` parameter in the `cli_parse_global_options` function is a pointer to an
 * integer. This parameter is used to store the index of the command in the `argv` array after parsing
 * the global options. By updating the value that `cmd_index` points to, the function can indicate
 * 
 * @return The function `cli_parse_global_options` returns an integer value. It returns `1` in case of
 * displaying help or version information, `-1` in case of missing values or unknown options, and `0`
 * for successful parsing of global options.
 */
int cli_parse_global_options(int argc, char **argv, struct cli_context *ctx, int *cmd_index){
    
    memset(&ctx->glob_options, 0, sizeof(struct global_options));
    ctx->glob_options.timeout = 1000;
    ctx->glob_options.retries = 3;
    ctx->glob_options.log_format = LOG_TEXT;

    struct cli_option options[] = {

        { "--quiet",      CLI_OPT_FLAG,   &ctx->glob_options.quiet,         0, NULL,              NULL, "Display the output reduced to the raw frame format." },
        { "-q",           CLI_OPT_FLAG,   &ctx->glob_options.quiet,         0, NULL,              NULL, "Display the output reduced to the raw frame format." },
        { "--verbose",    CLI_OPT_FLAG,   &ctx->glob_options.verbose,       0, NULL,              NULL, "Improves the display with the different fields of the frame and the decoded message." },
        { "-v",           CLI_OPT_FLAG,   &ctx->glob_options.verbose,       0, NULL,              NULL, "Improves the display with the different fields of the frame and the decoded message." },
        { "--json",       CLI_OPT_FLAG,   &ctx->glob_options.json,          0, NULL,              NULL, "Output machine-readable JSON (when applicable)" },
        { "--no-color",   CLI_OPT_FLAG,   &ctx->glob_options.no_color,      0, NULL,              NULL, "Disable ANSI colors" },
        { "--timeout",    CLI_OPT_INT,    &ctx->glob_options.timeout,       0, NULL,              NULL, "Timeout reply of command (ms), 1000 by default if not specified." },
        { "-t",           CLI_OPT_INT,    &ctx->glob_options.timeout,       0, NULL,              NULL, "Timeout reply of command (ms), 1000 by default if not specified." },
        { "--retries",    CLI_OPT_INT,    &ctx->glob_options.retries,       0, NULL,              NULL, "Retry count, 3 by default if not specified." },
        { "--log",        CLI_OPT_STRING, &ctx->glob_options.log_file_name, 0, NULL,              NULL, "Write logs to file (in addition to stderr/stdout)."},
        { "--log-format", CLI_OPT_ENUM,   &ctx->glob_options.log_format,    0, NULL, log_format_values, "Log format (text|json|pcapng-like), text by default if not specified."},
        { "--timestamp",  CLI_OPT_FLAG,   &ctx->glob_options.timestamp,     0, NULL,              NULL, "Prefix logs with timestamps."},
        { "--sc-params",  CLI_OPT_FLAG,   &ctx->glob_options.sc_params,     0, NULL,              NULL, "Display non-secret Secure Channel parameters; keys are redacted."},
        { "--version",    CLI_OPT_FLAG,   &ctx->glob_options.version,       0, NULL,              NULL, "osdp-cli version requested."},        
        { "-V",           CLI_OPT_FLAG,   &ctx->glob_options.version,       0, NULL,              NULL, "osdp-cli version requested."}
    };

    // globale options parsing
    int ret = parse_command_line_options(argc, argv, options, sizeof(options) / sizeof(options[0])) ;

    if (ret == -1) { // Help requested, handle help option case.
        print_globale_line_options(options, sizeof(options) / sizeof(options[0]));
        return 1;
    }
    else if(ret == -2){// osdp-cli version requested
        printf("%s version %s\n", APP_NAME, APP_VERSION);
        printf("Build date: %s\n", APP_BUILD_DATE);
        return 1;
    }
    else if (ret < 0) { // Parsing format error, handle error case.
        return ret;
    }

    *cmd_index = ret + 1; // adds 1 to align with the full command line, accounting for the first argument (the application name)
    
    return 0;    
}


/**
 * @brief Print help messages for globale command-line options
 * 
 * This function is responsible for printing the available globale-line options along with their descriptions.
 * It takes an array of `cli_option` structures and the count of options as parameters.
 * 
 * @param[in] opts Array of cli_option structures to print contains the options to print
 * @param[in] opt_count Number of options in the array
 */
void print_globale_line_options(struct cli_option *opts, int opt_count)
{
    printf("\nUsage: osdp-cli [global options] <command> [command options] [arguments]\n");
    printf("Usage: osdp-cli --help\n");
    printf("Usage: osdp-cli <command> -h\n");
    printf("Usage: osdp-cli <command> --port COM3 --baud 9600 [command options]\n\n");
    printf("Global options:\n");
    printf("  %-24s %s\n", "-h, --help", "Display this help.");
    printf("  %-24s %s\n", "-u, --unattended", "Use strict JSON automation mode: place before probe, authenticate or set-key.");

    for (int i = 0; i < opt_count; i++) {
        char label[64];
        snprintf(label, sizeof(label), "%s%s%s",
                 opts[i].name,
                 global_option_value_hint(&opts[i])[0] != '\0' ? " " : "",
                 global_option_value_hint(&opts[i]));

        printf("  %-24s", label);

        if (opts[i].help || (opts[i].type == CLI_OPT_ENUM && opts[i].enum_values != NULL)) {
            printf("  ");
        }

        printf("%s",
               opts[i].help ? opts[i].help : "");

        if (opts[i].type == CLI_OPT_ENUM && opts[i].enum_values != NULL) {
            printf(" Values: ");
            print_global_enum_values(opts[i].enum_values);
            printf(".");
        }

        printf("\n");
    }

    printf("\nCommon command options:\n");
    printf("  %-24s %s\n", "--port <port>", "Serial port used by commands that access RS485, e.g. COM3, COM11, or /dev/ttyUSB0.");
    printf("  %-24s %s\n", "--baud <baud>", "Serial baud rate. Default is 9600 in commands that use a serial port.");
    printf("  %-24s %s\n", "--addr <int>", "OSDP PD address. Default is usually 0 when supported by the command.");
    printf("  %-24s %s\n", "--scbk <hex16>", "16-byte Secure Channel Base Key, encoded as 32 hexadecimal characters.");
    printf("  %-24s %s\n", "--scbk-d", "Use the default SCBK for secure channel establishment.");
    printf("  %-24s %s\n", "--scsk <hex16>", "16-byte Secure Channel Session Key, encoded as 32 hexadecimal characters where supported.");

    int cmdCount = cli_get_command_count();
    struct cli_command ** ptrCmd = cli_get_command_tree();  

    printf("\nCommands:\n");
    for(int i = 0; i < cmdCount; i++) {
        if (ptrCmd[i]->parent != NULL) {
            continue;
        }
        printf("\t%s\t%s\n", ptrCmd[i]->name, ptrCmd[i]->help);    // get the root commands
    }

    printf("\n");
}

static const char *global_option_value_hint(const struct cli_option *opt)
{
    switch (opt->type) {
        case CLI_OPT_FLAG:
            return "";
        case CLI_OPT_INT:
            return "<int>";
        case CLI_OPT_STRING:
            if (strcmp(opt->name, "--log") == 0) {
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

static void print_global_enum_values(const struct cli_enum *table)
{
    for (int i = 0; table[i].name != NULL; i++) {
        if (i > 0) {
            printf("|");
        }
        printf("%s", table[i].name);
    }
}


/**
 * @brief Displays the parsed global CLI options (for testing purposes).
 *
 * This function inspects the CLI context structure and prints the current state of all global options that were parsed from
 * the command line. It is mainly intended for debugging purposes to verify that arguments have been correctly interpreted.
 *
 * The displayed information includes:
 * - Output modes (quiet, verbose, JSON, color handling)
 * - Numeric parameters (timeout, retries)
 * - Logging configuration (log file name and format)
 * - Timestamp activation
 *
 * @param ctx Pointer to the CLI context structure containing the parsed global options.
 *
 * @return int
 *         - 0 on successful execution
 *         - Other values may be used in the future to signal
 *           specific conditions (e.g., early exit for version display)
 */
int cli_global_options_test(struct cli_context *ctx)
{
        if (ctx->glob_options.version > 0) {
            printf("Requested version of osdp-cli\n");
        }

        if (ctx->glob_options.quiet > 0) {
            printf("Quiet mode enabled, \"--quiet\" or \"-q\"\n");
        }

        if (ctx->glob_options.verbose > 0) {
            printf("Verbose mode enabled, \"--verbose\" or \"-v\"\n");
        }

        if (ctx->glob_options.json > 0) {
            printf("JSON output enabled, \"--json\"\n");
        }

        if (ctx->glob_options.no_color > 0) {
            printf("Color output disabled, \"--no-color\"\n");
        }

        printf("Timeout set to: %d ms\n", ctx->glob_options.timeout);

        printf("Retries set to: %d\n", ctx->glob_options.retries);

        if (ctx->glob_options.log_file_name) {
            printf("Log file set to: %s\n", ctx->glob_options.log_file_name);
        }

        if (ctx->glob_options.log_file_name) {

            int i = 0;

            for(i = 0; i < sizeof(log_format_values)/sizeof(struct cli_enum); i++)
            {
                if(log_format_values[i].value == ctx->glob_options.log_format)
                    break;
            } 
            if(i < sizeof(log_format_values)/sizeof(struct cli_enum))
                printf("Log format set to: %s\n", log_format_values[i].name);
            else
                printf("Log format set to: unknown value!!!!\n");    
        }

        if (ctx->glob_options.timestamp) {
            printf("Timestamp enabled, \"--timestamp\"\n");
        }    
        
        if (ctx->glob_options.sc_params) {
            printf("Display SC params enabled, \"--sc-params\"\n");
        } 
        
    return 0;
}
