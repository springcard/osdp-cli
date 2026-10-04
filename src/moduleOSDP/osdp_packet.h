#ifndef OSDP_PACKET_H
#define OSDP_PACKET_H

#include <stdint.h>

int buildMsgPacket(uint8_t pdIndex, uint8_t addr, const void *msg2Send, size_t sizeCmd);
int extractMsgPacket(uint8_t pdIndex, void * buffer, size_t buffer_size/*, void * msg, size_t lenMsg*/);
int decryptMsgPacket(uint8_t pdIndex, void * buffer, size_t buffer_size);
// void setSecureChannelModePacket(uint8_t pdIndex);
// void setPlainTextModePacket(uint8_t pdIndex);
void osdpPacketInit(uint8_t pdIndex, uint8_t deviceType);//! a supp, uint8_t comType);

#endif // OSDP_PACKET_H