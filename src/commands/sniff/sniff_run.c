#include "../../cli/command_result.h"
/**
 * @file sniff_run.c
 * @brief Implementation for executing the sniff command
 * 
 * The `sniff_run` function is responsible for executing the sniff command based on the parsed options and the CLI context. It checks for the presence of required options such as the serial port and 
 * baud rate, and it prints out the configuration for the sniffing operation. It also handles optional settings such as security keys, display options for raw, decoded, and error data, output file name, 
 * output format, and log rotation settings. The function returns an integer status code indicating the success or failure of the command execution.
 */
#include <stdio.h>
#include <signal.h>
#include <string.h>
#include <stdint.h>
#include <stdlib.h>

#include "sniff_options.h"
#include "sniff_worker.h"
#include "../../cli/global_options_parser.h"
#include "../../platform/serial.h"
#include "../../logger/logger.h"
#include "../../moduleOSDP/osdp_common.h"
#include "../../moduleOSDP/osdp_packet.h"
#include "../../util/tiny_queue.h"


// Polling loop stop management variable
static volatile sig_atomic_t stop_requested = 0;

// queue for incoming frames
tiny_queue_t *inFrameQueue = NULL;


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


//todo a revoir!!!!!!
//todo a revoir!!!!!!
//todo a revoir!!!!!!
/** @brief Execute the sniff command
 * 
 * This function executes the sniff command based on the CLI context (globale and specific command options).
 * It checks for the presence of required options such as the serial port and baud rate.
 * It also handles optional settings such as security keys, display options for raw, decoded, and error data, output file name, output format, and log rotation settings.
 * The function returns an integer status code indicating the success or failure of the command execution.
 * 
 * @param[in] ctx The CLI context
 * 
 * @return An integer status code indicating the success or failure of the command execution
 */
int sniff_run(struct cli_context *ctx)
{
    uint8_t buf[100];
    int status = 0;
    bool worker_started = false;

    signal(SIGINT, handle_sigint);

    if (!ctx->sniff_opts.port) {
        cli_diagnostic(stderr, "sniff: --port required\n");
        return 1;
    }

    if (!serial_open(ctx->sniff_opts.port, ctx->sniff_opts.baud, ctx->glob_options.timeout)) {
        cli_diagnostic(stderr, "Error opening RS485 port\n");
        return -1;
    }

	// Create the queue for incoming frames
	inFrameQueue = tiny_queue_create();
	if (inFrameQueue == NULL) {
		cli_diagnostic(stderr, "Cannot allocate queue\n");
        serial_close();
		return -1;
	}

    if (sniff_worker_start() != 0) {
        cli_set_error("Failed to start capture worker");
        status = 1;
        goto cleanup;
    }
    worker_started = true;

    osdpPacketInit(0, SNIFFER_FRAME);  
 
    sniff_logger_console_init(ctx);
    
    if(ctx->sniff_opts.out_file != NULL)
    {
        if (sniff_logger_file_init(ctx) != 0) {
            cli_set_error("Failed to open log file");
            status = 1;
            goto cleanup;
        }
    }     

    // Main sniffing loop: read frames from the serial port and push them to the queue for processing by the worker thread
    do
    {
        int n = serial_read(buf, sizeof(buf));

        if (n < 0) {
            cli_set_error("Serial read failed");
            status = 1;
            break;
        }
        if (n > 0) {

            sniff_frame_t *chunk = malloc(sizeof(sniff_frame_t));
            if (!chunk) {
                cli_set_error("Cannot allocate capture frame");
                status = 1;
                break;
            }
            memcpy(chunk->data, buf, n);
            chunk->len = n;

	        if (tiny_queue_push(inFrameQueue, chunk) != 0){
                cli_set_error("Cannot push frame to queue");
                free(chunk);
                status = 1;
                break;
            }
        }
    } while (!stop_requested);

cleanup:
    if (worker_started) sniff_worker_stop();
    if(ctx->sniff_opts.out_file != NULL)
        sniff_logger_file_close();  

    serial_close();

    /* Worker has stopped before closing its logger. */
    
    tiny_queue_destroy(inFrameQueue);

    return status;
}


// todo a revoir!!!!!, pour les test unitaires, on peut faire un sniff_test qui affiche les options et vérifie que les options sont bien parsées, et un sniff_run qui fait le vrai travail de sniffing, et qui sera testé manuellement
int sniff_test(struct cli_context *ctx)
{

    if (!ctx->sniff_opts.port) {
        printf("sniff: --port required\n");
        return 1;
    }

    if (!ctx->sniff_opts.baud) {
        printf("sniff: --baud required\n");
        return 1;
    }

    printf("Sniff start\n");
    printf("port: %s\n", ctx->sniff_opts.port);
    printf("baud: %s\n", ctx->sniff_opts.baud);
    
    if(ctx->sniff_opts.scbk_set) {
        printf("SCBK: [redacted]\n");
    }

    if(ctx->sniff_opts.scsk_set) {
        printf("SCSK: [redacted]\n");
    }

    if (ctx->sniff_opts.show_raw)
        printf("raw frames enabled\n");

    if (ctx->sniff_opts.show_decoded)
        printf("decoded frames enabled\n");

    if (ctx->sniff_opts.show_errors)
        printf("error frames enabled\n");

    if(ctx->sniff_opts.out_file != NULL)
        printf("output file: %s\n", ctx->sniff_opts.out_file);

    static char* sniff_out_format_enum[] = {

         "text",
         "json",
         "bin"
    }; 

    printf("output format: (%d) %s\n", ctx->sniff_opts.out_format, sniff_out_format_enum[ctx->sniff_opts.out_format]);

    if(ctx->sniff_opts.rotate > 0)
        printf("log rotation enabled, max size: %d MB\n", ctx->sniff_opts.rotate);

    return 0;    

}