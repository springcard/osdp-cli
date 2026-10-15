#include "../../../cli/command_result.h"
/**
 * @file put_run.c
 * @brief Implementation for executing the put subcommand
 * 
 * The code provided implements the execution of the "put" subcommand in the osdp-cli application. 
 * The code provided also displays the parsed options for the put subcommand for testing purposes.
 * The functions takes the CLI context and the parsed options for the put subcommand, and it performs the desired actions based on those options.
 */
#include <stdio.h>
#include <string.h>
#include <stdint.h>
#ifndef _WIN32
#include <unistd.h>
#endif

#include "put_options.h"
#include "../../../cli/global_options_parser.h"
#include "../../../util/utils.h"
#include "../../../platform/serial.h"
#include "../../../logger/logger.h"
#include "../../../moduleOSDP/osdp_cp.h"
#include "../../../moduleOSDP/osdp_packet.h"
#include "../../../moduleOSDP/osdp_cmd.h"
#include "../../../moduleOSDP/osdp_sc.h"

#include <stdlib.h>


#define OK_TO_PROCEED 0
#define FILE_CONTENTS_PROCESSED 1
#define REBOOT_NOW 2
#define IDLE_MSG 3
#define ABORT_FILE_TRANSFER -1
#define UNRECOGNIZED_FILE_CONTENTS -2
#define FILE_DATA_UNACCEPTABLE -3

//todo, suivant le type choisi, il y a une grammaire/syntaxe à respecter pour les options...
//todo  Il faudrait ajouter des vérifications pour s'assurer que les options fournies DANS LA LIGNE DE COMMANDE sont cohérentes.
//.\osdp-cli.exe file put --port <name> --baud <rate> --addr <n> [security options] --type <config|firmware|blob> --in <path> [options]

/**
 * @brief Execute the put subcommand
 * 
 * This function executes the put subcommand based on the CLI context (globale and specific command options).
 * It checks for the presence of required options such as the serial port and more...
 * The function establishes a secure channel session if the appropriate options are provided, sends the osdp_TRANSFERT command to manage Reader TRANSFERT operations, 
 * and handles the response from the device.
 * The secure channel session is established using either the provided SCBK or the default SCBK, and the function handles the communication with the OSDP device accordingly.
 * 
 * @param[in] ctx The CLI context
 * 
 * @return An integer status code indicating the success or failure of the command execution
 */
int put_run(struct cli_context *ctx)
{
    #define DEFAULT_MSG_SIZE 128

    uint8_t replyPacket[MAX_SIZE_MSG_PACKET];
    uint8_t replyCode;
    int size = 0;
    bool flagContinue = true;
    int status = 1;
    FILE *f = NULL;
    uint16_t ftFragmentRequested = DEFAULT_MSG_SIZE;// Payload message size
    uint8_t is_Retry = false; // Flag to indicate if the current FILETRANSFER command is a retry due to a NAK response

    // FILETRANSFER command parameters
    const uint8_t ftType = 0x01;  // opaque file transfer type
    uint32_t ftSize = 0;          // file size in bytes, to be set based on the input file
    uint32_t ftOffset = 0;        // file offset in bytes, to be updated as the file is transferred in chunks
    uint16_t ftFragmentSize = 64; // size of the data fragment in bytes, to be set based on the chunk size option or the default value
    uint8_t ftData[MAX_SIZE_MSG_PACKET] = {0};      // file data fragment, to be filled with the actual data to be transferred in each chunk

    // FTSTAT reply parameters
    uint16_t ftDelay = 0;
    int16_t ftDetailStatus = 0;
    uint16_t ftUpdateMsgMax = 0;


    if (!ctx->put_opts.port) {
        cli_diagnostic(stdout, "put: --port required\n");
        return 1;
    }
    
    if(ctx->put_opts.type == PUT_TYPE_NONE) {
        cli_diagnostic(stdout, "put: --type <config|firmware|blob> required\n");
        return 1;
    }

    if(!ctx->put_opts.input_file) {
        cli_diagnostic(stderr, "put: --in <path> required for file to transfer\n");
        return 1;
    }

    logger_console_init(ctx);
    if (!serial_open(ctx->put_opts.port, ctx->put_opts.baud, ctx->glob_options.timeout)) {
        cli_diagnostic(stdout, "Error opening communication port (port/speed)\n");
        return -1;
    }

    osdpPacketInit(0 /*indReader*/, CONTROL_PANEL);


    if(ctx->glob_options.log_file_name != NULL)
    {
        if (logger_file_init(ctx) != 0) {
            cli_diagnostic(stderr, "Failed to open log file\n");
            goto cleanup;
        }
    }    

    // if secure channel mode is requested, establishing a secure channel is required before sending any command
    if(ctx->put_opts.scbk_d || ctx->put_opts.scbk_set) {
        // if scbk is provided set and use it for the secure channel establishment process 
        if(ctx->put_opts.scbk_set)
            setNewScbk(ctx->put_opts.scbk);
        setSecureChannelMode(0);
        
        if(!openSecureChannelSession2(0, ctx->put_opts.addr, ctx->glob_options.retries))
            goto cleanup;
    }

    f = fopen(ctx->put_opts.input_file, "rb");
    if (!f) {
        cli_diagnostic(stderr, "Failed to open file: %s\n", ctx->put_opts.input_file);
        goto cleanup;
    }
    else {
        // Get the file size
        if (fseek(f, 0, SEEK_END) != 0) {
            cli_set_error("Failed to seek input file");
            goto cleanup;
        }
        long length = ftell(f);
        if (length < 0 || (unsigned long long)length > UINT32_MAX || fseek(f, 0, SEEK_SET) != 0) {
            cli_set_error("Invalid input file size or seek failure");
            goto cleanup;
        }
        ftSize = (uint32_t)length;
    }    

    do
    {
        // reads the file if the PD is OK and if it's not a retry
        if(ftDetailStatus == OK_TO_PROCEED && !is_Retry)
        {
            if (ftFragmentRequested > sizeof(ftData) - 16) {
                cli_set_error("Device requested an oversized file fragment");
                goto cleanup;
            }
            ftFragmentSize = (uint16_t)fread(ftData, 1, ftFragmentRequested, f);
            if (ferror(f) || (ftFragmentSize == 0 && ftOffset < ftSize)) {
                cli_set_error("Failed to read input file");
                goto cleanup;
            }
        }
        
        Command_OSDP_FILETRANSFER(0, ctx->put_opts.addr, ftType, ftSize, ftOffset, ftFragmentSize, ftData);

        size = getReply(0, &replyCode, replyPacket, sizeof(replyPacket));

        if(size == sizeof(osdp_FTSTAT) && replyCode == REPLY_FTSTAT)
        {
            osdp_FTSTAT * replyMsg = (osdp_FTSTAT *)  replyPacket;
            ftDelay = (replyMsg->ftDelay[1] << 8) | replyMsg->ftDelay[0];
            ftDetailStatus = (replyMsg->ftStatusDetail[1] << 8) | replyMsg->ftStatusDetail[0];
            ftUpdateMsgMax = (replyMsg->ftUpdateMsgMax[1] << 8) | replyMsg->ftUpdateMsgMax[0];

            is_Retry = false; // Clear the retry flag upon receiving a valid FTSTAT response, as the next FILETRANSFER command will not be a retry anymore

            switch (ftDetailStatus)
            {
                case OK_TO_PROCEED:// 0
                    /* Read the following "ftUpdateMsgMax" bytes from the file. */
                    if(ftUpdateMsgMax > 0)
                    {
                        ftFragmentRequested = ftUpdateMsgMax; // PD change FtFragmentSize parameter for next FILETRANSFER msg
                    }
                    ftOffset += ftFragmentSize;
                    if (ftOffset > ftSize) {
                        cli_diagnostic(stderr, "Offset overflow\n");
                        flagContinue = false;
                    }

                    break;

                case IDLE_MSG:
                    /* code */
                    ftOffset = ftSize;
                    ftFragmentSize = 0;
                    break;

                case FILE_CONTENTS_PROCESSED:
                case REBOOT_NOW:
                    status = 0;
                    flagContinue = false;
                    break;
                case FILE_DATA_UNACCEPTABLE:
                case ABORT_FILE_TRANSFER:
                case UNRECOGNIZED_FILE_CONTENTS:
                    cli_set_error("Device rejected file transfer (status %d)", ftDetailStatus);
                    /* code */
                    flagContinue = false;
                    break;                    
                
                default:
                    cli_set_error("Unexpected file transfer status (%d)", ftDetailStatus);
                    flagContinue = false;
                    break;
            }
        }
        else if(replyCode == REPLY_NAK)
        {
            osdp_NAK * replyMsg = (osdp_NAK *)  replyPacket;
            is_Retry = true; // Set the retry flag to indicate that the next FILETRANSFER command will be a retry due to a NAK response
            if(--ctx->glob_options.retries <= 0) {
                cli_reply_result(size, replyCode, 0);
                flagContinue = false;
            }
            else if(ctx->put_opts.scbk_d || ctx->put_opts.scbk_set){// if secure channel mode is requested, establishing a secure channel is required before sending any command
                setSecureChannelMode(0);
                if(!openSecureChannelSession2(0, ctx->put_opts.addr, ctx->glob_options.retries))
                    goto cleanup;
            }
                
        }
        else 
        {
            printf("<-- receive unexpected reply code: %02x\n", replyCode);
            is_Retry = true; // Set the retry flag to indicate that the next FILETRANSFER command will be a retry due to an unexpected reply code
            if(--ctx->glob_options.retries <= 0) {
                cli_reply_result(size, replyCode, 0);
                flagContinue = false;
            }
        }

        // Requested time delay in milliseconds before next FILETRANSFER message.
        delay_ms(ftDelay);
        
        if(ctx->put_opts.pause > 0)
            delay_ms(ctx->put_opts.pause);

    } while(flagContinue); 

cleanup:
    if (f) fclose(f);

    if(ctx->glob_options.log_file_name != NULL)
        logger_file_close();    

    serial_close();

    return status;
}


// todo -> a revoir!!!!!, pour les test unitaires, on peut faire un put_test qui affiche les options et vérifie que les options sont bien parsées, 
/**
 * @brief Display the parsed options for the put subcommand (for testing purposes)
 * 
 * The function is responsible for displaying the parsed options for the put subcommand in the osdp-cli application.
 * The function takes the CLI context and the parsed options for the command. 
 * The function checks for the required options such as the serial port, baud rate and device address...
 * 
 * @param[in] ctx The CLI context
 * @param[in] opts The parsed options for the put subcommand
 * 
 * @return An integer status code indicating the success or failure of the check options of comand.
 */
int put_test(struct cli_context *ctx)
{

    if (!ctx->put_opts.port) {
        printf("put: --port required\n");
        return 1;
    }

    if (!ctx->put_opts.baud) {
        printf("put: --baud required\n");
        return 1;
    }

    if (ctx->put_opts.addr == -1) {
        printf("put: --addr required\n");
        return 1;
    }

    if(ctx->put_opts.type == PUT_TYPE_NONE) {
        printf("put: --type <config|firmware|blob> required\n");
        return 1;
    }

    if(!ctx->put_opts.input_file) {
        printf("put: --in <path> required\n");
        return 1;
    }

    printf("PUT command options:\n");

    printf("put start\n");
    printf("port: %s\n", ctx->put_opts.port);
    printf("baud: %s\n", ctx->put_opts.baud);
    printf("addr: %d\n", ctx->put_opts.addr);

    if(ctx->put_opts.scbk_set) 
    {
        printf("SCBK: [redacted]\n");
    }
    else 
    {
        printf("SCBK not set \n");
    }

    (ctx->put_opts.scbk_d ) ? printf("SCBK-D flag is set \n") : printf("SCBK-D flag is not set \n");

    char *type_str[] = {"config", "firmware", "blob"};
    printf("type: %s\n", type_str[ctx->put_opts.type]);

    printf("in: %s\n", ctx->put_opts.input_file);
    if(ctx->put_opts.chunk != -1){
        printf("chunk: %d\n", ctx->put_opts.chunk); 
    }
    else {
        printf("chunk: not set\n");
    }

    (ctx->put_opts.resume) ? printf("resume flag is set \n") : printf("resume flag is not set \n");
    (ctx->put_opts.reboot) ? printf("reboot flag is set \n") : printf("reboot flag is not set \n");
    (ctx->put_opts.progress) ? printf("progress flag is set \n") : printf("progress flag is not set \n");

    return 0;    
}
