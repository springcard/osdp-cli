/**
 * @file raw_run.c
 * @brief Implementation for executing the raw subcommand
 * 
 * The code provided implements the execution of the "raw" subcommand in the osdp-cli application. 
 * The code provided also displays the parsed options for the raw subcommand for testing purposes.
 * The functions takes the CLI context and the parsed options for the raw subcommand, and it performs the desired actions based on those options.
 */
#include <stdio.h>
#include <string.h>
#include <stdint.h>
#ifndef _WIN32
#include <unistd.h>
#endif

#include "raw_options.h"
#include "../../../cli/global_options_parser.h"
#include "../../../util/utils.h"
#include "../../../platform/serial.h"
#include "../../../logger/logger.h"
#include "../../../moduleOSDP/osdp_cp.h"
#include "../../../moduleOSDP/osdp_packet.h"
#include "../../../moduleOSDP/osdp_cmd.h"
#include "../../../moduleOSDP/osdp_sc.h"

#include <stdlib.h>

//todo  Il faudrait ajouter des vérifications pour s'assurer que les options fournies sont cohérentes avec la commande.


/**
 * @brief Execute the raw subcommand
 * 
 * This function executes the raw subcommand based on the CLI context (globale and specific command options).
 * It checks for the presence of required options such as the serial port, baud rate, and device addressand more...
 * ! The function establishes a secure channel session if the appropriate options are provided, sends the osdp_XXXXXX command to manage Reader XXXXXXXXXX indicators, 
 * and handles the response from the device.
 * The secure channel session is established using either the provided SCBK or the default SCBK, and the function handles the communication with the OSDP device accordingly.
 * 
 * @param[in] ctx The CLI context
 * 
 * @return An integer status code indicating the success or failure of the command execution
 */
int raw_run(struct cli_context *ctx)
{
    uint8_t replyPacket[MAX_SIZE_MSG_PACKET];
    uint8_t replyCode;
    int size = 0;

    if (!ctx->raw_opts.port) {
        printf("raw: --port required\n");
        return 1;
    }

    if(!ctx->raw_opts.cmd) {
            printf("raw: --cmd <hex string> required\n");
            return 1;
    }

    if(!ctx->raw_opts.data) {
        printf("raw: --data <hex string> required\n");
        return 1;
    }

    if (!serial_open(ctx->raw_opts.port, ctx->raw_opts.baud, ctx->glob_options.timeout)) {
        printf("Error opening communication port (port/speed)\n");
        return -1;
    }

    osdpPacketInit(0 /*indReader*/, CONTROL_PANEL);

    logger_console_init(ctx);
    
    if(ctx->glob_options.log_file_name != NULL)
    {
        if (logger_file_init(ctx) != 0) {
            fprintf(stderr, "Failed to open log file\n");
            return 1;
        }
    }

    // if secure channel mode is requested, establishing a secure channel is required before sending any command
    if(ctx->raw_opts.scbk_d || ctx->raw_opts.scbk_set) {
        // if scbk is provided set and use it for the secure channel establishment process 
        if(ctx->raw_opts.scbk_set)
            setNewScbk(ctx->raw_opts.scbk);
        setSecureChannelMode(0);
        
        if(!openSecureChannelSession2(0, ctx->raw_opts.addr, ctx->glob_options.retries))
            return 1;
    }

    do{
        Command_OSDP_raw(0, (uint8_t)ctx->raw_opts.addr, *ctx->raw_opts.cmd, (uint8_t *)ctx->raw_opts.data, ctx->raw_opts.data_len);

        size = getReply(0, &replyCode, replyPacket, sizeof(replyPacket));

        if(replyCode == REPLY_ACK) 
        {
            break;
        }
        else if(replyCode == REPLY_NAK)
        {
            ctx->glob_options.retries--;
        }
        else if(replyCode == REPLY_MFGREP)
        {
            break;
        }
        else
        {
            ctx->glob_options.retries--;
        }
        
    }while(replyCode == REPLY_NAK && ctx->glob_options.retries);

    if(ctx->glob_options.log_file_name != NULL)
        logger_file_close();

    serial_close();

    return 0;   
}


// todo -> a revoir!!!!!, pour les test unitaires, on peut faire un raw_test qui affiche les options et vérifie que les options sont bien parsées, 
/**
 * @brief Display the parsed options for the raw subcommand (for testing purposes)
 * 
 * The function is responsible for displaying the parsed options for the raw subcommand in the osdp-cli application.
 * The function takes the CLI context and the parsed options for the command. 
 * The function checks for the required options such as the serial port, baud rate, device address...
 * 
 * @param[in] ctx The CLI context
 * 
 * @return An integer status code indicating the success or failure of the check options of comand.
 */
int raw_test(struct cli_context *ctx)
{

    if (!ctx->raw_opts.port) {
        printf("raw: --port required\n");
        return 1;
    }

    if (!ctx->raw_opts.baud) {
        printf("raw: --baud required\n");
        return 1;
    }

    if (ctx->raw_opts.addr == -1) {
        printf("raw: --addr required\n");
        return 1;
    }

    if(!ctx->raw_opts.cmd) {
        printf("raw: --cmd <hex string> required\n");
        return 1;
    }

    if(!ctx->raw_opts.data) {
        printf("raw: --data <hex string> required\n");
        return 1;
    }

    printf("Raw command options:\n");

    printf("raw start\n");
    printf("port: %s\n", ctx->raw_opts.port);
    printf("baud: %s\n", ctx->raw_opts.baud);
    printf("addr: %d\n", ctx->raw_opts.addr);

    if(ctx->raw_opts.scbk_set) 
    {
        printf("SCBK: [redacted]\n");
    }
    else 
    {
        printf("SCBK not set \n");
    }

    (ctx->raw_opts.scbk_d ) ? printf("SCBK-D flag is set \n") : printf("SCBK-D flag is not set \n");

    if(ctx->raw_opts.cmd_len) 
    {
        printf("cmd: ");
        for (int i = 0; i < ctx->raw_opts.cmd_len; i++) 
        {
            printf("%02X ", *(ctx->raw_opts.cmd + i));
        }
        printf("\n");
    }
    else 
    {
        printf("cmd not set \n");
    }

    if(ctx->raw_opts.cmd_len && ctx->raw_opts.cmd[0] == CMD_KEYSET) {
        printf("data: [redacted]\n");
    }
    else if(ctx->raw_opts.data_len)
    {
        printf("data: ");
        for (int i = 0; i < ctx->raw_opts.data_len; i++) 
        {
            printf("%02X ", ctx->raw_opts.data[i]);
        }
        printf("\n");
    }
    else 
    {
        printf("data not set \n");
    }

    return 0;    
}
