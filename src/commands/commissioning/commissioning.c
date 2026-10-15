#include "../../cli/command_result.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <limits.h>
#include "commissioning.h"
#include "../../core/commissioning_control.h"
#include "../../platform/serial.h"
#include "../../moduleOSDP/osdp_cp.h"
#include "../../moduleOSDP/osdp_cmd.h"
#include "../../moduleOSDP/osdp_packet.h"
#include "../../moduleOSDP/osdp_sc.h"
#include "../../util/utils.h"
#ifdef _WIN32
#include <io.h>
#include <fcntl.h>
#endif

static const char *operation_name(enum commissioning_operation operation)
{
    return operation == COMMISSIONING_AUTHENTICATE ? "authenticate" :
           operation == COMMISSIONING_SET_KEY ? "set_key" : "probe";
}
static const char *code_name(enum commissioning_code code)
{
    static const char *names[] = {"ok", "invalid_request", "port_unavailable", "no_response",
        "communication_error", "authentication_failed", "unsupported_device", "verification_failed",
        "verification_timeout", "outcome_unknown", "cancelled"};
    return names[code];
}
static bool parse_integer(const char *text, int minimum, int maximum, int *value)
{
    char *end;
    errno = 0;
    long parsed = strtol(text, &end, 10);
    if (errno || text == end || *end || parsed < minimum || parsed > maximum) return false;
    *value = (int)parsed;
    return true;
}
static int nibble(char c) {
    if (c >= '0' && c <= '9') return c - '0';
    if (c >= 'a' && c <= 'f') return c - 'a' + 10;
    if (c >= 'A' && c <= 'F') return c - 'A' + 10;
    return -1;
}
static bool parse_key(const char *text, uint8_t *key) {
    if (strlen(text) != 32) return false;
    for (int i = 0; i < 16; ++i) {
        int high = nibble(text[2*i]), low = nibble(text[2*i+1]);
        if (high < 0 || low < 0) return false;
        key[i] = (uint8_t)((high << 4) | low);
    }
    return true;
}
bool commissioning_parse(int argc, char **argv, struct commissioning_request *request)
{
    memset(request, 0, sizeof(*request));
    request->device.addr = -1;
    request->tx_control = -1;
    if (argc >= 2) {
        if (strcmp(argv[1], "authenticate") == 0) request->operation = COMMISSIONING_AUTHENTICATE;
        else if (strcmp(argv[1], "set-key") == 0) request->operation = COMMISSIONING_SET_KEY;
        else if (strcmp(argv[1], "probe") != 0) return false;
    }
    if (argc < 2 || (strcmp(argv[0], "--unattended") && strcmp(argv[0], "-u"))) return false;
    unsigned seen = 0;
    for (int i = 2; i < argc;) {
        const char *name = argv[i++];
        unsigned bit = 0;
        if (!strcmp(name, "--scbk-d")) {
            bit = 16;
            request->device.scbk_d = 1;
        } else {
            if (i >= argc) return false;
            const char *value = argv[i++];
            if (!strcmp(name, "--port")) {
                bit = 1; if (!*value || strlen(value) > 256) return false;
                request->device.port = value;
            } else if (!strcmp(name, "--baud") || !strcmp(name, "--baudrate")) {
                int baud; bit = 2;
                if (!parse_integer(value, 1200, 921600, &baud)) return false;
                request->device.baud = value;
            } else if (!strcmp(name, "--addr")) {
                bit = 4; if (!parse_integer(value, 0, 126, &request->device.addr)) return false;
            } else if (!strcmp(name, "--timeout")) {
                bit = 8; if (!parse_integer(value, 1, 60000, &request->timeout_ms)) return false;
            } else if (!strcmp(name, "--scbk")) {
                bit = 16; if (!parse_key(value, request->device.key)) return false;
                request->device.key_set = 1;
            } else if (!strcmp(name, "--set-key")) {
                bit = 32; if (!parse_key(value, request->target_key)) return false;
            } else if (!strcmp(name, "--tx-control")) {
                bit = 64; if (!parse_integer(value, 0, 65535, &request->tx_control)) return false;
            } else return false;
        }
        if (seen & bit) return false;
        seen |= bit;
    }
    unsigned expected = request->operation == COMMISSIONING_PROBE ? 15 :
                        request->operation == COMMISSIONING_AUTHENTICATE ? 31 : 63;
#ifdef _WIN32
    if (seen & 64u) return false;
#endif
    return (seen & ~64u) == expected;
}

static enum commissioning_code probe_code(enum probe_status status) {
    switch (status) {
        case PROBE_OK: return COMM_OK;
        case PROBE_NO_RESPONSE: return COMM_NO_RESPONSE;
        case PROBE_AUTHENTICATION_FAILED: return COMM_AUTHENTICATION_FAILED;
        case PROBE_PORT_UNAVAILABLE: return COMM_PORT_UNAVAILABLE;
        case PROBE_UNSUPPORTED_DEVICE: return COMM_UNSUPPORTED_DEVICE;
        case PROBE_INVALID_ARGUMENT: return COMM_INVALID_REQUEST;
        default: return COMM_COMMUNICATION_ERROR;
    }
}
static enum commissioning_code authenticate_open(struct commissioning_request *request,
                                                const uint8_t *key, bool transport, struct probe_result *device)
{
    if (commissioning_cancelled()) return COMM_CANCELLED;
    if (commissioning_remaining_ms() <= 0) return COMM_NO_RESPONSE;
    clearSecureChannelKeys();
    if (!transport) setNewScbk((uint8_t *)key);
    osdpPacketInit(0, CONTROL_PANEL);
    if (!serial_open(request->device.port, request->device.baud, commissioning_remaining_ms()))
        return COMM_PORT_UNAVAILABLE;
    enum osdp_session_result session = osdp_open_secure_channel(0, request->device.addr, 1);
    if (commissioning_cancelled()) return COMM_CANCELLED;
    switch (session) {
        case OSDP_SESSION_OK: break;
        case OSDP_SESSION_NO_RESPONSE: return COMM_NO_RESPONSE;
        case OSDP_SESSION_AUTHENTICATION_FAILED: return COMM_AUTHENTICATION_FAILED;
        case OSDP_SESSION_UNSUPPORTED_DEVICE: return COMM_UNSUPPORTED_DEVICE;
        default: return COMM_COMMUNICATION_ERROR;
    }
    /* Authenticated protected ID/CAP, not just a handshake or plaintext identity. */
    memset(device, 0, sizeof(*device));
    enum probe_status status = probe_read_information(&request->device, 1, device);
    if (commissioning_cancelled()) return COMM_CANCELLED;
    device->authenticated = status == PROBE_OK;
    return probe_code(status);
}
static bool supports_security(const struct probe_result *device) {
    for (size_t i = 0; i < device->capabilities_size; i += 3)
        if (device->capabilities[i] == 9 && (device->capabilities[i+1] & 1)) return true;
    return false;
}

void commissioning_execute(struct commissioning_request *request, struct commissioning_result *result)
{
    memset(result, 0, sizeof(*result));
    result->code = COMM_INVALID_REQUEST;
    bool mutation = false;
    if (!serial_set_tx_control(request->tx_control)) return;
    serial_set_diagnostics(false);
    commissioning_begin(request->timeout_ms);
    if (request->operation == COMMISSIONING_PROBE) {
        probe_device(&request->device, request->timeout_ms, 1, &result->device);
        result->code = probe_code(result->device.status);
        goto cleanup;
    }
    if (request->operation == COMMISSIONING_SET_KEY) {
        /* Repeated calls first prove the target key and avoid another write. */
        result->code = authenticate_open(request, request->target_key, false, &result->device);
        if (result->code == COMM_OK) {
            result->authenticated = true;
            result->verified_target_key = true;
            goto cleanup;
        }
        serial_close(); clearSecureChannelKeys();
        if (result->code != COMM_AUTHENTICATION_FAILED) goto cleanup;
    }
    result->code = authenticate_open(request, request->device.key, request->device.scbk_d != 0, &result->device);
    if (result->code != COMM_OK) goto cleanup;
    result->authenticated = true;
    if (request->operation == COMMISSIONING_AUTHENTICATE) goto cleanup;
    if (!supports_security(&result->device)) {
        result->code = COMM_UNSUPPORTED_DEVICE;
        goto cleanup;
    }
    if (commissioning_cancelled() || commissioning_remaining_ms() <= 0) {
        result->code = commissioning_cancelled() ? COMM_CANCELLED : COMM_NO_RESPONSE;
        goto cleanup;
    }
    uint8_t keyset[18] = {1, 16};
    memcpy(keyset + 2, request->target_key, 16);
    mutation = true;
    int sent = (int)buildMsg(0, request->device.addr, CMD_KEYSET, keyset, sizeof(keyset));
    secure_zero(keyset, sizeof(keyset));
    result->unknown = true;
    result->code = COMM_OUTCOME_UNKNOWN;
    if (sent <= 0) goto cleanup;
    uint8_t reply[MAX_SIZE_MSG_PACKET] = {0}, code = 0;
    int size = getReply(0, &code, reply, sizeof(reply));
    if (size == sizeof(osdp_NAK) && code == REPLY_NAK) {
        result->unknown = false; mutation = false;
        result->code = reply[0] == 3 || reply[0] == 5 ? COMM_UNSUPPORTED_DEVICE : COMM_VERIFICATION_FAILED;
        secure_zero(reply, sizeof(reply));
        goto cleanup;
    }
    secure_zero(reply, sizeof(reply));
    if (size != 0 || code != REPLY_ACK) goto cleanup;
    serial_close(); clearSecureChannelKeys();
    enum commissioning_code verification = authenticate_open(request, request->target_key, false, &result->device);
    if (verification == COMM_OK) {
        result->code = COMM_OK; result->unknown = false;
        result->verified_target_key = true;
    } else if (verification == COMM_CANCELLED) result->code = COMM_CANCELLED;
    else if (verification == COMM_NO_RESPONSE) result->code = COMM_VERIFICATION_TIMEOUT;
    else result->code = COMM_VERIFICATION_FAILED;
cleanup:
    if (commissioning_cancelled()) {
        result->code = COMM_CANCELLED;
        result->unknown = mutation;
        result->verified_target_key = false;
    }
    serial_close();
    clearSecureChannelKeys(); setPlainTextMode(0);
    serial_set_diagnostics(true);
    commissioning_end();
    secure_zero(request->device.key, sizeof(request->device.key)); request->device.key_set = 0;
    secure_zero(request->target_key, sizeof(request->target_key));
}

static int emit_result(const struct commissioning_request *request, const struct commissioning_result *result)
{
    bool success = result->code == COMM_OK && !result->unknown;
    if (request->operation != COMMISSIONING_PROBE && !result->authenticated) success = false;
    if (request->operation == COMMISSIONING_SET_KEY && !result->verified_target_key) success = false;
    const char *outcome = result->unknown ? "unknown" : success ? "succeeded" : "failed";
    printf("{\"operation\":\"%s\",\"protocol\":\"osdp\",\"address\":", operation_name(request->operation));
    if (request->device.addr < 0) printf("null"); else printf("%d", request->device.addr);
    printf(",\"outcome\":\"%s\",\"code\":\"%s\",\"authenticated\":%s,\"verifiedTargetKey\":%s",
           outcome, code_name(result->code), result->authenticated ? "true" : "false",
           result->verified_target_key ? "true" : "false");
    if (result->device.has_id) {
        const uint8_t *id = result->device.id;
        printf(",\"manufacturer\":\"%02X%02X%02X\",\"model\":%u,\"firmware\":\"%u.%u.%u\"",
               id[2], id[1], id[0], id[3], id[9], id[10], id[11]);
    }
    if (result->device.has_capabilities) {
        printf(",\"capabilities\":[");
        for (size_t i = 0; i < result->device.capabilities_size; i += 3) {
            if (i) printf(",");
            printf("{\"function\":%u,\"compliance\":%u,\"count\":%u}", result->device.capabilities[i],
                   result->device.capabilities[i+1], result->device.capabilities[i+2]);
        }
        printf("]");
    }
    printf("}\n");
    if (fflush(stdout) != 0 || ferror(stdout)) return 1;
    return success ? 0 : result->unknown ? 2 : 1;
}

int commissioning_main(int argc, char **argv)
{
#ifdef _WIN32
    _setmode(_fileno(stdout), _O_BINARY); /* Contract requires LF, not CRLF. */
#endif
    struct commissioning_request request;
    struct commissioning_result result = {0};
    result.code = COMM_INVALID_REQUEST;
    if (commissioning_parse(argc, argv, &request)) commissioning_execute(&request, &result);
    else {
        secure_zero(request.device.key, sizeof(request.device.key));
        secure_zero(request.target_key, sizeof(request.target_key));
    }
    if (result.code != COMM_OK) cli_diagnostic(stderr, "osdp commissioning: %s\n", code_name(result.code));
    if (result.code != COMM_OK) cli_error_code(code_name(result.code));
    int exit_code = emit_result(&request, &result);
    secure_zero(&request, sizeof(request)); secure_zero(&result, sizeof(result));
    return exit_code;
}
