/**
 * @file render_frame.c
 * @brief Colored raw-frame renderer for OSDP console output.
 *
 * This module provides a compact visual representation of raw OSDP frames using ANSI color codes to highlight protocol fields directly in the
 * terminal output.
 *
 * Unlike other renderers (hexdump or structured packet view), this module focuses on a continuous inline representation of the frame, where each
 * protocol segment is color-coded for quick visual parsing.
 *
 * The following fields are highlighted:
 *   - OSDP header (SOM, address, length)
 *   - control byte (CTRL)
 *   - secure channel block (SCB, when present)
 *   - command / response code
 *   - payload data
 *   - MAC (if applicable)
 *   - CRC / checksum
 *
 * This renderer is primarily intended for interactive debugging sessions where fast visual identification of protocol segments is required.
 *
 * Note: ANSI escape sequences are used for coloring and may not render correctly in non-terminal environments.
 */
#include "renderer.h"
#include "../moduleOSDP/osdp_common.h"
#include "../core/osdp_event.h"


/**
 * @brief Print a segment of bytes using a specific ANSI color.
 *
 * Helper function used to apply color formatting to a continuous range of bytes within a frame. The output is written directly to stdout.
 *
 * @param file Output stream where the hexdump will be written.
 * @param buf   Pointer to the full frame buffer.
 * @param start Start index within the buffer.
 * @param len   Number of bytes to print.
 * @param color ANSI escape sequence for the desired color.
 */
static void print_bytes_colored(FILE *file, const uint8_t *buf, size_t start, size_t len, const char *color)
{
    fprintf(file, "%s", color);
    for (size_t i = start; i < start + len; i++)
        fprintf(file, "%02X", buf[i]);
}


/**
 * @brief Render a colorized inline representation of an OSDP frame.
 *
 * Parses a raw OSDP frame and prints a single-line colored visualization where each protocol field is highlighted with a specific ANSI color.
 *
 * This function is intended for fast human inspection of protocol traffic, especially during live sniffing or debugging sessions.
 *
 * @param file Output stream where the hexdump will be written.
 * @param frame Pointer to the raw OSDP frame buffer.
 * @param len   Total length of the frame in bytes.
 * @param direction The direction of the frame (CP_2_PD or CP<-PD).
 * @param no_color  Flag to disable ANSI colors in the output.
 */
void render_frame(FILE *file, const uint8_t *frame, size_t len, int direction, int no_color)
{
    if (render_frame_is_sensitive(frame, len, direction)) {
        fprintf(file, "%s Frame (%zu bytes): [redacted]\n",
                direction == CP_2_PD ? "CP->PD" : "CP<-PD", len);
        return;
    }
    osdp_header_msg *pHeader =(osdp_header_msg *) frame;
    uint8_t nbBytes2Subtract = sizeof(osdp_header_msg) + 2 + 1; //+2 CRC_LEN +1 code CMD/RPLY 

    fprintf(file, "%s Frame (%zu bytes): ",(direction == CP_2_PD)? "CP->PD" : "CP<-PD", len);

    if(len == 0){ //response timeout -> no data
        // Add spacing after PD replies for readability
        if(direction == PD_2_CP) fprintf(file, "\n");
        return; 
    }

    if(no_color || (file != stdout && file != stderr))
    {
        // If no_color flag is set, print the frame as a simple hex string without colors
        for (size_t i = 0; i < len; i++)
            fprintf(file, "%02X", frame[i]);
        
        fprintf(file, "\n");
    }
    else
    {
        // If no_color flag is not set, proceed with colored rendering
        fprintf(file, "%s", C_RESET); // Ensure we start with default color

        // Header
        print_bytes_colored(file, (uint8_t *) pHeader,  0, sizeof(osdp_header_msg) -1, C_GREEN);

        //CTRL
        print_bytes_colored(file, (uint8_t *) &(pHeader->ctrl),  0, 1, C_YELLOW);

        // Secure channel
        if(pHeader->ctrl & 0x08)
        {
            // SEC_BLK
            print_bytes_colored(file, (uint8_t *) &(pHeader->scb), 0, pHeader->scb[0 ], C_ORANGE);  // 031100            

            switch (pHeader->scb[1])
            {
                case 0x11:
                case 0x12:
                case 0x13:
                case 0x14:
                    nbBytes2Subtract += pHeader->scb[0]; //+SCB_LEN 
                    break;
                
                case 0x15:
                case 0x16:              
                case 0x17:
                case 0x18:
                    nbBytes2Subtract += (pHeader->scb[0] + 4); //+ SCB_LEN + 4 (MAC_LEN)
                    break;            
                default:
                    break;
            }
        }
        
        // Pointer to the command or response in the sent or received frame.
        const uint8_t * cmd_reply = (frame + sizeof(osdp_header_msg) + ((pHeader->ctrl & 0x08)? pHeader->scb[0] : 0));

        // Code CMD/REPLY
        print_bytes_colored(file, cmd_reply,  0, 1, C_WHITE);

        // DATA
        print_bytes_colored(file, cmd_reply + 1 , 0, len - nbBytes2Subtract, C_GRAY);
        
        // MAC
        if((pHeader->ctrl & 0x08) && (pHeader->scb[1] > 0x14))
            print_bytes_colored(file, &frame[len-6], 0, 4, C_ORANGE); 

        // CRC
        print_bytes_colored(file, &frame[len - 2], 0, 2, C_BLUE);

        fprintf(file, "%s\n", C_RESET);

    }

    // Add spacing after PD replies for readability
    if(direction == PD_2_CP) fprintf(file, "\n");
}
