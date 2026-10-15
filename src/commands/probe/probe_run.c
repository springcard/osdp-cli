#include "../../cli/command_result.h"
#include <stdio.h>
#include <string.h>
#include "probe_result.h"
#include "../../cli/global_options_parser.h"
#include "../../platform/serial.h"
#include "../../moduleOSDP/osdp_cp.h"
#include "../../moduleOSDP/osdp_cmd.h"
#include "../../moduleOSDP/osdp_packet.h"
#include "../../moduleOSDP/osdp_sc.h"
#include "../../util/utils.h"
#include "../../logger/logger.h"

const char *probe_status_name(enum probe_status status)
{
    switch (status) {
        case PROBE_OK: return "ok";
        case PROBE_NO_RESPONSE: return "no_response";
        case PROBE_COMMUNICATION_ERROR: return "communication_error";
        case PROBE_AUTHENTICATION_FAILED: return "authentication_failed";
        case PROBE_PORT_UNAVAILABLE: return "port_unavailable";
        case PROBE_UNSUPPORTED_DEVICE: return "unsupported_device";
        case PROBE_ENTROPY_ERROR: return "entropy_error";
        default: return "invalid_argument";
    }
}

enum probe_status probe_read_information(struct probe_options *opts, int attempts,
                                          struct probe_result *result)
{
    uint8_t reply[MAX_SIZE_MSG_PACKET] = {0};
    enum probe_status status = PROBE_OK;
    for (int stage = 0; stage < 2; ++stage) {
        status = PROBE_NO_RESPONSE;
        for (int attempt = 0; attempt < attempts; ++attempt) {
            uint8_t command = stage == 0 ? CMD_ID : CMD_CAP;
            uint8_t data = 0, code = 0;
            if ((int)buildMsg(0, opts->addr, command, &data, 1) <= 0) {
                status = PROBE_COMMUNICATION_ERROR;
                break;
            }
            int size = getReply(0, &code, reply, sizeof(reply));
            if (size == -1) continue; /* Silence is not key rejection. */
            if (size < 0) {
                status = PROBE_COMMUNICATION_ERROR;
                break;
            }
            result->responded = true;
            if (code == REPLY_NAK && size == sizeof(osdp_NAK)) {
                if (stage == 0) result->identification_refused = true;
                status = reply[0] == 3 ? PROBE_UNSUPPORTED_DEVICE : PROBE_COMMUNICATION_ERROR;
                break;
            }
            if (stage == 0 && code == REPLY_PDID && size == REPLY_PDID_DATA_LEN) {
                memcpy(result->id, reply, size);
                result->has_id = true;
                status = PROBE_OK;
                break;
            }
            if (stage == 1 && code == REPLY_PDCAP && size > 0 &&
                size % REPLY_PDCAP_ENTITY_DATA_LEN == 0) {
                memcpy(result->capabilities, reply, size);
                result->capabilities_size = size;
                result->has_capabilities = true;
                status = PROBE_OK;
                break;
            }
            status = PROBE_COMMUNICATION_ERROR;
            break;
        }
        if (status != PROBE_OK) break;
    }
    secure_zero(reply, sizeof(reply));
    return status;
}

void probe_device(struct probe_options *opts, int timeout, int attempts, struct probe_result *result)
{
    bool opened = false;
    memset(result, 0, sizeof(*result));
    result->status = PROBE_INVALID_ARGUMENT;
    if (!opts->port || !opts->baud || opts->addr < 0 || opts->addr > 126 ||
        timeout <= 0 || attempts < 0 || (opts->key_set && opts->scbk_d)) goto cleanup;
    if (attempts == 0) attempts = 1;
    clearSecureChannelKeys();
    osdpPacketInit(0, CONTROL_PANEL);
    serial_set_diagnostics(false);
    if (!serial_open(opts->port, opts->baud, timeout)) {
        result->status = PROBE_PORT_UNAVAILABLE;
        goto cleanup;
    }
    opened = true;
    result->status = probe_read_information(opts, attempts, result);
    if (!opts->key_set && !opts->scbk_d) goto cleanup;
    /* Some devices refuse plain ID/CAP; authentication may still allow them. */
    if (opts->key_set) setNewScbk(opts->key);
    osdpPacketInit(0, CONTROL_PANEL);
    enum osdp_session_result session = osdp_open_secure_channel(0, opts->addr, attempts);
    switch (session) {
        case OSDP_SESSION_OK: break;
        case OSDP_SESSION_NO_RESPONSE: result->status = PROBE_NO_RESPONSE; goto cleanup;
        case OSDP_SESSION_AUTHENTICATION_FAILED: result->status = PROBE_AUTHENTICATION_FAILED; goto cleanup;
        case OSDP_SESSION_UNSUPPORTED_DEVICE: result->status = PROBE_UNSUPPORTED_DEVICE; goto cleanup;
        case OSDP_SESSION_ENTROPY_ERROR: result->status = PROBE_ENTROPY_ERROR; goto cleanup;
        default: result->status = PROBE_COMMUNICATION_ERROR; goto cleanup;
    }
    /* Publish authenticated identity only after both secured reads succeed. */
    struct probe_result secured = {0};
    result->status = probe_read_information(opts, attempts, &secured);
    if (result->status == PROBE_OK) {
        *result = secured;
        result->authenticated = true;
    }
    secure_zero(&secured, sizeof(secured));
cleanup:
    if (opened) serial_close();
    serial_set_diagnostics(true);
    setPlainTextMode(0);
    clearSecureChannelKeys();
    secure_zero(opts->key, sizeof(opts->key));
    opts->key_set = 0;
    opts->scbk_d = 0;
}

int probe_run(struct cli_context *ctx)
{
    struct probe_result result;
    if (ctx->glob_options.json) {
        secure_zero(ctx->probe_opts.key, sizeof(ctx->probe_opts.key));
        ctx->probe_opts.key_set = 0;
        cli_diagnostic(stderr, "probe: use --unattended (or -u) for commissioning JSON\n");
        return 1;
    }
    logger_console_init(ctx);
    probe_device(&ctx->probe_opts, ctx->glob_options.timeout, ctx->glob_options.retries, &result);
    printf("probe: %s; identification=%s; capabilities=%s; authenticated=%s\n",
           probe_status_name(result.status), result.has_id ? "yes" : "no",
           result.has_capabilities ? "yes" : "no", result.authenticated ? "yes" : "no");
    if (result.has_id) {
        printf("vendor=%02X%02X%02X; model=%u; firmware=%u.%u.%u\n",
               result.id[0], result.id[1], result.id[2], result.id[3],
               result.id[9], result.id[10], result.id[11]);
    }
    if (result.has_capabilities) {
        for (size_t i = 0; i < result.capabilities_size; i += 3)
            printf("capability=%u; compliance=%u; count=%u\n", result.capabilities[i],
                   result.capabilities[i + 1], result.capabilities[i + 2]);
    }
    int status = result.status == PROBE_OK ? 0 : 1;
    if (status) cli_error_code(probe_status_name(result.status));
    secure_zero(&result, sizeof(result));
    return status;
}
