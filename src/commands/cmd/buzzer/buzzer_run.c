/**
 * @file buzzer_run.c
 * @brief Implementation for executing the buzzer subcommand
 * 
 * The code provided implements the execution of the "buzzer" subcommand in the osdp-cli application. 
 * The code provided also displays the parsed options for the buzzer subcommand for testing purposes.
 * The functions takes the CLI context and the parsed options for the buzzer subcommand, and it performs the desired actions based on those options.
 */
#include <stdio.h>
#include <string.h>
#include <stdint.h>
#ifndef _WIN32
#include <unistd.h>
#endif

#include "buzzer_options.h"
#include "../../../cli/global_options_parser.h"
#include "../../../util/utils.h"
#include "../../../platform/serial.h"
#include "../../../logger/logger.h"
#include "../../../moduleOSDP/osdp_cp.h"
#include "../../../moduleOSDP/osdp_packet.h"
#include "../../../moduleOSDP/osdp_cmd.h"
#include "../../../moduleOSDP/osdp_sc.h"

#include <stdlib.h>

//todo, suivant le mode choisi, il y a une grammaire à respecter pour les options, par exemple, si le mode est "on" ou "off", 
//todo  les options onTime, offTime et count ne sont pas nécessaires... 
//todo  si le mode est "beep",??????????????????????? 
//todo  si le mode est pattern, ?????????????????????? 
//todo  Il faudrait ajouter des vérifications (ici ou ailleurs) pour s'assurer que les options fournies sont cohérentes avec le mode choisi.


/**
 * @brief Execute the buzzer subcommand
 * 
 * This function executes the buzzer subcommand based on the CLI context (globale and specific command options).
 * It checks for the presence of required options such as the serial port, baud rate, and device addressand more...
 * The function establishes a secure channel session if the appropriate options are provided, sends the osdp_BUZZER command to manage Reader BUZZER indicators, 
 * and handles the response from the device.
 * The secure channel session is established using either the provided SCBK or the default SCBK, and the function handles the communication with the OSDP device accordingly.
 * 
 * @param[in] ctx The CLI context
 * 
 * @return An integer status code indicating the success or failure of the command execution
 */
int buzzer_run(struct cli_context *ctx)
{
    uint8_t replyPacket[MAX_SIZE_MSG_PACKET];
    uint8_t replyCode;
    int size = 0;
    uint8_t toneCode = 0; // Assuming a default tone code no change in tone

    if (!ctx->buzzer_opts.port) {
        printf("buzzer: --port <> required\n");
        return 1;
    }

    if(ctx->buzzer_opts.buzzer_mode == BUZZER_MODE_NONE) {
        printf("buzzer: --mode <on|off|beep|pattern> required\n");
        return 1;
    }

    if (!serial_open(ctx->buzzer_opts.port, ctx->buzzer_opts.baud, ctx->glob_options.timeout)) {
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
    if(ctx->buzzer_opts.scbk_d || ctx->buzzer_opts.scbk_set) {
        // if scbk is provided set and use it for the secure channel establishment process 
        if(ctx->buzzer_opts.scbk_set)
            setNewScbk(ctx->buzzer_opts.scbk);
        setSecureChannelMode(0);

        if(!openSecureChannelSession2(0, ctx->buzzer_opts.addr, ctx->glob_options.retries))
            return 1;        
    }

    //memset(&permParam,0,sizeof(perm_BUZZER_Settings));
    switch (ctx->buzzer_opts.buzzer_mode)
    {
        case BUZZER_MODE_OFF:
            toneCode = 1;
            ctx->buzzer_opts.onTime = 0;
            ctx->buzzer_opts.offTime = 0;
            ctx->buzzer_opts.count = 0;
            break;

        case BUZZER_MODE_ON:
            toneCode = 2;
            ctx->buzzer_opts.onTime = 100;
            ctx->buzzer_opts.offTime = 0;
            ctx->buzzer_opts.count = 0;  
            break;

        case BUZZER_MODE_BEEP:
            toneCode = 2;
            if(ctx->buzzer_opts.count == -1 && ctx->buzzer_opts.onTime == -1 && ctx->buzzer_opts.offTime == -1) 
            {// count is not set
                ctx->buzzer_opts.onTime = 100;
                ctx->buzzer_opts.offTime = 0;
                ctx->buzzer_opts.count = 1;
            }
            else if( ctx->buzzer_opts.count == -1 && (ctx->buzzer_opts.onTime != -1 || ctx->buzzer_opts.offTime != -1))
            {
                ctx->buzzer_opts.count = 0; // tone continues until another tone command is received
            }
            break;

        case BUZZER_MODE_PATTERN:
            toneCode = 2;
            break;
    
        default:
            break;
    }

    do{
        Command_OSDP_BUZ(0, (uint8_t)ctx->buzzer_opts.addr, 0 /* readerNumber */, toneCode, (uint8_t)ctx->buzzer_opts.onTime/100, (uint8_t)ctx->buzzer_opts.offTime/100, (uint8_t)ctx->buzzer_opts.count);
        size = getReply(0, &replyCode, replyPacket, sizeof(replyPacket));

        if(replyCode == REPLY_ACK) 
        {
            break;
        }
        else if(replyCode == REPLY_NAK)
        {
            ctx->glob_options.retries--;
        }
        else
        {
            ctx->glob_options.retries--;
        }

    }while(replyCode == REPLY_NAK && ctx->glob_options.retries); // retry if NAK received and retry option not zero


    if(ctx->glob_options.log_file_name != NULL)
        logger_file_close();

    serial_close();

    return 0;   
}


// todo -> a revoir!!!!!, pour les test unitaires, on peut faire un buzzer_test qui affiche les options et vérifie que les options sont bien parsées, 
/**
 * @brief Display the parsed options for the buzzer subcommand (for testing purposes)
 * 
 * The function is responsible for displaying the parsed options for the buzzer subcommand in the osdp-cli application.
 * The function takes the CLI context and the parsed options for the command. 
 * The function checks for the required options such as the serial port, baud rate and device address...
 * 
 * @param[in] ctx The CLI context
 * 
 * @return An integer status code indicating the success or failure of the check options of comand.
 */
int buzzer_test(struct cli_context *ctx)
{

    if (!ctx->buzzer_opts.port) {
        printf("buzzer: --port required\n");
        return 1;
    }

    if (!ctx->buzzer_opts.baud) {
        printf("buzzer: --baud required\n");
        return 1;
    }

    if (ctx->buzzer_opts.addr == -1) {
        printf("buzzer: --addr required\n");
        return 1;
    }

    if(ctx->buzzer_opts.buzzer_mode == BUZZER_MODE_NONE) {
        printf("buzzer: --mode <on|off|beep|pattern> required\n");
        return 1;
    }

    printf("BUZZER command options:\n");

    printf("port: %s\n", ctx->buzzer_opts.port);
    printf("baud: %s\n", ctx->buzzer_opts.baud);
    printf("addr: %d\n", ctx->buzzer_opts.addr);

    if(ctx->buzzer_opts.scbk_set) 
    {
        printf("SCBK: [redacted]\n");
    }
    else 
    {
        printf("SCBK not set \n");
    }

    (ctx->buzzer_opts.scbk_d ) ? printf("SCBK-D flag is set \n") : printf("SCBK-D flag is not set \n");

    char * pszMode[] = { "OFF", "ON", "BEEP", "PATTERN" };
    printf("buzzer mode: %s\n", pszMode[ctx->buzzer_opts.buzzer_mode]);
    printf("buzzer on time: %d\n", ctx->buzzer_opts.onTime);
    printf("buzzer off time: %d\n", ctx->buzzer_opts.offTime);
    printf("buzzer count: %d\n", ctx->buzzer_opts.count);

    return 0;    
}
