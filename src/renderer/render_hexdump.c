/**
 * @file render_hexdump.c
 * @brief Hexadecimal dump renderer for output streams.
 *
 * This module provides a classic hexdump-style visualization of raw binary data, combining both hexadecimal and ASCII representations.
 *
 * It is primarily used for low-level debugging of OSDP frames, allowing developers to inspect raw packet contents in a structured and readable format.
 *
 * The output format includes:
 *   - byte offset per line
 *   - hexadecimal representation of each byte
 *   - ASCII representation for printable characters
 *   - placeholder '.' for non-printable bytes
 *
 * The renderer writes its output to a configurable C standard I/O stream (FILE*), which can be redirected to stdout, stderr, or a file. This provides greater
 * flexibility for logging, debugging, and integration with different backends.
 *
 * This renderer is intended for diagnostic purposes within logging or console output systems.
 */ 

#include "renderer.h"
#include "../core/osdp_event.h"

#include <stdio.h>
#include <ctype.h>


/**
 * @brief Render a buffer as a classic hexadecimal dump to an output stream.
 *
 * Displays binary data in a structured format consisting of:
 *   - offset column
 *   - hexadecimal byte values
 *   - ASCII interpretation
 *
 * Each line represents 16 bytes of input data.
 *
 * The output is written to the provided C standard I/O stream (FILE*), allowing redirection to stdout, stderr, or a file.
 *
 * @param file Output stream where the hexdump will be written.
 * @param data Pointer to the data buffer to display.
 * @param len  Number of bytes in the buffer.
 * @param direction Communication direction identifier.
 *   - `CP_2_PD` : Command sent from Control Panel to Peripheral Device
 *   - `PD_2_CP` : Reply sent from Peripheral Device to Control Panel
 * @param no_color Flag to disable ANSI colors in the output.
 */
void render_hexdump(FILE *file, const uint8_t *data, size_t len, int direction, int no_color)
{
    if (render_frame_is_sensitive(data, len, direction)) {
        fprintf(file, "%s Frame (%zu bytes): [redacted]\n",
                direction == CP_2_PD ? "CP->PD" : "CP<-PD", len);
        return;
    }
    const size_t width = 16;
    const char *color;

    if(!no_color && (file == stdout || file == stderr))
    {
        // Color choice based on direction
        if(direction == CP_2_PD) color = C_BLUE;
        else if(direction == PD_2_CP) color = C_GREEN;
        else color = C_RESET;

        // set print color 
        fprintf(file, "%s", color);
    }

    fprintf(file, "%s Frame (%zu bytes):\n",(direction == CP_2_PD)? "CP->PD" : "CP<-PD", len);

    for (size_t i = 0; i < len; i += width) {

        // offset
        fprintf(file, "%04zx  ", i);

        // hex
        for (size_t j = 0; j < width; j++) {

            if (i + j < len)
                fprintf(file, "%02X ", data[i + j]);
            else
                fprintf(file, "   ");
        }

        fprintf(file, " ");

        // ASCII
        for (size_t j = 0; j < width; j++) {

            if (i + j < len) {
                uint8_t c = data[i + j];
                fprintf(file, "%c", isprint(c) ? c : '.');
            }
        }

        fprintf(file, "\n");
    }

    if(!no_color && (file == stdout || file == stderr))   // Restore default terminal color
        fprintf(file, "%s", C_RESET);

    // Add spacing after PD replies for readability
    if(direction == PD_2_CP) fprintf(file, "\n");    
}
