/**
 * @file osdp_message_printer_json.c
 * @brief Implements JSON formatted printing of decoded OSDP commands and replies.
 *
 * This module provides helper functions for displaying decoded OSDP  messages in a human-readable format. It supports both OSDP commands
 * and replies by printing the individual fields associated with each  message type.
 *
 * The module operates on already decoded OSDP structures and does not perform any frame parsing or protocol validation. Its sole purpose
 * is to convert OSDP command and reply data structures into readable JSON textual output.
 *
 * The printing functions are designed to be shared by multiple output backends such as console renderers, file loggers, and diagnostic
 * tools, ensuring a consistent representation of OSDP messages across the application.
 */
#include "osdp_message_printer_json.h"




/* --------------------------------------------------------------------------
 * OSDP Command Rendering Functions
 * -------------------------------------------------------------------------- */


 /**
 * @brief Render an OSDP CHLNG command.
 *
 * Displays the random challenge value (RndA) used during secure channel establishment.
 *
 * @param chlng Pointer to decoded OSDP command structure.
 */
static void print_json_osdp_CHLNG(FILE *out, const osdp_cmd *chlng)
{
    fprintf(out, "\t\"Data\" : {\n");
    fprintf(out, "\t\t\"RndA\": ");

    for(int i=0; i<8; i++)
    {
        fprintf(out, "%02X", chlng->chlng.roundA[i]);
    }
    
    fprintf(out, "\n");
    fprintf(out, "\t}\n");
}


/**
 * @brief Render an OSDP SCRYPT command.
 *
 * Displays the server cryptogram exchanged during secure channel authentication.
 *
 * @param scrypt Pointer to decoded OSDP command structure.
 */
static void print_json_osdp_SCRYPT(FILE *out, const osdp_cmd *scrypt)
{
    fprintf(out, "\t\"Data\" : {\n");

    fprintf(out, "\t\t\"Srv Cryptogram\": ");

    for(int i=0; i<16; i++)
    {
        fprintf(out, "%02X", scrypt->scrypt.srvCryptogram[i]);
    }

    fprintf(out, "\n");
    fprintf(out, "\t}\n");
}


/**
 * @brief Render an OSDP POLL command.
 *
 * The POLL command contains no additional payload fields.
 *
 * @param poll Pointer to decoded OSDP command structure.
 */
static void print_json_osdp_POLL(FILE *out, const osdp_cmd *poll)
{
    // No additional data to display for POLL command
    (void)poll;
}


/**
 * @brief Render an OSDP ID command.
 *
 * @param id Pointer to decoded OSDP command structure.
 */
static void print_json_osdp_ID(FILE *out, const osdp_cmd *id)
{
    fprintf(out, "\t\"Data\" : %02X\n", id->id.data[0]);
}


/**
 * @brief Render an OSDP CAP command.
 *
 * @param cap Pointer to decoded OSDP command structure.
 */
static void print_json_osdp_CAP(FILE *out, const osdp_cmd *cap)
{
    fprintf(out, "\t\"Data\" : %02X\n", cap->cap.data[0]);
}


/**
 * @brief Render an OSDP OUT command.
 *
 * Displays output control configuration parameters.
 *
 * @param out Pointer to decoded OSDP command structure.
 */
static void print_json_osdp_OUT(FILE *out, const osdp_cmd *out_cmd)
{
    fprintf(out, "\t\"Data\" : {\n");

    fprintf(out, "\t\t\"Output Number\" : %02X,\n",  out_cmd->out.outputNber);
    fprintf(out, "\t\t\"Output Control\" : %02X,\n", out_cmd->out.ctrlCode);
    fprintf(out, "\t\t\"Timer LSB\" : %02X,\n", out_cmd->out.lsbTimer);
    fprintf(out, "\t\t\"Timer MSB\" : %02X\n", out_cmd->out.msbTimer);

    fprintf(out, "\t}\n");
}


/**
 * @brief Render an OSDP LED command.
 *
 * Displays temporary and permanent LED behavior settings.
 *
 * @param led Pointer to decoded OSDP command structure.
 */
static void print_json_osdp_LED(FILE *out, const osdp_cmd *led)
{
    fprintf(out, "\t\"Data\" : {\n");

    fprintf(out, "\t\t\"Reader Number\" : %02X,\n", led->led.readerNumber);
    fprintf(out, "\t\t\"LED Number\" : %02X,\n", led->led.ledNumber);

    fprintf(out, "\t\t\"Temporary Settings : {\n");
    fprintf(out, "\t\t\t\"Control Code\" : %02X,\n", led->led.tmpSettings.tmpControlCode);
    fprintf(out, "\t\t\t\"On Time\" : %02X,\n", led->led.tmpSettings.tmpOnTime);
    fprintf(out, "\t\t\t\"Off Time\" : %02X,\n", led->led.tmpSettings.tmpOffTime);
    fprintf(out, "\t\t\t\"On Color\" : %02X,\n", led->led.tmpSettings.tmpOnColor);
    fprintf(out, "\t\t\t\"Off Color\" : %02X,\n", led->led.tmpSettings.tmpOffColor);
    fprintf(out, "\t\t\t\"Timer LSB\" : %02X,\n", led->led.tmpSettings.tmpTimerLsb);
    fprintf(out, "\t\t\t\"Timer MSB\" : %02X,\n", led->led.tmpSettings.tmpTimerMsb);
    fprintf(out, "\t\t}\n");

    fprintf(out, "\t\t\"Permanent Settings : {\n");
    fprintf(out, "\t\t\t\"Control Code\" : %02X,\n", led->led.permSettings.permControlCode);
    fprintf(out, "\t\t\t\"On Time\" : %02X,\n", led->led.permSettings.permOnTime);
    fprintf(out, "\t\t\t\"Off Time\" : %02X,\n", led->led.permSettings.permOffTime);
    fprintf(out, "\t\t\t\"On Color\" : %02X,\n", led->led.permSettings.permOnColor);
    fprintf(out, "\t\t\t\"Off Color\" : %02X,\n", led->led.permSettings.permOffColor);
    fprintf(out, "\t\t}\n");

    fprintf(out, "\t}\n");    
}


/**
 * @brief Render an OSDP BUZ command.
 *
 * Displays buzzer configuration parameters.
 *
 * @param buz Pointer to decoded OSDP command structure.
 */
static void print_json_osdp_BUZ(FILE *out, const osdp_cmd *buz)
{
    fprintf(out, "\t\"Data\" : {\n");

    fprintf(out, "\t\t\"Reader Number\" : %02X,\n", buz->buz.readerNumber);
    fprintf(out, "\t\t\"Tone Code\" : %02X,\n", buz->buz.toneCode);
    fprintf(out, "\t\t\"On Time\" : %02X,\n", buz->buz.onTime);
    fprintf(out, "\t\t\"Off Time\" : %02X,\n", buz->buz.offTime);
    fprintf(out, "\t\t\"Number of Cycles\" : %02X\n", buz->buz.numberOfCycles);

    fprintf(out, "\t}\n");     
}


/**
 * @brief Render an OSDP TEXT command.
 *
 * Displays text output configuration and associated string data.
 *
 * @param text Pointer to decoded OSDP command structure.
 * @param len Length of the command data.
 */
static void print_json_osdp_TEXT(FILE *out, const osdp_cmd *text, size_t len)
{
    fprintf(out, "\t\"Data\" : {\n");

    fprintf(out, "\t\t\"Reader Number\" : %02X,\n", text->text.readerNber);
    fprintf(out, "\t\t\"Text command\" : %02X,\n", text->text.textCommand);
    fprintf(out, "\t\t\"Temporary Text Time\" : %02X,\n", text->text.tempTextTime);
    fprintf(out, "\t\t\"Row\" : %d,\n", text->text.row);
    fprintf(out, "\t\t\"Column\" : %d,\n", text->text.column);
    fprintf(out, "\t\t\"Text Length\" : %d,\n", text->text.textLength);
    fprintf(out, "\t\t\"String\" : \"");

    for(int i=0; i<text->text.textLength; i++)
    {
        fprintf(out, "%c", text->text.textData[i]);
    }
    fprintf(out, "\"\n");

    fprintf(out, "\t}\n");     
}


/**
 * @brief Render an OSDP COMSET command.
 *
 * Displays the new communication settings requested by the Control Panel, including the target address and baud rate.
 *
 * @param comset Pointer to decoded OSDP command structure.
 */
static void print_json_osdp_COMSET(FILE *out, const osdp_cmd *comset)
{
    fprintf(out, "\t\"Data\" : {\n");

    fprintf(out, "\t\t\"New Address\" : %02X,\n", comset->comSet.addr);
    fprintf(out, "\t\t\"New Baudrate: %u\n", comset->comSet.baudrate);

    fprintf(out, "\t}\n");     
}


/**
 * @brief Render an OSDP KEYSET command.
 *
 * Displays secure channel key configuration parameters, including key type and length; key material is redacted.
 *
 * @param keyset Pointer to decoded OSDP command structure.
 */
static void print_json_osdp_KEYSET(FILE *out, const osdp_cmd *keyset)
{
    fprintf(out, "\t\"Data\" : {\n");

    fprintf(out, "\t\t\"Key Type\" : %02X,\n", keyset->key.keyType);
    fprintf(out, "\t\t\"Key Length\" : %02X,\n", keyset->key.keyLen);
    fprintf(out, "\t\t\"Key Data\": \"[redacted]\"\n");

    fprintf(out, "\t}\n");     
}


/**
 * @brief Render an OSDP FILETRANSFER command.
 *
 * Displays file transfer metadata and fragment information exchanged during OSDP file transfer operations.
 *
 * This renderer is currently incomplete and reserved for future implementation.
 *
 * @param fileTransfer Pointer to decoded OSDP command structure.
 * @param len Length of the command data.
 */
static void print_json_osdp_FILETRANSFER(FILE *out, const osdp_cmd *fileTransfer, size_t len)
{
    fprintf(out, "\t\"Data\" : {\n");

    fprintf(out, "\t\t\"FtType\" : %02X,\n", fileTransfer->fileTransfer.ftType);
    fprintf(out, "\t\t\"FtSize\" : %ld,\n", *(long *)&fileTransfer->fileTransfer.ftSize[0]);
    fprintf(out, "\t\t\"FtOffset\" : %ld,\n", *(long *)&fileTransfer->fileTransfer.ftOffset[0]);
    fprintf(out, "\t\t\"FtFragmentSize\" : %d\n", *(int16_t *)&fileTransfer->fileTransfer.ftFragmentSize[0]);
    
    // fprintf(out, "\t\tFtData: ");
    // for(int i=0; i<fileTransfer->fileTransfer.ftFragmentSize; i++)
    // {
    //     fprintf(out, "%02X", fileTransfer->fileTransfer.ftData[i]);
    // }
    // fprintf(out, "\n");

    fprintf(out, "\t}\n");      
}   


/**
 * @brief Render an OSDP MFG command.
 *
 * Displays a manufacturer specific reply, 3-bytes vendor code and specific reply data.
 *
 * @param out    Output file stream.
 * @param mfgRep Pointer to decoded OSDP reply structure.
 * @param len payload length in bytes.
 */
static void print_json_osdp_MFG(FILE *out, const osdp_cmd *mfgCmd, int len)
{
    len--; // -1 character, since "len" accounts for the MFG 0x80 code

    fprintf(out, "\t\"Data\" : {\n");
    fprintf(out, "\t\t\"VendorCode1st\": %02X,\n",mfgCmd->mfg.vendorCode1st);
    fprintf(out, "\t\t\"VendorCode2nd:\" %02X,\n",mfgCmd->mfg.vendorCode2nd);
    fprintf(out, "\t\t\"VendorCode3rd:\" %02X,\n",mfgCmd->mfg.vendorCode3rd);

    int specCmdLen = len - 3;
    fprintf(out, "\t\t\"VendorData\" : ");

    if(specCmdLen > 0){
        for(int i=0; i < specCmdLen; i++)
        {
            (i + 1 == specCmdLen)?    fprintf(out, "%02X\n", mfgCmd->mfg.mfgData[i]) : fprintf(out, "%02X", mfgCmd->mfg.mfgData[i]);
        }
    }
    else
        fprintf(out, "no data\n");
    fprintf(out, "\t}\n");
}

/* 
 * End of OSDP command rendering functions */


/* --------------------------------------------------------------------------
 * OSDP Reply Rendering Functions
 * -------------------------------------------------------------------------- */


/**
 * @brief Render an OSDP CCRYPT reply.
 *
 * Displays secure channel authentication parameters returned by the Peripheral Device during secure channel establishment.
 *
 * Printed fields include:
 *   - client identifier
 *   - random challenge value (RndB)
 *   - client cryptogram
 *
 * @param ccrypt Pointer to decoded OSDP reply structure.
 * @param out    Output file stream.
 */ 
static void print_json_osdp_CCRYPT(FILE *out, const osdp_reply *ccrypt)
{
    fprintf(out, "\t\"Data\" : {\n");

    fprintf(out, "\t\t\"Client ID\" : ");

    for(int i=0; i<8; i++)
    {
        fprintf(out, "%02X", ccrypt->ccrypt.clientId[i]);
    }

    fprintf(out, ",\n");

    fprintf(out, "\t\t\"RndB\" : ");
    for(int i=0; i<8; i++)
    {
        fprintf(out, "%02X", ccrypt->ccrypt.roundB[i]);
    }
    fprintf(out, ",\n");

    fprintf(out, "\t\t\"Client Cryptogram\" : ");
    for(int i=0; i<16; i++)
    {
        fprintf(out, "%02X", ccrypt->ccrypt.cliCryptogram[i]);
    }
    fprintf(out, "\n");

    fprintf(out, "\t}\n");      
}


/**
 * @brief Render an OSDP RMAC_I reply.
 *
 * Displays the reply MAC initialization value exchanged during secure channel establishment.
 *
 * @param rmac_i Pointer to decoded OSDP reply structure.
 * @param out    Output file stream.
 */
static void print_json_osdp_RMAC_I(FILE *out, const osdp_reply *rmac_i)
{
    fprintf(out, "\t\"Data\" : {\n");

    fprintf(out, "\t\t\"RMAC_I\" : ");

    for(int i=0; i<16; i++)
    {
        fprintf(out, "%02X", rmac_i->rmac_i.rmac_i[i]);
    }

    fprintf(out, "\n");

    fprintf(out, "\t}\n");      
}


/**
 * @brief Render an OSDP ACK reply.
 *
 * The ACK reply contains no additional payload data.
 *
 * @param ack Pointer to decoded OSDP reply structure.
 * @param out    Output file stream.
 */
static void print_json_osdp_ACK(FILE *out, const osdp_reply *ack)
{
    // No additional data to display for ACK reply
    (void)ack;
    (void)out;
}


/**
 * @brief Render an OSDP NAK reply.
 *
 * Displays the protocol error code returned by the Peripheral Device.
 *
 * @param nak Pointer to decoded OSDP reply structure.
 * @param out    Output file stream.
 */
static void print_json_osdp_NAK(FILE *out, const osdp_reply *nak)
{
    fprintf(out, "\t\"Data\" : {\n");

    fprintf(out, "\t\t\"Error code\" : %02X\n", nak->error.code[0]);

    fprintf(out, "\t}\n");      
}


/**
 * @brief Render an OSDP PDID reply.
 *
 * Displays Peripheral Device identification information, including vendor identifier, serial number and firmware version.
 *
 * @param pdid Pointer to decoded OSDP reply structure.
 * @param out    Output file stream.
 */
void print_json_osdp_PDID(FILE *out, const osdp_reply *pdid)
{
    fprintf(out, "\t\"Data\" : {\n");
    fprintf(out, "\t\t\"Vendor Code\" : %02X%02X%02X,\n", pdid->pdid.vendor_code1, pdid->pdid.vendor_code2, pdid->pdid.vendor_code3);
    fprintf(out, "\t\t\"Model Number\" : %02X,\n", pdid->pdid.mfModelNumber);
    fprintf(out, "\t\t\"Serial Number\" : %02X%02X%02X%02X%02X,\n", 
        pdid->pdid.serial_number[4], pdid->pdid.serial_number[3], pdid->pdid.serial_number[2], 
        pdid->pdid.serial_number[1], pdid->pdid.serial_number[0]);
    fprintf(out, "\t\t\"Firmware Version\" : %d.%d.%d\n", pdid->pdid.firmwareVersion[0], pdid->pdid.firmwareVersion[1], pdid->pdid.firmwareVersion[2]);
    fprintf(out, "\t}\n");
}


/**
 * @brief Render an OSDP PDCAP reply.
 *
 * Displays the list of capabilities supported by the Peripheral Device.
 *
 * @param pdcap          Pointer to decoded OSDP reply structure.
 * @param number_records Capability payload length in bytes.
 * @param out    Output file stream.
 */
static void print_json_osdp_PDCAP(FILE *out, const osdp_reply *pdcap, int number_records)
{
    int num_entities = number_records / sizeof(osdp_Capabilities);

    fprintf(out, "\t\"Data\" : [\n");

    for(int i=0; i<num_entities; i++)
    {
        //fprintf(out, "\t\t{"
        fprintf(out, "\t\t{\"Function code\" : %02X, \"Compliance\" : %02X, \"Number of\" : %02X}%c\n", 
            pdcap->pdcap.data[i].funcCode, 
            pdcap->pdcap.data[i].compliance, 
            pdcap->pdcap.data[i].numberOf,
            (i+1 != num_entities) ? ',' : ' ');
    }
    fprintf(out, "\t]\n");
}


/**
 * @brief Render an OSDP LSTATR reply.
 *
 * Displays local tamper and power status information.
 *
 * @param lstatr Pointer to decoded OSDP reply structure.
 * @param out    Output file stream.
 */
static void print_json_osdp_LSTATR(FILE *out, const osdp_reply *lstatr)
{
    fprintf(out, "\t\"Data\" : {\n");

    fprintf(out, "\t\t\"Tamper Status: %02X,\n", lstatr->localStatusReport.tamperStatus);
    fprintf(out, "\t\t\"Power Status: %02X\n", lstatr->localStatusReport.powerStatus);

    fprintf(out, "\t}\n");      
}


/**
 * @brief Render an OSDP ISTATR reply.
 *
 * Displays input status information.
 *
 * @param istatr Pointer to decoded OSDP reply structure.
 * @param out    Output file stream.
 */
static void print_json_osdp_ISTATR(FILE *out, const osdp_reply *istatr)
{
    fprintf(out, "\t\"Data\" : {\n");

    fprintf(out, "\t\t\"Input Status: %02X\n", istatr->inputState.status[0]);

    fprintf(out, "\t}\n");     
}


/**
 * @brief Render an OSDP OSTATR reply.
 *
 * Displays output status information.
 *
 * @param ostatr Pointer to decoded OSDP reply structure.
 * @param out    Output file stream.
 */
static void print_json_osdp_OSTATR(FILE *out, const osdp_reply *ostatr)
{
    fprintf(out, "\t\"Data\" : {\n");

    fprintf(out, "\t\t\"Output Status: %02X\n", ostatr->outputState.status[0]);

    fprintf(out, "\t}\n");      
}


/**
 * @brief Render an OSDP RSTATR reply.
 *
 * Displays reader tamper status report.
 *
 * @param rstatr Pointer to decoded OSDP reply structure.
 * @param out    Output file stream.
 */
static void print_json_osdp_RSTATR(FILE *out, const osdp_reply *rstatr)
{
    fprintf(out, "\t\"Data\" : {\n");

    fprintf(out, "\t\t\"Reader Status: %02X,\n", rstatr->localStatusReport.tamperStatus);
    fprintf(out, "\t\t\"Power Status: %02X\n", rstatr->localStatusReport.powerStatus);

    fprintf(out, "\t}\n");      
}


/**
 * @brief Render an OSDP RAW reply.
 *
 * Displays raw reader data returned by the Peripheral Device.
 *
 * @param raw Pointer to decoded OSDP reply structure.
 * @param out    Output file stream.
 */
static void print_json_osdp_RAW(FILE *out, const osdp_reply *raw)
{
    fprintf(out, "\t\"Data\" : {\n");

    fprintf(out, "\t\t\"Reader Number: %02X\n", raw->rawData.reader);
    fprintf(out, "\t\t\"Format Code: %02X\n", raw->rawData.formatCode);
    fprintf(out, "\t\t\"LSB: %02X\n", raw->rawData.lsb);
    fprintf(out, "\t\t\"MSB: %02X\n", raw->rawData.msb);
    fprintf(out, "\t\t\"Data: ");
    for(int i=0; i<(raw->rawData.lsb + (raw->rawData.msb << 8)); i++)
    {
        fprintf(out, "%02X", raw->rawData.data[i]);
    }
    fprintf(out, "\n");

    fprintf(out, "\t}\n");      
}


/**
 * @brief Render an OSDP FTSTAT reply.
 *
 * Displays file transfer status information returned by the Peripheral Device.
 *
 * This renderer is currently incomplete and reserved for future implementation.
 *
 * @param ftstat Pointer to decoded OSDP reply structure.
 * @param out    Output file stream.
 */
static void print_json_osdp_FTSTAT(FILE *out, const osdp_reply *ftstat)
{
    //(void)ftstat;

    fprintf(out, "\t\"Data\" : {\n");

    fprintf(out, "\t\t\"FtAction: %02X,\n", ftstat->ftstatus.ftAction);
    fprintf(out, "\t\t\"FtDelay : %d,\n", *(int16_t *)&ftstat->ftstatus.ftDelay[0]);
    fprintf(out, "\t\t\"FtStatusDetail: %d,\n", *(int16_t *)&ftstat->ftstatus.ftStatusDetail[0]);
    fprintf(out, "\t\t\"FtUpdateMsgMax : %d\n", *(int16_t *)&ftstat->ftstatus.ftUpdateMsgMax[0]);
    fprintf(out, "\n");

    fprintf(out, "\t}\n");      
}


/**
 * @brief Render an OSDP KEYPAD reply.
 *
 * Displays keypad digits entered on the Peripheral Device.
 *
 * @param keypad Pointer to decoded OSDP reply structure.
 * @param out    Output file stream.
 */
static void print_json_osdp_KEYPAD(FILE *out, const osdp_reply *keypad)
{
    fprintf(out, "\t\"Data\" : {\n");

    fprintf(out, "\t\t\"Reader Number: %02X\n", keypad->keypad.reader);
    fprintf(out, "\t\t\"Digit Count: %02X\n", keypad->keypad.digitCount);
    fprintf(out, "\t\t\"Digits: ");
    for(int i=0; i<keypad->keypad.digitCount; i++)
    {
        fprintf(out, "%02X ", keypad->keypad.data[i]);
    }
    fprintf(out, "\n");

    fprintf(out, "\t}\n");      
}


/**
 * @brief Render an OSDP COM reply.
 *
 * Displays communication settings acknowledged by the Peripheral Device.
 *
 * @param comSetReply Pointer to decoded OSDP reply structure.
 * @param out    Output file stream.
 */
static void print_json_osdp_COM(FILE *out, const osdp_reply *comSetReply)
{
    fprintf(out, "\t\"Data\" : {\n");

    fprintf(out, "\t\t\"Address: %02X\n", comSetReply->comSetReply.addr);
    fprintf(out, "\t\t\"Baudrate: %u\n", comSetReply->comSetReply.baudrate);

    fprintf(out, "\t}\n");      
}


/**
 * @brief Render an OSDP MFGREP reply.
 *
 * Displays a manufacturer specific reply, 3-bytes vendor code and specific reply data.
 *
 * @param out    Output file stream.
 * @param mfgRep Pointer to decoded OSDP reply structure.
 * @param len payload length in bytes.
 */
static void print_json_osdp_MFGREP(FILE *out, const osdp_reply *mfgRep, int len)
{
    fprintf(out, "\t\"Data\" : {\n");
    fprintf(out, "\t\t\"VendorCode1st\": %02X,\n",mfgRep->mfgReply.vendorCode1st);
    fprintf(out, "\t\t\"VendorCode2nd:\" %02X,\n",mfgRep->mfgReply.vendorCode2nd);
    fprintf(out, "\t\t\"VendorCode3rd:\" %02X,\n",mfgRep->mfgReply.vendorCode3rd);

    int specCmdLen = len - 3;
    fprintf(out, "\t\t\"VendorData\" : ");

    if(specCmdLen > 0){
        for(int i=0; i < specCmdLen; i++)
        {
            (i + 1 == specCmdLen)?    fprintf(out, "%02X\n", mfgRep->mfgReply.mfgData[i]) : fprintf(out, "%02X", mfgRep->mfgReply.mfgData[i]);
        }
    }
    else
        fprintf(out, "no data\n");
    fprintf(out, "\t}\n");        
}


/* 
 * End of OSDP reply rendering functions */


 /**
 * @brief Prints a decoded OSDP command.
 *
 * Generates a human-readable representation of the specified OSDP command and writes the result to the provided output stream.
 *
 * Depending on the command identifier, the function displays the command-specific fields and associated payload data.
 *
 * @param out Output stream receiving the formatted command.
 * @param cmd Pointer to the decoded OSDP command structure.
 * @param len Length of the decoded command payload in bytes.
 */
void print_osdp_cmd_json(FILE *out, const osdp_cmd *cmd, size_t len){

    switch(cmd->cmdId)
    {
        case CMD_CHLNG:
            print_json_osdp_CHLNG(out, cmd);
        break;

        case CMD_SCRYPT:
            print_json_osdp_SCRYPT(out, cmd);
        break;

        case CMD_POLL:
            print_json_osdp_POLL(out, cmd);
        break;

        case CMD_ID:
            print_json_osdp_ID(out, cmd);
        break;

        case CMD_CAP:
            print_json_osdp_CAP(out, cmd);
        break;

        case CMD_OUT:
            print_json_osdp_OUT(out, cmd);
        break;

        case CMD_LED:
            print_json_osdp_LED(out, cmd);
        break;
        
        case CMD_BUZ:
            print_json_osdp_BUZ(out, cmd);
        break;

        case CMD_TEXT:
            print_json_osdp_TEXT(out, cmd, len);
        break;

        case CMD_COMSET:
            print_json_osdp_COMSET(out, cmd);
        break;

        case CMD_KEYSET:
            print_json_osdp_KEYSET(out, cmd);
        break;

        case CMD_FILETRANSFER:
            print_json_osdp_FILETRANSFER(out, cmd, len);
        break;

        case CMD_MFG:
            print_json_osdp_MFG(out, cmd, len);
        break;        

        default:// UNKNOWN
        break;
    }
}


/**
 * @brief Prints a decoded OSDP reply.
 *
 * Generates a human-readable representation of the specified OSDP reply and writes the result to the provided output stream.
 *
 * Depending on the reply identifier, the function displays the reply-specific fields and associated payload data.
 *
 * @param out Output stream receiving the formatted reply.
 * @param reply Pointer to the decoded OSDP reply structure.
 * @param len Length of the decoded reply payload in bytes.
 */
void print_osdp_reply_json(FILE *out, const osdp_reply *reply, size_t len){
    switch(reply->replyId)
            {
                case REPLY_CCRYPT:
                    print_json_osdp_CCRYPT(out, reply);
                break;

                case REPLY_RMAC_I:
                    print_json_osdp_RMAC_I(out, reply);
                break;

                case REPLY_ACK:
                    print_json_osdp_ACK(out, reply);
                break;

                case REPLY_NAK:
                    print_json_osdp_NAK(out, reply);    
                break;

                case REPLY_PDID:
                    print_json_osdp_PDID(out, reply);
                break;

                case REPLY_PDCAP:
                    print_json_osdp_PDCAP(out, reply, len -1); // -1 for replyId
                break;

                case REPLY_LSTATR:
                    print_json_osdp_LSTATR(out, reply);
                break;
                case REPLY_ISTATR:
                    print_json_osdp_ISTATR(out, reply);
                break;

                case REPLY_OSTATR:
                    print_json_osdp_OSTATR(out, reply);
                break;
                case REPLY_RSTATR:
                    print_json_osdp_RSTATR(out, reply);
                break;

                case REPLY_RAW:
                    print_json_osdp_RAW(out, reply);
                break;

                case REPLY_FTSTAT:
                    print_json_osdp_FTSTAT(out, reply);
                break;

                case REPLY_KEYPAD:
                    print_json_osdp_KEYPAD(out, reply);
                break;

                case REPLY_COM:
                    print_json_osdp_COM(out, reply);
                break;

                case REPLY_MFGREP:
                    print_json_osdp_MFGREP(out, reply, len);
                break;                

                default:
                break;
            }
}