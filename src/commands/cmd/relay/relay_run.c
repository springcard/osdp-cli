/**
 * @file relay_run.c
 * @brief Implementation for executing the relay subcommand
 * 
 * The code provided implements the execution of the "relay" subcommand in the osdp-cli application. 
 * The code provided also displays the parsed options for the relay subcommand for testing purposes.
 * The functions takes the CLI context and the parsed options for the relay subcommand, and it performs the desired actions based on those options.
 */
#include <stdio.h>
#include <string.h>
#include <stdint.h>
#ifndef _WIN32
#include <unistd.h>
#endif

#include "relay_options.h"
#include "../../../cli/global_options_parser.h"
#include "../../../util/utils.h"
#include "../../../platform/serial.h"
#include "../../../logger/logger.h"
#include "../../../moduleOSDP/osdp_cp.h"
#include "../../../moduleOSDP/osdp_packet.h"
#include "../../../moduleOSDP/osdp_cmd.h"
#include "../../../moduleOSDP/osdp_sc.h"

#include <stdlib.h>

static void populateRelayParameters(struct relay_options *opts, uint8_t *ctrlCode, uint8_t *lsbTimer, uint8_t *msbTimer);

//todo, suivant le mode choisi, il y a une grammaire/syntaxe à respecter pour les options, par exemple le mode est "off" n'utilise pas l'option duraton... 
//todo  Il faudrait ajouter des vérifications pour s'assurer que les options fournies sont cohérentes avec le mode choisi.
//  .\build\osdp-cli.exe cmd relay --port COM11 --baud 9600 --addr 0 --output 0 --mode on
//  .\build\osdp-cli.exe cmd relay --port COM11 --baud 9600 --addr 0 --output 0 --mode on --duration 100
//  .\build\osdp-cli.exe cmd relay --port COM11 --baud 9600 --addr 0 --output 2 --mode off
//  .\build\osdp-cli.exe cmd relay --port COM11 --baud 9600 --addr 0 --output 3 --mode pulse
//  .\build\osdp-cli.exe cmd relay --port COM11 --baud 9600 --addr 0 --output 0 --mode pulse --duration 5000


/**
 * @brief Execute the relay subcommand
 * 
 * This function executes the relay subcommand based on the CLI context (globale and specific command options).
 * It checks for the presence of required options such as the serial port, baud rate, and device addressand more...
 * The function establishes a secure channel session if the appropriate options are provided, sends the osdp_OUT command to manage Reader relay outputs, 
 * and handles the response from the device.
 * The secure channel session is established using either the provided SCBK or the default SCBK, and the function handles the communication with the OSDP device accordingly.
 * 
 * @param[in] ctx The CLI context
 * 
 * @return An integer status code indicating the success or failure of the command execution
 */
int relay_run(struct cli_context *ctx)
{
    uint8_t replyPacket[MAX_SIZE_MSG_PACKET];
    uint8_t replyCode;

    uint8_t ctrlCode = 0;
    uint8_t lsbTimer = 0;
    uint8_t msbTimer = 0;
    int durationSec = 0;

    int size = 0;

    if (!ctx->relay_opts.port) {
        printf("relay: --port required\n");
        return 1;
    }

    if(ctx->relay_opts.output_number == -1) {
        printf("relay: --output <n> required\n");
        return 1;
    }

    if(ctx->relay_opts.relay_mode == RELAY_MODE_NONE) {
        //printf("relay: --mode <on|off|pulse> required\n");
        printf("relay: --mode <on|off> required\n");
        return 1;
    }

    if (!serial_open(ctx->relay_opts.port, ctx->relay_opts.baud, ctx->glob_options.timeout)) {
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
    if(ctx->relay_opts.scbk_d || ctx->relay_opts.scbk_set) {
        // if scbk is provided set and use it for the secure channel establishment process 
        if(ctx->relay_opts.scbk_set)
            setNewScbk(ctx->relay_opts.scbk);
        setSecureChannelMode(0);
        
        if(!openSecureChannelSession2(0, ctx->relay_opts.addr, ctx->glob_options.retries))
            return 1;
    }

    populateRelayParameters(&ctx->relay_opts, &ctrlCode, &lsbTimer, &msbTimer);

    do{
        Command_OSDP_OUT(0, (uint8_t)ctx->relay_opts.addr, (uint8_t)ctx->relay_opts.output_number, ctrlCode, lsbTimer, msbTimer);
        size = getReply(0, &replyCode, replyPacket, sizeof(replyPacket));

        if(replyCode == REPLY_ACK) 
        {
            break; // exit the loop if ACK is received
        }
        else if(replyCode == REPLY_NAK)
        {
            ctx->glob_options.retries--;
        }
        else
        {
            ctx->glob_options.retries--;
        }

    } while(replyCode == REPLY_NAK && ctx->glob_options.retries); // retry sending the command until retries are exhausted or an ACK is received
 
    if(ctx->glob_options.log_file_name != NULL)
        logger_file_close();
        
    serial_close();

    return 0;   
}


// todo -> a revoir!!!!!, pour les test unitaires, on peut faire un relay_test qui affiche les options et vérifie que les options sont bien parsées, 
/**
 * @brief Display the parsed options for the relay subcommand (for testing purposes)
 * 
 * The function is responsible for displaying the parsed options for the relay subcommand in the osdp-cli application.
 * The function takes the CLI context and the parsed options for the command. 
 * The function checks for the required options such as the serial port, baud rate and device address...
 * 
 * @param[in] ctx The CLI context
 * 
 * @return An integer status code indicating the success or failure of the check options of comand.
 */
int relay_test(struct cli_context *ctx)
{

    if (!ctx->relay_opts.port) {
        printf("relay: --port required\n");
        return 1;
    }

    if (!ctx->relay_opts.baud) {
        printf("relay: --baud required\n");
        return 1;
    }

    if (ctx->relay_opts.addr == -1) {
        printf("relay: --addr required\n");
        return 1;
    }

    if(ctx->relay_opts.output_number == -1) {
        printf("relay: --output <n> required\n");
        return 1;
    }

    if(ctx->relay_opts.relay_mode == RELAY_MODE_NONE) {
        //printf("relay: --mode <on|off|pulse> required\n");
        printf("relay: --mode <on|off> required\n");
        return 1;
    }

    printf("RELAY command options:\n");

    printf("relay start\n");
    printf("port: %s\n", ctx->relay_opts.port);
    printf("baud: %s\n", ctx->relay_opts.baud);
    printf("addr: %d\n", ctx->relay_opts.addr);

    if(ctx->relay_opts.scbk_set) 
    {
        printf("SCBK: [redacted]\n");
    }
    else 
    {
        printf("SCBK not set \n");
    }

    (ctx->relay_opts.scbk_d ) ? printf("SCBK-D flag is set \n") : printf("SCBK-D flag is not set \n");

    printf("relay output number: %d\n", ctx->relay_opts.output_number);
    char *mode[] = {"off", "on"/*, "pulse"*/};
    printf("relay mode: %s\n", mode[ctx->relay_opts.relay_mode]);
    if(ctx->relay_opts.duration != -1 ) 
    {
       printf("relay duration: %d ms\n", ctx->relay_opts.duration);
    }
    else 
    {
        printf("relay duration: not set\n");
    }

    return 0;    
}


/**
 * @brief Populate relay parameters based on the relay mode and duration
 * 
 * This function initializes the control code and timer values for the relay command based on the specified mode and duration.
 * 
 * @param[in] opts The parsed options for the relay subcommand
 * @param[out] ctrlCode The control code for the relay command
 * @param[out] lsbTimer The least significant byte of the timer value
 * @param[out] msbTimer The most significant byte of the timer value
 */
static void populateRelayParameters(struct relay_options *opts, uint8_t *ctrlCode, uint8_t *lsbTimer, uint8_t *msbTimer)
{
    int durationSec = 0;
    
    switch (opts->relay_mode)
    {
        case RELAY_MODE_OFF:
            if(opts->duration != -1) 
            {
                *ctrlCode = 0x06; // temporary state off
                durationSec = (opts->duration /100); // ms -> seconds
                *lsbTimer = (uint8_t)(durationSec & 0xFF);
                *msbTimer = (uint8_t)((durationSec >> 8) & 0xFF);               
            }
            else
            {
                *ctrlCode = 0x01; // permanent state off
                *lsbTimer = 0;
                *msbTimer = 0;
            }
            break;

        case RELAY_MODE_ON:
            if(opts->duration != -1) 
            {
                *ctrlCode = 0x05; // temporary state on
                durationSec = (opts->duration /100); // ms -> seconds
                *lsbTimer = (uint8_t)(durationSec & 0xFF);
                *msbTimer = (uint8_t)((durationSec >> 8) & 0xFF);               
            }
            else
            {
                *ctrlCode = 0x02; // permanent state on
                *lsbTimer = 0;
                *msbTimer = 0;
            }
            break;

        // case RELAY_MODE_PULSE:
        //     *ctrlCode = 0x03; // pulse
        //     durationSec = (opts->duration /100); // ms -> seconds
        //     *lsbTimer = (uint8_t)(durationSec & 0xFF);
        //     *msbTimer = (uint8_t)((durationSec >> 8) & 0xFF);               
        //     break;
    
        default:
            break;
    }
}
