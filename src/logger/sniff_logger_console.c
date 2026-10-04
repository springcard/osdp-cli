/**
 * @file sniff_logger_console.c
 * @brief Console logger implementation for the sniff command.
 *
 * This module implements a console-based event listener dedicated to the OSDP sniffing subsystem.
 *
 * The logger subscribes to the global OSDP event dispatcher and renders captured frames according to the sniff command configuration.
 *
 * Supported display modes include:
 *   - raw colored frame rendering
 *   - decoded OSDP command/reply rendering
 *
 * Rendering behavior is controlled through the sniff_options structure provided as user context during initialization.
 */

#include "../renderer/renderer.h"
#include "../core/osdp_event.h"
#include "../cli/global_options_parser.h" 
#include "../commands/sniff/sniff_options.h"

#include <stdio.h>


/**
 * @brief Console event listener for sniffed OSDP frames.
 *
 * This callback is invoked whenever a new OSDP event is emitted by the event pipeline.
 *
 * Depending on the active sniff options, the logger either:
 *   - displays the raw OSDP frame
 *   - displays the decoded OSDP command or reply
 *
 * @param ev         Pointer to the emitted OSDP event.
 * @param user_data  Pointer to the sniff context structure.
 */
static void sniff_logger_console(const struct osdp_event *ev, void *user_data)
{
    struct cli_context *ctx = user_data;

    if(ctx->sniff_opts.show_raw)
    {
        render_frame(stdout, ev->frame, ev->frame_len, ev->direction, ctx->glob_options.no_color);
    }

    else if(ctx->sniff_opts.show_decoded)
    {
        render_osdp_message(stdout, ev->payload, ev->frame_len, ev->direction, ctx->glob_options.no_color);
    }
}


/**
 * @brief Initialize the sniff console logger.
 *
 * Registers the console logger callback into the global OSDP event dispatcher.
 *
 * @param user_data Pointer to the sniff_options configuration structure.
 */
void sniff_logger_console_init(void *user_data)
{
    osdp_register_listener(sniff_logger_console, user_data);
}