/**
 * @file led_run.c
 * @brief Implementation for executing the led subcommand
 * 
 * The code provided implements the execution of the "led" subcommand in the osdp-cli application. 
 * The code provided also displays the parsed options for the led subcommand for testing purposes.
 * The functions takes the CLI context and the parsed options for the led subcommand, and it performs the desired actions based on those options.
 */
#include <stdio.h>
#include <string.h>
#include <stdint.h>
#ifndef _WIN32
#include <unistd.h>
#endif

#include "led_options.h"
#include "../../../cli/global_options_parser.h"
#include "../../../util/utils.h"
#include "../../../platform/serial.h"
#include "../../../logger/logger.h"
#include "../../../moduleOSDP/osdp_cp.h"
#include "../../../moduleOSDP/osdp_packet.h"
#include "../../../moduleOSDP/osdp_cmd.h"
#include "../../../moduleOSDP/osdp_sc.h"

#include <stdlib.h>

static void populateLedParameters(struct led_options *opts, tmp_LED_Settings *tmpParam, perm_LED_Settings *permParam);

//todo, suivant le mode choisi, il y a une grammaire/syntaxe à respecter pour les options, par exemple, si le mode est "on" ou "off", 
//todo  les options onTime, offTime et duration ne sont pas nécessaires, 
//todo  si le mode est "blink", les options onTime et offTime sont nécessaires, mais la duration est optionnelle, si le mode est "temp", 
//todo  la duration est nécessaire et les options onTime et offTime ne sont pas nécessaires. 
//todo  Il faudrait ajouter des vérifications pour s'assurer que les options fournies sont cohérentes avec le mode choisi.
//  .\build\osdp-cli.exe cmd led --port COM11 --baud 9600 --addr 0 --led 0 --mode on --color blue
//  .\build\osdp-cli.exe cmd led --port COM11 --baud 9600 --addr 0 --led 0 --mode off
//  .\build\osdp-cli.exe cmd led --port COM11 --baud 9600 --addr 0 --led 0 --mode blink --color green --on 500  --off 500
//  .\build\osdp-cli.exe cmd led --port COM11 --baud 9600 --addr 0 --led 0 --mode blink --color green --on 500  --off 500 --duration 5000
//  .\build\osdp-cli.exe cmd led --port COM11 --baud 9600 --addr 0 --led 0 --mode temp --color amber  --duration 50

/**
 * @brief Execute the led subcommand
 * 
 * This function executes the led subcommand based on the CLI context (globale and specific command options).
 * It checks for the presence of required options such as the serial port, baud rate, and device addressand more...
 * The function establishes a secure channel session if the appropriate options are provided, sends the osdp_LED command to manage Reader LED indicators, 
 * and handles the response from the device.
 * The secure channel session is established using either the provided SCBK or the default SCBK, and the function handles the communication with the OSDP device accordingly.
 * 
 * @param[in] ctx The CLI context
 * 
 * @return An integer status code indicating the success or failure of the command execution
 */
int led_run(struct cli_context *ctx)
{
    uint8_t replyPacket[MAX_SIZE_MSG_PACKET];
    uint8_t replyCode;
    int size = 0;

    if (!ctx->led_opts.port) {
        printf("led: --port required\n");
        return 1;
    }

    if (!ctx->led_opts.baud) {
        printf("led: --baud required\n");
        return 1;
    }

    if (ctx->led_opts.addr == -1) {
        printf("led: --addr required\n");
        return 1;
    }

    if(ctx->led_opts.led_number == -1) {
        printf("led: --led <n> required\n");
        return 1;
    }

    if (!serial_open(ctx->led_opts.port, ctx->led_opts.baud, ctx->glob_options.timeout)) {
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
    if(ctx->led_opts.scbk_d || ctx->led_opts.scbk_set) {
        // if scbk is provided set and use it for the secure channel establishment process 
        if(ctx->led_opts.scbk_set)
            setNewScbk(ctx->led_opts.scbk);
        setSecureChannelMode(0);
        
        if(!openSecureChannelSession2(0, ctx->led_opts.addr, ctx->glob_options.retries))
            return 1;
    }

    tmp_LED_Settings tmpParam = {0};
    perm_LED_Settings permParam= {0};

    populateLedParameters(&ctx->led_opts, &tmpParam, &permParam);       

    do{
        Command_OSDP_LED(0, (uint8_t)ctx->led_opts.addr, 0 , (uint8_t)ctx->led_opts.led_number, tmpParam, permParam);

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
    }while(replyCode == REPLY_NAK && ctx->glob_options.retries); // Retry if NAK is received and retry option is not zero

    if(ctx->glob_options.log_file_name != NULL)
        logger_file_close();

    serial_close();

    return 0;   
}


// todo -> a revoir!!!!!, pour les test unitaires, on peut faire un led_test qui affiche les options et vérifie que les options sont bien parsées, 
/**
 * @brief Display the parsed options for the led subcommand (for testing purposes)
 * 
 * The function is responsible for displaying the parsed options for the led subcommand in the osdp-cli application.
 * The function takes the CLI context and the parsed options for the command. 
 * The function checks for the required options such as the serial port, baud rate and device address...
 * 
 * @param[in] ctx The CLI context
 * 
 * @return An integer status code indicating the success or failure of the check options of comand.
 */
int led_test(struct cli_context *ctx)
{

    if (!ctx->led_opts.port) {
        printf("led: --port required\n");
        return 1;
    }

    if(ctx->led_opts.led_number == -1) {
        printf("led: --led <n> required\n");
        return 1;
    }

    if(ctx->led_opts.led_color == LED_COLOR_NONE) {
        printf("led: --color <color> required\n");
        return 1;
    }

    if(ctx->led_opts.led_mode == LED_MODE_NONE) {
        printf("led: --mode <mode> required\n");
        return 1;
    }

    if(ctx->led_opts.onTime == -1) {
        printf("led: --on <time ms> required\n");
        return 1;
    }

    if(ctx->led_opts.offTime == -1) {
        printf("led: --off <time ms> required\n");
        return 1;
    }

    if(ctx->led_opts.duration == -1) {
        printf("led: --duration <time ms> required\n");
        return 1;
    }

    printf("LED command options:\n");

    printf("led start\n");
    printf("port: %s\n", ctx->led_opts.port);
    printf("baud: %s\n", ctx->led_opts.baud);
    printf("addr: %d\n", ctx->led_opts.addr);

    if(ctx->led_opts.scbk_set) 
    {
        printf("SCBK: [redacted]\n");
    }
    else 
    {
        printf("SCBK not set \n");
    }

    (ctx->led_opts.scbk_d ) ? printf("SCBK-D flag is set \n") : printf("SCBK-D flag is not set \n");

    printf("led number: %d\n", ctx->led_opts.led_number);
    printf("led color: %d\n", ctx->led_opts.led_color);
    printf("led mode: %d\n", ctx->led_opts.led_mode);
    printf("led on time: %d\n", ctx->led_opts.onTime);
    printf("led off time: %d\n", ctx->led_opts.offTime);
    printf("led duration: %d\n", ctx->led_opts.duration);

    return 0;    
}


/**
 * @brief Populate the LED parameters based on the parsed options
 * 
 * The function is responsible for populating the LED parameters based on the parsed options for the led subcommand in the osdp-cli application.
 * The function takes the parsed options for the led subcommand, and it fills in the temporary and permanent LED settings structures based on the mode and color options provided by the user.
 * The function handles different LED modes such as on, off, blink, and temp, and it sets the appropriate control codes, on/off times, colors, and duration for the LED settings.
 * @param[in] opts The parsed options for the led subcommand
 * @param[out] tmpParam The temporary LED settings structure to be populated based on the options   
 * @param[out] permParam The permanent LED settings structure to be populated based on the options
 * @return void
 */
static void populateLedParameters(struct led_options *opts, tmp_LED_Settings *tmpParam, perm_LED_Settings *permParam)
{
    switch (opts->led_mode)
    {
        case LED_MODE_OFF:
            permParam->permControlCode = 1;
            permParam->permOnTime = 1;
            permParam->permOffTime = 0;
            permParam->permOnColor = LED_OFF;
            permParam->permOffColor = LED_OFF;
            break;

        case LED_MODE_ON:
            permParam->permControlCode = 1;
            permParam->permOnTime = 1;
            permParam->permOffTime = 0;
            permParam->permOnColor = opts->led_color;
            permParam->permOffColor = LED_OFF;  
            break;

        case LED_MODE_BLINK:
            if(opts->duration == -1) 
            {// duration is not set
                permParam->permControlCode = 1;
                permParam->permOnTime = (uint8_t)(opts->onTime / 100);
                permParam->permOffTime = (uint8_t)(opts->offTime / 100);
                permParam->permOnColor = opts->led_color;
                permParam->permOffColor = LED_OFF;  
            }
            else
            {
                tmpParam->tmpControlCode = 2;
                tmpParam->tmpOnTime = (uint8_t)(opts->onTime / 100);
                tmpParam->tmpOffTime = (uint8_t)(opts->offTime / 100);
                tmpParam->tmpOnColor = opts->led_color;
                tmpParam->tmpOffColor = LED_OFF; // Assuming the off color is the same as the on color, adjust if needed
                int durationSec = (opts->duration /100); // Duration in seconds, 0 for indefinite
                tmpParam->tmpTimerLsb = (uint8_t)(durationSec & 0xFF); // Duration LSB in seconds
                tmpParam->tmpTimerMsb = (uint8_t)((durationSec >> 8) & 0xFF); // Duration MSB in seconds
            }
            break;

        case LED_MODE_TEMP:
                tmpParam->tmpControlCode = 2;
                tmpParam->tmpOnTime = 1;
                tmpParam->tmpOffTime = 0;
                tmpParam->tmpOnColor = opts->led_color;
                tmpParam->tmpOffColor = LED_OFF; // Assuming the off color is the same as the on color, adjust if needed
                int durationSec = (opts->duration /100); // Duration in seconds, 0 for indefinite
                tmpParam->tmpTimerLsb = (uint8_t)(durationSec & 0xFF); // Duration LSB in seconds
                tmpParam->tmpTimerMsb = (uint8_t)((durationSec >> 8) & 0xFF); // Duration MSB in seconds                
            break;    

        default:
            break;      
    }
}   
