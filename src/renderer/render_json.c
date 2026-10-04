/**
 * @file render_json.c
 * @brief JSON renderer for file-based OSDP event logging.
 *
 * This module serializes OSDP protocol events into structured JSON objects suitable for offline analysis.
 *
 * The renderer extracts and encodes key protocol information from captured OSDP frames, including:
 *   - timestamp metadata
 *   - communication direction
 *   - raw frame data
 *   - header fields
 *   - secure channel information
 *   - command / reply identifiers
 *   - payload content
 *   - MAC and CRC fields
 *
 * Each emitted event is written as a single-line JSON document in order to simplify log streaming
 *
 * The generated JSON format is intended to remain lightweight while preserving sufficient protocol visibility for debugging and analysis.
 */

#include "renderer.h"
#include "../core/osdp_event.h"
#include "../moduleOSDP/osdp_common.h"
#include "../util/utils.h"

#include <stdio.h>


/**
 * @brief Encode binary data as a hexadecimal string.
 *
 * Writes the provided byte buffer as a continuous uppercase hexadecimal sequence directly into the specified output stream.
 *
 * This helper is primarily used for serializing raw frame data and payload fields into JSON-compatible string values.
 *
 * @param data Pointer to the input buffer.
 * @param len  Number of bytes to encode.
 * @param f    Output file stream.
 */
static void hex_encode(const uint8_t *data, size_t len, FILE *f)
{
    for (size_t i = 0; i < len; i++)
    {
        fprintf(f, "%02X", data[i]);
    }

}


/**
 * @brief Render an OSDP event as a JSON document.
 *
 * Serializes the provided OSDP event into a JSON object containing protocol metadata and decoded frame information.
 *
 * The generated JSON output includes:
 *   - event timestamp
 *   - frame length
 *   - secure channel status
 *   - communication direction
 *   - raw frame bytes
 *   - decoded header fields
 *   - secure channel metadata
 *   - command / reply identifiers
 *   - payload data
 *   - MAC and CRC fields
 *
 * Output is written directly to the specified file stream.
 *
 * @param file  Output file stream.
 * @param event Pointer to the OSDP event structure.
 */
void render_json(FILE *file, const void *event, int no_color)
{

    const struct osdp_event *ev =  (const struct osdp_event *) event;
    if (render_frame_is_sensitive(ev->frame, ev->frame_len, ev->direction) ||
        (ev->direction == CP_2_PD && ev->payload && ev->payload_len > 0 &&
         *(const uint8_t *)ev->payload == CMD_KEYSET)) {
        fprintf(file, "{\"ts\":%llu,\"len\":%zu,\"dir\":\"%s\",\"redacted\":true}\n",
                (unsigned long long)ev->timestamp, ev->frame_len,
                ev->direction == CP_2_PD ? "CP->PD" : "CP<-PD");
        fflush(file);
        return;
    }
    osdp_header_msg *pHeader =(osdp_header_msg *) ev->frame;
    uint8_t nbBytes2Subtract = sizeof(osdp_header_msg) + 2 + 1; //+2 CRC_LEN +1 code CMD/RPLY 
    const char *color;


    if(!no_color)
    {
        // Color choice based on direction
        if(ev->direction == CP_2_PD) color = C_BLUE;
        else if(ev->direction == PD_2_CP) color = C_GREEN;
        else color = C_RESET;
        // set print color 
        fprintf(file, "%s", color);        
    }

    fprintf(file, "{\n");

    // Event metadata
    fprintf(file, "\t\"ts\":%llu,\n", (unsigned long long)ev->timestamp);

    fprintf(file, "\t\"len\":%zu,\n", ev->frame_len);

    fprintf(file, "\t\"secure\":%d,\n", ev->is_secure);

    fprintf(file, "\t\"dir\":\"%s\",\n", (ev->direction == CP_2_PD)? "CP->PD" : "CP<-PD");

    fprintf(file, "\t\"frame\":\"");

    hex_encode(ev->frame, ev->frame_len, file);

    fprintf(file, "\",\n");

    fprintf(file, "\t\"msg\": {\n");

    // HEADER
    fprintf(file, "\t\t\"SOM\":\"0x%02X\",\n", pHeader->som);
    fprintf(file, "\t\t\"ADDR\": %u,\n", pHeader->addr);    

    uint16_t length = *((uint16_t *) (ev->frame + 3));
    fprintf(file, "\t\t\"LEN\": %u,\n", length);
    fprintf(file, "\t\t\"CTRL\":\"0x%02X\",\n", pHeader->ctrl);

    fprintf(file, "\t\t\"SCB\":\"%s\",\n", (pHeader->ctrl & 0x08) ? "yes":"no");
    fprintf(file, "\t\t\"error_detection\":\"%s\",\n", (pHeader->ctrl & 0x04) ? "CRC":"CKSUM");
    fprintf(file, "\t\t\"SQN\":%u,\n", pHeader->ctrl & 0x03);

    // Secure channel
    if(pHeader->ctrl & 0x08)
    {
        // SEC_BLK
        fprintf(file, "\t\t\"SEC_BLK_LEN\":%u,\n", pHeader->scb[0]);
        fprintf(file, "\t\t\"SEC_BLK_TYPE\":\"0x%02X\",\n", pHeader->scb[1]);
        if(pHeader->scb[0] == 3)
            fprintf(file, "\t\t\"SEC_BLK_DATA\":\"%02X\",\n", pHeader->scb[2]);

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

    const uint8_t * cmd_reply = (ev->frame + sizeof(osdp_header_msg) + ((pHeader->ctrl & 0x08)? pHeader->scb[0] : 0));

    // CMD/REPLY   
    fprintf(file, "\t\t\"%s\":\"0x%02X\",\n",(ev->direction == 0)? "CMD":"REPLY", *cmd_reply);
    fprintf(file, "\t\t\"NAME\":\"%s\",\n", (ev->direction == 0)? cmd_name(*cmd_reply):reply_name(*cmd_reply));

    // DATA
    fprintf(file, "\t\t\"DATA\":\"");
    if(ev->frame_len - nbBytes2Subtract) // if there is a payload
    {
        for(int i = 0; i < ev->frame_len - nbBytes2Subtract; i++)   
            ((i + 1) == (ev->frame_len - nbBytes2Subtract)) ? fprintf(file, "%02X\",\n", *(cmd_reply + 1 + i)) :  fprintf(file, "%02X", *(cmd_reply + 1 + i));  
    }
    else 
    {
        fprintf(file, "\",\n");
    }

    // MAC
    if((pHeader->ctrl & 0x08) && (pHeader->scb[1] > 0x14))
        fprintf(file, "\t\t\"MAC\":\"%02X%02X%02X%02X\",\n", ev->frame[ev->frame_len-6], ev->frame[ev->frame_len-5],ev->frame[ev->frame_len-4], ev->frame[ev->frame_len-3]);
    
    // CRC
    fprintf(file, "\t\t\"CRC\":\"0x%02X%02X\"\n", ev->frame[ev->frame_len-1], ev->frame[ev->frame_len-2]);
    
    fprintf(file, "\t}\n");

    fprintf(file, "}\n");

    if(!no_color)// Restore default terminal color
        fprintf(file, "%s", C_RESET);    

    // Add spacing after PD replies for readability
    if(ev->direction == PD_2_CP) fprintf(file,"\n");   

    fflush(file);    
}
