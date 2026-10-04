/**
 * @file render_packet.c
 * @brief OSDP packet decoding and structured console rendering.
 *
 * This module implements a detailed decoder for OSDP frames and renders their content in a human-readable, structured format on the console.
 *
 * It interprets the raw frame buffer according to the OSDP protocol specification and extracts key protocol fields such as:
 *   - header metadata (SOM, address, length, control flags)
 *   - secure channel information (SCB / MAC when present)
 *   - command or reply identifiers
 *   - payload data
 *   - integrity fields (CRC / checksum)
 *
 * The output is intended for debugging, protocol analysis, and sniffing use cases where full visibility into OSDP exchanges is required.
 *
 * This renderer is used by the console logging backend when verbose packet inspection mode is enabled.
 */
#include "renderer.h"
#include "../moduleOSDP/osdp_common.h"
#include "../util/utils.h"
#include "../core/osdp_event.h"
#include "osdp_message_printer.h"


/**
 * @brief Render a decoded OSDP packet in structured form.
 *
 * Parses the raw OSDP frame and prints a detailed breakdown of all protocol fields including header, secure channel data, command/reply
 * identifier, payload, MAC (if present), and CRC.
 *
 * This function is intended for verbose debugging mode where full visibility into protocol exchanges is required.
 *
 * @param file      Output file stream.
 * @param payload   Pointer to the decoded message payload (command or reply structure).
 * @param payload_len Length of the payload in bytes.
 * @param frame     Pointer to raw OSDP frame buffer.
 * @param len       Total length of the frame in bytes.
 * @param direction Communication direction (CP->PD or PD->CP).
 * @param no_color  Flag to disable ANSI colors in the output.
 */
void render_packet(FILE *file, const void *payload, size_t payload_len, const uint8_t *frame, size_t len, int direction, int no_color)
{
    if (render_frame_is_sensitive(frame, len, direction) ||
        (direction == CP_2_PD && payload && payload_len > 0 &&
         ((const osdp_cmd *)payload)->cmdId == CMD_KEYSET)) {
        fprintf(file, "%s Frame (%zu bytes): [redacted]\n",
                direction == CP_2_PD ? "CP->PD" : "CP<-PD", len);
        return;
    }
    osdp_header_msg *pHeader =(osdp_header_msg *) frame;
    uint8_t nbBytes2Subtract = sizeof(osdp_header_msg) + 2 + 1; //+2 CRC_LEN +1 code CMD/RPLY 
    osdp_cmd *cmd = (osdp_cmd *) payload;
    osdp_reply *reply = (osdp_reply *) payload;

    const char *color;

    if(!no_color && (file == stdout || file == stderr))
    {
        // Color choice based on direction
        if(direction == CP_2_PD) color = C_BLUE_2;
        else if(direction == PD_2_CP) color = C_GREEN;
        else color = C_RESET;

        // set print color 
        fprintf(file, "%s", color);
    }

    fprintf(file, "%s Frame (%zu bytes)\n",(direction == CP_2_PD)? "CP->PD" : "CP<-PD", len);

    // HEADER
    fprintf(file, "\tOSDP packet:\n");
    fprintf(file, "\t\tSOM..: 0x%02X\n", pHeader->som);
    fprintf(file, "\t\tADDR.: 0x%02X (%u)\n", pHeader->addr, pHeader->addr);
    uint16_t length = *((uint16_t *) (frame + 3));
    fprintf(file, "\t\tLEN..: %u\n", length);
    fprintf(file, "\t\tCTRL.: 0x%02X -> %s, %s, SQN(%X)\n", pHeader->ctrl, 
        (pHeader->ctrl & 0x08) ? "SCB":"no SCB",
        (pHeader->ctrl & 0x04) ? "CRC":"CKSUM",
        (pHeader->ctrl & 0x03));

    // Secure channel
    if(pHeader->ctrl & 0x08)
    {
        // SEC_BLK
        if(pHeader->scb[0] == 3)
            fprintf(file, "\t\tSC_BK: LEN(%X), TYPE(0x%02X), Data(0x%02X)\n", pHeader->scb[0], pHeader->scb[1], pHeader->scb[2]);
        else
            fprintf(file, "\t\tSC_BK: LEN(%X), TYPE(0x%02X)\n", pHeader->scb[0], pHeader->scb[1]);

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

    const uint8_t * cmd_reply = (frame + sizeof(osdp_header_msg) + ((pHeader->ctrl & 0x08)? pHeader->scb[0] : 0));

    // CMD/REPLY   
    fprintf(file, "\t\t%s: 0x%02X (%s)\n",(direction == 0)? "CMD..":"REPLY", *cmd_reply, (direction == 0)? cmd_name(*cmd_reply):reply_name(*cmd_reply));

    // DATA
    fprintf(file, "\t\tDATA.: ");
    for(int i = 0; i < len - nbBytes2Subtract; i++)   
        ((i + 1) == (len - nbBytes2Subtract)) ? fprintf(file, "%02X\n", *(cmd_reply + 1 + i)) :  fprintf(file, "%02X", *(cmd_reply + 1 + i));       

    // MAC
    if((pHeader->ctrl & 0x08) && (pHeader->scb[1] > 0x14))
        fprintf(file, "\t\tMAC..: %02X%02X%02X%02X\n", frame[len-6], frame[len-5],frame[len-4], frame[len-3]);
    
    // CRC
    fprintf(file, "\t\tCRC..: 0x%02X%02X\n", frame[len-1], frame[len-2]);

    // Decoded content (CMD/REPLY fields)
    if(!no_color && (file == stdout || file == stderr))
    {
        // Color choice based on direction
        if(direction == CP_2_PD) color = C_BLUE;
        else if(direction == PD_2_CP) color = C_GREEN_2;
        else color = C_RESET;

        // set print color 
        fprintf(file, "%s", color);
    }

    if(payload != NULL){
        fprintf(file, "\tUseful data:\n");
        if(direction == CP_2_PD)
            print_osdp_cmd(file, cmd, payload_len);
        else
            print_osdp_reply(file, reply, payload_len);
    }

    if(!no_color && (file == stdout || file == stderr))
        fprintf(file, "%s", C_RESET); // Restore default terminal color
           
    // Add spacing after PD replies for readability    
    if(direction == PD_2_CP) fprintf(file, "\n");    
}
