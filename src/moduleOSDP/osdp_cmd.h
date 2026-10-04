#ifndef OSDP_CMD_H
#define OSDP_CMD_H

#include <stdint.h>
#include <stddef.h>


void setSecureChannelMode(uint8_t pdIndex);
void setPlainTextMode(uint8_t pdIndex);
size_t buildMsg(uint8_t iLecteur, uint8_t addr, uint8_t msgCode, const void *data, size_t sizeData);
int extractReply(uint8_t pdIndex, uint8_t *replyCode, void *replyData, size_t capacity);

#endif // OSDP_CMD
