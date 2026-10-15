#include "../../cli/command_result.h"
/**
 * @file cp_run.c
 * @brief Implementation for executing the cp command
 * 
 * The code provided implements the execution of the "cp" command in the osdp-cli application. 
 * The code provided also displays the parsed options for the cp command for testing purposes.
 * The functions takes the CLI context and the parsed options for the cp command, and it performs the desired actions based on those options.
 */
#include <stdio.h>
#include <signal.h>
#include <string.h>

#include "cp_options.h"
#include "../../cli/global_options_parser.h"
#include "../../util/utils.h"
#include "../../platform/serial.h"
#include "../../logger/logger.h"
#include "../../moduleOSDP/osdp_common.h"
#include "../../moduleOSDP/osdp_cp.h"
#include "../../moduleOSDP/osdp_packet.h"
#include "../../moduleOSDP/osdp_cmd.h"
#include "../../moduleOSDP/osdp_sc.h"


// Polling loop stop management variable
static volatile sig_atomic_t stop_requested = 0;


/**
 * @brief The function sets the `stop_requested` flag to 1 when a SIGINT signal is received.
 * 
 * @param sig  is an integer representing the signal number that triggered the signal handler.
 * In this case, it is used to handle the SIGINT signal, which is typically sent to a process 
 * when the user presses Ctrl+C in the terminal.
 */
static void handle_sigint(int sig)
{
    (void)sig;          // Avoid warning
    stop_requested = 1; // Request a stop-
}


/**
 * @brief Send a osdp_PDID command
 * 
 * The `cp_run_single` function sends commands to retrieve information from one or more devices on the bus for enumerate PD and exit
 * 
 * @param ctx - `ctx` parameter is a pointer to a `cli context` structure, which contains the  
 * global and specific options provided by the user through the command line.
 * 
 * @return An integer status code indicating the success or failure of the command execution.
 */
int cp_run_single(struct cli_context *ctx)
{
    uint8_t replyPacket[MAX_SIZE_MSG_PACKET];
    bool flagContinue = true;
    uint8_t addr = ctx->cp_opts.addr;
    unsigned char replyCode;
    int size;
    int retries = ctx->glob_options.retries > 0 ? ctx->glob_options.retries : 1;
    int found = 0;
    
    if (ctx->cp_opts.addr_all)
        addr = 0; // First address to looking for PD, the last is 126

    do{
        // send the ID command to retrieve the vendor code, model number, serial number and firmware version of the device
        Command_OSDP_ID(0, addr);

        memset(replyPacket, 0, MAX_SIZE_MSG_PACKET);
        replyCode = 0;
        size = getReply(0, &replyCode, replyPacket, sizeof(replyPacket));

        if(size == sizeof(osdp_PDID) && replyCode == REPLY_PDID)
        {   
            ++found;
            if(ctx->cp_opts.addr_all)
            {
                retries = ctx->glob_options.retries > 0 ? ctx->glob_options.retries : 1;
                addr++;  // next address to process
                if (addr == 0x7F)
                    flagContinue = false;
            }         
            else
                flagContinue = false;
        }
        else if (replyCode == REPLY_NAK)
        {
            if(--retries == 0)
            {
                if(ctx->cp_opts.addr_all)
                {
                    retries = ctx->glob_options.retries > 0 ? ctx->glob_options.retries : 1;
                    addr++;  // next address to process
                    if (addr == 0x7F)
                        flagContinue = false;
                }         
                else
                    flagContinue = false;                
            }
        }
        else
        {
            if(--retries == 0)
            {
                if(ctx->cp_opts.addr_all)
                {
                    retries = ctx->glob_options.retries > 0 ? ctx->glob_options.retries : 1;
                    addr++;  // next address to process
                    if (addr == 0x7F)
                        flagContinue = false;
                }         
                else                
                    flagContinue = false;
            }
        }

        if(ctx->cp_opts.delay > 0)
            delay_ms(ctx->cp_opts.delay);
        
    }while(flagContinue && !stop_requested);

    if (stop_requested) { cli_error_code("cancelled"); return 1; }
    if (ctx->cp_opts.addr_all && found > 0) return 0;
    return cli_reply_result(size, replyCode, size == sizeof(osdp_PDID) && replyCode == REPLY_PDID);
}


/**
 * @brief polling one or more PD on the bus
 * 
 * The function `cp_run_poll` polls devices for information using the OSDP protocol and handles
 * different reply codes accordingly.
 * 
 * @param ctx - `ctx` parameter is a pointer to a `cli context` structure, which contains the  
 * global and specific options provided by the user through the command line.
 * 
 * @return An integer status code indicating the success or failure of the command execution.
 */
int cp_run_poll(struct cli_context *ctx)
{
    uint8_t reply[MAX_SIZE_MSG_PACKET], code = 0;
    uint8_t addr = ctx->cp_opts.addr_all ? 0 : ctx->cp_opts.addr;
    int attempts = ctx->glob_options.retries > 0 ? ctx->glob_options.retries : 1;
    int remaining = attempts;
    int received = 0;
    while (!stop_requested) {
        Command_OSDP_POLL(0, addr);
        int size = getReply(0, &code, reply, sizeof(reply));
        int valid = size >= 0 && ((code == REPLY_ACK && size == 0) ||
            code == REPLY_KEYPAD || code == REPLY_RAW || code == REPLY_ISTATR ||
            code == REPLY_LSTATR || code == REPLY_OSTATR || code == REPLY_RSTATR ||
            code == REPLY_BIOREADR || code == REPLY_BIOMATCHR);
        if (valid) {
            ++received;
            remaining = attempts;
            if (ctx->cp_opts.addr_all) addr = addr == 126 ? 0 : addr + 1;
        } else if (--remaining <= 0) {
            if (!ctx->cp_opts.addr_all) return cli_reply_result(size, code, 0);
            remaining = attempts;
            addr = addr == 126 ? 0 : addr + 1;
        }
        if (ctx->cp_opts.delay > 0) delay_ms(ctx->cp_opts.delay);
    }
    if (received > 0) return 0;
    cli_error_code("cancelled");
    return 1;
}


/**
 * @brief Execute the cp command
 * 
 * This function executes the cp command based on the CLI context (globale and specific command options).
 * It checks for the presence of required options such as the serial port, baud rate, and device address...
 * Switches on execution of polling of one or more devices or the execution of enumeration of devices on the bus.
 * 
 * @param[in] ctx The CLI context
 * 
 * @return An integer status code indicating the success or failure of the command execution
 */
int cp_run(struct cli_context *ctx)
{
    int status = 1;
    
    signal(SIGINT, handle_sigint);

    if (!serial_open(ctx->cp_opts.port, ctx->cp_opts.baud, ctx->glob_options.timeout)) {
        cli_diagnostic(stdout, "Error, arguments missing(port/baudrate)\n");
        return -1;
    }

    osdpPacketInit(0 /*indReader*/, CONTROL_PANEL);

    logger_console_init(ctx);
    
    if(ctx->glob_options.log_file_name != NULL)
    {
        if (logger_file_init(ctx) != 0) {
            cli_diagnostic(stderr, "Failed to open log file\n");
            goto cleanup;
        }
    }

    // if secure channel mode is requested, establishing a secure channel is required before sending any command
    if(ctx->cp_opts.scbk_d || ctx->cp_opts.scbk_set) {
        // if scbk is provided use it for the secure channel establishment process 
        if(ctx->cp_opts.scbk_set)
            setNewScbk(ctx->cp_opts.scbk);
        setSecureChannelMode(0);
        
        if(!openSecureChannelSession2(0, ctx->cp_opts.addr, ctx->glob_options.retries))
            goto cleanup;
    }

    status = ctx->cp_opts.single ? cp_run_single(ctx) : cp_run_poll(ctx);

cleanup:
    if(ctx->glob_options.log_file_name != NULL)
        logger_file_close();

    serial_close();

    return status;
}


/**
 * @brief Display the parsed options for the cp command (for testing purposes)
 * 
 * The function is responsible for displaying the parsed options for the cp command in the osdp-cli application.
 * The function takes the CLI context and the parsed options for the command. 
 * The function checks for the required options such as the serial port, baud rate and device address.
 * 
 * @param[in] ctx The CLI context
 * @param[in] opts The parsed options for the cp command
 * 
 * @return An integer status code indicating the success or failure of the check options of comand.
 */
int cp_test(struct cli_context *ctx)
{

    if (!ctx->cp_opts.port) {
        printf("cp: --port required\n");
        return 1;
    }

    if (!ctx->cp_opts.baud) {
        printf("cp: --baud required\n");
        return 1;
    }

    if (!ctx->cp_opts.addr && !ctx->cp_opts.addr_all) {
        printf("cp: --addr <n> or --addr-all required\n");
        return 1;
    }

    printf("cp start\n");
    printf("port: %s\n", ctx->cp_opts.port);
    printf("baud: %s\n", ctx->cp_opts.baud);
    if(ctx->cp_opts.addr != 0xFF)
        printf("addr: %d\n", ctx->cp_opts.addr);
    else
        printf("addr is not specified\n");

    if(ctx->cp_opts.scbk_set) {
        printf("SCBK: [redacted]\n");
    }
    else 
    {
        printf("SCBK not set \n");
    }

    (ctx->cp_opts.scbk_d ) ? printf("SCBK-D flag is set\n") : printf("SCBK-D flag is not set \n");

    (ctx->cp_opts.single) ? printf("Single flag is set\n") : printf("Single flag is not set\n");

    (ctx->cp_opts.addr_all) ? printf("addr-all flag is set\n") : printf("addr-all flag is not set\n");

    if (ctx->cp_opts.delay != -1) {
        printf("CP loop delay: %d ms\n", ctx->cp_opts.delay);
    }
    else{
        printf("CP loop delay is not specified\n");
    }

    return 0;    
}

