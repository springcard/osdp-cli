/**
 * @file render_message_json.c
 * @brief Renders decoded OSDP commands and replies JSON structured objects suitable console/log file.
 *
 * This module implements a high-level OSDP message renderer used by the sniffing subsystem and debugging tools.
 *
 * Unlike raw frame or packet renderers, this renderer operates on decoded OSDP command and reply structures extracted from captured frames.
 *
 * The renderer displays:
 *   - OSDP command identifiers and associated fields
 *   - OSDP reply identifiers and associated fields
 *   - secure channel data
 *   - protocol parameters and payload contents
 *
 * Messages are colorized according to communication direction:
 *   - CP -> PD messages are displayed in blue
 *   - PD -> CP messages are displayed in green
 *
 * The module contains specialized helper functions responsible for formatting and printing individual OSDP command and reply types.
 */

#include "renderer.h"
#include "../core/osdp_event.h"
#include "../moduleOSDP/osdp_common.h"
#include "../util/utils.h"
#include "osdp_message_printer_json.h"


/**
 * @brief Renders a decoded OSDP command or reply message to a stream  (console/log file) in structured JSON objects.
 *
 * This function displays a high-level decoded representation of an OSDP payload previously extracted from a raw OSDP frame.
 *
 * Depending on the communication direction, the payload is interpreted either as an OSDP command (`osdp_cmd`) or as an OSDP reply (`osdp_reply`).
 *
 * The renderer:
 *   - Selects a display color based on frame direction.
 *   - Displays the OSDP command or reply identifier and symbolic name.
 *   - Dispatches the payload to a dedicated decoder/renderer function
 *     associated with the command or reply type.
 *   - Prints detailed field values specific to the decoded structure.
 *
 * Supported commands and replies include:
 *   - Secure channel messages
 *   - Device identification
 *   - Capability reports
 *   - LED / buzzer / output control
 *   - Keypad and raw card data
 *   - Communication settings
 *   - File transfer messages
 *   - Status reports
 *
 * ANSI terminal colors are used to visually distinguish traffic directions:
 *   - Blue   : CP -> PD
 *   - Green  : PD -> CP
 *
 * @param payload
 * Pointer to the decoded OSDP payload structure.
 * Must point to either an `osdp_cmd` or an `osdp_reply` depending on the communication direction.
 *
 * @param len
 * Size of the decoded payload in bytes.
 *
 * @param direction
 * Communication direction:
 *   - `CP_2_PD` : Command sent from Control Panel to Peripheral Device
 *   - `PD_2_CP` : Reply sent from Peripheral Device to Control Panel
 * 
 * @param no_color  Flag to disable ANSI colors in the output.
 */ 
void render_osdp_message_json(FILE *outputStream, const void *payload, size_t len, int direction, int no_color)
{

    osdp_cmd *cmd = (osdp_cmd *) payload;
    osdp_reply *reply = (osdp_reply *) payload;
    const char *color;

    if(!no_color)
    {
        // Color choice based on direction
        if(direction == CP_2_PD) color = C_BLUE;
        else if(direction == PD_2_CP) color = C_GREEN;
        else color = C_RESET;
        // set print color 
        fprintf(outputStream, "%s", color);        
    }

    fprintf(outputStream, "{\n");    

    // CMD/REPLY   
    fprintf(outputStream, "\t\"%s\" : \"0x%02X\",\n",(direction == CP_2_PD)? "CMD":"REPLY", (direction == CP_2_PD)? cmd->cmdId : reply->replyId);

    // CMD/REPLY   
    fprintf(outputStream, "\t\"Name\" : \"%s\",\n",
        (direction == CP_2_PD)? cmd_name(cmd->cmdId):reply_name(reply->replyId));

    if(direction == CP_2_PD)
        print_osdp_cmd_json(outputStream, cmd, len);
    else
        print_osdp_reply_json(outputStream, reply, len);

    fprintf(outputStream, "}\n");         

    if(!no_color)// Restore default terminal color
        fprintf(outputStream, "%s", C_RESET);

    // Add spacing after PD replies for readability
    if(direction == PD_2_CP) fprintf(outputStream, "\n");

}
