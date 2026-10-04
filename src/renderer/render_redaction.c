#include "renderer.h"
#include "../core/osdp_event.h"
#include "../moduleOSDP/osdp_common.h"

int render_frame_is_sensitive(const uint8_t *frame, size_t len, int direction)
{
    /* Layout includes the leading OSDP mark: FF 53 ADDR LENlo LENhi CTRL. */
    if (!frame || len < 7 || frame[0] != OSDP_PKT_MARK || frame[1] != OSDP_MSG_SOM)
        return 1;
    if (((size_t)frame[3] | ((size_t)frame[4] << 8)) != len - 1)
        return 1;
    size_t code_offset = 6;
    size_t trailer = (frame[5] & OSDP_MSG_CONTROL_CRC) ? 2 : 1;
    if (frame[5] & OSDP_MSG_CONTROL_SCB) {
        if (len < 9 || frame[6] < 2 || frame[6] > len - 6) return 1;
        if (frame[7] >= 0x15 && frame[7] <= 0x18) trailer += MAC_MSG_LEN;
        code_offset += frame[6];
    }
    if (code_offset >= len || len - code_offset <= trailer) return 1;
    return direction == CP_2_PD && frame[code_offset] == CMD_KEYSET;
}
