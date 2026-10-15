#include "../../cli/command_result.h"
/**
 * @file config_run.c
 * @brief Implementation for executing the config command
 * 
 * The code provided implements the execution of the "config" command in the osdp-cli application. 
 * The code provided also displays the parsed options for the config command for testing purposes.
 * The functions takes the CLI context and the parsed options for the config command, and it performs the desired actions based on those options.
 */
#include <stdio.h>
#include <string.h>
#include <stdint.h>
#ifndef _WIN32
#include <unistd.h>
#endif

#include "config_options.h"
#include "../../cli/global_options_parser.h"
#include "../../util/utils.h"
#include "../../platform/serial.h"
#include "../../logger/logger.h"
#include "../../moduleOSDP/osdp_cp.h"
#include "../../moduleOSDP/osdp_packet.h"
#include "../../moduleOSDP/osdp_cmd.h"
#include "../../moduleOSDP/osdp_sc.h"

#include <stdlib.h>

/**
 * @brief Execute the config command
 * 
 * This function executes the config command based on the CLI context (globale and specific command options).
 * It checks for the presence of required options such as the serial port, baud rate, and device address.
 * The function establishes a secure channel session if the appropriate options are provided, sends the COMSET command to update the device configuration, and handles the response from the device.
 * The function also includes an optional verification step to verify the new configuration after the update.
 * The secure channel session is established using either the provided SCBK or the default SCBK, and the function handles the communication with the OSDP device accordingly.
 * 
 * @param[in] ctx The CLI context
 * 
 * @return An integer status code indicating the success or failure of the command execution
 */
int config_run(struct cli_context *ctx)
{
    uint8_t reply[MAX_SIZE_MSG_PACKET] = {0};
    uint8_t code = 0;
    int size = -1, status = 1;
    bool opened = false;
    int attempts = ctx->glob_options.retries > 0 ? ctx->glob_options.retries : 1;
    struct config_options *opts = &ctx->config_opts;
    if (!opts->port) {
        cli_set_error("config: --port required");
        return 1;
    }
    if (!serial_open(opts->port, opts->baud, ctx->glob_options.timeout)) {
        cli_set_error("Serial port unavailable");
        return 1;
    }
    opened = true;
    osdpPacketInit(0, CONTROL_PANEL);
    logger_console_init(ctx);
    if (ctx->glob_options.log_file_name && logger_file_init(ctx) != 0) {
        cli_set_error("Failed to open log file");
        goto cleanup;
    }
    if (opts->scbk_d || opts->scbk_set) {
        if (opts->scbk_set) setNewScbk(opts->scbk);
        if (!openSecureChannelSession2(0, opts->addr, attempts)) goto cleanup;
    }
    if (!opts->new_addr_set) opts->new_addr = opts->addr;
    if (!opts->new_baud_set) opts->new_baud = (unsigned int)strtoul(opts->baud, NULL, 10);
    for (int i = 0; i < attempts; ++i) {
        Command_OSDP_COMSET(0, opts->addr, opts->new_addr, opts->new_baud);
        size = getReply(0, &code, reply, sizeof(reply));
        if (size == sizeof(osdp_COM) && code == REPLY_COM) break;
        /* A write may already have taken effect if its reply was lost. */
        if (size < 0 || code != REPLY_NAK) break;
    }
    if (cli_reply_result(size, code, size == sizeof(osdp_COM) && code == REPLY_COM)) goto cleanup;
    osdp_COM *applied = (osdp_COM *)reply;
    if (applied->addr != opts->new_addr || applied->baudrate != opts->new_baud) {
        cli_set_error("Device did not apply the requested configuration");
        goto cleanup;
    }
    if (opts->reconnect) {
        char baud[16];
        snprintf(baud, sizeof(baud), "%u", opts->new_baud);
        serial_close();
        opened = false;
        delay_ms(1000);
        if (!serial_open(opts->port, baud, ctx->glob_options.timeout)) {
            cli_set_error("Failed to reopen serial port after configuration change");
            goto cleanup;
        }
        opened = true;
        osdpPacketInit(0, CONTROL_PANEL);
        if ((opts->scbk_d || opts->scbk_set) &&
            !openSecureChannelSession2(0, opts->new_addr, attempts)) goto cleanup;
        for (int i = 0; i < attempts; ++i) {
            Command_OSDP_ID(0, opts->new_addr);
            size = getReply(0, &code, reply, sizeof(reply));
            if (size == sizeof(osdp_PDID) && code == REPLY_PDID) break;
        }
        if (cli_reply_result(size, code, size == sizeof(osdp_PDID) && code == REPLY_PDID)) goto cleanup;
    }
    status = 0;
cleanup:
    if (ctx->glob_options.log_file_name) logger_file_close();
    if (opened) serial_close();
    return status;
}


// todo -> a revoir!!!!!, pour les test unitaires, on peut faire un config_test qui affiche les options et vérifie que les options sont bien parsées, 
/**
 * @brief Display the parsed options for the config command (for testing purposes)
 * 
 * The function is responsible for displaying the parsed options for the config command in the osdp-cli application.
 * The function takes the CLI context and the parsed options for the command. 
 * The function checks for the required options such as the serial port, baud rate and device address.
 * 
 * @param[in] ctx The CLI context
 * @param[in] opts The parsed options for the config command
 * 
 * @return An integer status code indicating the success or failure of the check options of comand.
 */
int config_test(struct cli_context *ctx)
{

    if (!ctx->config_opts.port) {
        printf("config: --port required\n");
        return 1;
    }

    if (!ctx->config_opts.baud) {
        printf("config: --baud required\n");
        return 1;
    }

    if (ctx->config_opts.addr == -1) {
        printf("config: --addr required\n");
        return 1;
    }

    printf("Config start\n");
    printf("port: %s\n", ctx->config_opts.port);
    printf("baud: %s\n", ctx->config_opts.baud);
    printf("addr: %d\n", ctx->config_opts.addr);

    if(ctx->config_opts.scbk_set) 
    {
        printf("SCBK: [redacted]\n");
    }
    else 
    {
        printf("SCBK not set \n");
    }

    (ctx->config_opts.scbk_d ) ? printf("SCBK-D flag is set \n") : printf("SCBK-D flag is not set \n");

    (ctx->config_opts.new_addr_set) ? printf("New address: %d\n", ctx->config_opts.new_addr) : printf("New address not set \n");

    (ctx->config_opts.new_baud_set)  ? printf("New baud rate: %d\n", ctx->config_opts.new_baud) : printf("New baud rate not set \n");

    (ctx->config_opts.reconnect) ? printf("Reconnect flag is set \n") : printf("Reconnect flag is not set \n");

    return 0;    
}
