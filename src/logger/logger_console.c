/**
 * @file logger_console.c
 * @brief Console logging backend for OSDP frame visualization.
 *
 * This module implements the console-based logging backend used by osdp-cli to display OSDP communication events in a human-readable form.
 *
 * The logger receives protocol events emitted by the core event dispatcher and renders captured frames using multiple output formats depending on
 * the active CLI configuration.
 *
 * Supported display modes include:
 *   - compact single-line frame display
 *   - formatted packet decoding
 *   - hexadecimal dump visualization
 *
 * The console logger is primarily intended for:
 *   - protocol debugging
 *   - frame inspection
 *   - development and troubleshooting
 *   - interactive terminal usage
 *
 * The module integrates with the OSDP event system through a listener registration mechanism and operates as a passive event consumer.
 */

#include "../renderer/renderer.h"
#include "../core/osdp_event.h"
#include "../cli/global_options_parser.h" 


void showOspdSC_parameter(FILE * outputStream, int32_t nbTrame);

/**
 * @brief Console logging callback for OSDP frame events.
 *
 * This logger renders captured OSDP frames to the standard output using different display modes depending on the active CLI context:
 *
 *   - quiet mode:
 *       Displays the complete frame on a single line.
 *
 *   - verbose mode:
 *       Displays a decoded and formatted packet view.
 *
 *   - default mode:
 *       Displays a classic hexadecimal dump.
 *
 * The logger is registered as an OSDP event listener and is invoked for every transmitted or received frame.
 *
 * @param ev         Pointer to the emitted OSDP event.
 * @param user_data  User-defined context pointer (expected to be
 *                   a struct cli_context instance).
 */
static void console_logger(const struct osdp_event *ev, void *user_data)
{
    struct cli_context *ctx = user_data;
    static int32_t nbTrame = 0;

    if(ctx->glob_options.quiet)
    {
        render_frame(stdout, ev->frame, ev->frame_len, ev->direction, ctx->glob_options.no_color);
        if(ctx->glob_options.sc_params)
            showOspdSC_parameter(stdout, nbTrame++);
    }

    else if(ctx->glob_options.verbose)
    {
        render_packet(stdout, ev->payload, ev->payload_len, ev->frame, ev->frame_len, ev->direction, ctx->glob_options.no_color);
        if(ctx->glob_options.sc_params)
            showOspdSC_parameter(stdout, nbTrame++);
    }

    else if(ctx->glob_options.json){
        render_json(stdout, ev, ctx->glob_options.no_color);
    }

    else
    {
        render_hexdump(stdout, ev->frame, ev->frame_len, ev->direction, ctx->glob_options.no_color);
        if(ctx->glob_options.sc_params)
            showOspdSC_parameter(stdout, nbTrame++);
    }
}


/**
 * @brief Initialize the console logging backend.
 *
 * Registers the console logger as an OSDP event listener so that all emitted protocol events are rendered to the terminal.
 *
 * @param user_data User-defined context passed back to the logger callback during event dispatching.
 */
void logger_console_init(void *user_data)
{
    osdp_register_listener(console_logger, user_data);
}