#include "../../cli/command_result.h"
#include <stdio.h>
#include <string.h>
#ifdef _WIN32
#include <fcntl.h>
#include <io.h>
#endif
#include "istat.h"
#include "../../cli/global_options_parser.h"
#include "../../platform/serial.h"
#include "../../moduleOSDP/osdp_cp.h"
#include "../../moduleOSDP/osdp_cmd.h"
#include "../../moduleOSDP/osdp_packet.h"
#include "../../moduleOSDP/osdp_sc.h"
#include "../../core/commissioning_control.h"
#include "../../util/utils.h"
#include "../../logger/logger.h"

static const char *input_state(uint8_t value)
{
    return value == 0 ? "inactive" : value == 1 ? "active" : "unknown";
}

void istat_read(struct istat_options *opts, struct istat_result *result)
{
    bool opened = false, started = false;
    uint8_t reply[MAX_SIZE_MSG_PACKET] = {0};
    memset(result, 0, sizeof(*result));
    result->code = "invalid_request";
    result->reply_code = result->nak_code = -1;
    if (!opts->port || !opts->baud || opts->addr < 0 || opts->addr > 126 ||
        opts->timeout < 1 || opts->timeout > 60000 || opts->retries < 1 || opts->retries > 100 ||
        (opts->scbk_set && opts->scbk_d)) goto cleanup;
    clearSecureChannelKeys();
    osdpPacketInit(0, CONTROL_PANEL);
    serial_set_diagnostics(false);
    int budget = opts->timeout * opts->retries * 3 + 1000;
    commissioning_begin(budget > 600000 ? 600000 : budget);
    started = true;
    if (!serial_open(opts->port, opts->baud, opts->timeout)) {
        result->code = "port_unavailable";
        goto cleanup;
    }
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
    for (int attempt = 0; attempt < opts->retries && commissioning_remaining_ms() > 0; ++attempt) {
        uint8_t code = 0;
        if ((int)buildMsg(0, opts->addr, CMD_ISTAT, NULL, 0) <= 0) {
            result->code = "communication_error";
            break;
        }
        int size = getReply(0, &code, reply, sizeof(reply));
        if (size == -1) { result->code = "no_response"; continue; }
        if (size < 0 || size > (int)sizeof(reply)) {
            result->code = "communication_error";
            break;
        }
        result->reply_code = code;
        if (code == REPLY_BUSY && size == 0) { result->code = "device_busy"; continue; }
        if (code == REPLY_NAK && size == 1) {
            result->nak_code = reply[0];
            result->code = reply[0] == 3 ? "unsupported_device" : "device_refused";
            break;
        }
        if (code != REPLY_ISTATR) {
            result->code = "unexpected_reply";
            break;
        }
        /* ISTATR has one byte per input; an empty report means zero inputs. */
        memcpy(result->inputs, reply, size);
        result->count = (size_t)size;
        result->authenticated = opts->scbk_set || opts->scbk_d;
        result->code = "ok";
        break;
    }
cleanup:
    if (started && commissioning_cancelled()) {
        result->code = "cancelled";
        result->count = 0;
        result->authenticated = false;
    }
    if (started) commissioning_end();
    if (opened) serial_close();
    serial_set_diagnostics(true);
    setPlainTextMode(0);
    clearSecureChannelKeys();
    secure_zero(reply, sizeof(reply));
    secure_zero(opts->scbk, sizeof(opts->scbk));
    opts->scbk_set = opts->scbk_d = 0;
}

void istat_emit(const struct istat_options *opts, const struct istat_result *result)
{
    bool ok = !strcmp(result->code, "ok");
    if (opts->json) {
#ifdef _WIN32
        _setmode(_fileno(stdout), _O_BINARY);
#endif
        printf("{\"protocol\":\"osdp\",\"operation\":\"istat\",\"address\":%d,"
               "\"outcome\":\"%s\",\"code\":\"%s\",\"authenticated\":%s,\"replyCode\":",
               opts->addr, ok ? "succeeded" : "failed", result->code,
               result->authenticated ? "true" : "false");
        if (result->reply_code < 0) fputs("null", stdout); else printf("%d", result->reply_code);
        fputs(",\"nakCode\":", stdout);
        if (result->nak_code < 0) fputs("null", stdout); else printf("%d", result->nak_code);
        printf(",\"inputCount\":%zu,\"inputs\":[", result->count);
        for (size_t i = 0; i < result->count; ++i) {
            if (i) putchar(',');
            printf("{\"number\":%zu,\"raw\":%u,\"state\":\"%s\"}",
                   i + 1, result->inputs[i], input_state(result->inputs[i]));
        }
        puts("]}");
    } else {
        printf("istat: %s; address=%d; authenticated=%s\n", result->code,
               opts->addr, result->authenticated ? "yes" : "no");
        if (result->reply_code >= 0) printf("Reply: 0x%02X%s\n", result->reply_code,
                                                result->reply_code == REPLY_ISTATR ? " (ISTATR)" : "");
        if (result->nak_code >= 0) printf("NAK reason: 0x%02X\n", result->nak_code);
        if (ok) {
            printf("Inputs read: %zu (numbered from 1, in reply order)\n", result->count);
            for (size_t i = 0; i < result->count; ++i)
                printf("  Input %zu: %s (raw=0x%02X)\n", i + 1,
                       input_state(result->inputs[i]), result->inputs[i]);
        } else puts("No input state was read.");
    }
}

int istat_run(struct cli_context *ctx)
{
    struct istat_result result;
    ctx->glob_options.json = ctx->glob_options.json || ctx->istat_opts.json;
    logger_console_init(ctx);
    istat_read(&ctx->istat_opts, &result);
    istat_emit(&ctx->istat_opts, &result);
    int status = strcmp(result.code, "ok") != 0;
    if (status) cli_error_code(result.code);
    secure_zero(&result, sizeof(result));
    return fflush(stdout) == 0 && !ferror(stdout) ? status : 1;
}
