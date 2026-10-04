#include <stdio.h>
#include <string.h>
#include "probe_options.h"
#include "../../cli/parser_engine.h"
#include "../../util/utils.h"

int probe_parse(int argc, char **argv, struct probe_options *opts)
{
    memset(opts, 0, sizeof(*opts));
    opts->baud = "9600";
    struct cli_option options[] = {
        {"--port", CLI_OPT_STRING, &opts->port, 0, NULL, NULL, "Serial port (COM11 or /dev/ttyUSB0)"},
        {"--baud", CLI_OPT_STRING, &opts->baud, 0, NULL, NULL, "Serial baud rate, default 9600"},
        {"--addr", CLI_OPT_INT, &opts->addr, 0, NULL, NULL, "OSDP address 0..126"},
        {"--key", CLI_OPT_HEX, opts->key, 16, &opts->key_set, NULL, "Explicit candidate base key; never falls back to SCBK-D"},
        {"--scbk", CLI_OPT_HEX, opts->key, 16, &opts->key_set, NULL, "Alias for --key"}
    };
    int ret = parse_command_line_options(argc, argv, options, sizeof(options)/sizeof(options[0]));
    if (ret == -1) {
        puts("Usage: probe --port <port> [--baud <baud>] [--addr <int>] [--key <hex16>]");
        print_command_line_options(options, sizeof(options)/sizeof(options[0]));
        secure_zero(opts->key, sizeof(opts->key)); opts->key_set = 0;
        return 1;
    }
    if (ret != argc || !opts->port || opts->addr < 0 || opts->addr > 126) {
        if (ret >= 0) fprintf(stderr, "probe: require --port and address 0..126; no positional arguments\n");
        secure_zero(opts->key, sizeof(opts->key)); opts->key_set = 0;
        return -3;
    }
    return 0;
}
