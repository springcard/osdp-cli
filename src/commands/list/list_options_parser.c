#include "../../cli/command_result.h"
#include <stdio.h>
#include <string.h>
#include "list_options.h"
#include "../../cli/parser_engine.h"

int list_parse(int argc, char **argv, struct list_options *opts)
{
    memset(opts, 0, sizeof(*opts));
    opts->baud = "9600";
    opts->first_addr = 0;
    opts->last_addr = 126;
    opts->timeout = 100;
    opts->retries = 1;
    opts->tx_control = -1;

    struct cli_option options[] = {
        {"--port", CLI_OPT_STRING, &opts->port, 0, NULL, NULL, "Serial port (COM11 or /dev/ttyUSB0)"},
        {"--tx-control", CLI_OPT_INT, &opts->tx_control, 0, NULL, NULL, "Optional Linux sysfs GPIO: 1 transmit, 0 receive"},
        {"--baud", CLI_OPT_STRING, &opts->baud, 0, NULL, NULL, "Serial baud rate, default 9600"},
        {"--first-addr", CLI_OPT_INT, &opts->first_addr, 0, NULL, NULL, "First OSDP address to scan, default 0"},
        {"--last-addr", CLI_OPT_INT, &opts->last_addr, 0, NULL, NULL, "Last OSDP address to scan, default 126"},
        {"--timeout", CLI_OPT_INT, &opts->timeout, 0, NULL, NULL, "Reply timeout per attempt in ms, default 100"},
        {"--retries", CLI_OPT_INT, &opts->retries, 0, NULL, NULL, "Attempts per request, default 1"},
        {"--json", CLI_OPT_FLAG, &opts->json, 0, NULL, NULL, "Output one machine-readable JSON document"},
        {"--progress", CLI_OPT_FLAG, &opts->progress, 0, NULL, NULL, "With --json: flushed progress/device/result JSON lines"},
        {"--deadline", CLI_OPT_INT, &opts->deadline, 0, NULL, NULL, "Whole scan budget in ms, 1..600000; default computed and capped"}
    };
    int ret = parse_command_line_options(argc, argv, options, sizeof(options) / sizeof(options[0]));
    int explicit_deadline = 0, tx_arguments = 0;
    for (int i = 0; i < argc; ++i) if (!strcmp(argv[i], "--deadline")) explicit_deadline = 1;
    for (int i = 0; i < argc; ++i) if (!strcmp(argv[i], "--tx-control")) ++tx_arguments;
#ifdef _WIN32
    if (tx_arguments) return -3;
#endif
    if (ret == -1) {
        puts("Usage: osdp-cli --list --port <port> [options]");
        puts("       osdp-cli [global options] list --port <port> [options]");
        print_command_line_options(options, sizeof(options) / sizeof(options[0]));
        return 1;
    }
    if (ret != argc || !opts->port || opts->first_addr < 0 || opts->last_addr > 126 ||
        opts->first_addr > opts->last_addr || opts->timeout < 1 || opts->timeout > 60000 ||
        opts->retries < 1 || opts->retries > 100 || (explicit_deadline && opts->deadline < 1) || opts->deadline > 600000 ||
        tx_arguments > 1 || (tx_arguments && (opts->tx_control < 0 || opts->tx_control > 65535))) {
        if (ret >= 0)
            cli_diagnostic(stderr, "list: require --port, addresses 0..126, timeout 1..60000 and retries 1..100\n");
        return -3;
    }
    return 0;
}
