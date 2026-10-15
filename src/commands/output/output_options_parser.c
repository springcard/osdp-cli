#include "../../cli/command_result.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "output.h"
#include "../../cli/parser_engine.h"
#include "../../util/utils.h"

int output_parse(int argc, char **argv, struct output_options *opts, bool status_only)
{
    memset(opts, 0, sizeof(*opts));
    opts->status_only = status_only;
    opts->baud = "9600";
    opts->timeout = 1000;
    opts->retries = 3;
    opts->output = -1;
    opts->mode = OUTPUT_UNSET;
    static const struct cli_enum modes[] = {{"off", OUTPUT_OFF}, {"on", OUTPUT_ON}, {NULL, 0}};
    struct cli_option options[] = {
        {"--port", CLI_OPT_STRING, &opts->port, 0, NULL, NULL, "Serial port, required"},
        {"--baud", CLI_OPT_STRING, &opts->baud, 0, NULL, NULL, "Baud rate (alias --baudrate), default 9600"},
        {"--addr", CLI_OPT_INT, &opts->addr, 0, NULL, NULL, "OSDP address 0..126, default 0"},
        {"--scbk", CLI_OPT_HEX, opts->scbk, 16, &opts->scbk_set, NULL, "Authenticate with a base key (32 hex characters)"},
        {"--scbk-d", CLI_OPT_FLAG, &opts->scbk_d, 0, NULL, NULL, "Authenticate with the default SCBK"},
        {"--timeout", CLI_OPT_INT, &opts->timeout, 0, &opts->timeout_set, NULL, "Reply timeout in ms, 1..60000"},
        {"--retries", CLI_OPT_INT, &opts->retries, 0, &opts->retries_set, NULL, "Authentication/status attempts, 1..100; OUT is sent once"},
        {"--json", CLI_OPT_FLAG, &opts->json, 0, NULL, NULL, "Output one JSON result, including on failure"},
        {"--output", CLI_OPT_INT, &opts->output, 0, NULL, NULL, "Output index 0..255, required (0 = first relay)"},
        {"--mode", CLI_OPT_ENUM, &opts->mode, 0, NULL, modes, "Required output state: on or off"},
        {"--duration", CLI_OPT_INT, &opts->duration, 0, &opts->duration_set, NULL, "Temporary state in ms, 100..6553500, multiple of 100; omitted = permanent"}
    };
    int count = status_only ? 8 : (int)(sizeof(options)/sizeof(options[0]));
    int ret = parse_command_line_options(argc, argv, options, count);
    if (ret == -1) {
        printf("Usage: osdp-cli %s --port <port> [--baud <baud>] [--addr <address>] %s[--scbk <hex16> | --scbk-d] [--json]\n",
               status_only ? "ostat" : "out", status_only ? "" : "--output <0..255> --mode <on|off> [--duration <ms>] ");
        print_command_line_options(options, count);
        secure_zero(opts->scbk, sizeof(opts->scbk)); opts->scbk_set = 0;
        return 1;
    }
    char *end;
    unsigned long baud = strtoul(opts->baud, &end, 10);
    if (ret != argc || !opts->port || !*opts->port || !*opts->baud || *end ||
        opts->baud[0] == '-' || baud == 0 || baud > 4000000 ||
        opts->addr < 0 || opts->addr > 126 || opts->timeout < 1 || opts->timeout > 60000 ||
        opts->retries < 1 || opts->retries > 100 || (opts->scbk_set && opts->scbk_d) ||
        (!status_only && (opts->output < 0 || opts->output > 255 || opts->mode == OUTPUT_UNSET ||
         (opts->duration_set && (opts->duration < 100 || opts->duration > 6553500 || opts->duration % 100))))) {
        if (ret >= 0) cli_diagnostic(stderr, "%s: invalid connection/key options, output, mode or duration (100 ms units)\n", status_only ? "ostat" : "out");
        secure_zero(opts->scbk, sizeof(opts->scbk)); opts->scbk_set = 0;
        return -3;
    }
    return 0;
}
