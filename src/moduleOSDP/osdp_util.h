#ifndef OSDP_UTIL_H
#define OSDP_UTIL_H

#include <stdint.h>
#include <stdbool.h>
#include "osdp_common.h"


uint16_t computeCrc16(const uint8_t *buf, size_t len);
uint8_t computeChecksum(uint8_t *msg, int length);
void printPacket(const char *buffer, size_t len, char direction);
char* byteArrayToHexaString(const uint8_t* byteArray, size_t byteArraySize);
uint8_t* hexaString2ByteArray(const char* hexString, size_t byteArraySize);
bool getRandomValue(uint8_t *buf, int len);
void oneComplementOfNumber(uint8_t * dest, const uint8_t * src);

#endif // OSDP_UTIL_H
