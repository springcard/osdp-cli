#include <string.h>
#include "output.h"
#include "../../cli/commands_registry.h"
#include "../../cli/global_options_parser.h"

static int handle(struct cli_context *ctx, int argc, char **argv, bool status_only)
{
    int ret = output_parse(argc, argv, &ctx->output_opts, status_only);
    for (int i = 0; i < argc; ++i)
        if (!strcmp(argv[i], "--json")) ctx->output_opts.json = 1;
    if (ctx->glob_options.json) ctx->output_opts.json = 1;
    if (ret == 1) return 0;
    if (!ctx->output_opts.timeout_set) ctx->output_opts.timeout = ctx->glob_options.timeout;
    if (!ctx->output_opts.retries_set) ctx->output_opts.retries = ctx->glob_options.retries;
    if (ret != 0) {
        struct output_result result = {.code = "invalid_request", .reply_code = -1, .nak_code = -1};
        output_emit(&ctx->output_opts, &result);
        return 1;
    }
    return output_run(ctx);
}
static int out_handler(struct cli_context *ctx, int argc, char **argv) { return handle(ctx, argc, argv, false); }
static int ostat_handler(struct cli_context *ctx, int argc, char **argv) { return handle(ctx, argc, argv, true); }
static struct cli_command out_cmd = {
    .name = "out", .help = "Control one relay output: on/off, permanent or timed, optionally JSON",
    .handler = out_handler
};
static struct cli_command ostat_cmd = {
    .name = "ostat", .help = "Read all output states (OSTAT), optionally using Secure Channel or JSON",
    .handler = ostat_handler
};
REGISTER_COMMAND(out_cmd);
REGISTER_COMMAND(ostat_cmd);
