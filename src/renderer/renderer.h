/**
 * @file renderer.h
 * @brief Rendering utilities for OSDP frame visualization.
 *
 * This module defines a set of rendering interfaces used to display OSDP protocol data in a human-readable format.
 *
 * It provides multiple rendering strategies depending on the desired level of verbosity:
 *   - raw hexadecimal dump view
 *   - structured packet interpretation
 *   - compact frame summary view
 *   - detailed message content display
 *
 * The rendering functions operate on generic C standard I/O streams (FILE*), allowing output to be directed to stdout, stderr, or files. This makes the
 * renderer flexible and suitable for various logging backends and diagnostics workflows.
 *
 * This module is primarily used by logging backends (such as console or file loggers) to present captured OSDP traffic during debugging and sniffing sessions.
 *
 * The renderer layer is intentionally separated from the logging system to ensure a clear distinction between:
 *   - event/log generation (protocol and logger layers)
 *   - presentation logic (rendering layer)
 */ 
#ifndef RENDERER_H
#define RENDERER_H

#pragma once

#include <stdint.h>
#include <stddef.h>
#include <stdio.h>

/* Suppress KEYSET and frames that cannot be safely classified. */
int render_frame_is_sensitive(const uint8_t *frame, size_t len, int direction);


#define C_ORANGE   "\x1b[38;5;208m"   // orange (palette étendue)
#define C_GREEN    "\x1b[32m"
#define C_GREEN_2  "\x1b[38;5;22m"   // (palette étendue)
#define C_BLUE     "\x1b[34m"
#define C_BLUE_2   "\x1b[38;5;33m"   // (palette étendue)
#define C_YELLOW   "\x1b[33m"
#define C_WHITE    "\x1b[37m"
#define C_GRAY     "\x1b[90m"
#define C_RESET    "\x1b[0m"

/**
 * @brief Render a raw memory buffer as a hexadecimal dump.
 *
 * Displays the provided byte buffer in a classic hex + ASCII format, typically used for low-level debugging of OSDP frames.
 *
 * @param file Output stream where the hexdump will be written.
 * @param data Pointer to the raw data buffer.
 * @param len  Number of bytes to display.
 * @param direction Communication direction identifier.
 * @param no_color  Flag to disable ANSI colors in the output.
 */
void render_hexdump(FILE *file, const uint8_t *data, size_t len, int direction, int no_color);

/**
 * @brief Render a decoded OSDP packet in a human-readable format.
 *
 * Interprets the frame content according to the OSDP protocol structure and prints a structured representation depending on the communication
 * direction (CP -> PD or PD -> CP).
 *
 * @param file Output stream where the hexdump will be written.
 * @param payload   Pointer to the decoded message payload (command or reply structure).
 * @param payload_len Length of the payload in bytes.
 * @param frame     Pointer to the raw OSDP frame.
 * @param len       Length of the frame in bytes.
 * @param direction Communication direction identifier.
 * @param no_color  Flag to disable ANSI colors in the output.
 */
void render_packet(FILE *file, const void *payload, size_t payload_len, const uint8_t *frame, size_t len, int direction, int no_color);

/**
 * @brief Render a compact summary of an OSDP frame.
 *
 * Provides a minimal one-line representation of the frame content, typically used in quiet or condensed output modes.
 *
 * @param file Output stream where the hexdump will be written.
 * @param frame Pointer to the raw OSDP frame.
 * @param len   Length of the frame in bytes.
 * @param direction Communication direction identifier.
 * @param no_color  Flag to disable ANSI colors in the output.
 */
void render_frame(FILE *file, const uint8_t *frame, size_t len, int direction, int no_color);

/**
 * @brief Render a decoded OSDP message (command or reply) in a human-readable format.
 *
 * Interprets the payload of an OSDP message and prints its content in a structured way, depending on the message type and communication direction.
 *
 * @param file Output stream where the hexdump will be written.
 * @param payload   Pointer to the decoded message payload (command or reply structure).
 * @param len       Length of the original frame from which the message was extracted (used for context).
 * @param direction Communication direction identifier (CP -> PD or PD -> CP).
 * @param no_color  Flag to disable ANSI colors in the output.
 */
void render_osdp_message(FILE *file, const void *payload, size_t len, int direction, int no_color);


/**
 * @brief Render a complete OSDP event as JSON.
 *
 * Serializes a higher-level event object into structured JSON format.
 *
 * @param file  Output file stream.
 * @param event Pointer to the event object to serialize.
 * @param no_color  Flag to disable ANSI colors in the output.
 */
void render_json(FILE *file, const void *event, int no_color);


/**
 * @brief Render a complete OSDP command or reply as JSON.
 *
 * Serializes a higher-level cmd/reply object into structured JSON format.
 *
 * @param file  Output file stream.
 * @param payload Pointer to the payload structure of command or reply.
 * @param len Length of the decoded payload in bytes.
 * @param direction Communication direction identifier (CP -> PD or PD -> CP).
 * @param no_color  Flag to disable ANSI colors in the output.
 */
void render_osdp_message_json(FILE *file, const void *payload, size_t len, int direction, int no_color);

#endif // RENDERER_H
