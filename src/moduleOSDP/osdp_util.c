#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <time.h>

#include "osdp_util.h"  // Inclure le fichier d'en-tête
#include "../platform/random.h"


static uint16_t crc16_itu_t(uint16_t seed, const uint8_t *src, size_t len)
{
	for (; len > 0; len--) {
		seed = (seed >> 8U) | (seed << 8U);
		seed ^= *src++;
		seed ^= (seed & 0xffU) >> 4U;
		seed ^= seed << 12U;
		seed ^= (seed & 0xffU) << 5U;
	}
	return seed;
}


uint16_t computeCrc16(const uint8_t *buf, size_t len)
{
	return crc16_itu_t(0x1D0F, buf, len);
}


uint8_t computeChecksum(uint8_t *msg, int length)
{
	uint8_t checksum = 0;
	int i, whole_checksum;

	whole_checksum = 0;
	for (i = 0; i < length; i++) {
		whole_checksum += msg[i];
		checksum = ~(0xff & whole_checksum) + 1;
	}
	return checksum;
}


void printPacket(const char *buffer, size_t len, char direction){

    if(direction == 's' || direction == 'S')
    	printf("Sended: ");
	else if(direction == 'r' || direction == 'R')
    	printf("Received: ");

    for(int i=0; i < len; i++){
        printf("%02x",(uint8_t) buffer[i]);
    }
    printf("\n\n");
}


bool getRandomValue(uint8_t *buf, int len)
{
    return len >= 0 && osdp_random_bytes(buf, (size_t)len);
}


void oneComplementOfNumber(uint8_t * dest, const uint8_t * src)
{
	for (int i = 0; i < 16; i++) {
		dest[i] = ~src[i];
	}
}

