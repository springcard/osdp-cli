#include "../../cli/command_result.h"
#include <stdio.h>
#include <string.h>
#ifdef _WIN32
#include <fcntl.h>
#include <io.h>
#endif
#include "output.h"
#include "../../cli/global_options_parser.h"
#include "../../platform/serial.h"
#include "../../moduleOSDP/osdp_cp.h"
#include "../../moduleOSDP/osdp_cmd.h"
#include "../../moduleOSDP/osdp_packet.h"
#include "../../moduleOSDP/osdp_sc.h"
#include "../../core/commissioning_control.h"
#include "../../util/utils.h"
#include "../../logger/logger.h"

static const char *state(uint8_t value) { return value == 0 ? "inactive" : value == 1 ? "active" : "unknown"; }

void output_execute(struct output_options *opts, struct output_result *result)
{
    bool opened = false, started = false, sent = false;
    uint8_t reply[MAX_SIZE_MSG_PACKET] = {0};
    memset(result, 0, sizeof(*result));
    result->code = "invalid_request";
    result->reply_code = result->nak_code = -1;
    if (!opts->port || !opts->baud || opts->addr < 0 || opts->addr > 126 ||
        opts->timeout < 1 || opts->timeout > 60000 || opts->retries < 1 || opts->retries > 100 ||
        (opts->scbk_set && opts->scbk_d) || (!opts->status_only &&
        (opts->output < 0 || opts->output > 255 || (opts->mode != OUTPUT_ON && opts->mode != OUTPUT_OFF) ||
         (opts->duration_set && (opts->duration < 100 || opts->duration > 6553500 || opts->duration % 100))))) goto cleanup;
    clearSecureChannelKeys();
    osdpPacketInit(0, CONTROL_PANEL);
    serial_set_diagnostics(false);
    int budget = opts->timeout * opts->retries * 3 + 1000;
    commissioning_begin(budget > 600000 ? 600000 : budget);
    started = true;
    if (!serial_open(opts->port, opts->baud, opts->timeout)) { result->code = "port_unavailable"; goto cleanup; }
    opened = true;
    if (opts->scbk_set || opts->scbk_d) {
        if (opts->scbk_set) setNewScbk(opts->scbk);
        enum osdp_session_result session = osdp_open_secure_channel(0, opts->addr, opts->retries);
        switch (session) {
            case OSDP_SESSION_OK: break;
            case OSDP_SESSION_NO_RESPONSE: result->code = "no_response"; goto cleanup;
            case OSDP_SESSION_AUTHENTICATION_FAILED: result->code = "authentication_failed"; goto cleanup;
            case OSDP_SESSION_UNSUPPORTED_DEVICE: result->code = "unsupported_device"; goto cleanup;
            case OSDP_SESSION_ENTROPY_ERROR: result->code = "entropy_error"; goto cleanup;
            default: result->code = "communication_error"; goto cleanup;
        }
    }
    result->code = "no_response";
    int attempts = opts->status_only ? opts->retries : 1;
    for (int attempt = 0; attempt < attempts && commissioning_remaining_ms() > 0; ++attempt) {
        uint8_t code = 0, payload[4];
        size_t size_sent;
        if (opts->status_only) size_sent = buildMsg(0, opts->addr, CMD_OSTAT, NULL, 0);
        else {
            int ticks = opts->duration_set ? opts->duration / 100 : 0;
            payload[0] = (uint8_t)opts->output;
            payload[1] = opts->duration_set ? (opts->mode == OUTPUT_ON ? 5 : 6) : (opts->mode == OUTPUT_ON ? 2 : 1);
            payload[2] = (uint8_t)ticks; payload[3] = (uint8_t)(ticks >> 8);
            /* A missing acknowledgement must never restart a timed operation. */
            sent = true;
            size_sent = buildMsg(0, opts->addr, CMD_OUT, payload, sizeof(payload));
        }
        if ((int)size_sent <= 0) { result->code = "communication_error"; break; }
        int size = getReply(0, &code, reply, sizeof(reply));
        if (size == -1) { result->code = "no_response"; continue; }
        if (size < 0 || size > (int)sizeof(reply)) { result->code = "communication_error"; break; }
        result->reply_code = code;
        if (code == REPLY_BUSY && size == 0) { sent = false; result->code = "device_busy"; continue; }
        if (code == REPLY_NAK && size == 1) {
            sent = false;
            result->nak_code = reply[0];
            result->code = reply[0] == 3 ? "unsupported_device" : "device_refused";
            break;
        }
        if (code == REPLY_OSTATR) {
            memcpy(result->states, reply, size);
            result->count = (size_t)size;
            result->has_status = true;
        } else if (opts->status_only || code != REPLY_ACK || size != 0) {
            result->code = "unexpected_reply";
            break;
        }
        result->acknowledged = !opts->status_only;
        result->authenticated = opts->scbk_set || opts->scbk_d;
        result->code = "ok";
        break;
    }
cleanup:
    if (started && commissioning_cancelled() && strcmp(result->code, "ok")) {
        result->code = "cancelled";
        result->count = 0; result->has_status = false; result->authenticated = false;
    }
    if (sent && !result->acknowledged) result->uncertain = true;
    if (started) commissioning_end();
    if (opened) serial_close();
    serial_set_diagnostics(true);
    setPlainTextMode(0); clearSecureChannelKeys();
    secure_zero(reply, sizeof(reply)); secure_zero(opts->scbk, sizeof(opts->scbk));
    opts->scbk_set = opts->scbk_d = 0;
}

void output_emit(const struct output_options *opts, const struct output_result *result)
{
    const char *operation = opts->status_only ? "ostat" : "out";
    const char *outcome = result->uncertain ? "unknown" : !strcmp(result->code, "ok") ? "succeeded" : "failed";
    if (opts->json) {
#ifdef _WIN32
        _setmode(_fileno(stdout), _O_BINARY);
#endif
        printf("{\"protocol\":\"osdp\",\"operation\":\"%s\",\"address\":%d,\"outcome\":\"%s\","
               "\"code\":\"%s\",\"authenticated\":%s,\"acknowledged\":%s,\"statusRead\":%s,\"replyCode\":",
               operation, opts->addr, outcome, result->code, result->authenticated ? "true" : "false",
               result->acknowledged ? "true" : "false", result->has_status ? "true" : "false");
        if (result->reply_code < 0) fputs("null", stdout); else printf("%d", result->reply_code);
        fputs(",\"nakCode\":", stdout);
        if (result->nak_code < 0) fputs("null", stdout); else printf("%d", result->nak_code);
        if (!opts->status_only) {
            printf(",\"output\":%d,\"requestedState\":", opts->output);
            if (opts->mode == OUTPUT_UNSET) fputs("null", stdout); else printf("\"%s\"", opts->mode == OUTPUT_ON ? "on" : "off");
            fputs(",\"durationMs\":", stdout);
            if (opts->duration_set) printf("%d", opts->duration); else fputs("null", stdout);
        }
        printf(",\"outputCount\":%zu,\"outputs\":[", result->count);
        for (size_t i = 0; i < result->count; ++i) {
            if (i) putchar(',');
            printf("{\"output\":%zu,\"number\":%zu,\"raw\":%u,\"state\":\"%s\"}",
                   i, i + 1, result->states[i], state(result->states[i]));
        }
        puts("]}");
    } else {
        printf("%s: %s; address=%d; outcome=%s; authenticated=%s\n", operation,
               result->code, opts->addr, outcome, result->authenticated ? "yes" : "no");
        if (!opts->status_only && opts->mode != OUTPUT_UNSET) {
            printf("Requested: output %d (relay %d), %s", opts->output, opts->output + 1, opts->mode == OUTPUT_ON ? "on" : "off");
            if (opts->duration_set) printf(" for %d ms; then resume permanent state\n", opts->duration);
            else puts(" permanently (cancel active timer)");
            printf("Command acknowledged: %s\n", result->acknowledged ? "yes" : "no");
        }
        if (result->reply_code >= 0) printf("Reply: 0x%02X\n", result->reply_code);
        if (result->nak_code >= 0) printf("NAK reason: 0x%02X\n", result->nak_code);
        if (result->has_status) {
            printf("Outputs read: %zu (output index 0 = first relay)\n", result->count);
            for (size_t i = 0; i < result->count; ++i)
                printf("  Output %zu (relay %zu): %s (raw=0x%02X)\n", i, i + 1, state(result->states[i]), result->states[i]);
        } else puts("No output state was read.");
        if (result->uncertain) puts("The command may have taken effect; query ostat before repeating it.");
    }
}

int output_run(struct cli_context *ctx)
{
    struct output_result result;
    ctx->glob_options.json = ctx->glob_options.json || ctx->output_opts.json;
    logger_console_init(ctx);
    output_execute(&ctx->output_opts, &result);
    output_emit(&ctx->output_opts, &result);
    int status = result.uncertain ? 2 : strcmp(result.code, "ok") != 0;
    if (status) cli_error_code(result.code);
    secure_zero(&result, sizeof(result));
    return fflush(stdout) == 0 && !ferror(stdout) ? status : 1;
}
