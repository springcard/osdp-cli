#include "../../cli/command_result.h"
#include <stdio.h>
#include "sckmgmt_options.h"
#include "../../cli/global_options_parser.h"
#include "../../platform/serial.h"
#include "../../moduleOSDP/osdp_common.h"
#include "../../moduleOSDP/osdp_cp.h"
#include "../../moduleOSDP/osdp_packet.h"
#include "../../moduleOSDP/osdp_cmd.h"
#include "../../moduleOSDP/osdp_sc.h"
#include "../../util/utils.h"

/* Local statuses pending integration of the public controller contract:
 * 0: freshly authenticated target key; 1: failed before mutation/refused;
 * 2: KEYSET may have taken effect, installation could not be proved.
 */
int sckmgmt_run(struct cli_context *ctx)
{
    struct sckmgmt_options *opts = &ctx->sckmgmt_opts;
    uint8_t reply[MAX_SIZE_MSG_PACKET] = {0};
    uint8_t code = 0;
    int result = 1;
    bool port_open = false;

    if (!opts->port || !opts->baud || !opts->new_scbk_set ||
        (!opts->scbk_set && !opts->scbk_d) ||
        (opts->scbk_set && opts->scbk_d) || opts->scsk_set ||
        opts->addr < 0 || opts->addr > 126 || ctx->glob_options.retries < 0) {
        cli_diagnostic(stderr, "sckmgmt: require --port, --set-key, one of --scbk/--scbk-d and address 0..126; --scsk unsupported\n");
        goto cleanup;
    }

    /* No frame/payload/session-key logging during key management. */
    clearSecureChannelKeys();
    if (!serial_open(opts->port, opts->baud, ctx->glob_options.timeout)) {
        cli_diagnostic(stderr, "sckmgmt: serial port unavailable\n");
        goto cleanup;
    }
    port_open = true;
    osdpPacketInit(0, CONTROL_PANEL);
    setPlainTextMode(0);
    if (opts->scbk_set) setNewScbk(opts->scbk);
    if (!openSecureChannelSession2(0, opts->addr, ctx->glob_options.retries)) {
        cli_diagnostic(stderr, "sckmgmt: current-key session failed; no KEYSET sent\n");
        goto cleanup;
    }

    /* Never retry a write whose effect is uncertain. */
    result = 2;
    Command_OSDP_KEYSET(0, opts->addr, 0x01, 16, opts->new_scbk);
    int size = getReply(0, &code, reply, sizeof(reply));
    if (size < 0) {
        cli_diagnostic(stderr, "sckmgmt: outcome unknown after KEYSET; probe target key before retrying\n");
        goto cleanup;
    }
    if (code == REPLY_NAK && size == sizeof(osdp_NAK)) {
        result = 1;
        cli_diagnostic(stderr, "sckmgmt: KEYSET refused\n");
        goto cleanup;
    }
    if (code != REPLY_ACK || size != 0) {
        cli_diagnostic(stderr, "sckmgmt: invalid KEYSET reply; outcome unknown\n");
        goto cleanup;
    }

    serial_close();
    port_open = false;
    clearSecureChannelKeys();
    setNewScbk(opts->new_scbk);
    osdpPacketInit(0, CONTROL_PANEL);
    setPlainTextMode(0);
    if (!serial_open(opts->port, opts->baud, ctx->glob_options.timeout)) {
        cli_diagnostic(stderr, "sckmgmt: verification port unavailable; outcome unknown\n");
        goto cleanup;
    }
    port_open = true;
    /* ACK alone is never installation proof; verify even without --verify. */
    if (!openSecureChannelSession2(0, opts->addr, ctx->glob_options.retries)) {
        cli_diagnostic(stderr, "sckmgmt: target-key verification failed; outcome unknown\n");
        goto cleanup;
    }
    result = 0;

cleanup:
    if (port_open) serial_close();
    setPlainTextMode(0);
    clearSecureChannelKeys();
    secure_zero(reply, sizeof(reply));
    sckmgmt_clear_options(opts);
    return result;
}

void sckmgmt_clear_options(struct sckmgmt_options *opts)
{
    secure_zero(opts->scbk, sizeof(opts->scbk));
    secure_zero(opts->scsk, sizeof(opts->scsk));
    secure_zero(opts->new_scbk, sizeof(opts->new_scbk));
    opts->scbk_set = opts->scsk_set = opts->new_scbk_set = 0;
}
