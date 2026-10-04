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
    uint8_t replyPacket[MAX_SIZE_MSG_PACKET];
    uint8_t replyCode;
    int size = 0;

    if (!ctx->config_opts.port) {
        printf("config: --port required\n");
        return 1;
    }

    if (!serial_open(ctx->config_opts.port, ctx->config_opts.baud, ctx->glob_options.timeout)) {
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
    if(ctx->config_opts.scbk_d || ctx->config_opts.scbk_set) {
        // if scbk is provided set and use it for the secure channel establishment process 
        if(ctx->config_opts.scbk_set)
            setNewScbk(ctx->config_opts.scbk);
        setSecureChannelMode(0);
        
        if(!openSecureChannelSession2(0, ctx->config_opts.addr, ctx->glob_options.retries))
            return 1;
    }

    if(!ctx->config_opts.new_addr_set) {
        ctx->config_opts.new_addr = ctx->config_opts.addr;
    }

    if(!ctx->config_opts.new_baud_set) {
        ctx->config_opts.new_baud = (unsigned int) strtoul(ctx->config_opts.baud, NULL, 10);
    }

    do{
        // send the COMSET command to configure the device
        Command_OSDP_COMSET(0, ctx->config_opts.addr,  ctx->config_opts.new_addr, ctx->config_opts.new_baud);  
        size = getReply(0, &replyCode, replyPacket, sizeof(replyPacket));

        if(replyCode == REPLY_COM) 
        {
            osdp_COM * replyMsg = (osdp_COM *)  replyPacket;

            if(ctx->config_opts.new_addr == replyMsg->addr && ctx->config_opts.new_baud == replyMsg->baudrate) 
            {
                if(ctx->config_opts.reconnect) 
                {
                    serial_close();
                    delay_ms(1000); // wait for the device to apply the new configuration and be ready for a new connection

                    char strBaudrate[7]; // max 6 digits for baud rates up to 921600 plus null terminator
                    snprintf(strBaudrate, sizeof(strBaudrate), "%u", replyMsg->baudrate);

                    if (!serial_open(ctx->config_opts.port, strBaudrate, ctx->glob_options.timeout)) 
                    {
                        return -1;
                    }

                    // if secure channel mode is requested, establishing a secure channel is required before sending any command
                    if(ctx->config_opts.scbk_d || ctx->config_opts.scbk_set) 
                    {

                        setSecureChannelMode(0);

                        if(!openSecureChannelSession2(0, ctx->config_opts.addr, ctx->glob_options.retries))
                            return 1;
                    }
                    
                    do{
                        // send the ID command to retrieve the vendor code, model number, serial numberi and firmware version of the device
                        Command_OSDP_ID(0, ctx->config_opts.addr);

                        size = getReply(0, &replyCode, replyPacket, sizeof(replyPacket));

                        if(replyCode == REPLY_PDID) 
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
                    }while(replyCode == REPLY_NAK && ctx->glob_options.retries); // retry sending the command if a NAK is received, and if retries are still available              
                }
            }
            else 
            {
                printf("Configuration not applied \n");
            }

        }
        else if(replyCode == REPLY_NAK)
        {
            ctx->glob_options.retries--; // decrement the retry count in the CLI context
        }
        else
        {
            ctx->glob_options.retries--; // decrement the retry count in the CLI context
        }
    }while(replyCode == REPLY_NAK && ctx->glob_options.retries); // retry sending the command if a NAK is received, and if retries are still available

    if(ctx->glob_options.log_file_name != NULL)
        logger_file_close();

    serial_close();

    return 0;   
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
