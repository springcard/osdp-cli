#include "../../cli/command_result.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "istat.h"
#include "../../cli/parser_engine.h"
#include "../../util/utils.h"

int istat_parse(int argc, char **argv, struct istat_options *opts)
{
    memset(opts, 0, sizeof(*opts));
    opts->baud = "9600";
    opts->timeout = 1000;
    opts->retries = 3;
    struct cli_option options[] = {
        {"--port", CLI_OPT_STRING, &opts->port, 0, NULL, NULL, "Serial port, required"},
        {"--baud", CLI_OPT_STRING, &opts->baud, 0, NULL, NULL, "Baud rate (alias --baudrate), default 9600"},
        {"--addr", CLI_OPT_INT, &opts->addr, 0, NULL, NULL, "OSDP address 0..126, default 0"},
        {"--scbk", CLI_OPT_HEX, opts->scbk, 16, &opts->scbk_set, NULL, "Authenticate with a base key (32 hexadecimal characters)"},
        {"--scbk-d", CLI_OPT_FLAG, &opts->scbk_d, 0, NULL, NULL, "Authenticate with the default SCBK"},
        {"--timeout", CLI_OPT_INT, &opts->timeout, 0, &opts->timeout_set, NULL, "Reply timeout in ms, 1..60000"},
        {"--retries", CLI_OPT_INT, &opts->retries, 0, &opts->retries_set, NULL, "Attempts per request, 1..100"},
        {"--json", CLI_OPT_FLAG, &opts->json, 0, NULL, NULL, "Output one JSON result, including on failure"}
    };
    int ret = parse_command_line_options(argc, argv, options, sizeof(options)/sizeof(options[0]));
    if (ret == -1) {
        puts("Usage: osdp-cli istat --port <port> [--baud <baud>] [--addr <address>] [--scbk <hex16> | --scbk-d] [--json]");
        print_command_line_options(options, sizeof(options)/sizeof(options[0]));
        secure_zero(opts->scbk, sizeof(opts->scbk));
        opts->scbk_set = 0;
        return 1;
    }
    char *end;
    unsigned long baud = strtoul(opts->baud, &end, 10);
    if (ret != argc || !opts->port || !*opts->port || !*opts->baud || *end ||
        opts->baud[0] == '-' || baud == 0 || baud > 4000000 ||
        opts->addr < 0 || opts->addr > 126 || opts->timeout < 1 || opts->timeout > 60000 ||
        opts->retries < 1 || opts->retries > 100 || (opts->scbk_set && opts->scbk_d)) {
        if (ret >= 0) cli_diagnostic(stderr, "istat: invalid port, baud, address, timeout, retries or conflicting keys\n");
        secure_zero(opts->scbk, sizeof(opts->scbk));
        opts->scbk_set = 0;
        return -3;
    }
    return 0;
}
