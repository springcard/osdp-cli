/**
 * @file logger_file.c
 * @brief File-based logging backend for OSDP event capture.
 *
 * This module implements a persistent logging backend that records OSDP protocol events into an output file.
 *
 * The file logger subscribes to the internal OSDP event dispatcher and serializes captured frames using different rendering strategies based
 * on the active CLI configuration.
 *
 * Supported output formats include:
 *   - compact raw frame logging
 *   - structured packet decoding
 *   - hexadecimal dump representation
 *   - JSON event serialization
 *
 * The backend is primarily intended for:
 *   - offline protocol analysis
 *   - debugging sessions
 *   - automated capture workflows
 *   - audit and trace collection
 *
 * Rendering logic is delegated to dedicated file renderer modules in order to maintain a clear separation between:
 *   - event collection
 *   - logging backend management
 *   - output formatting
 */

#include "../renderer/renderer.h"
#include "../core/osdp_event.h"
#include "../cli/global_options_parser.h" 

#include <string.h>

// file handle associated with the active file logger backend
static FILE *g_file = NULL;

void showOspdSC_parameter(FILE * outputStream, int32_t nbTrame);

/**
 * @brief File logger callback invoked for each emitted OSDP event.
 *
 * Depending on the active CLI configuration, the event is rendered using:
 *   - compact raw frame output
 *   - structured packet rendering
 *   - hexadecimal dump formatting
 *   - JSON serialization
 *
 * Output is written directly to the active log file.
 *
 * @param ev         Pointer to the emitted OSDP event.
 * @param user_data  Pointer to the CLI runtime context.
 */
static void file_logger(const struct osdp_event *ev, void *user_data)
{
    struct cli_context *ctx = user_data;
    static int32_t nbTrame = 0;
    if (!g_file)
        return;

    if(ctx->glob_options.log_format == LOG_TEXT)
    {
        if(ctx->glob_options.quiet)
        {         
            render_frame(g_file, ev->frame, ev->frame_len, ev->direction, ctx->glob_options.no_color);
            if(ctx->glob_options.sc_params)
                showOspdSC_parameter(g_file, nbTrame++);
        }

        else if(ctx->glob_options.verbose)
        {
            render_packet(g_file, ev->payload, ev->payload_len, (const uint8_t*)ev->frame, ev->frame_len, ev->direction, ctx->glob_options.no_color);
            if(ctx->glob_options.sc_params)
                showOspdSC_parameter(g_file, nbTrame++);
        }

        else
        {
            render_hexdump(g_file, ev->frame, ev->frame_len, ev->direction, ctx->glob_options.no_color);
            if(ctx->glob_options.sc_params)
                showOspdSC_parameter(g_file, nbTrame++);
        }
        
    }

    else if(ctx->glob_options.log_format == LOG_JSON)
    {
        render_json(g_file, ev, ctx->glob_options.no_color);
    }
}


/**
 * @brief Initialize the file logging backend.
 *
 * Opens the configured log file and registers the file logger as an OSDP event listener.
 *
 * Once initialized, all emitted protocol events are written to the associated output file.
 *
 * @param user_data Pointer to the CLI runtime context.
 *
 * @return 0 on success, non-zero on failure.
 */
int logger_file_init(void *user_data)
{
    struct cli_context *ctx = user_data;

    if (!ctx->glob_options.log_file_name)
        return 0;
    g_file = fopen(ctx->glob_options.log_file_name, "w");
    if (!g_file)
    {
        fprintf(stderr, "Pb création fichier!\n");
        return -1;    
    }

    osdp_register_listener(file_logger, user_data);

    return 0;
}


/**
 * @brief Shutdown the file logging backend.
 *
 * Flushes pending data and closes the active output file.
 */
void logger_file_close(void)
{
    if (g_file) {
        fclose(g_file);
        g_file = NULL;
    }
}
