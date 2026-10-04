#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "osdp_common.h"
#include "osdp_packet.h"
#include "osdp_util.h"
#include "../util/utils.h"

#ifdef ENV_FREERTOS
    #include "dbgWrapper.h"
#endif


// module osdp_packet.c
#ifdef ENV_WINDOWS
    void setSecureChannelModePacket(uint8_t pdIndex);
    void setPlainTextModePacket(uint8_t pdIndex);
#endif

// module osdp_sc.c
void setRndA(uint8_t pdIndex, uint8_t* rendA);
uint8_t * getServerCryptogram(uint8_t pdIndex);

     
size_t buildMsg(uint8_t iLecteur, uint8_t addr, uint8_t msgCode, const void *data, size_t sizeData){
    // cmd code + size data
    size_t size = 1 + sizeData;
    osdp_cmd cmd = {.cmdId = msgCode};
    osdp_cmd *ptrCmd = &cmd;
    //todo a revoir, on peut regrouper les cases du type memcpy(&cmd.xxxxx, data, sizeData) dans un switch case, 
    //todo et faire un seul appel à memcpy(&cmd +1, data, size) à la fin du switch case, pour éviter de faire plusieurs appels à memcpy
	switch(msgCode){
        case CMD_MFG:
            memcpy(&cmd.mfg, data, sizeData);
        break;

        case CMD_FILETRANSFER:
            ptrCmd = (osdp_cmd *) malloc(1 + sizeof(osdp_FILETRANSFER) + sizeData);
            ptrCmd->cmdId = msgCode;
            memcpy(&ptrCmd->fileTransfer, data, sizeData);  
		break;

        case CMD_TEXT:
            memcpy(&cmd.text, data, sizeData);            
		break;

        case CMD_COMSET:
            memcpy(&cmd.comSet, data, sizeData);            
		break;

        case CMD_BIOMATCH:
            memcpy(&cmd.bioMatch, data, sizeData);
		break;

        case CMD_BIOREAD:
            memcpy(&cmd.bioRead, data, sizeData);
        break;

        case CMD_OUT:
            memcpy(&cmd.out, data, sizeData);
		break;

        case CMD_KEYSET:
            memcpy(&cmd.key, data, sizeData);            
		break;

		case CMD_CAP:
            memcpy(cmd.cap.data, data, sizeData);
		break;

        case CMD_OSTAT: // rien a faire ici
        case CMD_ISTAT: // cmd sans data
        case CMD_LSTAT:
		case CMD_POLL:
		break;  

		case CMD_LED:
            memcpy(&cmd.led, data, sizeData);// ou sizeof(struct osdp_BUZ)
		break;   

		case CMD_BUZ:
            memcpy(&cmd.buz, data, sizeData);// ou sizeof(struct osdp_BUZ)
		break;       

		case CMD_ID:
            memcpy(cmd.id.data, data, sizeData);
		break;

		case CMD_CHLNG:
            // cas particulier CHLNG car sizeData est null dans l'appel
            size = 1 + CMD_CHLNG_DATA_LEN; 
            if (!getRandomValue(cmd.chlng.roundA, 8)) {
                secure_zero(&cmd, sizeof(cmd));
                return 0;
            }
            // uint8_t val [] = {1,1,1,1,1,1,1,1};
            // memcpy(cmd.chlng.roundA, &val[0], 8);
            setRndA(iLecteur, cmd.chlng.roundA);
		break;
	
		case CMD_SCRYPT:
            // cas particulier SCRYPT car sizeData est null dans l'appel
            size = 1 + CMD_SCRYPT_DATA_LEN;
            memcpy(cmd.scrypt.srvCryptogram, getServerCryptogram(iLecteur), 16);
		break;
	}

    size = buildMsgPacket(iLecteur, addr, ptrCmd, size);
    if (msgCode == CMD_KEYSET || msgCode == CMD_CHLNG || msgCode == CMD_SCRYPT)
        secure_zero(&cmd, sizeof(cmd));

	return size;
}


int extractReply(uint8_t pdIndex, uint8_t *replyCode, void *replyData, size_t capacity){
    unsigned char replyMsg[MAX_SIZE_MSG_PACKET];

    int size = extractMsgPacket(pdIndex, replyMsg, sizeof(replyMsg)/*, msgReceived, sizeData*/);

    if (size <= 0) {
        secure_zero(replyMsg, sizeof(replyMsg));
        return size;
    }

    size_t data_size = size - 1;
    if (data_size > capacity) {
        secure_zero(replyMsg, sizeof(replyMsg));
        return -12;
    }

    memcpy(replyData, &replyMsg[1], data_size);
    *replyCode = replyMsg[0];
    secure_zero(replyMsg, sizeof(replyMsg));

    return data_size;	
}


#ifdef ENV_WINDOWS

void setSecureChannelMode(uint8_t pdIndex)
{
    setSecureChannelModePacket(pdIndex);
}


void setPlainTextMode(uint8_t pdIndex)
{
    setPlainTextModePacket(pdIndex);
}

#endif
