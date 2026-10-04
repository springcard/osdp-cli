#ifndef OSDP_CP_H
#define OSDP_CP_H

#include <stdint.h>
#include <stdbool.h>
#include "osdp_common.h"

void osdpCpInit(uint8_t indReader, uint8_t argTypeCom);
int getReply(uint8_t pdIndex, uint8_t *msg_code, void *buffer, size_t buffer_size);
bool openSecureChannelSession2(uint8_t pdIndex, uint8_t addr, int retries);
void Command_OSDP_COMSET(uint8_t pdIndex, const uint8_t addr, uint8_t new_addr, uint32_t new_baudrate);
void Command_OSDP_ID(uint8_t pdIndex, const uint8_t pdAddr);
int Command_OSDP_CHLNG(uint8_t pdIndex, const uint8_t pdAddr);
int Command_OSDP_SCRYPT(uint8_t pdIndex, const uint8_t pdAddr);

/* Internal outcomes; these values are independent of process exit codes. */
enum osdp_session_result {
    OSDP_SESSION_OK,
    OSDP_SESSION_NO_RESPONSE,
    OSDP_SESSION_COMMUNICATION_ERROR,
    OSDP_SESSION_AUTHENTICATION_FAILED,
    OSDP_SESSION_UNSUPPORTED_DEVICE,
    OSDP_SESSION_ENTROPY_ERROR
};
enum osdp_session_result osdp_open_secure_channel(uint8_t pdIndex, uint8_t addr, int retries);
void Command_OSDP_BUZ(uint8_t pdIndex, const uint8_t pdAddr, const uint8_t readerNumber, const uint8_t toneCode, const uint8_t onTime, const uint8_t offTime, const uint8_t numberOfCycles);
void Command_OSDP_LED(uint8_t pdIndex, const uint8_t pdAddr, const uint8_t readerNumber, const uint8_t ledNumber, tmp_LED_Settings tmpParam, perm_LED_Settings permParam);
void Command_OSDP_POLL(uint8_t pdIndex, const uint8_t pdAddr);
void Command_OSDP_CAP(uint8_t pdIndex, const uint8_t addr);
void Command_OSDP_KEYSET(uint8_t pdIndex, const uint8_t addr, uint8_t keyType,uint8_t keyLen, uint8_t* keyData);
void Command_OSDP_LSTAT(uint8_t pdIndex, const uint8_t addr);
void Command_OSDP_OUT(uint8_t pdIndex, const uint8_t addr, const uint8_t output, const uint8_t ctrlCode, const uint8_t lsbTimer, const uint8_t msbTimer);
void Command_OSDP_ISTAT(uint8_t pdIndex, const uint8_t addr);
void Command_OSDP_OSTAT(uint8_t pdIndex, const uint8_t addr);
void Command_OSDP_BIOREAD(uint8_t pdIndex, const uint8_t addr, uint8_t readerNber, uint8_t bioType, uint8_t bioFormat, uint8_t bioQuality);
void Command_OSDP_BIOMATCH(uint8_t pdIndex, const uint8_t addr, uint8_t readerNber, uint8_t bioType, uint8_t bioFormat, uint8_t bioQuality, uint16_t bioLength, uint8_t * bioData);
void Command_OSDP_TEXT(uint8_t pdIndex, const uint8_t addr, osdp_TEXT *textParam);
void Command_OSDP_FILETRANSFER(uint8_t pdIndex, const uint8_t addr, uint8_t ftType, uint32_t ftSize, uint32_t ftOffset, uint16_t ftFragmentSize, uint8_t *pftData);
void Command_OSDP_raw(uint8_t pdIndex, const uint8_t addr, uint8_t msgCode, uint8_t *rawData, size_t dataSize);

#endif // OSDP_CP_H
