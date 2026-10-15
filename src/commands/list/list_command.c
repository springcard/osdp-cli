#include "../../cli/command_result.h"
#include <stdio.h>
#include <string.h>
#ifdef _WIN32
#include <fcntl.h>
#include <io.h>
#endif
#include "list_options.h"
#include "../../cli/commands_registry.h"
#include "../../cli/global_options_parser.h"

static int list_handler(struct cli_context *ctx, int argc, char **argv)
{
    int ret = list_parse(argc, argv, &ctx->list_opts);
    /* Preserve requested machine framing even if parsing stopped before flags. */
    for (int i = 0; i < argc; ++i) {
        if (!strcmp(argv[i], "--json")) ctx->list_opts.json = 1;
        if (!strcmp(argv[i], "--progress")) ctx->list_opts.progress = 1;
    }
    if (ret != 0) {
        if (ret != 1 && (ctx->glob_options.json || ctx->list_opts.json)) {
#ifdef _WIN32
            _setmode(_fileno(stdout), _O_BINARY);
#endif
            if (ctx->list_opts.progress) fputs("{\"type\":\"result\",", stdout);
            else putchar('{');
            printf("\"protocol\":\"osdp\",\"operation\":\"list\",\"outcome\":\"failed\","
                   "\"code\":\"invalid_request\",\"deviceCount\":0,\"devices\":[]}\n");
        }
        return ret == 1 ? 0 : 1;
    }
    if (ctx->glob_options.json) ctx->list_opts.json = 1;
    if (ctx->list_opts.tx_control < 0) ctx->list_opts.tx_control = ctx->glob_options.tx_control;
    if (ctx->list_opts.progress && !ctx->list_opts.json) {
        cli_diagnostic(stderr, "list: --progress requires --json\n");
        return 1;
    }
    return list_run(ctx);
}

static struct cli_command list_cmd = {
    .name = "list",
    .help = "Discover OSDP peripherals when their bus addresses are unknown",
    .handler = list_handler
};
REGISTER_COMMAND(list_cmd);
