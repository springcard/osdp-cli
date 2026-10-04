#include "probe_options.h"
#include "../../cli/commands_registry.h"
#include "../../cli/global_options_parser.h"

static int probe_handler(struct cli_context *ctx, int argc, char **argv)
{
    int ret = probe_parse(argc, argv, &ctx->probe_opts);
    if (ret != 0) return ret == 1 ? 0 : 1;
    return probe_run(ctx);
}
static struct cli_command probe_cmd = {
    .name = "probe",
    .help = "Read identification/capabilities, optionally authenticate an explicit key; never mutate device configuration",
    .handler = probe_handler
};
REGISTER_COMMAND(probe_cmd);
