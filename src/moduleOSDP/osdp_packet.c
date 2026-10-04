#include <stdint.h>
#include <string.h>
#include <stdlib.h>

#include "osdp_common.h"
#include "osdp_util.h"
#include "osdp_sc.h"
#include "../platform/serial.h"
#include "../core/osdp_event.h"
#include "../core/commissioning_control.h"
#include "../util/utils.h"


#include <stdio.h> //! remove this include when the debug printf will be removed

#define SECURE_CHANNEL_MODE 1
#define PLAIN_TEXT_MODE 0

#define AES_PAD_LEN(x) ((x + 16 - 1) & (~(16 - 1)))

#define COM_TCP 1
#define COM_RS485 0

#define CMD_MSG 1
#define REPLY_MSG 0

#define PTR_MSG_DATA(x) x+1

#define PTR_SOM_PKT(x) x+1
#define PTR_CODE_MSG_PKT(x, scb_len) x + sizeof(osdp_header_msg) + scb_len
#define PTR_DATA_MSG_PKT(x, scb_len) x + sizeof(osdp_header_msg) + scb_len + CODE_MSG_LEN

// todo amÃƒÂ©lioration possible dÃƒÂ©finir une macro identique pour les deux cas cmd et reply
#define LEN_TX_MSG(x) x - CRC_MSG_LEN
#define LEN_RX_MSG(x) x - CRC_MSG_LEN - 1  // -1 pour ne pas compter le 0xFF du dÃƒÂ©but du packet

#define LEN_TX_MSG_MAC(x) LEN_TX_MSG(x) - MAC_MSG_LEN
#define LEN_RX_MSG_MAC(x) LEN_RX_MSG(x) - MAC_MSG_LEN
#define LEN_CIPHERED_DATA(x, y) x - y - CODE_MSG_LEN - MAC_MSG_LEN - CRC_MSG_LEN


static int seq_number[4] = {-1};
static uint8_t expected_addr[4];
static int expected_addr_set[4];
static int osdpDeviceType = CONTROL_PANEL;
static int mode[4] = {PLAIN_TEXT_MODE, PLAIN_TEXT_MODE, PLAIN_TEXT_MODE, PLAIN_TEXT_MODE};


static int getSeqNumber(int seqNumber)
{
	return (seqNumber > 3) ? 1: seqNumber;
}


static void fillHeader(uint8_t addr, uint16_t len, uint8_t control, uint8_t *scb, uint8_t* msg){

	osdp_header_msg *header = (osdp_header_msg*) msg;

	header->charTime = OSDP_PKT_MARK;       /**< character time 0xFF OSDP_PKT_MARK*/
    header->som = OSDP_MSG_SOM;             /**< start of message, constante value 0x53 OSDP_PKT_SOM*/
    header->addr = addr;                    /**< RS485 reader address 0x00 - 0x7E*/
    header->lsbLen = BYTE_0(len);           /**< LSB of number of bytes contained in the message, including the SOM through the CKSUM or CRC characters. */
	header->msbLen = BYTE_1(len);           /**< MSB of number of bytes contained in the message, including the SOM through the CKSUM or CRC characters. */           
	header->ctrl = control;    				/**< Message control information */
	if(header->ctrl & OSDP_MSG_CONTROL_SCB)
	{
		memcpy(header->scb, scb,scb[0]);
	}
}


static size_t getCmdReplyDataLength(uint8_t  pdIndex, uint8_t replyCode, size_t sizeCmd) {
	int dataLength = sizeCmd - 1; // -1 -> on ne prend pas en compte l'octet du cmdCode

    switch (replyCode) {
		case CMD_CHLNG:       // REPLY_CCRYPT meme code 0x76 que CHLNG               
        case CMD_SCRYPT: 
        case REPLY_RMAC_I: 
		case REPLY_NAK: return dataLength;
		default:
			if(mode[pdIndex] == SECURE_CHANNEL_MODE){	
				return (dataLength % 16 == 0) ? dataLength : AES_PAD_LEN(dataLength);
			}
			else{
				return dataLength;
			}	
		break;
    }
}


int buildMsgPacket(uint8_t pdIndex, uint8_t addr, const void *msg2Send, size_t sizeCmd){
    if (osdpDeviceType == CONTROL_PANEL) {
        expected_addr[pdIndex] = addr;
        expected_addr_set[pdIndex] = 1;
    }

	uint16_t len = 0;
	uint8_t control=0;
	uint8_t scb[3]={0};
	uint8_t * cipheredData = NULL;
	uint8_t codeMsg;
	int is_cmd = (osdpDeviceType == CONTROL_PANEL) ? 1 : 0;
	struct osdp_event ev;

    ev.payload = msg2Send;
    ev.payload_len = sizeCmd;

    if(is_cmd) ev.direction = CP_2_PD;
	else ev.direction = PD_2_CP;

	control |= (mode[pdIndex] == SECURE_CHANNEL_MODE)? OSDP_MSG_CONTROL_SCB : 0;
	if(is_cmd)
		seq_number[pdIndex] = getSeqNumber(seq_number[pdIndex] + 1); // 0->1->2->3->1->2->3->1..
	control |= seq_number[pdIndex] & OSDP_MSG_CONTROL_SQN;
	control |= OSDP_MSG_CONTROL_CRC;

	codeMsg = ((uint8_t *) msg2Send)[0]; 

	switch(codeMsg){

		case CMD_CHLNG: //!REPLY_CCRYPT c'est le mÃƒÂªme code 0x76
			// force Security Control Block
			control |= OSDP_MSG_CONTROL_SCB;
			if(is_cmd){
				scb[0]= 0x03; scb[1] = SCS_11; scb[2] = checkScbkExists() ? 0x01 : 0x00;}
			else
				{scb[0]= 0x03; scb[1] = SCS_12; scb[2] = checkScbkExists() ? 0x01 : 0x00;}  				
		break;

		case CMD_SCRYPT:
			// force Security Control Block
			control |= OSDP_MSG_CONTROL_SCB;
			scb[0]= 0x03; scb[1] = SCS_13; scb[2] = checkScbkExists() ? 0x01 : 0x00;
		break;

		case REPLY_RMAC_I:
			// force Security Control Block
			control |= OSDP_MSG_CONTROL_SCB;
			scb[0]= 0x03; scb[1] = SCS_14; scb[2] = checkScbkExists() ? 0x01 : 0x00;
		break;

		case REPLY_NAK:
			if(mode[pdIndex] == SECURE_CHANNEL_MODE){
				scb[0]= 0x02;
				scb[1] = SCS_16; // Secure Session msg with MAC & no data encryption
			}				
		break;

		default:
			{
				if(mode[pdIndex] == SECURE_CHANNEL_MODE){
					scb[0]= 0x02;
					if(is_cmd){
						if(sizeCmd > 1) 
						{
							scb[1] = SCS_17; 
							cipheredData = encryptData(pdIndex, is_cmd, (unsigned char *) PTR_MSG_DATA(msg2Send), sizeCmd-1);//! -1 car il y a le code Cmd ds msg2Send !!!!!!!!
							cipheredData[0] = codeMsg;
						}
						else
							scb[1] = SCS_15; 
					}
					else{
						if(sizeCmd > 1) 
						{
							scb[1] = SCS_18; // Secure Session msg with MAC & data encryption
							cipheredData = encryptData(pdIndex, is_cmd, (uint8_t *) PTR_MSG_DATA(msg2Send), sizeCmd-1);//! -1 car il y a le code Reply ds msg2Send !!!!!!!!
							cipheredData[0] = codeMsg;	
						}
						else
							scb[1] = SCS_16; // Secure Session msg with MAC & no data encryption
					}
				}
			}
		break;
	}
	// Definit la longueur du paquet
	// prend en compte l'ajout du MAC ds le cas SC
	len = (mode[pdIndex] == SECURE_CHANNEL_MODE && codeMsg != CMD_CHLNG && codeMsg != CMD_SCRYPT && codeMsg != REPLY_RMAC_I)? MAC_MSG_LEN : 0; 
	// prend en compte le padding ds le cas SC
	int dataMsgLength = getCmdReplyDataLength(pdIndex, codeMsg, sizeCmd);
	// longueur totale
	len += sizeof(osdp_header_msg) + scb[0] + CODE_MSG_LEN + dataMsgLength + CRC_MSG_LEN; //0xFF included

	// pointe et raz le paquet
	uint8_t msgPacket[MAX_SIZE_MSG_PACKET] = {0};
	uint8_t * ptrPacket = (uint8_t * ) msgPacket;
	memset(ptrPacket, 0, MAX_SIZE_MSG_PACKET);	

	fillHeader(addr, len - 1, control, scb, ptrPacket); // -1 do not include 0xFF

	// copie le message ds packet, pointe la position code CMD ou REPLY en fct de la longueur scb
	if(mode[pdIndex] == PLAIN_TEXT_MODE || scb[1] == SCS_11 || scb[1] == SCS_12 || scb[1] == SCS_13 || scb[1] == SCS_14){
		memcpy(PTR_CODE_MSG_PKT(ptrPacket, scb[0]), msg2Send, sizeCmd);	
	}
	else{  

		if(scb[1] == SCS_15 || scb[1] == SCS_16)//cas Secure Session msg with MAC and no Data Security
			memcpy(PTR_CODE_MSG_PKT(ptrPacket, scb[0]), msg2Send, sizeCmd);
		else{

			//cas Secure Session msg with MAC & Data Security
			memcpy(PTR_CODE_MSG_PKT(ptrPacket, scb[0]), cipheredData, dataMsgLength + 1); // +1 car il y a le code Msg ds cipheredData

			if (cipheredData != NULL) {
                secure_zero(cipheredData, dataMsgLength + 1);
                free(cipheredData);
            }
		}
  
		computeMac(pdIndex, is_cmd, ptrPacket + 1, LEN_TX_MSG_MAC(len - 1)); // do not include 0xFF
	}

	// le CRC se calule sur le msg, ne pas compter 2 bytes (CRC) de fin de trame objet du calcul
	uint16_t crc = 0;
	if(control & OSDP_MSG_CONTROL_CRC)
	{ 
		crc = computeCrc16(ptrPacket + 1, LEN_TX_MSG(len - 1)); // do not include 0xFF also CRC

		// pointe la position pour le CRC
		(ptrPacket + len-2)[0] = BYTE_0(crc);
		(ptrPacket + len-2)[1] = BYTE_1(crc); 
	}
	else
	{	// todo !!!warning!!! LEN_TX_MSG(len - 1) est faut car prend en compte CRC_MSG_LEN qui est = 2 ÃƒÂ  corriger
		crc = computeChecksum(ptrPacket + 1, LEN_TX_MSG(len - 1)); // do not include 0xFF
		(ptrPacket + len-2)[0] = BYTE_0(crc);
	}
	
	ev.frame = ptrPacket;
    ev.frame_len = len;
	ev.is_secure = mode[pdIndex];
    ev.timestamp = get_timestamp_ns();

    int written = serial_write(ptrPacket, len);

	osdp_emit_event(&ev);

    secure_zero(msgPacket, sizeof(msgPacket));
    return written == len ? written : -1;
}


int extractMsgPacket(uint8_t pdIndex, void *buffer, size_t buffer_size)
{
    uint8_t frame[MAX_SIZE_MSG_PACKET];
    int received = serial_read(frame, sizeof(frame));
    /* Serial reads may split a frame; collect only within the operation deadline. */
    while (commissioning_active() && received > 0 && received < sizeof(frame)) {
        if (received >= 6) {
            int expected = 1 + frame[3] + ((int)frame[4] << 8);
            if (expected <= received || expected > sizeof(frame)) break;
        }
        int count = serial_read(frame + received, sizeof(frame) - received);
        if (count <= 0) { if (count < 0) received = -1; break; }
        received += count;
    }

    int result = -4;
    int is_cmd = osdpDeviceType == PERIPHERAL_DEVICE;
    struct osdp_event ev = {0};
    ev.direction = is_cmd ? CP_2_PD : PD_2_CP;
    ev.timestamp = get_timestamp_ns();
    ev.is_secure = mode[pdIndex];
    if (received <= 0) {
        osdp_emit_event(&ev);
        return received == 0 ? -1 : -7;
    }
    if (received > sizeof(frame)) return -4;
    ev.frame = frame;
    ev.frame_len = received;
    /* Check every field before reading it, including SCB and integrity trailer. */
    if (received < sizeof(osdp_header_msg) + CODE_MSG_LEN + 1) goto done;
    if (frame[0] != OSDP_PKT_MARK || frame[1] != OSDP_MSG_SOM) {
        result = -2; goto done;
    }
    osdp_header_msg *header = (osdp_header_msg *)frame;
    if (osdpDeviceType == CONTROL_PANEL &&
        (!(header->addr & 0x80) || (expected_addr_set[pdIndex] &&
          (header->addr & 0x7f) != expected_addr[pdIndex]))) goto done;
    int trailer = header->ctrl & OSDP_MSG_CONTROL_CRC ? CRC_MSG_LEN : 1;
    int length = header->lsbLen | ((int)header->msbLen << 8);
    int offset = sizeof(osdp_header_msg);
    if (length != received - 1 || received < offset + CODE_MSG_LEN + trailer) goto done;
    if (trailer == CRC_MSG_LEN) {
        uint16_t crc = computeCrc16(frame + 1, received - 1 - trailer);
        if (frame[received - 2] != BYTE_0(crc) || frame[received - 1] != BYTE_1(crc)) {
            result = -3; goto done;
        }
    } else if (frame[received - 1] != computeChecksum(frame + 1, received - 2)) {
        result = -3; goto done;
    }
    if (!is_cmd && osdpDeviceType == CONTROL_PANEL && seq_number[pdIndex] >= 0 &&
        (header->ctrl & OSDP_MSG_CONTROL_SQN) != seq_number[pdIndex]) {
        result = -5; goto done;
    }
    int payload_length = received - offset - trailer;
    if (header->ctrl & OSDP_MSG_CONTROL_SCB) {
        if (payload_length < 3) goto done;
        int scb_length = header->scb[0];
        int type = header->scb[1];
        if (scb_length < 2 || scb_length >= payload_length) goto done;
        offset += scb_length;
        payload_length -= scb_length;
        if (type >= SCS_15 && type <= SCS_18) {
            if (scb_length != 2 || mode[pdIndex] != SECURE_CHANNEL_MODE ||
                (is_cmd && type != SCS_15 && type != SCS_17) ||
                (!is_cmd && type != SCS_16 && type != SCS_18) ||
                payload_length < CODE_MSG_LEN + MAC_MSG_LEN) goto done;
            payload_length -= MAC_MSG_LEN;
            if (!verifyMac(pdIndex, is_cmd, frame + 1, received - 1 - trailer - MAC_MSG_LEN)) {
                result = -6; goto done;
            }
            if (type == SCS_17 || type == SCS_18) {
                int decrypted = decryptData(pdIndex, is_cmd, frame + offset + CODE_MSG_LEN,
                                            payload_length - CODE_MSG_LEN);
                if (decrypted < 0) goto done;
                payload_length = decrypted + CODE_MSG_LEN;
            }
        } else {
            if (mode[pdIndex] == SECURE_CHANNEL_MODE || scb_length != 3) goto done;
            int expected = 0;
            uint8_t expected_code = 0;
            switch (type) {
                case SCS_11: expected = CMD_CHLNG_DATA_LEN; expected_code = CMD_CHLNG; break;
                case SCS_12: expected = REPLY_CCRYPT_DATA_LEN; expected_code = REPLY_CCRYPT; break;
                case SCS_13: expected = CMD_SCRYPT_DATA_LEN; expected_code = CMD_SCRYPT; break;
                case SCS_14: expected = CMD_SCRYPT_DATA_LEN; expected_code = REPLY_RMAC_I; break;
                default: goto done;
            }
            if (payload_length != expected + CODE_MSG_LEN || frame[offset] != expected_code ||
                (type == SCS_14 && header->scb[2] != 1) ||
                (is_cmd && type != SCS_11 && type != SCS_13) ||
                (!is_cmd && type != SCS_12 && type != SCS_14)) goto done;
        }
    } else if (mode[pdIndex] == SECURE_CHANNEL_MODE) {
        /* An unauthenticated ACK/NAK cannot establish a KEYSET outcome. */
        result = -6; goto done;
    }
    if (payload_length < CODE_MSG_LEN || payload_length > buffer_size) goto done;
    if (is_cmd) seq_number[pdIndex] = header->ctrl & OSDP_MSG_CONTROL_SQN;
    memcpy(buffer, frame + offset, payload_length);
    ev.payload = buffer;
    ev.payload_len = payload_length;
    result = payload_length;
done:
    osdp_emit_event(&ev);
    secure_zero(frame, sizeof(frame));
    return result;
}


int decryptMsgPacket(uint8_t pdIndex, void * buffer, size_t buffer_size/*, void * msg, size_t lenMsg*/){
	//uint8_t buf[MAX_SIZE_MSG_PACKET];
    unsigned char * frame;
	int received = buffer_size;
	size_t data_size = 0;
	int is_cmd = 0;
	struct osdp_event ev;

	if(received > 0)
	{
		frame = buffer;
		ev.frame = (uint8_t *) malloc(received);
		memcpy(ev.frame, buffer, received);
		ev.frame_len = received;
		ev.timestamp = get_timestamp_ns();
		ev.is_secure = mode[pdIndex];
	}
	else 
		return -1;

	is_cmd = (frame[2] & 0x80) ? 0 : 1; // bit 7 of addr field is set for reply from PD to CP

    if(is_cmd) ev.direction = CP_2_PD;
	else ev.direction = PD_2_CP;


	//VÃƒÂ©rifica existence du 0xFF en tÃƒÂªte de packet
	if (frame[0] != OSDP_PKT_MARK) return -2;

	// le CRC se calule sur le msg ÃƒÂ  partir du SOM et sans les 2 bytes (CRC) de fin de trame objet du calcul
	if(frame[5] & OSDP_MSG_CONTROL_CRC)
	{	
		uint16_t crc = computeCrc16(PTR_SOM_PKT(frame),LEN_RX_MSG(received));
		if(frame[received - 2] != BYTE_0(crc) || frame[received - 1] != BYTE_1(crc)) return -3;	
	}
	else
	{	
		uint16_t chksum = computeChecksum(PTR_SOM_PKT(frame),LEN_RX_MSG(received + 1 ));
		if(frame[received - 1] != chksum) return -3;
	}

	// traitement du header
	osdp_header_msg * header = (osdp_header_msg *) frame;
	int offset = sizeof(osdp_header_msg) ; // le OSDP_PKT_MARK en dÃƒÂ©but de packet est inclu

	// Assemble lsb et msb, verifie longueur trame attendue avec nb caractÃƒÂ¨re lus, -1 car on ne compte pas OSDP_PKT_MARK
    uint16_t lenFrame = ((uint16_t)header->msbLen << 8) | (uint16_t)header->lsbLen;
	if(lenFrame != received - 1)  return -4;

	// VÃƒÂ©rifier le SQN
	//! Comment traiter le SQN dans le cas d'un PD/CP
	uint8_t sqnTrame = header->ctrl & OSDP_MSG_CONTROL_SQN;

	if(osdpDeviceType == PERIPHERAL_DEVICE) {
		seq_number[pdIndex] = sqnTrame;
		if (((header->ctrl & OSDP_MSG_CONTROL_SCB) == 0) && (mode[pdIndex] == SECURE_CHANNEL_MODE))
			mode[pdIndex] = PLAIN_TEXT_MODE; 
	}

	// ! Comment traiter le SQN dans le cas d'un PD/CP
	// if((osdpDeviceType == CONTROL_PANEL) && (sqnTrame != seq_number)) return -5;
	// else if((osdpDeviceType == PERIPHERAL_DEVICE) && (sqnTrame != ++seq_number)) return -5;

	if( (header->ctrl & OSDP_MSG_CONTROL_SCB) !=0) {//cas Secure Channel
		offset += header->scb[0]; //ajoute la longueur du SCB

		if(header->scb[1] == SCS_15 || header->scb[1] == SCS_16 || header->scb[1] == SCS_17 || header->scb[1] == SCS_18 )
		{		
			// calul du MAC de la trame recue
			computeMac(pdIndex, is_cmd, PTR_SOM_PKT(frame), LEN_RX_MSG_MAC(received));

			//todo vÃƒÂ©rification des 4 premiers octets du MAC ÃƒÂ  faire
			
			if(header->scb[1] == SCS_17 || header->scb[1] == SCS_18 )
			{
				data_size = decryptData(pdIndex, is_cmd, PTR_DATA_MSG_PKT(frame,header->scb[0]), LEN_CIPHERED_DATA(received, offset)) + CODE_MSG_LEN;
			}		
			// cas cmd ou reply  avec MAC sans data chiffree (cas du POll, Ack sans data, cas du NAK avec data non chiffree )
			else data_size =  received - sizeof(osdp_header_msg) - header->scb[0] - MAC_MSG_LEN - CRC_MSG_LEN; //todo traiter le cas checksum
		}
		else if(header->scb[1] == SCS_11){
			data_size = CMD_CHLNG_DATA_LEN + CODE_MSG_LEN;
		}
		else if(header->scb[1] == SCS_12){
			data_size = REPLY_CCRYPT_DATA_LEN + CODE_MSG_LEN;
		}
		else if(header->scb[1] == SCS_13 || header->scb[1] == SCS_14){
			data_size = CMD_SCRYPT_DATA_LEN + CODE_MSG_LEN;
		}
	}
	else{//cas plain text
		data_size = received - sizeof(osdp_header_msg) - CRC_MSG_LEN; //todo traiter le cas checksum
	}

	// memcpy(buffer, frame + offset, data_size);

	ev.payload = frame + offset;
    ev.payload_len = data_size;
	osdp_emit_event(&ev);
	
	free(ev.frame);

    return data_size;
}	


void osdpPacketInit(uint8_t pdIndex, uint8_t deviceType){
	mode[pdIndex] = PLAIN_TEXT_MODE;
    expected_addr_set[pdIndex] = 0;
	if(deviceType == CONTROL_PANEL){
		osdpDeviceType = CONTROL_PANEL;
		seq_number[pdIndex] = -1;
	}

//#ifdef ENV_WINDOWS
	else if(deviceType == PERIPHERAL_DEVICE){
		osdpDeviceType = PERIPHERAL_DEVICE;
		seq_number[pdIndex] = 0;
	}
//#endif

	else if(deviceType == SNIFFER_FRAME){
		osdpDeviceType = SNIFFER_FRAME;
		seq_number[pdIndex] = 0;
	}

}


void setSecureChannelModePacket(uint8_t pdIndex){
	
	mode[pdIndex] = SECURE_CHANNEL_MODE;
}


void setPlainTextModePacket(uint8_t pdIndex){
	
	mode[pdIndex] = PLAIN_TEXT_MODE;
}
