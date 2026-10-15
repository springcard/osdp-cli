#include <string.h>
#include "istat.h"
#include "../../cli/commands_registry.h"
#include "../../cli/global_options_parser.h"

static int istat_handler(struct cli_context *ctx, int argc, char **argv)
{
    int ret = istat_parse(argc, argv, &ctx->istat_opts);
    for (int i = 0; i < argc; ++i)
        if (!strcmp(argv[i], "--json")) ctx->istat_opts.json = 1;
    if (ctx->glob_options.json) ctx->istat_opts.json = 1;
    if (ret == 1) return 0;
    if (!ctx->istat_opts.timeout_set) ctx->istat_opts.timeout = ctx->glob_options.timeout;
    if (!ctx->istat_opts.retries_set) ctx->istat_opts.retries = ctx->glob_options.retries;
    if (ret != 0) {
        struct istat_result result = {.code = "invalid_request", .reply_code = -1, .nak_code = -1};
        istat_emit(&ctx->istat_opts, &result);
        return 1;
    }
    return istat_run(ctx);
}
static struct cli_command istat_cmd = {
    .name = "istat",
    .help = "Read all input contact states (ISTAT), optionally using Secure Channel or JSON",
    .handler = istat_handler
};
REGISTER_COMMAND(istat_cmd);
