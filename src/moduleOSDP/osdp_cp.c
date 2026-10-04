#include <stdlib.h>
#include <string.h>
#include <stdio.h>

#include "osdp_common.h"
#include "osdp_cp.h"
#include "osdp_util.h"
#include "osdp_cmd.h"
#include "osdp_sc.h"

#include "../platform/serial.h"
#include "../util/utils.h"


void Command_OSDP_BIOMATCH(uint8_t pdIndex, const uint8_t addr, uint8_t readerNber, uint8_t bioType, uint8_t bioFormat, uint8_t bioQuality, uint16_t bioLength, uint8_t * bioData)
{
	osdp_BIOMATCH *data = (osdp_BIOMATCH *) malloc(sizeof(osdp_BIOMATCH) + bioLength);

	data->readerNber = readerNber;
	data->bioType = bioType;
	data->bioFormat = bioFormat;
	data->bioQuality = bioQuality;
	data->bioLength = bioLength;
	memcpy(data->bioData, bioData, bioLength);
					
	size_t len = buildMsg(pdIndex, addr, CMD_BIOMATCH, data, sizeof(osdp_BIOMATCH) + bioLength);
	//todo free!!!! free(data);
}


void Command_OSDP_BIOREAD(uint8_t pdIndex, const uint8_t addr, uint8_t readerNber, uint8_t bioType, uint8_t bioFormat, uint8_t bioQuality)
{
	osdp_BIOREAD data;

	data.readerNber = readerNber;
	data.bioType = bioType;
	data.bioFormat = bioFormat;
	data.bioQuality = bioQuality;

	size_t len = buildMsg(pdIndex, addr, CMD_BIOREAD, &data, sizeof(osdp_BIOREAD));
}


void Command_OSDP_OSTAT(uint8_t pdIndex, const uint8_t addr)
{
	size_t len = buildMsg(pdIndex, addr, CMD_OSTAT, NULL, 0);
}


void Command_OSDP_ISTAT(uint8_t pdIndex, const uint8_t addr)
{
	size_t len = buildMsg(pdIndex, addr, CMD_ISTAT, NULL, 0);
}


void Command_OSDP_OUT(uint8_t pdIndex, const uint8_t addr, const uint8_t output, const uint8_t ctrlCode, const uint8_t lsbTimer, const uint8_t msbTimer)
{
	osdp_OUT data;

	data.outputNber = output;
	data.ctrlCode = ctrlCode;
	data.lsbTimer = lsbTimer;
	data.msbTimer = msbTimer;

	size_t len = buildMsg(pdIndex, addr, CMD_OUT, &data, sizeof(osdp_OUT));
}


void Command_OSDP_LSTAT(uint8_t pdIndex, const uint8_t addr)
{
	size_t len = buildMsg(pdIndex, addr, CMD_LSTAT, NULL, 0);
}


void Command_OSDP_CAP(uint8_t pdIndex, const uint8_t addr)
{
	uint8_t data = 0;

	size_t len = buildMsg(pdIndex, addr, CMD_CAP, &data, sizeof(data));
}


void Command_OSDP_POLL(uint8_t pdIndex, const uint8_t addr)
{	
	size_t len = buildMsg(pdIndex, addr, CMD_POLL, NULL, 0);
}


void Command_OSDP_LED(uint8_t pdIndex, const uint8_t addr, const uint8_t readerNumber, const uint8_t ledNumber, tmp_LED_Settings tmpParam, perm_LED_Settings permParam)
{
	osdp_LED data;

	data.readerNumber = readerNumber;
	data.ledNumber = ledNumber;
	memcpy(&data.tmpSettings, &tmpParam, sizeof(tmp_LED_Settings));
	memcpy(&data.permSettings, &permParam, sizeof(perm_LED_Settings));

	size_t len = buildMsg(pdIndex, addr, CMD_LED, &data, sizeof(data));	
}


void Command_OSDP_BUZ(uint8_t pdIndex, const uint8_t addr, const uint8_t readerNumber, const uint8_t toneCode, const uint8_t onTime, const uint8_t offTime, const uint8_t numberOfCycles)
{
	osdp_BUZ data;

	data.readerNumber = readerNumber;
	data.toneCode = toneCode;
	data.onTime = onTime;
	data.offTime = offTime;
	data.numberOfCycles = numberOfCycles;

	size_t len = buildMsg(pdIndex, addr, CMD_BUZ, &data, sizeof(data));	
}


void Command_OSDP_ID(uint8_t pdIndex, const uint8_t addr)
{
	uint8_t data = 0;

	size_t len = buildMsg(pdIndex, addr, CMD_ID, &data, sizeof(data));
}


int Command_OSDP_CHLNG(uint8_t pdIndex, const uint8_t addr)
{
    return (int)buildMsg(pdIndex, addr, CMD_CHLNG, NULL, 0);
}


int Command_OSDP_SCRYPT(uint8_t pdIndex, const uint8_t addr)
{	
    return (int)buildMsg(pdIndex, addr, CMD_SCRYPT, NULL, 0);
}


void Command_OSDP_KEYSET(uint8_t pdIndex, const uint8_t addr, uint8_t keyType,uint8_t keyLen, uint8_t* keyData)
{
	osdp_KEYSET data;

	data.keyType = keyType;
	data.keyLen = keyLen;
	memcpy(data.keyData, keyData, keyLen);

	size_t len = buildMsg(pdIndex, addr, CMD_KEYSET, &data, sizeof(data));
	secure_zero(&data, sizeof(data));
}

                     
void Command_OSDP_COMSET(uint8_t pdIndex, const uint8_t addr, uint8_t new_addr, uint32_t new_baudrate)
{
	osdp_COMSET data = {0};

	data.addr = new_addr;
	data.baudrate = new_baudrate;

	size_t len = buildMsg(pdIndex, addr, CMD_COMSET, &data, sizeof(data));
}


void Command_OSDP_TEXT(uint8_t pdIndex, const uint8_t addr, osdp_TEXT *textParam){

	size_t len = buildMsg(pdIndex, addr, CMD_TEXT, textParam, sizeof(osdp_TEXT) + textParam->textLength);
}


void Command_OSDP_FILETRANSFER(uint8_t pdIndex, const uint8_t addr, uint8_t ftType, uint32_t ftSize, uint32_t ftOffset, uint16_t ftFragmentSize, uint8_t *pftData){
	osdp_FILETRANSFER *data = (osdp_FILETRANSFER *) malloc(sizeof(osdp_FILETRANSFER) + ftFragmentSize);

	data->ftType = ftType;
	data->ftSize[0] = BYTE_0(ftSize);
	data->ftSize[1] = BYTE_1(ftSize);
	data->ftSize[2] = BYTE_2(ftSize);
	data->ftSize[3] = BYTE_3(ftSize);

	data->ftOffset[0] = BYTE_0(ftOffset);
	data->ftOffset[1] = BYTE_1(ftOffset);
	data->ftOffset[2] = BYTE_2(ftOffset);
	data->ftOffset[3] = BYTE_3(ftOffset);

	data->ftFragmentSize[0] = BYTE_0(ftFragmentSize);
	data->ftFragmentSize[1] = BYTE_1(ftFragmentSize);

	if(ftFragmentSize > 0)
		memcpy(data->ftData, pftData, ftFragmentSize);

	size_t len = buildMsg(pdIndex, addr, CMD_FILETRANSFER, data, sizeof(osdp_FILETRANSFER) + ftFragmentSize);

	free(data);
}


void Command_OSDP_raw(uint8_t pdIndex, const uint8_t addr, uint8_t msgCode, uint8_t *rawData, size_t dataSize){

	size_t len = buildMsg(pdIndex, addr, msgCode, rawData, dataSize);
}


int getReply(uint8_t pdIndex, uint8_t *msg_code, void *buffer, size_t buffer_size)
{
    *msg_code = 0;
    return extractReply(pdIndex, msg_code, buffer, buffer_size);
}

static enum osdp_session_result session_reply_error(int size, uint8_t code, const uint8_t *reply)
{
    if (size == -1) return OSDP_SESSION_NO_RESPONSE;
    if (size == sizeof(osdp_NAK) && code == REPLY_NAK &&
        (reply[0] == 3 || reply[0] == 5)) return OSDP_SESSION_UNSUPPORTED_DEVICE;
    return OSDP_SESSION_COMMUNICATION_ERROR;
}

enum osdp_session_result osdp_open_secure_channel(uint8_t pdIndex, uint8_t addr, int retries)
{
    uint8_t reply[MAX_SIZE_MSG_PACKET] = {0};
    uint8_t code = 0;
    enum osdp_session_result result = OSDP_SESSION_NO_RESPONSE;
    if (retries < 0) return OSDP_SESSION_COMMUNICATION_ERROR;
    int attempts = retries > 0 ? retries : 1;
    setPlainTextMode(pdIndex);
    for (int attempt = 0; attempt < attempts; ++attempt) {
        int sent = Command_OSDP_CHLNG(pdIndex, addr);
        if (sent <= 0) {
            result = sent == 0 ? OSDP_SESSION_ENTROPY_ERROR : OSDP_SESSION_COMMUNICATION_ERROR;
            break;
        }
        int size = getReply(pdIndex, &code, reply, sizeof(reply));
        if (size != sizeof(osdp_CCRYPT) || code != REPLY_CCRYPT) {
            enum osdp_session_result error = session_reply_error(size, code, reply);
            if (error != OSDP_SESSION_NO_RESPONSE) result = error;
            if (error == OSDP_SESSION_UNSUPPORTED_DEVICE) break;
            continue;
        }
        osdp_CCRYPT *ccrypt = (osdp_CCRYPT *)reply;
        computeSecureSessionParametersOfCP(pdIndex, ccrypt->roundB,
                                          ccrypt->clientId, ccrypt->cliCryptogram);
        if (verifyCryptogram(pdIndex, ccrypt->cliCryptogram, CONTROL_PANEL) != 0) {
            result = OSDP_SESSION_AUTHENTICATION_FAILED;
            break;
        }
        if (Command_OSDP_SCRYPT(pdIndex, addr) <= 0) {
            result = OSDP_SESSION_COMMUNICATION_ERROR;
            break;
        }
        size = getReply(pdIndex, &code, reply, sizeof(reply));
        if (size != sizeof(osdp_RMAC_I) || code != REPLY_RMAC_I) {
            enum osdp_session_result error = session_reply_error(size, code, reply);
            if (error != OSDP_SESSION_NO_RESPONSE) result = error;
            if (error == OSDP_SESSION_UNSUPPORTED_DEVICE) break;
            continue;
        }
        osdp_RMAC_I *rmac = (osdp_RMAC_I *)reply;
        if (!verifyInitialRmac(pdIndex, rmac->rmac_i)) {
            result = OSDP_SESSION_AUTHENTICATION_FAILED;
            break;
        }
        setInitialRmac(pdIndex, rmac->rmac_i);
        setSecureChannelMode(pdIndex);
        result = OSDP_SESSION_OK;
        break;
    }
    secure_zero(reply, sizeof(reply));
    return result;
}

bool openSecureChannelSession2(uint8_t pdIndex, uint8_t addr, int retries)
{
    return osdp_open_secure_channel(pdIndex, addr, retries) == OSDP_SESSION_OK;
}
