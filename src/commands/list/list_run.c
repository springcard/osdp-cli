#include "../../cli/command_result.h"
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#ifdef _WIN32
#include <fcntl.h>
#include <io.h>
#endif

#include "list_options.h"
#include "../probe/probe_result.h"
#include "../../cli/global_options_parser.h"
#include "../../moduleOSDP/osdp_cmd.h"
#include "../../moduleOSDP/osdp_packet.h"
#include "../../moduleOSDP/osdp_sc.h"
#include "../../platform/serial.h"
#include "../../core/commissioning_control.h"
#include "../../util/utils.h"
#include "../../logger/logger.h"

static void emit_capabilities_json(const struct probe_result *device)
{
    putchar('[');
    for (size_t i = 0; i < device->capabilities_size; i += 3) {
        if (i) putchar(',');
        printf("{\"function\":%u,\"compliance\":%u,\"count\":%u}",
               device->capabilities[i], device->capabilities[i + 1],
               device->capabilities[i + 2]);
    }
    putchar(']');
}

static void emit_device_json(int address, const struct probe_result *device)
{
    const uint8_t *id = device->id;
    printf("{\"address\":%d,\"informationStatus\":\"%s\",", address,
           device->status == PROBE_OK ? "complete" : "partial");
    printf("\"manufacturer\":\"%02X%02X%02X\",\"model\":%u,\"version\":%u,", id[2], id[1], id[0], id[3], id[4]);
    printf("\"serialNumber\":\"%02X%02X%02X%02X\",", id[8], id[7], id[6], id[5]);
    printf("\"firmware\":\"%u.%u.%u\",\"capabilities\":", id[9], id[10], id[11]);
    emit_capabilities_json(device);
    putchar('}');
}

static void emit_device_text(int address, const struct probe_result *device)
{
    const uint8_t *id = device->id;
    printf("Address %d: vendor=%02X%02X%02X; model=%u; version=%u; serial=%02X%02X%02X%02X; "
           "firmware=%u.%u.%u; information=%s\n", address, id[2], id[1], id[0], id[3], id[4],
           id[8], id[7], id[6], id[5], id[9], id[10], id[11],
           device->status == PROBE_OK ? "complete" : "partial");
    for (size_t i = 0; i < device->capabilities_size; i += 3)
        printf("  capability=%u; compliance=%u; count=%u\n", device->capabilities[i],
               device->capabilities[i + 1], device->capabilities[i + 2]);
}

int list_run(struct cli_context *ctx)
{
    struct list_options *opts = &ctx->list_opts;
    ctx->glob_options.json = ctx->glob_options.json || opts->json;
    logger_console_init(ctx);
    bool opened = false;
    int found = 0;
    int status = 0;
    int scanned = 0, partial = 0;
    static struct probe_result results[127];
    memset(results, 0, sizeof(results));
#ifdef _WIN32
    if (opts->json) _setmode(_fileno(stdout), _O_BINARY);
#endif
    serial_set_diagnostics(false);
    clearSecureChannelKeys();
    osdpPacketInit(0, CONTROL_PANEL);
    {
        int budget = (opts->last_addr - opts->first_addr + 1) * opts->timeout * opts->retries * 2 + 1000;
        if (budget > 600000) budget = 600000;
        commissioning_begin(opts->deadline ? opts->deadline : budget);
    }
    if (!serial_set_tx_control(opts->tx_control) || !serial_open(opts->port, opts->baud, opts->timeout)) {
        if (opts->json) {
            if (opts->progress) fputs("{\"type\":\"result\",", stdout);
            else putchar('{');
            printf("\"protocol\":\"osdp\",\"operation\":\"list\",\"outcome\":\"failed\","
                   "\"code\":\"port_unavailable\",\"deviceCount\":0,\"devices\":[]}\n");
            if (fflush(stdout) != 0 || ferror(stdout)) status = 1;
        } else
            cli_diagnostic(stderr, "list: serial port unavailable\n");
        cli_error_code("port_unavailable");
        status = 1;
        goto cleanup;
    }
    opened = true;

    if (!opts->json)
        printf("Scanning OSDP addresses %d..%d on %s at %s baud\n",
               opts->first_addr, opts->last_addr, opts->port, opts->baud);

    for (int address = opts->first_addr; address <= opts->last_addr; ++address) {
        struct probe_options probe = {0};
        struct probe_result *device = &results[address];
        if (commissioning_remaining_ms() == 0) { partial = 1; break; }
        probe.port = opts->port;
        probe.baud = opts->baud;
        probe.addr = address;
        osdpPacketInit(0, CONTROL_PANEL);
        device->status = probe_read_information(&probe, opts->retries, device);
        ++scanned;
        if (device->status != PROBE_OK && (device->has_id || device->responded || device->status != PROBE_NO_RESPONSE)) partial = 1;
        if (device->has_id) ++found;
        if (opts->progress) {
            printf("{\"type\":\"progress\",\"address\":%d,\"scannedAddressCount\":%d,\"requestedAddressCount\":%d,\"baud\":%d}\n",
                address, scanned, opts->last_addr - opts->first_addr + 1, atoi(opts->baud));
            if (device->has_id) {
                fputs("{\"type\":\"device\",\"device\":", stdout);
                emit_device_json(address, device); puts("}");
            }
            if (fflush(stdout) != 0 || ferror(stdout)) { status = 1; goto cleanup; }
        }
        /* Yield between peripherals, after their complete exchange in RX.
         * Never sleep while draining TX or before receiving a reply. */
        if (opts->tx_control >= 0 && address < opts->last_addr)
            delay_ms(15);
    }
    if (commissioning_cancelled()) partial = 1;
    status = partial ? 2 : 0;
    if (partial) cli_error_code(commissioning_cancelled() ? "cancelled" : "partial_scan");
    if (opts->json) {
        int emitted = 0;
        if (opts->progress) fputs("{\"type\":\"result\",", stdout);
        else putchar('{');
        printf("\"protocol\":\"osdp\",\"operation\":\"list\",\"outcome\":\"%s\",\"code\":\"%s\","
               "\"firstAddress\":%d,\"lastAddress\":%d,\"requestedAddressCount\":%d,\"scannedAddressCount\":%d,\"devices\":[",
               partial ? "partial" : "succeeded", commissioning_cancelled() ? "cancelled" : partial ? "partial_scan" : "ok",
               opts->first_addr, opts->last_addr, opts->last_addr - opts->first_addr + 1, scanned);
        for (int address = opts->first_addr; address < opts->first_addr + scanned; ++address) {
            if (!results[address].has_id) continue;
            if (emitted++) putchar(',');
            emit_device_json(address, &results[address]);
        }
        printf("],\"deviceCount\":%d,\"observations\":[", found);
        emitted = 0;
        for (int address = opts->first_addr; address < opts->first_addr + scanned; ++address) {
            const struct probe_result *device = &results[address];
            const char *code;
            if (device->status == PROBE_OK || (device->status == PROBE_NO_RESPONSE && !device->has_id && !device->responded)) continue;
            code = device->has_id ? "capabilities_unavailable" : device->identification_refused ? "identification_refused" :
                   device->status == PROBE_UNSUPPORTED_DEVICE ? "unsupported_device" : "communication_error";
            if (emitted++) putchar(',');
            printf("{\"address\":%d,\"code\":\"%s\",\"identificationAvailable\":%s,\"responded\":%s,\"reason\":\"%s\"}",
                   address, code, device->has_id ? "true" : "false", device->responded || device->has_id ? "true" : "false",
                   probe_status_name(device->status));
        }
        puts("]}");
    } else {
        for (int address = opts->first_addr; address < opts->first_addr + scanned; ++address) {
            const struct probe_result *device = &results[address];
            if (device->has_id) emit_device_text(address, device);
            if (device->status != PROBE_OK && (device->responded || device->has_id || device->status != PROBE_NO_RESPONSE))
                printf("Address %d: %s%s\n", address, probe_status_name(device->status), device->has_id ? " (identity available)" : " (identity unavailable)");
        }
        printf("Found %d OSDP device%s.\n", found, found == 1 ? "" : "s");
    }
    if (fflush(stdout) != 0 || ferror(stdout)) status = 1;

cleanup:
    if (opened) serial_close();
    commissioning_end();
    clearSecureChannelKeys();
    setPlainTextMode(0);
    serial_set_diagnostics(true);
    return status;
}
