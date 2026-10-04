#include <string.h>
#include <stdlib.h>
#include <stdio.h> //! pour le printf

#include "osdp_common.h"
#include "osdp_sc.h"
#include "aes.h"
#include "osdp_util.h"
#include "../util/utils.h"

#ifdef ENV_FREERTOS
    #include "dbgWrapper.h"
#endif


#define OSDP_SC_EOM_MARKER             0x80  /* End Of Message Marker */
#define AES_PAD_LEN(x) ((x + 16 - 1) & (~(16 - 1)))

/* Default key as specified in OSDP specification */
static const uint8_t osdp_scbk_default[16] = {
	0x30, 0x31, 0x32, 0x33, 0x34, 0x35, 0x36, 0x37,
	0x38, 0x39, 0x3A, 0x3B, 0x3C, 0x3D, 0x3E, 0x3F
};

#ifdef ENV_WINDOWS
//uint8_t scbk[16] = {0}; 

//uint8_t *osdp_scbk = scbk;  // -> utilisation de la clÃ© personnalisÃ©e
uint8_t *osdp_scbk = NULL;  // -> use osdp_scbk_default
#endif

#ifdef ENV_FREERTOS
uint8_t *osdp_scbk = NULL;
#endif

// Todo passer Ã  une structure unique plus besoin du tableau de 4 unitÃ©s
osdp_secure_channel osdp_sc[4];
static uint8_t configured_scbk[16];


int checkScbkExists(){

	return (osdp_scbk != NULL);
}


void setNewScbk(uint8_t* newKey){

	clearSecureChannelKeys();
	memcpy(configured_scbk, newKey, 16);
	osdp_scbk = configured_scbk;
}

void clearSecureChannelKeys(void)
{
    secure_zero(osdp_sc, sizeof(osdp_sc));
    secure_zero(configured_scbk, sizeof(configured_scbk));
    osdp_scbk = NULL;
}

int verifyInitialRmac(uint8_t pdIndex, const uint8_t *rmac)
{
    struct AES_ctx ctx;
    uint8_t expected[16];
    unsigned char diff = 0;
    memcpy(expected, osdp_sc[pdIndex].cp_cryptogram, 16);
    AES_init_ctx(&ctx, osdp_sc[pdIndex].s_mac1);
    AES_ECB_encrypt(&ctx, expected);
    AES_init_ctx(&ctx, osdp_sc[pdIndex].s_mac2);
    AES_ECB_encrypt(&ctx, expected);
    for (int i = 0; i < 16; ++i) diff |= expected[i] ^ rmac[i];
    secure_zero(expected, sizeof(expected));
    secure_zero(&ctx, sizeof(ctx));
    return diff == 0;
}


static void generateSessionKey(uint8_t pdIndex, uint8_t firstValue, uint8_t secondValue, uint8_t * sessionKey){
	struct AES_ctx ctx;

	sessionKey[0] = firstValue;
	sessionKey[1] = secondValue;
	memcpy(sessionKey + 2, osdp_sc[pdIndex].cp_random, 6);
	AES_init_ctx(&ctx, osdp_sc[pdIndex].scbk);
	AES_ECB_encrypt(&ctx, sessionKey);
    secure_zero(&ctx, sizeof(ctx));
}


static void computePdCryptogram(uint8_t pdIndex){
	struct AES_ctx ctx;

	// concat rndA|rndB
	memcpy(osdp_sc[pdIndex].pd_cryptogram, osdp_sc[pdIndex].cp_random, 8);
	memcpy(osdp_sc[pdIndex].pd_cryptogram + 8, osdp_sc[pdIndex].pd_random, 8);

	AES_init_ctx(&ctx, osdp_sc[pdIndex].s_enc);
	AES_ECB_encrypt(&ctx, osdp_sc[pdIndex].pd_cryptogram);	
    secure_zero(&ctx, sizeof(ctx));
}


static void computeCpCryptogram(uint8_t pdIndex){
	struct AES_ctx ctx;

	// concat rndB|rndA
	memcpy(osdp_sc[pdIndex].cp_cryptogram, osdp_sc[pdIndex].pd_random, 8);
	memcpy(osdp_sc[pdIndex].cp_cryptogram + 8, osdp_sc[pdIndex].cp_random, 8);

    AES_init_ctx(&ctx, osdp_sc[pdIndex].s_enc);
	AES_ECB_encrypt(&ctx, osdp_sc[pdIndex].cp_cryptogram);	
    secure_zero(&ctx, sizeof(ctx));
}


#ifdef ENV_WINDOWS
void computeSecureSessionParametersOfPD(uint8_t pdIndex, uint8_t * randA)
{
    struct AES_ctx ctx;
	//printf("Je rentre dans computeSecureSessionParametersOfPD!!!!!\n");//! a supp
	memset(osdp_sc, 0, sizeof(osdp_secure_channel) * 4);

	if(osdp_scbk == NULL)
		memcpy(osdp_sc[0].scbk, osdp_scbk_default, 16);
	else
		memcpy(osdp_sc[0].scbk, osdp_scbk, 16);

	AES_init_ctx(&ctx, osdp_sc[0].scbk);

	memcpy(osdp_sc[0].cp_random, randA, 8);

	// session keys s_enc, s_mac1 ans s_mac2
	generateSessionKey(0, 0x01, 0x82, osdp_sc[0].s_enc);
	generateSessionKey(0, 0x01, 0x01, osdp_sc[0].s_mac1);
	generateSessionKey(0,0x01, 0x02, osdp_sc[0].s_mac2);
	
    if (!getRandomValue(osdp_sc[0].pd_random, 8)) {
        clearSecureChannelKeys();
        secure_zero(&ctx, sizeof(ctx));
        return;
    }
	memcpy(osdp_sc[0].pd_client_uid, hexaString2ByteArray("F51BC00100000000",8), 8);

	computePdCryptogram(pdIndex);
	//printf("Je sors de computeSecureSessionParametersOfPD!!!!!\n");//! a supp
}
#endif


void computeSecureSessionParametersOfCP(uint8_t pdIndex, uint8_t * randB, uint8_t * clientId, uint8_t * cliCryptogram)
{

	if(osdp_scbk == NULL)
		memcpy(osdp_sc[pdIndex].scbk, osdp_scbk_default, 16);
	else
		memcpy(osdp_sc[pdIndex].scbk, osdp_scbk, 16);

	// session keys s_enc, s_mac1 ans s_mac2
    generateSessionKey(pdIndex, 0x01, 0x82, osdp_sc[pdIndex].s_enc);
	generateSessionKey(pdIndex, 0x01, 0x01, osdp_sc[pdIndex].s_mac1);
	generateSessionKey(pdIndex, 0x01, 0x02, osdp_sc[pdIndex].s_mac2);

	memcpy(osdp_sc[pdIndex].pd_random, randB, 8);
	memcpy(osdp_sc[pdIndex].pd_client_uid, clientId, 8);
	memcpy(osdp_sc[pdIndex].pd_cryptogram, cliCryptogram, 16);

	computeCpCryptogram(pdIndex);
}


int verifyCryptogram(uint8_t pdIndex, uint8_t *cryptogram, uint8_t typeDevice)
{
    struct AES_ctx ctx;
    uint8_t expected[16];
    unsigned char diff = 0;
    if (typeDevice != CONTROL_PANEL && typeDevice != PERIPHERAL_DEVICE) return 1;
    memcpy(expected, typeDevice == CONTROL_PANEL ? osdp_sc[pdIndex].cp_random : osdp_sc[pdIndex].pd_random, 8);
    memcpy(expected + 8, typeDevice == CONTROL_PANEL ? osdp_sc[pdIndex].pd_random : osdp_sc[pdIndex].cp_random, 8);
    AES_init_ctx(&ctx, osdp_sc[pdIndex].s_enc);
    AES_ECB_encrypt(&ctx, expected);
    for (int i = 0; i < 16; ++i) diff |= expected[i] ^ cryptogram[i];
    secure_zero(expected, sizeof(expected));
    secure_zero(&ctx, sizeof(ctx));
    return diff != 0;
}


void computeInitialRmac()
{
	struct AES_ctx ctx;

	memcpy(osdp_sc[0].r_mac, osdp_sc[0].cp_cryptogram, 16);

    AES_init_ctx(&ctx, osdp_sc[0].s_mac1);
    AES_ECB_encrypt(&ctx, osdp_sc[0].r_mac);
    AES_init_ctx(&ctx, osdp_sc[0].s_mac2);
    AES_ECB_encrypt(&ctx, osdp_sc[0].r_mac);	
}


uint8_t * getClientUid(){
	return osdp_sc[0].pd_client_uid;
}


uint8_t * getRndB(){
	return osdp_sc[0].pd_random;
}


uint8_t * getClientCryptogram(){
	return osdp_sc[0].pd_cryptogram;
}


void setRndA(uint8_t pdIndex, uint8_t* rendA){
	memset(&osdp_sc[pdIndex], 0, sizeof(osdp_secure_channel));
	memcpy(osdp_sc[pdIndex].cp_random, rendA, 8);
}


uint8_t * getServerCryptogram(uint8_t pdIndex){
	return	osdp_sc[pdIndex].cp_cryptogram;
}


uint8_t * getRmac(){
	return osdp_sc[0].r_mac;	
}


void setInitialRmac(uint8_t pdIndex, uint8_t * rmac_i){
	memcpy(osdp_sc[pdIndex].r_mac, rmac_i,16);
	oneComplementOfNumber(osdp_sc[pdIndex].r_mac_oneCompl, osdp_sc[pdIndex].r_mac);
}


void computeMac(uint8_t pdIndex,int is_cmd, uint8_t * rawDataToSend, int len)
{
    struct AES_ctx ctx;
    uint8_t *KeyToUse;
    uint8_t rawdata[16] = {0};
    uint8_t mac[16] = {0};
	int pad_len;
	uint8_t buf[MAX_SIZE_MSG_PACKET] = { 0 };

	// ici pading si len n'est pas multiple de 16
	memcpy(buf, rawDataToSend, len);
	pad_len = (len % 16 == 0) ? len : AES_PAD_LEN(len);
	if (len % 16 != 0) {
		buf[len] = OSDP_SC_EOM_MARKER; /* End Of Message marker */
	}

    //Init IV with last MAC replied
	memcpy(mac, is_cmd ? osdp_sc[pdIndex].r_mac : osdp_sc[pdIndex].c_mac, 16);
	for (int i = 0; i < pad_len; i = i + 16)
    {
        memcpy(rawdata, buf + i, 16);
     
        //case of last computation use S_SMAC2
        if (i + 16 == pad_len)
            KeyToUse = osdp_sc[pdIndex].s_mac2;	
        else
            KeyToUse = osdp_sc[pdIndex].s_mac1;	
    
        AES_init_ctx_iv(&ctx, KeyToUse, mac);
        AES_CBC_encrypt_buffer(&ctx, rawdata, 16);

        memcpy(mac, rawdata,16);
    }

	// sauvegarde le MAC calculer pour le prochain echange
    memcpy(is_cmd ? osdp_sc[pdIndex].c_mac : osdp_sc[pdIndex].r_mac, mac, 16);
	oneComplementOfNumber(is_cmd ? osdp_sc[pdIndex].c_mac_oneCompl : osdp_sc[pdIndex].r_mac_oneCompl, mac);
    // ajoute les 4 octets du MAC calculer a la suite des donnees de la trame
	memcpy(rawDataToSend + len, mac, 4);
    secure_zero(&ctx, sizeof(ctx));
    secure_zero(buf, sizeof(buf));
    secure_zero(rawdata, sizeof(rawdata));
    secure_zero(mac, sizeof(mac));
}

/* computeMac writes its result over the received MAC: save it first, and
 * restore chaining state on rejection so unauthenticated input cannot alter it.
 */
int verifyMac(uint8_t pdIndex, int is_cmd, uint8_t *frame, int length)
{
    uint8_t received[4];
    osdp_secure_channel saved = osdp_sc[pdIndex];
    unsigned char diff = 0;
    memcpy(received, frame + length, 4);
    computeMac(pdIndex, is_cmd, frame, length);
    for (int i = 0; i < 4; ++i) diff |= received[i] ^ frame[length + i];
    memcpy(frame + length, received, 4);
    if (diff) osdp_sc[pdIndex] = saved;
    secure_zero(&saved, sizeof(saved));
    return diff == 0;
}


uint8_t * encryptData(uint8_t pdIndex, int is_cmd, uint8_t *rawDataToSend, int length)
{
    struct AES_ctx ctx;
	int pad_len;
	uint8_t iv[16];
	uint8_t buf[MAX_SIZE_MSG_PACKET] = { 0 };
	uint8_t *cipheredDada = NULL;

	// ici pading si len n'est pas multiple de 16
	memcpy(buf, rawDataToSend, length);
	pad_len = (length % 16 == 0) ? length : AES_PAD_LEN(length);
	if (length % 16 != 0) {
		buf[length] = OSDP_SC_EOM_MARKER; /* End Of Message marker */
	}   

    // Copie le MAC adequat comme iv
	memcpy(iv, is_cmd ? osdp_sc[pdIndex].r_mac_oneCompl : osdp_sc[pdIndex].c_mac_oneCompl, 16);

    AES_init_ctx_iv(&ctx, osdp_sc[pdIndex].s_enc, iv);
    AES_CBC_encrypt_buffer(&ctx, buf, pad_len);

	cipheredDada = (uint8_t *) malloc((sizeof(uint8_t) * pad_len) + 1);

	memcpy( cipheredDada+1, buf, pad_len);
    secure_zero(&ctx, sizeof(ctx));
    secure_zero(iv, sizeof(iv));
    secure_zero(buf, sizeof(buf));

	return cipheredDada;
}


int decryptData(uint8_t pdIndex, int is_cmd, uint8_t *rawDataReceive, int length)
{
    if (length <= 0 || length > MAX_SIZE_MSG_PACKET || length % 16 != 0)
        return -1;
    struct AES_ctx ctx;
	int i;
	uint8_t iv[16];
    uint8_t s_enc[16];
	uint8_t buf[MAX_SIZE_MSG_PACKET] = { 0 };

    memcpy(buf, rawDataReceive, length);
	//char * strData = byteArrayToHexaString(buf, length);
	//printf("decryptData, cipheredRawDataReceived = %s\n", strData);
	//free(strData);
    
	memcpy(iv, is_cmd ? osdp_sc[pdIndex].r_mac_oneCompl : osdp_sc[pdIndex].c_mac_oneCompl, 16);

    AES_init_ctx_iv(&ctx, osdp_sc[pdIndex].s_enc, iv);
	AES_CBC_decrypt_buffer(&ctx, buf, length);

	//strData = byteArrayToHexaString(buf, length);
    //printf("decryptData, plainData = %s\n", strData);
	//free(strData);

	// **** Traitement du cas du padding
	while (length && buf[length-1] == 0x00) {
		length--;
	}

    if (length > 0 && buf[length -1] == 0x80) {
		buf[length-1] = 0;
		length--;
	}
	// **** Traitement du cas du padding

	//strData = byteArrayToHexaString(buf, length);
    //printf("decryptData, plainData = %s\n", strData);
	//printf("decryptData, length = %d\n", length);
	//free(strData);

	memcpy(rawDataReceive, buf, length);
    secure_zero(&ctx, sizeof(ctx));
    secure_zero(iv, sizeof(iv));
    secure_zero(buf, sizeof(buf));
	
	return length;
}


#if 1
void showOspdSC_parameter(FILE * outputStream, int32_t nbTrame){
	char * strValue = NULL;
	// static int32_t nbTrame = 0;

	// nbTrame+= 1;
	fprintf(outputStream, "trame %ld\n", (long)nbTrame);
	strValue = byteArrayToHexaString(osdp_sc[0].cp_random, 8);
	fprintf(outputStream, "cp_random.... = %s\n",strValue);
	free(strValue);

	strValue = byteArrayToHexaString(osdp_sc[0].pd_random, 8);
	fprintf(outputStream, "pd_random.... = %s\n",strValue);
	free(strValue);

	strValue = byteArrayToHexaString(osdp_sc[0].cp_cryptogram, 16);
	fprintf(outputStream, "cp_cryptogram = %s\n",strValue);
	free(strValue);

	strValue = byteArrayToHexaString(osdp_sc[0].pd_cryptogram, 16);
	fprintf(outputStream, "pd_cryptogram = %s\n",strValue);
	free(strValue);

	strValue = byteArrayToHexaString(osdp_sc[0].pd_client_uid, 8);
	fprintf(outputStream, "pd_client_uid = %s\n",strValue);
	free(strValue);

	fprintf(outputStream, "s_enc = [redacted]\n");

	fprintf(outputStream, "s_mac1 = [redacted]\n");

	fprintf(outputStream, "s_mac2 = [redacted]\n");

	strValue = byteArrayToHexaString(osdp_sc[0].r_mac, 16);
	fprintf(outputStream, "r_mac........ = %s\n",strValue);
	free(strValue);

	strValue = byteArrayToHexaString(osdp_sc[0].r_mac_oneCompl, 16);
	fprintf(outputStream, "r_mac_1_Compl = %s\n",strValue);
	free(strValue);

	strValue = byteArrayToHexaString(osdp_sc[0].c_mac, 16);
	fprintf(outputStream, "c_mac........ = %s\n",strValue);
	free(strValue);

	strValue = byteArrayToHexaString(osdp_sc[0].c_mac_oneCompl, 16);
	fprintf(outputStream, "c_mac_1_Compl = %s\n",strValue);
	free(strValue);
}
#endif

