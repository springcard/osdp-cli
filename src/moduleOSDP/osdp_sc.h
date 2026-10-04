#ifndef OSDP_SC_H
#define OSDP_SC_H

#include <stdint.h>

void computeSecureSessionParametersOfCP(uint8_t pdIndex, uint8_t * randB, uint8_t * clientId, uint8_t * cliCryptogram);
int verifyCryptogram(uint8_t pdIndex, uint8_t * cryptogram, uint8_t typeDevice);
void setInitialRmac(uint8_t pdIndex, uint8_t * rmac_i);
uint8_t * encryptData(uint8_t pdIndex, int is_cmd, uint8_t *rawDataToSend, int length);
void computeMac(uint8_t pdIndex, int is_cmd, uint8_t * rawDataToSend, int len);
int decryptData(uint8_t pdIndex, int is_cmd, uint8_t *rawDataReceive, int length);
int checkScbkExists();
void setNewScbk(uint8_t* newKey);
void clearSecureChannelKeys(void);
int verifyMac(uint8_t pdIndex, int is_cmd, uint8_t *frame, int length);
int verifyInitialRmac(uint8_t pdIndex, const uint8_t *rmac);

#endif // OSDP_SC_H
