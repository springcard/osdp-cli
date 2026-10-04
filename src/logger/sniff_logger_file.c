/**
 * @file sniff_logger_file.c
 * @brief Implements the file logger backend for the sniff command.
 *
 * This module provides a dedicated file logging backend used by the OSDP sniff command. It registers an event listener in the OSDP
 * event pipeline and writes captured frames or decoded messages to an output file.
 *
 * Depending on the sniff configuration, the logger can generate:
 *   - raw hexadecimal frame dumps,
 *   - decoded OSDP command/reply representations,
 *   - JSON formatted events,
 *   - binary/hexdump outputs.
 *
 * The module relies on the rendering subsystem to format captured OSDP traffic before writing it to the selected output stream.
 *
 * Supported output formats are selected through the sniff_options structure provided during initialization.
 */
#include "../renderer/renderer.h"
#include "../core/osdp_event.h"
#include "../cli/global_options_parser.h" 
#include "../commands/sniff/sniff_options.h"

#include <string.h>

// file handle associated with the active file logger backend
static FILE *g_file = NULL;

/**
 * @brief Callback function invoked for each emitted OSDP event.
 *
 * This function is registered as an OSDP event listener and is responsible for formatting and writing sniffed traffic to the
 * configured output file.
 *
 * The rendering behavior depends on the selected sniff options:
 *   - raw frame rendering,
 *   - decoded OSDP message rendering,
 *   - JSON event serialization,
 *   - binary/hexdump output.
 *
 * @param ev Pointer to the emitted OSDP event.
 * @param user_data Pointer to the sniff_options configuration structure.
 */
static void sniff_file_logger(const struct osdp_event *ev, void *user_data)
{
    struct cli_context *ctx = user_data;

    if (!g_file)
        return;

    if(ctx->sniff_opts.out_format == SNIFF_OUT_TEXT)
    {
        if(ctx->sniff_opts.show_raw)
        {
            render_frame(g_file, ev->frame, ev->frame_len, ev->direction, ctx->glob_options.no_color);
        }

        else if(ctx->sniff_opts.show_decoded)
        {
            render_osdp_message(g_file, ev->payload, ev->frame_len, ev->direction, ctx->glob_options.no_color);
        }

        else if(ctx->sniff_opts.show_errors)
        {
            //todo
        }
           
    }

    else if(ctx->sniff_opts.out_format == SNIFF_OUT_JSON)
    {
        render_osdp_message_json(g_file, ev->payload, ev->payload_len, ev->direction, ctx->glob_options.no_color);
    }

    else if(ctx->sniff_opts.out_format == SNIFF_OUT_BIN)
    {
        render_hexdump(g_file, ev->frame, ev->frame_len, ev->direction, ctx->glob_options.no_color);    
    }

}


/**
 * @brief Initializes the sniff file logger backend.
 *
 * Opens the configured output file and registers the file logger callback into the OSDP event pipeline.
 *
 * @param user_data Pointer to the sniff_options configuration structure.
 *
 * @return 0 on success, -1 if the output file cannot be created.
 */
int sniff_logger_file_init(void *user_data)
{
    struct cli_context *ctx = user_data;

    g_file = fopen(ctx->sniff_opts.out_file, "w");
    if (!g_file)
    {
        fprintf(stderr, "Pb création fichier!\n");
        return -1;    
    }

    osdp_register_listener(sniff_file_logger, user_data);

    return 0;
}


/**
 * @brief Closes the active sniff log file.
 *
 * Releases the file handle associated with the sniff file logger backend and resets the internal logger state.
 */
void sniff_logger_file_close(void)
{
    if (g_file) {
        fclose(g_file);
        g_file = NULL;
    }
}
