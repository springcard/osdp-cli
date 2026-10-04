#ifndef _OSDP_COMMON_H_
#define _OSDP_COMMON_H_

#include <stdint.h>

#define ENV_WINDOWS // Ou : #define ENV_FREERTOS

#define SNIFFER_FRAME     2
#define PERIPHERAL_DEVICE 1
#define CONTROL_PANEL     0

#define MAX_SIZE_MSG_PACKET 1440 // modifier pour le FILETRANSFER en mode SC

#define BYTE_0(x) (uint8_t)(((x) >> 0) & 0xFF)
#define BYTE_1(x) (uint8_t)(((x) >> 8) & 0xFF)
#define BYTE_2(x) (uint8_t)(((x) >> 16) & 0xFF)
#define BYTE_3(x) (uint8_t)(((x) >> 24) & 0xFF)

#define OSDP_PKT_MARK                  0xFF
#define OSDP_MSG_SOM                   0x53
#define OSDP_MSG_CONTROL_SQN           0x03
#define OSDP_MSG_CONTROL_CRC           0x04
#define OSDP_MSG_CONTROL_SCB           0x08

#define CRC_MSG_LEN        2 // CRC 2 bytes
#define CODE_MSG_LEN       1 // Code command or reply 1 byte
#define MAC_MSG_LEN        4

/**
 *  OSDP reserved commands
 */
#define CMD_INVALID      0x00
#define CMD_POLL	     0x60
#define CMD_ID		     0x61
#define CMD_CAP		     0x62
#define CMD_LSTAT	     0x64
#define CMD_ISTAT	     0x65
#define CMD_OSTAT	     0x66
#define CMD_RSTAT	     0x67
#define CMD_OUT		     0x68
#define CMD_LED		     0x69
#define CMD_BUZ		     0x6A
#define CMD_TEXT	     0x6B
#define CMD_RMODE	     0x6C
#define CMD_TDSET	     0x6D
#define CMD_COMSET	     0x6E
#define CMD_BIOREAD	     0x73
#define CMD_BIOMATCH	 0x74
#define CMD_KEYSET	     0x75
#define CMD_CHLNG	     0x76
#define CMD_SCRYPT	     0x77
#define CMD_ACURXSIZE	 0x7B
#define CMD_FILETRANSFER 0x7C
#define CMD_MFG		     0x80
#define CMD_XWR		     0xA1
#define CMD_ABORT	     0xA2
#define CMD_PIVDATA	     0xA3
#define CMD_GENAUTH	     0xA4
#define CMD_CRAUTH	     0xA5
#define CMD_KEEPACTIVE	 0xA7


#define CMD_POLL_DATA_LEN              0
#define CMD_LSTAT_DATA_LEN             0
#define CMD_ISTAT_DATA_LEN             0
#define CMD_OSTAT_DATA_LEN             0
#define CMD_RSTAT_DATA_LEN             0
#define CMD_ID_DATA_LEN                1
#define CMD_CAP_DATA_LEN               1
#define CMD_OUT_DATA_LEN               4
#define CMD_LED_DATA_LEN               14
#define CMD_BUZ_DATA_LEN               5
#define CMD_TEXT_DATA_LEN              6   /* variable length command */
#define CMD_COMSET_DATA_LEN            5
#define CMD_KEYSET_DATA_LEN            18
#define CMD_CHLNG_DATA_LEN             8
#define CMD_SCRYPT_DATA_LEN            16
#define CMD_ABORT_DATA_LEN             0
#define CMD_ACURXSIZE_DATA_LEN         2
#define CMD_KEEPACTIVE_DATA_LEN        2
#define CMD_MFG_DATA_LEN               3  /* variable length command */
#define CMD_BIOREAD_DATA_LEN	       4
#define CMD_BIOMATCH_DATA_LEN	       6  /* variable length command */
#define CMD_FILETRANSFER_DATA_LEN      11 /* variable length command */


/**
 * OSDP reserved responses
 */
#define REPLY_INVALID   0x00
#define REPLY_ACK	    0x40
#define REPLY_NAK	    0x41
#define REPLY_PDID	    0x45
#define REPLY_PDCAP	    0x46
#define REPLY_LSTATR	0x48
#define REPLY_ISTATR	0x49
#define REPLY_OSTATR	0x4A
#define REPLY_RSTATR	0x4B
#define REPLY_RAW	    0x50
#define REPLY_FMT	    0x51
#define REPLY_KEYPAD	0x53
#define REPLY_COM	    0x54
#define REPLY_BIOREADR	0x57
#define REPLY_BIOMATCHR 0x58
#define REPLY_CCRYPT	0x76
#define REPLY_RMAC_I	0x78
#define REPLY_BUSY	    0x79
#define REPLY_FTSTAT	0x7A
#define REPLY_PIVDATAR	0x80
#define REPLY_GENAUTHR	0x81
#define REPLY_CRAUTHR	0x82
#define REPLY_MFGSTATR	0x83
#define REPLY_MFGERRR	0x84
#define REPLY_MFGREP	0x90
#define REPLY_XRD	    0xB1


#define REPLY_ACK_DATA_LEN                  0 
#define REPLY_BUSY_DATA_LEN	    			0
#define REPLY_PDID_DATA_LEN                 12  
#define REPLY_PDCAP_DATA_LEN                0   /* variable length command */
#define REPLY_PDCAP_ENTITY_DATA_LEN         3
#define REPLY_LSTATR_DATA_LEN               2 
#define REPLY_ISTATR_DATA_LEN				1
#define REPLY_OSTATR_DATA_LEN				1
#define REPLY_RSTATR_DATA_LEN               2
#define REPLY_COM_DATA_LEN                  5
#define REPLY_NAK_DATA_LEN                  1 
#define REPLY_CCRYPT_DATA_LEN               32 
#define REPLY_RMAC_I_DATA_LEN               16 
#define REPLY_KEYPAD_DATA_LEN               2
#define REPLY_RAW_DATA_LEN                  4
#define REPLY_FMT_DATA_LEN                  3
#define REPLY_MFGREP_DATA_LEN               3 /* variable length command */
#define REPLY_FTSTAT_DATA_LEN               7


/**
 * secure block types
 */
#define SCS_11 0x11 /* CP -> PD -- CMD_CHLNG */
#define SCS_12 0x12 /* PD -> CP -- REPLY_CCRYPT */
#define SCS_13 0x13 /* CP -> PD -- CMD_SCRYPT */
#define SCS_14 0x14 /* PD -> CP -- REPLY_RMAC_I */

#define SCS_15 0x15 /* CP -> PD -- packets w MAC w/o ENC */
#define SCS_16 0x16 /* PD -> CP -- packets w MAC w/o ENC */
#define SCS_17 0x17 /* CP -> PD -- packets w MAC w ENC*/
#define SCS_18 0x18 /* PD -> CP -- packets w MAC w ENC*/


typedef struct {

	uint8_t charTime;       /**< character time 0xFF */
    uint8_t som;            /**< start of message, constante value 0x53 */
    uint8_t addr;           /**< RS485 reader address 0x00 - 0x7E*/
    uint8_t lsbLen;         /**< LSB of number of bytes contained in the message, including the SOM through the CKSUM or CRC characters. */
	uint8_t msbLen;         /**< MSB of number of bytes contained in the message, including the SOM through the CKSUM or CRC characters. */           
	uint8_t ctrl;           /**< Message control information */
    uint8_t scb[];          /**< Security Control Block */
} osdp_header_msg;


typedef struct {
	uint8_t scbk[16];
	uint8_t s_enc[16];
	uint8_t s_mac1[16];
	uint8_t s_mac2[16];
	uint8_t r_mac[16];
	uint8_t r_mac_oneCompl[16];
	uint8_t c_mac[16];
	uint8_t c_mac_oneCompl[16];
	uint8_t cp_random[8];
	uint8_t pd_random[8];
	uint8_t pd_client_uid[8];
	uint8_t cp_cryptogram[16];
	uint8_t pd_cryptogram[16];
} osdp_secure_channel;

#pragma pack(1)

typedef struct {
	uint8_t addr;       
    uint32_t baudrate;
}osdp_COMSET;

typedef struct {
	uint8_t readerNber;
	uint8_t bioType;
	uint8_t bioFormat;
	uint8_t bioQuality;
	uint16_t bioLength;
	uint8_t bioData[];
} osdp_BIOMATCH;
#pragma pack()

typedef struct {
	uint8_t vendorCode1st;		// vendor Code 1st
	uint8_t vendorCode2nd; 		// vendor Code 2nd
	uint8_t vendorCode3rd;		// vendor Code 3rd
	uint8_t mfgData[];			// manufacturer cmd data
} osdp_MFG;

typedef struct {
	uint8_t ftType;				// file transfer type
	uint8_t ftSize[4]; 			// file size in bytes, LSB first
	uint8_t ftOffset[4];		// file offset in bytes, LSB first
	uint8_t ftFragmentSize[2];  // size of the data fragment in bytes, LSB first. The maximum size of the data fragment is 64 bytes.
	uint8_t ftData[];			// file data fragment
} osdp_FILETRANSFER;

typedef struct {
	uint8_t readerNber;
	uint8_t textCommand; // how to treat the text
	uint8_t tempTextTime;// for temporary text, the time to display the text
	uint8_t row;         // The row where the first character will be displayed (0x01 is the top row)
	uint8_t column;		 // The column where the first character will be displayed (0x01 is the leftmost column)
	uint8_t textLength;  // The length of the text to be displayed
	uint8_t textData[];
} osdp_TEXT;


typedef struct {
	uint8_t readerNber;
	uint8_t bioType;
	uint8_t bioFormat;
	uint8_t bioQuality;
} osdp_BIOREAD;


typedef struct {
	uint8_t outputNber;
	uint8_t ctrlCode;
	uint8_t lsbTimer;
	uint8_t msbTimer;
} osdp_OUT;


typedef struct {
	uint8_t keyType;
	uint8_t keyLen;
	uint8_t keyData[16];
} osdp_KEYSET;


typedef struct {
	uint8_t data[1];
} osdp_CAP;


typedef struct {
	uint8_t permControlCode;
	uint8_t permOnTime;
	uint8_t permOffTime;
	uint8_t permOnColor;
	uint8_t permOffColor;
} perm_LED_Settings;


typedef struct {
	uint8_t tmpControlCode;
	uint8_t tmpOnTime;
	uint8_t tmpOffTime;
	uint8_t tmpOnColor;
	uint8_t tmpOffColor;
	uint8_t tmpTimerLsb;
	uint8_t tmpTimerMsb;
} tmp_LED_Settings;


typedef struct {
	uint8_t readerNumber;
	uint8_t ledNumber;
	tmp_LED_Settings tmpSettings;
	perm_LED_Settings permSettings;
} osdp_LED;


typedef struct {
	uint8_t readerNumber;
	uint8_t toneCode;
	uint8_t onTime;
	uint8_t offTime;
	uint8_t numberOfCycles;
} osdp_BUZ;


typedef struct {
	uint8_t roundA[8];
} osdp_CHLNG;


typedef struct {
	uint8_t data[1];
} osdp_ID;


typedef struct{
	uint8_t srvCryptogram[16];
} osdp_SCRYPT;

#pragma pack(1)
typedef struct {
	// command Id
	uint8_t cmdId;
	/** Command */
	union {
		osdp_CHLNG chlng;        
		osdp_SCRYPT scrypt;
		osdp_ID	id; 
		osdp_BUZ buz;   
		osdp_LED led;
		osdp_CAP cap;   
		osdp_KEYSET key;
		osdp_OUT out;
		osdp_BIOREAD bioRead;
		osdp_BIOMATCH bioMatch;
		osdp_COMSET comSet;
		osdp_TEXT text;
		osdp_FILETRANSFER fileTransfer;
		osdp_MFG mfg;
	};
} osdp_cmd;
#pragma pack()

#pragma pack(1)
typedef struct {

	uint8_t addr;       
    uint32_t baudrate;
}osdp_COM;
#pragma pack()

typedef struct {
	uint8_t vendorCode1st;		// vendor Code 1st
	uint8_t vendorCode2nd; 		// vendor Code 2nd
	uint8_t vendorCode3rd;		// vendor Code 3rd
	uint8_t mfgData[];			// manufacturer cmd data
} osdp_MFGREP;

typedef struct {
	uint8_t status[1];
} osdp_OSTATR;


typedef struct {
	uint8_t status[1];
} osdp_ISTATR;


typedef struct  {
	uint8_t funcCode;
	uint8_t compliance;
	uint8_t numberOf;
} osdp_Capabilities;


typedef struct {
    osdp_Capabilities data[16];     	
} osdp_PDCAP;


typedef struct {
	uint8_t reader;
	uint8_t digitCount;
    uint8_t data[];     	
} osdp_KEYPAD;


typedef struct {
	uint8_t reader;
	uint8_t formatCode;
	uint8_t lsb;
	uint8_t msb;
    uint8_t data[];     	
} osdp_RAW;


typedef struct {
	uint8_t tamperStatus;
	uint8_t powerStatus;
} osdp_LSTATR;


typedef struct {
	uint8_t code[1];
} osdp_NAK;


typedef struct {

	uint8_t vendor_code1;       /**< 3-bytes IEEE assigned OUI */
    uint8_t vendor_code2;
    uint8_t vendor_code3;
    uint8_t mfModelNumber;      /**< 1-byte Manufacturer's model number */
	uint8_t serial_number[5];   /**< 5-byte serial number for the PD */           
	uint8_t firmwareVersion[3]; /**< 3-byte version (major, minor, build) */
}osdp_PDID;


typedef struct {

	uint8_t clientId[8];
	uint8_t roundB[8];
	uint8_t cliCryptogram[16];
} osdp_CCRYPT;     


typedef struct {
	uint8_t rmac_i[16];
} osdp_RMAC_I;   


typedef struct {
	uint8_t ftAction;		   // Indicate use of secure channel or not
	uint8_t ftDelay[2]; 	   // delay in milliseconds before the next file transfer action, LSB first
	uint8_t ftStatusDetail[2]; // file transfer status detail, specific to the file transfer type, LSB first
	uint8_t ftUpdateMsgMax[2]; // alternate max message size, LSB first
} osdp_FTSTAT;

typedef struct {

	// reply Id
	uint8_t replyId;
	/** reply */
	union {
		osdp_CCRYPT ccrypt;        
		osdp_RMAC_I rmac_i;   
		osdp_PDID  pdid; 
		osdp_NAK error;
		osdp_RAW rawData;
		osdp_LSTATR localStatusReport;
		osdp_KEYPAD keypad;
		osdp_PDCAP pdcap;
		osdp_ISTATR inputState;
		osdp_OSTATR outputState;	
		osdp_COM comSetReply;	
		osdp_FTSTAT ftstatus;
		osdp_MFGREP mfgReply;
	};
} osdp_reply;


#endif	/* _OSDP_COMMON_H_ */