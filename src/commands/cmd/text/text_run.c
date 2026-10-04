/**
 * @file text_run.c
 * @brief Implementation for executing the text subcommand
 * 
 * The code provided implements the execution of the "text" subcommand in the osdp-cli application. 
 * The code provided also displays the parsed options for the text subcommand for testing purposes.
 * The functions takes the CLI context and the parsed options for the text subcommand, and it performs the desired actions based on those options.
 */
#include <stdio.h>
#include <string.h>
#include <stdint.h>
#ifndef _WIN32
#include <unistd.h>
#endif

#include "text_options.h"
#include "../../../cli/global_options_parser.h"
#include "../../../util/utils.h"
#include "../../../platform/serial.h"
#include "../../../logger/logger.h"
#include "../../../moduleOSDP/osdp_cp.h"
#include "../../../moduleOSDP/osdp_packet.h"
#include "../../../moduleOSDP/osdp_cmd.h"
#include "../../../moduleOSDP/osdp_sc.h"

#include <stdlib.h>

static void populateTextParameters(struct text_options *opts, osdp_TEXT **textParam);

//todo, suivant le mode choisi, il y a une grammaire à respecter pour les options, par exemple, si le mode est "on" ou "off", 
//todo  les options onTime, offTime et duration ne sont pas nécessaires, 
//todo  si le mode est "blink", les options onTime et offTime sont nécessaires, mais la duration est optionnelle, si le mode est "temp", 
//todo  la duration est nécessaire et les options onTime et offTime ne sont pas nécessaires. 
//todo  Il faudrait ajouter des vérifications pour s'assurer que les options fournies sont cohérentes avec le mode choisi.
//  .\build\osdp-cli.exe cmd text --port COM11 --baud 9600 --addr 0 --text 0 --mode on --color blue
//  .\build\osdp-cli.exe cmd text --port COM11 --baud 9600 --addr 0 --text 0 --mode off
//  .\build\osdp-cli.exe cmd text --port COM11 --baud 9600 --addr 0 --text 0 --mode blink --color green --on 500  --off 500
//  .\build\osdp-cli.exe cmd text --port COM11 --baud 9600 --addr 0 --text 0 --mode blink --color green --on 500  --off 500 --duration 5000
//  .\build\osdp-cli.exe cmd text --port COM11 --baud 9600 --addr 0 --text 0 --mode temp --color amber  --duration 50


/**
 * @brief Execute the text subcommand
 * 
 * This function executes the text subcommand based on the CLI context (globale and specific command options).
 * It checks for the presence of required options such as the serial port, baud rate, and device addressand more...
 * !The function establishes a secure channel session if the appropriate options are provided, sends the osdp_TEXT command to manage Reader TEXT indicators, 
 * and handles the response from the device.
 * The secure channel session is established using either the provided SCBK or the default SCBK, and the function handles the communication with the OSDP device accordingly.
 * 
 * @param[in] ctx The CLI context
 * 
 * @return An integer status code indicating the success or failure of the command execution
 */
int text_run(struct cli_context *ctx)
{
    uint8_t replyPacket[MAX_SIZE_MSG_PACKET];
    osdp_TEXT *textParam = NULL;
    uint8_t replyCode;
    int size = 0;

    if (!ctx->text_opts.port) {
        printf("text: --port required\n");
        return 1;
    }

    if (!serial_open(ctx->text_opts.port, ctx->text_opts.baud, ctx->glob_options.timeout)) {
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
    if(ctx->text_opts.scbk_d || ctx->text_opts.scbk_set) {
        // if scbk is provided set and use it for the secure channel establishment process 
        if(ctx->text_opts.scbk_set)
            setNewScbk(ctx->text_opts.scbk);
        setSecureChannelMode(0);
        
        if(!openSecureChannelSession2(0, ctx->text_opts.addr, ctx->glob_options.retries))
            return 1;
    }

    populateTextParameters(&ctx->text_opts, &textParam);

    do{
        Command_OSDP_TEXT(0, (uint8_t)ctx->text_opts.addr, textParam);
        size = getReply(0, &replyCode, replyPacket, sizeof(replyPacket));

        if(replyCode == REPLY_ACK) 
        {
            break; // command executed successfully, exit the retry loop
        }
        else if(replyCode == REPLY_NAK)
        {
            ctx->glob_options.retries--; // decrement the retry count in the CLI context
        }
        else 
        {
            ctx->glob_options.retries--; // decrement the retry count in the CLI context
        }

    }while(replyCode == REPLY_NAK && ctx->glob_options.retries); // retry sending the command if a NAK is received, and if the CLI context is still active (not exiting)

    if(ctx->glob_options.log_file_name != NULL)
        logger_file_close();

    serial_close();

    return 0;   
}


// todo -> a revoir!!!!!, pour les test unitaires, on peut faire un text_test qui affiche les options et vérifie que les options sont bien parsées, 
/**
 * @brief Display the parsed options for the text subcommand (for testing purposes)
 * 
 * The function is responsible for displaying the parsed options for the text subcommand in the osdp-cli application.
 * The function takes the CLI context and the parsed options for the command. 
 * The function checks for the required options such as the serial port, baud rate and device address...
 * 
 * @param[in] ctx The CLI context
 * 
 * @return An integer status code indicating the success or failure of the check options of comand.
 */
int text_test(struct cli_context *ctx)
{

    if (!ctx->text_opts.port) {
        printf("text: --port required\n");
        return 1;
    }

    if (!ctx->text_opts.baud) {
        printf("text: --baud required\n");
        return 1;
    }

    if (ctx->text_opts.addr == -1) {
        printf("text: --addr required\n");
        return 1;
    }

    if(!ctx->text_opts.msg) {
        printf("text: --msg <string> required\n");
        return 1;
    }

    printf("Text command options:\n");

    printf("text start\n");
    printf("port: %s\n", ctx->text_opts.port);
    printf("baud: %s\n", ctx->text_opts.baud);
    printf("addr: %d\n", ctx->text_opts.addr);

    if(ctx->text_opts.scbk_set) 
    {
        printf("SCBK: [redacted]\n");
    }
    else 
    {
        printf("SCBK not set \n");
    }

    (ctx->text_opts.scbk_d ) ? printf("SCBK-D flag is set \n") : printf("SCBK-D flag is not set \n");

    printf("text row: %d\n", ctx->text_opts.row);
    printf("text column: %d\n", ctx->text_opts.column);
    printf("text message: %s\n", ctx->text_opts.msg);

    (ctx->text_opts.clear ) ? printf("clear flag is set \n") : printf("clear flag is not set \n");

    return 0;    
}


/**
 * @brief Populate the osdp_TEXT structure based on the parsed options for the text subcommand
 * The function is responsible for populating the osdp_TEXT structure based on the parsed options for the text subcommand in the osdp-cli application.
 * The function takes the parsed options for the text subcommand and a pointer to an osdp_TEXT structure, which will be populated with the appropriate values based on the options provided by the user.
 * The function sets the reader number, text command, temporary text time, row, column, text length, and text data fields of the osdp_TEXT structure based on the options provided by the user.
 * The function also handles default values for the row and column if they are not set by the user, and it copies the message to the textData field of the osdp_TEXT structure if a message is provided.
 * @param[in] opts The parsed options for the text subcommand
 * @param[out] textParam A pointer to an osdp_TEXT structure that will be populated based on the parsed options
 */
static void populateTextParameters(struct text_options *opts, osdp_TEXT **textParam)
{
    uint8_t msgLength = (uint8_t)strlen(opts->msg); // set text length based on the provided message
    *textParam = (osdp_TEXT *)malloc(sizeof(osdp_TEXT) + msgLength); // allocate memory for the osdp_TEXT structure including the text data
    memset(*textParam, 0, sizeof(osdp_TEXT) + msgLength);
    (*textParam)->readerNber = 0;
    (*textParam)->textCommand = 0x01; // permanent text, no wrap, no clear
    (*textParam)->tempTextTime = 0; // not used for permanent text
    (*textParam)->row = (opts->row != -1) ? (uint8_t)opts->row : 0x01; // default to row 1 if not set
    (*textParam)->column = (opts->column != -1) ? (uint8_t)opts->column : 0x01; // default to column 1 if not set

    (*textParam)->textLength = msgLength; // set the text length in the osdp_TEXT structure
    if((*textParam)->textLength > 0) {
        memcpy((*textParam)->textData, opts->msg, msgLength); // copy the message to the textData field
    }
}    
