/**
 * @file info_run.c
 * @brief Implementation for executing the info command
 * 
 * The code provided implements the execution of the "info" command in the osdp-cli application. 
 * The code provided also displays the parsed options for the info command for testing purposes.
 * The functions takes the CLI context and the parsed options for the info command, and it performs the desired actions based on those options.
 */
#include <stdio.h>
#include <string.h>
#include <stdint.h>

#include "info_options.h"
#include "../../cli/global_options_parser.h"
#include "../../util/utils.h"
#include "../../platform/serial.h"
#include "../../logger/logger.h"
#include "../../moduleOSDP/osdp_common.h"
#include "../../moduleOSDP/osdp_cp.h"
#include "../../moduleOSDP/osdp_packet.h"
#include "../../moduleOSDP/osdp_cmd.h"
#include "../../moduleOSDP/osdp_sc.h"


#include <stdlib.h>

//todo a revoir les commentaires!!!!!!
/**
 * @brief Execute the info command
 * 
 * This function executes the info command based on the CLI context (globale and specific command options).
 * It checks for the presence of required options such as the serial port, baud rate, and device address.
 * todo Ajouter la description de l'enchaînement des commandes OSDP pour récupérer les informations de l'appareil, en utilisant les options fournies dans la structure `info_options` pour configurer la communication série et les paramètres de sécurité.
 * The function returns an integer status code indicating the success or failure of the command execution.
 * 
 * @param[in] ctx The CLI context
 * 
 * @return An integer status code indicating the success or failure of the command execution
 */
int info_run(struct cli_context *ctx)
{
    uint8_t replyPacket[MAX_SIZE_MSG_PACKET];
    unsigned char replyCode;

    if (!ctx->info_opts.port) {
        printf("info: --port required\n");
        return 1;
    }    

    if (!serial_open(ctx->info_opts.port, ctx->info_opts.baud, ctx->glob_options.timeout)) {
        printf("Error, arguments missing(port/baudrate)\n");
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
    if(ctx->info_opts.scbk_d || ctx->info_opts.scbk_set) {
        // if scbk is provided use it for the secure channel establishment process 
        if(ctx->info_opts.scbk_set)
            setNewScbk(ctx->info_opts.scbk);
        setSecureChannelMode(0);
        
        if(!openSecureChannelSession2(0, ctx->info_opts.addr, ctx->glob_options.retries))
            return 1;
    }

    do{
        // send the ID command to retrieve the vendor code, model number, serial numberi and firmware version of the device
        Command_OSDP_ID(0, ctx->info_opts.addr);

        int size = getReply(0, &replyCode, replyPacket, sizeof(replyPacket));

        if(replyCode == REPLY_PDID) 
        {
            // command executed successfully, skip to OSDP_CAP sending
        }
        else if(replyCode == REPLY_NAK)
        {
            ctx->glob_options.retries--; // decrement the retry count in the CLI context
            continue; // retry sending the command
        }
        else 
        {
            ctx->glob_options.retries--; // decrement the retry count in the CLI context
            continue; // retry sending the command
        }

        do{
            // send the CAP command to retrieve the capabilities of the device
            Command_OSDP_CAP(0, ctx->info_opts.addr);

            size = getReply(0, &replyCode, replyPacket, sizeof(replyPacket));

            if(replyCode == REPLY_PDCAP) 
            {
                // command executed successfully, exit the retry loop
                break;
            }
            else if(replyCode == REPLY_NAK)
            {
                ctx->glob_options.retries--; // decrement the retry count in the CLI context
                continue; // retry sending the command
            }
            else 
            {
                ctx->glob_options.retries--; // decrement the retry count in the CLI context
                continue; // retry sending the command
            }
        } while(replyCode == REPLY_NAK && ctx->glob_options.retries); // retry sending the OSDP_CAP command if a NAK is received, and if retries not exhausted
    } while(replyCode == REPLY_NAK && ctx->glob_options.retries); // retry sending the command OSDP_ID if a NAK is received, and if retries not exhausted

    if(ctx->glob_options.log_file_name != NULL)
        logger_file_close();

    serial_close();

    return 0;   
}


// todo -> a revoir!!!!!, pour les test unitaires, on peut faire un sniff_test qui affiche les options et vérifie que les options sont bien parsées, 
// todo -> et un sniff_run qui fait le vrai travail de sniffing, et qui sera testé manuellement
/**
 * @brief Display the parsed options for the info command (for testing purposes)
 * 
 * The function is responsible for displaying the parsed options for the info command in the osdp-cli application.
 * The function takes the CLI context and the parsed options for the command. 
 * The function checks for the required options such as the serial port, baud rate and device address.
 * 
 * @param[in] ctx The CLI context
 * @param[in] opts The parsed options for the info command
 * 
 * @return An integer status code indicating the success or failure of the check options of comand.
 */
int info_test(struct cli_context *ctx)
{

    if (!ctx->info_opts.port) {
        printf("info: --port required\n");
        return 1;
    }

    printf("Info start\n");
    printf("port: %s\n", ctx->info_opts.port);
    printf("baud: %s\n", ctx->info_opts.baud);
    printf("addr: %d\n", ctx->info_opts.addr);

    if(ctx->info_opts.scbk_set) {
        printf("SCBK: [redacted]\n");
    }

    if(ctx->info_opts.scsk_set) {
        printf("SCSK: [redacted]\n");
    }

    return 0;    
}