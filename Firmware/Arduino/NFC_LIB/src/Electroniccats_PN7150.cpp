/**
 * NXP PN7150 Driver 
 * Porting authors: 
 *        Salvador Mendoza - @Netxing - salmg.net
 *        Andres Sabas - Electronic Cats - Electroniccats.com
 *
 *  November 2020
 * 
 * This code is beerware; if you see me (or any other collaborator 
 * member) at the local, and you've found our code helpful, 
 * please buy us a round!
 * Distributed as-is; no warranty is given.
 *
 * Some methods and ideas were extract from https://github.com/Strooom/PN7150
 *
 *
 */
#include "Electroniccats_PN7150.h"
#include "P2P_NDEF.h"
#include "ndef_helper.h"
#include "RW_NDEF.h"
#include "T4T_NDEF_emu.h"

uint8_t gNextTag_Protocol = PROT_UNDETERMINED;

uint8_t NCIStartDiscovery_length = 0;
uint8_t NCIStartDiscovery[30];

unsigned char DiscoveryTechnologiesCE[] = { //Emulation
    MODE_LISTEN | MODE_POLL};

unsigned char DiscoveryTechnologiesRW[] = { //Read & Write
    MODE_POLL | TECH_PASSIVE_NFCA,
    MODE_POLL | TECH_PASSIVE_NFCF,
    MODE_POLL | TECH_PASSIVE_NFCB,
    MODE_POLL | TECH_PASSIVE_15693};

unsigned char DiscoveryTechnologiesP2P[] = { //P2P
    MODE_POLL | TECH_PASSIVE_NFCA,
    MODE_POLL | TECH_PASSIVE_NFCF,

    /* Only one POLL ACTIVE mode can be enabled, if both are defined only NFCF applies */
    MODE_POLL | TECH_ACTIVE_NFCA,
    //MODE_POLL | TECH_ACTIVE_NFCF,

    //MODE_LISTEN | TECH_PASSIVE_NFCA,

    MODE_LISTEN | TECH_PASSIVE_NFCF,
    MODE_LISTEN | TECH_ACTIVE_NFCA,
    MODE_LISTEN | TECH_ACTIVE_NFCF};

Electroniccats_PN7150::Electroniccats_PN7150(uint8_t IRQpin, uint8_t VENpin, uint8_t I2Caddress) : _IRQpin(IRQpin),
                                                                                                   _VENpin(VENpin),
                                                                                                   _I2Caddress(I2Caddress)
{
    pinMode(_IRQpin, INPUT);
    pinMode(_VENpin, OUTPUT);
}

uint8_t Electroniccats_PN7150::begin()
{
    Wire.begin(21,22);
    digitalWrite(_VENpin, HIGH);
    delay(1);
    digitalWrite(_VENpin, LOW);
    delay(1);
    digitalWrite(_VENpin, HIGH);
    delay(3);
    return SUCCESS;
}

bool Electroniccats_PN7150::hasMessage() const
{
    return (HIGH == digitalRead(_IRQpin)); // PN7150 indicates it has data by driving IRQ signal HIGH
}

uint8_t Electroniccats_PN7150::writeData(uint8_t txBuffer[], uint32_t txBufferLevel) const
{
    uint32_t nmbrBytesWritten = 0;
    Wire.beginTransmission((uint8_t)_I2Caddress);
    nmbrBytesWritten = Wire.write(txBuffer, txBufferLevel);
    if (nmbrBytesWritten == txBufferLevel)
    {
        byte resultCode;
        resultCode = Wire.endTransmission();
        return resultCode;
    }
    else
    {
        return 4; // Could not properly copy data to I2C buffer, so treat as other error, see i2c_t3
    }
}

uint32_t Electroniccats_PN7150::readData(uint8_t rxBuffer[]) const
{
    uint32_t bytesReceived; // keeps track of how many bytes we actually received
    if (hasMessage())
    {                                                              // only try to read something if the PN7150 indicates it has something
        bytesReceived = Wire.requestFrom(_I2Caddress, (uint8_t)3); // first reading the header, as this contains how long the payload will be

        rxBuffer[0] = Wire.read();
        rxBuffer[1] = Wire.read();
        rxBuffer[2] = Wire.read();
        uint8_t payloadLength = rxBuffer[2];
        if (payloadLength > 0)
        {
            bytesReceived += Wire.requestFrom(_I2Caddress, (uint8_t)payloadLength); // then reading the payload, if any
            uint32_t index = 3;
            while (index < bytesReceived)
            {
                rxBuffer[index] = Wire.read();
                index++;
            }
            index = 0;
        }
    }
    else
    {
        bytesReceived = 0;
    }
    return bytesReceived;
}

bool Electroniccats_PN7150::isTimeOut() const
{
    return ((millis() - timeOutStartTime) >= timeOut);
}

void Electroniccats_PN7150::setTimeOut(unsigned long theTimeOut)
{
    timeOutStartTime = millis();
    timeOut = theTimeOut;
}

bool Electroniccats_PN7150::getMessage(uint16_t timeout)
{ // check for message using timeout, 5 milisec as default
    setTimeOut(timeout);
    rxMessageLength = 0;
    while (!isTimeOut())
    {
        rxMessageLength = readData(rxBuffer);
        if (rxMessageLength)
            break;
        else if (timeout == 1337)
            setTimeOut(timeout);
    }
    return rxMessageLength;
}

uint8_t Electroniccats_PN7150::wakeupNCI()
{ // the device has to wake up using a core reset
    uint8_t NCICoreReset[] = {0x20, 0x00, 0x01, 0x01};
    uint16_t NbBytes = 0;

    // Reset RF settings restauration flag
    (void)writeData(NCICoreReset, 4);
    getMessage(15);
    NbBytes = rxMessageLength;
    if ((NbBytes == 0) || (rxBuffer[0] != 0x40) || (rxBuffer[1] != 0x00))
    {
        return ERROR;
    }
    getMessage();
    NbBytes = rxMessageLength;
    if (NbBytes != 0)
    {
        //NCI_PRINT_BUF("NCI << ", Answer, NbBytes);
        // Is CORE_GENERIC_ERROR_NTF ?
        if ((rxBuffer[0] == 0x60) && (rxBuffer[1] == 0x07))
        {
            /* Is PN7150B0HN/C11004 Anti-tearing recovery procedure triggered ? */
            //if ((rxBuffer[3] == 0xE6)) gRfSettingsRestored_flag = true;
        }
        else
        {
            return ERROR;
        }
    }
    return SUCCESS;
}

uint8_t Electroniccats_PN7150::connectNCI()
{
    uint8_t i = 2;
    uint8_t NCICoreInit[] = {0x20, 0x01, 0x00};

    // Open connection to NXPNCI
    begin();
    // Loop until NXPNCI answers
    while (wakeupNCI() != SUCCESS)
    {
        if (i-- == 0)
            return ERROR;
        delay(500);
    }

    (void)writeData(NCICoreInit, sizeof(NCICoreInit));
    getMessage();
    if ((rxBuffer[0] != 0x40) || (rxBuffer[1] != 0x01) || (rxBuffer[3] != 0x00))
        return ERROR;

    // Retrieve NXP-NCI NFC Controller generation
    if (rxBuffer[17 + rxBuffer[8]] == 0x08)
        gNfcController_generation = 1;
    else if (rxBuffer[17 + rxBuffer[8]] == 0x10)
        gNfcController_generation = 2;

    // Retrieve NXP-NCI NFC Controller FW version
    gNfcController_fw_version[0] = rxBuffer[17 + rxBuffer[8]]; //0xROM_CODE_V
    gNfcController_fw_version[1] = rxBuffer[18 + rxBuffer[8]]; //0xFW_MAJOR_NO
    gNfcController_fw_version[2] = rxBuffer[19 + rxBuffer[8]]; //0xFW_MINOR_NO
#ifdef SerialUSB
    Serial.println("0xROM_CODE_V: " + String(gNfcController_fw_version[0], HEX));
    Serial.println("FW_MAJOR_NO: " + String(gNfcController_fw_version[1], HEX));
    Serial.println("0xFW_MINOR_NO: " + String(gNfcController_fw_version[2], HEX));
    Serial.println("gNfcController_generation: " + String(gNfcController_generation, HEX));
#endif
    return SUCCESS;
}

int Electroniccats_PN7150::GetFwVersion()
{
    return ((gNfcController_fw_version[0] & 0xFF) << 16) | ((gNfcController_fw_version[1] & 0xFF) << 8) | (gNfcController_fw_version[2] & 0xFF);
}

uint8_t Electroniccats_PN7150::ConfigMode(uint8_t modeSE)
{
    unsigned mode = (modeSE == 1 ? MODE_RW : modeSE == 2 ? MODE_CARDEMU
                                                         : MODE_P2P);

    uint8_t Command[MAX_NCI_FRAME_SIZE];

    uint8_t Item = 0;
    uint8_t NCIDiscoverMap[] = {0x21, 0x00};

    //Emulation mode
    const uint8_t DM_CARDEMU[] = {0x4, 0x2, 0x2};
    const uint8_t R_CARDEMU[] = {0x1, 0x3, 0x0, 0x1, 0x4};

    //RW Mode
    const uint8_t DM_RW[] = {0x1, 0x1, 0x1, 0x2, 0x1, 0x1, 0x3, 0x1, 0x1, 0x4, 0x1, 0x2, 0x80, 0x01, 0x80};
    uint8_t NCIPropAct[] = {0x2F, 0x02, 0x00};

    //P2P Support
    const uint8_t DM_P2P[] = {0x5, 0x3, 0x3};
    const uint8_t R_P2P[] = {0x1, 0x3, 0x0, 0x1, 0x5};
    uint8_t NCISetConfig_NFC[] = {0x20, 0x02, 0x1F, 0x02, 0x29, 0x0D, 0x46, 0x66, 0x6D, 0x01, 0x01, 0x11, 0x03, 0x02, 0x00, 0x01, 0x04, 0x01, 0xFA, 0x61, 0x0D, 0x46, 0x66, 0x6D, 0x01, 0x01, 0x11, 0x03, 0x02, 0x00, 0x01, 0x04, 0x01, 0xFA};

    uint8_t NCIRouting[] = {0x21, 0x01, 0x07, 0x00, 0x01};
    uint8_t NCISetConfig_NFCA_SELRSP[] = {0x20, 0x02, 0x04, 0x01, 0x32, 0x01, 0x00};

    if (mode == 0)
        return SUCCESS;

    /* Enable Proprietary interface for T4T card presence check procedure */
    if (modeSE == 1)
    {
        if (mode == MODE_RW)
        {
            (void)writeData(NCIPropAct, sizeof(NCIPropAct));
            getMessage();

            if ((rxBuffer[0] != 0x4F) || (rxBuffer[1] != 0x02) || (rxBuffer[3] != 0x00))
                return ERROR;
        }
    }

    //* Building Discovery Map command
    Item = 0;

    if ((mode & MODE_CARDEMU and modeSE == 2) || (mode & MODE_P2P and modeSE == 3))
    {
        memcpy(&Command[4 + (3 * Item)], (modeSE == 2 ? DM_CARDEMU : DM_P2P), sizeof((modeSE == 2 ? DM_CARDEMU : DM_P2P)));
        Item++;
    }
    if (mode & MODE_RW and modeSE == 1)
    {
        memcpy(&Command[4 + (3 * Item)], DM_RW, sizeof(DM_RW));
        Item += sizeof(DM_RW) / 3;
    }
    if (Item != 0)
    {
        memcpy(Command, NCIDiscoverMap, sizeof(NCIDiscoverMap));
        Command[2] = 1 + (Item * 3);
        Command[3] = Item;
        (void)writeData(Command, 3 + Command[2]);
        getMessage(10);
        if ((rxBuffer[0] != 0x41) || (rxBuffer[1] != 0x00) || (rxBuffer[3] != 0x00))
        {
            return ERROR;
        }
    }

    // Configuring routing
    Item = 0;

    if (modeSE == 2 || modeSE == 3)
    { //Emulation or P2P
        memcpy(&Command[5 + (5 * Item)], (modeSE == 2 ? R_CARDEMU : R_P2P), sizeof((modeSE == 2 ? R_CARDEMU : R_P2P)));
        Item++;

        if (Item != 0)
        {
            memcpy(Command, NCIRouting, sizeof(NCIRouting));
            Command[2] = 2 + (Item * 5);
            Command[4] = Item;
            (void)writeData(Command, 3 + Command[2]);
            getMessage();
            if ((rxBuffer[0] != 0x41) || (rxBuffer[1] != 0x01) || (rxBuffer[3] != 0x00))
                return ERROR;
        }
        NCISetConfig_NFCA_SELRSP[6] += (modeSE == 2 ? 0x20 : 0x40);

        if (NCISetConfig_NFCA_SELRSP[6] != 0x00)
        {
            (void)writeData(NCISetConfig_NFCA_SELRSP, sizeof(NCISetConfig_NFCA_SELRSP));
            getMessage();

            if ((rxBuffer[0] != 0x40) || (rxBuffer[1] != 0x02) || (rxBuffer[3] != 0x00))
                return ERROR;
            else
                return SUCCESS;
        }

        if (mode & MODE_P2P and modeSE == 3)
        {
            (void)writeData(NCISetConfig_NFC, sizeof(NCISetConfig_NFC));
            getMessage();

            if ((rxBuffer[0] != 0x40) || (rxBuffer[1] != 0x02) || (rxBuffer[3] != 0x00))
                return ERROR;
        }
    }
    return SUCCESS;
}

uint8_t Electroniccats_PN7150::StartDiscovery(uint8_t modeSE)
{
    unsigned char TechTabSize = (modeSE == 1 ? sizeof(DiscoveryTechnologiesRW) : modeSE == 2 ? sizeof(DiscoveryTechnologiesCE)
                                                                                             : sizeof(DiscoveryTechnologiesP2P));

    NCIStartDiscovery_length = 0;
    NCIStartDiscovery[0] = 0x21;
    NCIStartDiscovery[1] = 0x03;
    NCIStartDiscovery[2] = (TechTabSize * 2) + 1;
    NCIStartDiscovery[3] = TechTabSize;
    for (uint8_t i = 0; i < TechTabSize; i++)
    {
        NCIStartDiscovery[(i * 2) + 4] = (modeSE == 1 ? DiscoveryTechnologiesRW[i] : modeSE == 2 ? DiscoveryTechnologiesCE[i]
                                                                                                 : DiscoveryTechnologiesP2P[i]);

        NCIStartDiscovery[(i * 2) + 5] = 0x01;
    }

    NCIStartDiscovery_length = (TechTabSize * 2) + 4;
    (void)writeData(NCIStartDiscovery, NCIStartDiscovery_length);
    getMessage();

    if ((rxBuffer[0] != 0x41) || (rxBuffer[1] != 0x03) || (rxBuffer[3] != 0x00))
        return ERROR;
    else
        return SUCCESS;
}

void Electroniccats_PN7150::ProcessCardMode(RfIntf_t RfIntf)
{
    uint8_t Answer[MAX_NCI_FRAME_SIZE];

    uint8_t NCIStopDiscovery[] = {0x21, 0x06, 0x01, 0x00};
    bool FirstCmd = true;

    /* Reset Card emulation state */
    T4T_NDEF_EMU_Reset();

    (void)writeData(NCIStartDiscovery, NCIStartDiscovery_length);
    getMessage(2000);
    //NxpNci_WaitForReception(Answer, sizeof(Answer), &AnswerSize, TIMEOUT_2S) == NXPNCI_SUCCESS

    while (rxMessageLength > 0)
    {
        /* is RF_DEACTIVATE_NTF ? */
        if ((rxBuffer[0] == 0x61) && (rxBuffer[1] == 0x06))
        {
            if (FirstCmd)
            {
                /* Restart the discovery loop */
                //NxpNci_HostTransceive(NCIStopDiscovery, sizeof(NCIStopDiscovery), Answer, sizeof(Answer), &AnswerSize);
                (void)writeData(NCIStartDiscovery, sizeof(NCIStopDiscovery));
                getMessage();
                do
                {
                    if ((rxBuffer[0] == 0x41) && (rxBuffer[1] == 0x06))
                        break;
                    //NxpNci_WaitForReception(Answer, sizeof(Answer), &AnswerSize, TIMEOUT_100MS);
                    (void)writeData(rxBuffer, rxMessageLength);
                    getMessage(100);
                } while (rxMessageLength != 0);
                //NxpNci_HostTransceive(NCIStartDiscovery, NCIStartDiscovery_length, Answer, sizeof(Answer), &AnswerSize);
                (void)writeData(NCIStartDiscovery, NCIStartDiscovery_length);
                getMessage();
            }
            /* Come back to discovery state */
            break;
        }
        /* is DATA_PACKET ? */
        else if ((rxBuffer[0] == 0x00) && (rxBuffer[1] == 0x00))
        {
            /* DATA_PACKET */
            uint8_t Cmd[MAX_NCI_FRAME_SIZE];
            uint16_t CmdSize;

            T4T_NDEF_EMU_Next(&rxBuffer[3], rxBuffer[2], &Cmd[3], (unsigned short *)&CmdSize);

            Cmd[0] = 0x00;
            Cmd[1] = (CmdSize & 0xFF00) >> 8;
            Cmd[2] = CmdSize & 0x00FF;

            //NxpNci_HostTransceive(Cmd, CmdSize+3, Answer, sizeof(Answer), &AnswerSize);
            (void)writeData(Cmd, CmdSize + 3);
            getMessage();
        }
        FirstCmd = false;
    }
}

bool Electroniccats_PN7150::CardModeSend(unsigned char *pData, unsigned char DataSize)
{
    bool status;
    uint8_t Cmd[MAX_NCI_FRAME_SIZE];

    /* Compute and send DATA_PACKET */
    Cmd[0] = 0x00;
    Cmd[1] = 0x00;
    Cmd[2] = DataSize;
    memcpy(&Cmd[3], pData, DataSize);
    (void)writeData(Cmd, DataSize + 3);
    return status;
}

bool Electroniccats_PN7150::CardModeReceive(unsigned char *pData, unsigned char *pDataSize)
{
    bool status = NFC_ERROR;
    uint8_t Ans[MAX_NCI_FRAME_SIZE];

    (void)writeData(Ans, sizeof(Ans));
    getMessage(2000);

    /* Is data packet ? */
    if ((rxBuffer[0] == 0x00) && (rxBuffer[1] == 0x00))
    {
#ifdef SerialUSB
        Serial.println(rxBuffer[2]);
#endif
        *pDataSize = rxBuffer[2];
        memcpy(pData, &rxBuffer[3], *pDataSize);
        status = NFC_SUCCESS;
    }
    else
    {
        status = NFC_ERROR;
    }
    return status;
}

void Electroniccats_PN7150::FillInterfaceInfo(RfIntf_t *pRfIntf, uint8_t *pBuf)
{
    uint8_t i, temp;

    switch (pRfIntf->ModeTech)
    {
    case (MODE_POLL | TECH_PASSIVE_NFCA):
        memcpy(pRfIntf->Info.NFC_APP.SensRes, &pBuf[0], 2);
        temp = 2;
        pRfIntf->Info.NFC_APP.NfcIdLen = pBuf[temp];
        temp++;
        memcpy(pRfIntf->Info.NFC_APP.NfcId, &pBuf[3], pRfIntf->Info.NFC_APP.NfcIdLen);
        temp += pBuf[2];
        pRfIntf->Info.NFC_APP.SelResLen = pBuf[temp];
        temp++;

        if (pRfIntf->Info.NFC_APP.SelResLen == 1)
            pRfIntf->Info.NFC_APP.SelRes[0] = pBuf[temp];

        temp += 4;
        if (pBuf[temp] != 0)
        {
            temp++;
            pRfIntf->Info.NFC_APP.RatsLen = pBuf[temp];
            memcpy(pRfIntf->Info.NFC_APP.Rats, &pBuf[temp + 1], pBuf[temp]);
        }
        else
        {
            pRfIntf->Info.NFC_APP.RatsLen = 0;
        }
        break;

    case (MODE_POLL | TECH_PASSIVE_NFCB):
        pRfIntf->Info.NFC_BPP.SensResLen = pBuf[0];
        memcpy(pRfIntf->Info.NFC_BPP.SensRes, &pBuf[1], pRfIntf->Info.NFC_BPP.SensResLen);
        temp = pBuf[0] + 4;
        if (pBuf[temp] != 0)
        {
            temp++;
            pRfIntf->Info.NFC_BPP.AttribResLen = pBuf[temp];
            memcpy(pRfIntf->Info.NFC_BPP.AttribRes, &pBuf[temp + 1], pBuf[temp]);
        }
        else
        {
            pRfIntf->Info.NFC_BPP.AttribResLen = 0;
        }
        break;

    case (MODE_POLL | TECH_PASSIVE_NFCF):
        pRfIntf->Info.NFC_FPP.BitRate = pBuf[0];
        pRfIntf->Info.NFC_FPP.SensResLen = pBuf[1];
        memcpy(pRfIntf->Info.NFC_FPP.SensRes, &pBuf[2], pRfIntf->Info.NFC_FPP.SensResLen);
        break;

    case (MODE_POLL | TECH_PASSIVE_15693):
        pRfIntf->Info.NFC_VPP.AFI = pBuf[0];
        pRfIntf->Info.NFC_VPP.DSFID = pBuf[1];

        for (i = 0; i < 8; i++)
            pRfIntf->Info.NFC_VPP.ID[7 - i] = pBuf[2 + i];

        break;

    default:
        break;
    }
}

bool Electroniccats_PN7150::ReaderTagCmd(unsigned char *pCommand, unsigned char CommandSize, unsigned char *pAnswer, unsigned char *pAnswerSize)
{
    bool status = ERROR;
    uint8_t Cmd[MAX_NCI_FRAME_SIZE];

    /* Compute and send DATA_PACKET */
    Cmd[0] = 0x00;
    Cmd[1] = 0x00;
    Cmd[2] = CommandSize;
    memcpy(&Cmd[3], pCommand, CommandSize);

    (void)writeData(Cmd, CommandSize + 3);
    getMessage();
    getMessage(1000);
    /* Wait for Answer 1S */

    if ((rxBuffer[0] == 0x0) && (rxBuffer[1] == 0x0))
        status = SUCCESS;

    *pAnswerSize = rxBuffer[2];
    memcpy(pAnswer, &rxBuffer[3], *pAnswerSize);

    return status;
}

bool Electroniccats_PN7150::WaitForDiscoveryNotification(RfIntf_t *pRfIntf, uint8_t tout)
{
    uint8_t NCIRfDiscoverSelect[] = {0x21, 0x04, 0x03, 0x01, PROT_ISODEP, INTF_ISODEP};

    //P2P Support
    uint8_t NCIStopDiscovery[] = {0x21, 0x06, 0x01, 0x00};
    uint8_t NCIRestartDiscovery[] = {0x21, 0x06, 0x01, 0x03};
    uint8_t saved_NTF[7];

    gNextTag_Protocol = PROT_UNDETERMINED;
wait:
    do
    {
        getMessage(
            tout > 0 ? tout : 1337
        ); //Infinite loop, waiting for response
    } while ((rxBuffer[0] != 0x61) || ((rxBuffer[1] != 0x05) && (rxBuffer[1] != 0x03)));

    gNextTag_Protocol = PROT_UNDETERMINED;

    /* Is RF_INTF_ACTIVATED_NTF ? */
    if (rxBuffer[1] == 0x05)
    {
        pRfIntf->Interface = rxBuffer[4];
        pRfIntf->Protocol = rxBuffer[5];
        pRfIntf->ModeTech = rxBuffer[6];
        pRfIntf->MoreTags = false;
        FillInterfaceInfo(pRfIntf, &rxBuffer[10]);

        //P2P
        /* Verifying if not a P2P device also presenting T4T emulation */
        if ((pRfIntf->Interface == INTF_ISODEP) && (pRfIntf->Protocol == PROT_ISODEP) && ((pRfIntf->ModeTech & MODE_LISTEN) != MODE_LISTEN))
        {
            memcpy(saved_NTF, rxBuffer, sizeof(saved_NTF));
            while (1)
            {
                /* Restart the discovery loop */
                (void)writeData(NCIRestartDiscovery, sizeof(NCIRestartDiscovery));
                getMessage();
                getMessage(100);
                /* Wait for discovery */
                do
                {
                    getMessage(1000); //Infinite loop, waiting for response
                } while ((rxMessageLength == 4) && (rxBuffer[0] == 0x60) && (rxBuffer[1] == 0x07));

                if ((rxMessageLength != 0) && (rxBuffer[0] == 0x61) && (rxBuffer[1] == 0x05))
                {
                    /* Is same device detected ? */
                    if (memcmp(saved_NTF, rxBuffer, sizeof(saved_NTF)) == 0)
                        break;
                    /* Is P2P detected ? */
                    if (rxBuffer[5] == PROT_NFCDEP)
                    {
                        pRfIntf->Interface = rxBuffer[4];
                        pRfIntf->Protocol = rxBuffer[5];
                        pRfIntf->ModeTech = rxBuffer[6];
                        pRfIntf->MoreTags = false;
                        FillInterfaceInfo(pRfIntf, &rxBuffer[10]);
                        break;
                    }
                }
                else
                {
                    if (rxMessageLength != 0)
                    {
                        /* Flush any other notification  */
                        while (rxMessageLength != 0)
                            getMessage(100);

                        /* Restart the discovery loop */
                        (void)writeData(NCIRestartDiscovery, sizeof(NCIRestartDiscovery));
                        getMessage();
                        getMessage(100);
                    }
                    goto wait;
                }
            }
        }
    }
    else
    { /* RF_DISCOVER_NTF */
        pRfIntf->Interface = INTF_UNDETERMINED;
        pRfIntf->Protocol = rxBuffer[4];
        pRfIntf->ModeTech = rxBuffer[5];
        pRfIntf->MoreTags = true;

        /* Get next NTF for further activation */
        do
        {
            if (!getMessage(100))
                return ERROR;
        } while ((rxBuffer[0] != 0x61) || (rxBuffer[1] != 0x03));
        gNextTag_Protocol = rxBuffer[4];

        /* Remaining NTF ? */

        while (rxBuffer[rxMessageLength - 1] == 0x02)
            getMessage(100);

        /* In case of multiple cards, select the first one */
        NCIRfDiscoverSelect[4] = pRfIntf->Protocol;
        if (pRfIntf->Protocol == PROT_ISODEP)
            NCIRfDiscoverSelect[5] = INTF_ISODEP;
        else if (pRfIntf->Protocol == PROT_NFCDEP)
            NCIRfDiscoverSelect[5] = INTF_NFCDEP;
        else if (pRfIntf->Protocol == PROT_MIFARE)
            NCIRfDiscoverSelect[5] = INTF_TAGCMD;
        else
            NCIRfDiscoverSelect[5] = INTF_FRAME;

        (void)writeData(NCIRfDiscoverSelect, sizeof(NCIRfDiscoverSelect));
        getMessage(100);

        if ((rxBuffer[0] == 0x41) || (rxBuffer[1] == 0x04) || (rxBuffer[3] == 0x00))
        {
            (void)writeData(rxBuffer, rxMessageLength);
            getMessage(100);

            if ((rxBuffer[0] == 0x61) || (rxBuffer[1] == 0x05))
            {
                pRfIntf->Interface = rxBuffer[4];
                pRfIntf->Protocol = rxBuffer[5];
                pRfIntf->ModeTech = rxBuffer[6];
                FillInterfaceInfo(pRfIntf, &rxBuffer[10]);
            }

            /* In case of P2P target detected but lost, inform application to restart discovery */
            else if (pRfIntf->Protocol == PROT_NFCDEP)
            {
                /* Restart the discovery loop */
                (void)writeData(NCIStopDiscovery, sizeof(NCIStopDiscovery));
                getMessage();
                getMessage(100);

                (void)writeData(NCIStartDiscovery, NCIStartDiscovery_length);
                getMessage();

                goto wait;
            }
        }
    }

    /* In case of unknown target align protocol information */
    if (pRfIntf->Interface == INTF_UNDETERMINED)
        pRfIntf->Protocol = PROT_UNDETERMINED;

    return SUCCESS;
}

void Electroniccats_PN7150::ProcessP2pMode(RfIntf_t RfIntf)
{
    uint8_t status = ERROR;
    bool restart = false;
    uint8_t NCILlcpSymm[] = {0x00, 0x00, 0x02, 0x00, 0x00};
    uint8_t NCIRestartDiscovery[] = {0x21, 0x06, 0x01, 0x03};

    /* Reset P2P_NDEF state */
    P2P_NDEF_Reset();

    /* Is Initiator mode ? */
    if ((RfIntf.ModeTech & MODE_LISTEN) != MODE_LISTEN)
    {
        /* Initiate communication (SYMM PDU) */
        (void)writeData(NCILlcpSymm, sizeof(NCILlcpSymm));
        getMessage();

        /* Save status for discovery restart */
        restart = true;
    }
    status = ERROR;
    getMessage(2000);
    if (rxMessageLength > 0)
        status = SUCCESS;

    /* Get frame from remote peer */
    while (status == SUCCESS)
    {
        /* is DATA_PACKET ? */
        if ((rxBuffer[0] == 0x00) && (rxBuffer[1] == 0x00))
        {
            uint8_t Cmd[MAX_NCI_FRAME_SIZE];
            uint16_t CmdSize;
            /* Handle P2P communication */
            P2P_NDEF_Next(&rxBuffer[3], rxBuffer[2], &Cmd[3], (unsigned short *)&CmdSize);
            /* Compute DATA_PACKET to answer */
            Cmd[0] = 0x00;
            Cmd[1] = (CmdSize & 0xFF00) >> 8;
            Cmd[2] = CmdSize & 0x00FF;
            status = ERROR;
            (void)writeData(Cmd, CmdSize + 3);
            getMessage();
            if (rxMessageLength > 0)
                status = SUCCESS;
        }
        /* is CORE_INTERFACE_ERROR_NTF ?*/
        else if ((rxBuffer[0] == 0x60) && (rxBuffer[1] == 0x08))
        {
            /* Come back to discovery state */
            break;
        }
        /* is RF_DEACTIVATE_NTF ? */
        else if ((rxBuffer[0] == 0x61) && (rxBuffer[1] == 0x06))
        {
            /* Come back to discovery state */
            break;
        }
        /* is RF_DISCOVERY_NTF ? */
        else if ((rxBuffer[0] == 0x61) && ((rxBuffer[1] == 0x05) || (rxBuffer[1] == 0x03)))
        {
            do
            {
                if ((rxBuffer[0] == 0x61) && ((rxBuffer[1] == 0x05) || (rxBuffer[1] == 0x03)))
                {
                    if ((rxBuffer[6] & MODE_LISTEN) != MODE_LISTEN)
                        restart = true;
                    else
                        restart = false;
                }
                status = ERROR;
                (void)writeData(rxBuffer, rxMessageLength);
                getMessage();
                if (rxMessageLength > 0)
                    status = SUCCESS;
            } while (rxMessageLength != 0);
            /* Come back to discovery state */
            break;
        }

        /* Wait for next frame from remote P2P, or notification event */
        status = ERROR;
        (void)writeData(rxBuffer, rxMessageLength);
        getMessage();
        if (rxMessageLength > 0)
            status = SUCCESS;
    }

    /* Is Initiator mode ? */
    if (restart)
    {
        /* Communication ended, restart discovery loop */
        (void)writeData(NCIRestartDiscovery, sizeof(NCIRestartDiscovery));
        getMessage();
        getMessage(100);
    }
}

void Electroniccats_PN7150::ReadNdef(RfIntf_t RfIntf)
{
    uint8_t Cmd[MAX_NCI_FRAME_SIZE];
    uint16_t CmdSize = 0;

    RW_NDEF_Reset(RfIntf.Protocol);

    while (1)
    {
        RW_NDEF_Read_Next(&rxBuffer[3], rxBuffer[2], &Cmd[3], (unsigned short *)&CmdSize);
        if (CmdSize == 0)
        {
            /// End of the Read operation
            break;
        }
        else
        {
            // Compute and send DATA_PACKET
            Cmd[0] = 0x00;
            Cmd[1] = (CmdSize & 0xFF00) >> 8;
            Cmd[2] = CmdSize & 0x00FF;

            (void)writeData(Cmd, CmdSize + 3);
            getMessage();
            getMessage(1000);

            // Manage chaining in case of T4T
            if ((RfIntf.Interface = INTF_ISODEP) && rxBuffer[0] == 0x10)
            {
                uint8_t tmp[MAX_NCI_FRAME_SIZE];
                uint8_t tmpSize = 0;
                while (rxBuffer[0] == 0x10)
                {
                    memcpy(&tmp[tmpSize], &rxBuffer[3], rxBuffer[2]);
                    tmpSize += rxBuffer[2];
                    getMessage(100);
                }
                memcpy(&tmp[tmpSize], &rxBuffer[3], rxBuffer[2]);
                tmpSize += rxBuffer[2];
                //* Compute all chained frame into one unique answer
                memcpy(&rxBuffer[3], tmp, tmpSize);
                rxBuffer[2] = tmpSize;
            }
        }
    }
}

void Electroniccats_PN7150::WriteNdef(RfIntf_t RfIntf)
{

    uint8_t Cmd[MAX_NCI_FRAME_SIZE];
    uint16_t CmdSize = 0;

    RW_NDEF_Reset(RfIntf.Protocol);

    while (1)
    {
        RW_NDEF_Write_Next(&rxBuffer[3], rxBuffer[2], &Cmd[3], (unsigned short *)&CmdSize);
        if (CmdSize == 0)
        {
            // End of the Write operation
            break;
        }
        else
        {
            // Compute and send DATA_PACKET
            Cmd[0] = 0x00;
            Cmd[1] = (CmdSize & 0xFF00) >> 8;
            Cmd[2] = CmdSize & 0x00FF;

            (void)writeData(Cmd, CmdSize + 3);
            getMessage();
            getMessage(2000);
        }
    }
}

void Electroniccats_PN7150::ProcessReaderMode(RfIntf_t RfIntf, RW_Operation_t Operation)
{
    switch (Operation)
    {
    case READ_NDEF:
        ReadNdef(RfIntf);
        break;
    case WRITE_NDEF:
        WriteNdef(RfIntf);
        break;
    case PRESENCE_CHECK:
        PresenceCheck(RfIntf);
        break;
    default:
        break;
    }
}

void Electroniccats_PN7150::PresenceCheck(RfIntf_t RfIntf)
{
    bool status;
    uint8_t i;

    uint8_t NCIPresCheckT1T[] = {0x00, 0x00, 0x07, 0x78, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
    uint8_t NCIPresCheckT2T[] = {0x00, 0x00, 0x02, 0x30, 0x00};
    uint8_t NCIPresCheckT3T[] = {0x21, 0x08, 0x04, 0xFF, 0xFF, 0x00, 0x01};
    uint8_t NCIPresCheckIsoDep[] = {0x2F, 0x11, 0x00};
    uint8_t NCIPresCheckIso15693[] = {0x00, 0x00, 0x0B, 0x26, 0x01, 0x40, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
    uint8_t NCIDeactivate[] = {0x21, 0x06, 0x01, 0x01};
    uint8_t NCISelectMIFARE[] = {0x21, 0x04, 0x03, 0x01, 0x80, 0x80};

    switch (RfIntf.Protocol)
    {
    case PROT_T1T:
        do
        {
            delay(500);
            (void)writeData(NCIPresCheckT1T, sizeof(NCIPresCheckT1T));
            getMessage();
            getMessage(100);
        } while ((rxBuffer[0] == 0x00) && (rxBuffer[1] == 0x00));
        break;

    case PROT_T2T:
        do
        {
            delay(500);
            (void)writeData(NCIPresCheckT2T, sizeof(NCIPresCheckT2T));
            getMessage();
            getMessage(100);
        } while ((rxBuffer[0] == 0x00) && (rxBuffer[1] == 0x00) && (rxBuffer[2] == 0x11));
        break;

    case PROT_T3T:
        do
        {
            delay(500);
            (void)writeData(NCIPresCheckT3T, sizeof(NCIPresCheckT3T));
            getMessage();
            getMessage(100);
        } while ((rxBuffer[0] == 0x61) && (rxBuffer[1] == 0x08) && ((rxBuffer[3] == 0x00) || (rxBuffer[4] > 0x00)));
        break;

    case PROT_ISODEP:
        do
        {
            delay(500);
            (void)writeData(NCIPresCheckIsoDep, sizeof(NCIPresCheckIsoDep));
            getMessage();
            getMessage(100);
        } while ((rxBuffer[0] == 0x6F) && (rxBuffer[1] == 0x11) && (rxBuffer[2] == 0x01) && (rxBuffer[3] == 0x01));
        break;

    case PROT_ISO15693:
        do
        {
            delay(500);
            for (i = 0; i < 8; i++)
                NCIPresCheckIso15693[i + 6] = RfIntf.Info.NFC_VPP.ID[7 - i];
            (void)writeData(NCIPresCheckIso15693, sizeof(NCIPresCheckIso15693));
            getMessage();
            getMessage(100);
            status = ERROR;
            if (rxMessageLength)
                status = SUCCESS;
        } while ((status == SUCCESS) && (rxBuffer[0] == 0x00) && (rxBuffer[1] == 0x00) && (rxBuffer[rxMessageLength - 1] == 0x00));
        break;

    case PROT_MIFARE:
        do
        {
            delay(500);
            /* Deactivate target */
            (void)writeData(NCIDeactivate, sizeof(NCIDeactivate));
            getMessage();
            getMessage(100);

            /* Reactivate target */
            (void)writeData(NCISelectMIFARE, sizeof(NCISelectMIFARE));
            getMessage();
            getMessage(100);
        } while ((rxBuffer[0] == 0x61) && (rxBuffer[1] == 0x05));
        break;

    default:
        /* Nothing to do */
        break;
    }
}

bool Electroniccats_PN7150::ReaderReActivate(RfIntf_t *pRfIntf)
{
    uint8_t NCIDeactivate[] = {0x21, 0x06, 0x01, 0x01};
    uint8_t NCIActivate[] = {0x21, 0x04, 0x03, 0x01, 0x00, 0x00};

    /* First de-activate the target */
    (void)writeData(NCIDeactivate, sizeof(NCIDeactivate));
    getMessage();
    getMessage(100);

    /* Then re-activate the target */
    NCIActivate[4] = pRfIntf->Protocol;
    NCIActivate[5] = pRfIntf->Interface;

    (void)writeData(NCIDeactivate, sizeof(NCIDeactivate));
    getMessage();
    getMessage(100);

    if ((rxBuffer[0] != 0x61) || (rxBuffer[1] != 0x05))
        return ERROR;
    return SUCCESS;
}

bool Electroniccats_PN7150::ReaderActivateNext(RfIntf_t *pRfIntf)
{
    uint8_t NCIStopDiscovery[] = {0x21, 0x06, 0x01, 0x01};
    uint8_t NCIRfDiscoverSelect[] = {0x21, 0x04, 0x03, 0x02, PROT_ISODEP, INTF_ISODEP};

    bool status = ERROR;

    pRfIntf->MoreTags = false;

    if (gNextTag_Protocol == PROT_UNDETERMINED)
    {
        pRfIntf->Interface = INTF_UNDETERMINED;
        pRfIntf->Protocol = PROT_UNDETERMINED;
        return ERROR;
    }

    /* First disconnect current tag */
    (void)writeData(NCIStopDiscovery, sizeof(NCIStopDiscovery));
    getMessage();

    if ((rxBuffer[0] != 0x41) && (rxBuffer[1] != 0x06) && (rxBuffer[3] != 0x00))
        return ERROR;
    getMessage(100);

    if ((rxBuffer[0] != 0x61) && (rxBuffer[1] != 0x06))
        return ERROR;

    NCIRfDiscoverSelect[4] = gNextTag_Protocol;
    if (gNextTag_Protocol == PROT_ISODEP)
        NCIRfDiscoverSelect[5] = INTF_ISODEP;
    else if (gNextTag_Protocol == PROT_ISODEP)
        NCIRfDiscoverSelect[5] = INTF_NFCDEP;
    else if (gNextTag_Protocol == PROT_MIFARE)
        NCIRfDiscoverSelect[5] = INTF_TAGCMD;
    else
        NCIRfDiscoverSelect[5] = INTF_FRAME;

    (void)writeData(NCIRfDiscoverSelect, sizeof(NCIRfDiscoverSelect));
    getMessage();

    if ((rxBuffer[0] == 0x41) && (rxBuffer[1] == 0x04) && (rxBuffer[3] == 0x00))
    {
        getMessage(100);
        if ((rxBuffer[0] == 0x61) || (rxBuffer[1] == 0x05))
        {
            pRfIntf->Interface = rxBuffer[4];
            pRfIntf->Protocol = rxBuffer[5];
            pRfIntf->ModeTech = rxBuffer[6];
            FillInterfaceInfo(pRfIntf, &rxBuffer[10]);
            status = SUCCESS;
        }
    }

    return status;
}

bool Electroniccats_PN7150::StopDiscovery(void)
{
    uint8_t NCIStopDiscovery[] = {0x21, 0x06, 0x01, 0x00};

    (void)writeData(NCIStopDiscovery, sizeof(NCIStopDiscovery));
    getMessage();
    getMessage(1000);

    return SUCCESS;
}

bool Electroniccats_PN7150::ConfigureSettings(void)
{

#if NXP_CORE_CONF
    /* NCI standard dedicated settings
 * Refer to NFC Forum NCI standard for more details
 */
    uint8_t NxpNci_CORE_CONF[] = {
        0x20, 0x02, 0x05, 0x01, /* CORE_SET_CONFIG_CMD */
        0x00, 0x02, 0x00, 0x01  /* TOTAL_DURATION */
    };
#endif

#if NXP_CORE_CONF_EXTN
    /* NXP-NCI extension dedicated setting
 * Refer to NFC controller User Manual for more details
 */
    uint8_t NxpNci_CORE_CONF_EXTN[] = {
        0x20, 0x02, 0x0D, 0x03, /* CORE_SET_CONFIG_CMD */
        0xA0, 0x40, 0x01, 0x00, /* TAG_DETECTOR_CFG */
        0xA0, 0x41, 0x01, 0x04, /* TAG_DETECTOR_THRESHOLD_CFG */
        0xA0, 0x43, 0x01, 0x00  /* TAG_DETECTOR_FALLBACK_CNT_CFG */
    };
#endif

#if NXP_CORE_STANDBY
    /* NXP-NCI standby enable setting
 * Refer to NFC controller User Manual for more details
 */
    uint8_t NxpNci_CORE_STANDBY[] = {0x2F, 0x00, 0x01, 0x01}; /* last byte indicates enable/disable */
#endif

#if NXP_TVDD_CONF
    /* NXP-NCI TVDD configuration
 * Refer to NFC controller Hardware Design Guide document for more details
 */
    /* RF configuration related to 1st generation of NXP-NCI controller (e.g PN7120) */
    uint8_t NxpNci_TVDD_CONF_1stGen[] = {0x20, 0x02, 0x05, 0x01, 0xA0, 0x13, 0x01, 0x00};

    /* RF configuration related to 2nd generation of NXP-NCI controller (e.g PN7150)*/
#if (NXP_TVDD_CONF == 1)
    /* CFG1: Vbat is used to generate the VDD(TX) through TXLDO */
    uint8_t NxpNci_TVDD_CONF_2ndGen[] = {0x20, 0x02, 0x07, 0x01, 0xA0, 0x0E, 0x03, 0x02, 0x09, 0x00};
#else
    /* CFG2: external 5V is used to generate the VDD(TX) through TXLDO */
    uint8_t NxpNci_TVDD_CONF_2ndGen[] = {0x20, 0x02, 0x07, 0x01, 0xA0, 0x0E, 0x03, 0x06, 0x64, 0x00};
#endif
#endif

#if NXP_RF_CONF
    /* NXP-NCI RF configuration
 * Refer to NFC controller Antenna Design and Tuning Guidelines document for more details
 */
    /* RF configuration related to 1st generation of NXP-NCI controller (e.g PN7120) */
    /* Following configuration is the default settings of PN7120 NFC Controller */
    uint8_t NxpNci_RF_CONF_1stGen[] = {
        0x20, 0x02, 0x38, 0x07,
        0xA0, 0x0D, 0x06, 0x06, 0x42, 0x01, 0x00, 0xF1, 0xFF, /* RF_CLIF_CFG_TARGET          CLIF_ANA_TX_AMPLITUDE_REG */
        0xA0, 0x0D, 0x06, 0x06, 0x44, 0xA3, 0x90, 0x03, 0x00, /* RF_CLIF_CFG_TARGET          CLIF_ANA_RX_REG */
        0xA0, 0x0D, 0x06, 0x34, 0x2D, 0xDC, 0x50, 0x0C, 0x00, /* RF_CLIF_CFG_BR_106_I_RXA_P  CLIF_SIGPRO_RM_CONFIG1_REG */
        0xA0, 0x0D, 0x04, 0x06, 0x03, 0x00, 0x70,             /* RF_CLIF_CFG_TARGET          CLIF_TRANSCEIVE_CONTROL_REG */
        0xA0, 0x0D, 0x03, 0x06, 0x16, 0x00,                   /* RF_CLIF_CFG_TARGET          CLIF_TX_UNDERSHOOT_CONFIG_REG */
        0xA0, 0x0D, 0x03, 0x06, 0x15, 0x00,                   /* RF_CLIF_CFG_TARGET          CLIF_TX_OVERSHOOT_CONFIG_REG */
        0xA0, 0x0D, 0x06, 0x32, 0x4A, 0x53, 0x07, 0x01, 0x1B  /* RF_CLIF_CFG_BR_106_I_TXA    CLIF_ANA_TX_SHAPE_CONTROL_REG */
    };

    /* RF configuration related to 2nd generation of NXP-NCI controller (e.g PN7150)*/
    /* Following configuration relates to performance optimization of OM5578/PN7150 NFC Controller demo kit */
    uint8_t NxpNci_RF_CONF_2ndGen[] = {
        0x20, 0x02, 0x94, 0x11,
        0xA0, 0x0D, 0x06, 0x04, 0x35, 0x90, 0x01, 0xF4, 0x01, /* RF_CLIF_CFG_INITIATOR        CLIF_AGC_INPUT_REG */
        0xA0, 0x0D, 0x06, 0x06, 0x30, 0x01, 0x90, 0x03, 0x00, /* RF_CLIF_CFG_TARGET           CLIF_SIGPRO_ADCBCM_THRESHOLD_REG */
        0xA0, 0x0D, 0x06, 0x06, 0x42, 0x02, 0x00, 0xFF, 0xFF, /* RF_CLIF_CFG_TARGET           CLIF_ANA_TX_AMPLITUDE_REG */
        0xA0, 0x0D, 0x06, 0x20, 0x42, 0x88, 0x00, 0xFF, 0xFF, /* RF_CLIF_CFG_TECHNO_I_TX15693 CLIF_ANA_TX_AMPLITUDE_REG */
        0xA0, 0x0D, 0x04, 0x22, 0x44, 0x23, 0x00,             /* RF_CLIF_CFG_TECHNO_I_RX15693 CLIF_ANA_RX_REG */
        0xA0, 0x0D, 0x06, 0x22, 0x2D, 0x50, 0x34, 0x0C, 0x00, /* RF_CLIF_CFG_TECHNO_I_RX15693 CLIF_SIGPRO_RM_CONFIG1_REG */
        0xA0, 0x0D, 0x06, 0x32, 0x42, 0xF8, 0x00, 0xFF, 0xFF, /* RF_CLIF_CFG_BR_106_I_TXA     CLIF_ANA_TX_AMPLITUDE_REG */
        0xA0, 0x0D, 0x06, 0x34, 0x2D, 0x24, 0x37, 0x0C, 0x00, /* RF_CLIF_CFG_BR_106_I_RXA_P   CLIF_SIGPRO_RM_CONFIG1_REG */
        0xA0, 0x0D, 0x06, 0x34, 0x33, 0x86, 0x80, 0x00, 0x70, /* RF_CLIF_CFG_BR_106_I_RXA_P   CLIF_AGC_CONFIG0_REG */
        0xA0, 0x0D, 0x04, 0x34, 0x44, 0x22, 0x00,             /* RF_CLIF_CFG_BR_106_I_RXA_P   CLIF_ANA_RX_REG */
        0xA0, 0x0D, 0x06, 0x42, 0x2D, 0x15, 0x45, 0x0D, 0x00, /* RF_CLIF_CFG_BR_848_I_RXA     CLIF_SIGPRO_RM_CONFIG1_REG */
        0xA0, 0x0D, 0x04, 0x46, 0x44, 0x22, 0x00,             /* RF_CLIF_CFG_BR_106_I_RXB     CLIF_ANA_RX_REG */
        0xA0, 0x0D, 0x06, 0x46, 0x2D, 0x05, 0x59, 0x0E, 0x00, /* RF_CLIF_CFG_BR_106_I_RXB     CLIF_SIGPRO_RM_CONFIG1_REG */
        0xA0, 0x0D, 0x06, 0x44, 0x42, 0x88, 0x00, 0xFF, 0xFF, /* RF_CLIF_CFG_BR_106_I_TXB     CLIF_ANA_TX_AMPLITUDE_REG */
        0xA0, 0x0D, 0x06, 0x56, 0x2D, 0x05, 0x9F, 0x0C, 0x00, /* RF_CLIF_CFG_BR_212_I_RXF_P   CLIF_SIGPRO_RM_CONFIG1_REG */
        0xA0, 0x0D, 0x06, 0x54, 0x42, 0x88, 0x00, 0xFF, 0xFF, /* RF_CLIF_CFG_BR_212_I_TXF     CLIF_ANA_TX_AMPLITUDE_REG */
        0xA0, 0x0D, 0x06, 0x0A, 0x33, 0x80, 0x86, 0x00, 0x70  /* RF_CLIF_CFG_I_ACTIVE         CLIF_AGC_CONFIG0_REG */
    };
#endif

#if NXP_CLK_CONF
    /* NXP-NCI CLOCK configuration
 * Refer to NFC controller Hardware Design Guide document for more details
 */
#if (NXP_CLK_CONF == 1)
    /* Xtal configuration */
    uint8_t NxpNci_CLK_CONF[] = {
        0x20, 0x02, 0x05, 0x01, /* CORE_SET_CONFIG_CMD */
        0xA0, 0x03, 0x01, 0x08  /* CLOCK_SEL_CFG */
    };
#else
    /* PLL configuration */
    uint8_t NxpNci_CLK_CONF[] = {
        0x20, 0x02, 0x09, 0x02, /* CORE_SET_CONFIG_CMD */
        0xA0, 0x03, 0x01, 0x11, /* CLOCK_SEL_CFG */
        0xA0, 0x04, 0x01, 0x01  /* CLOCK_TO_CFG */
    };
#endif
#endif

    uint8_t NCICoreReset[] = {0x20, 0x00, 0x01, 0x00};
    uint8_t NCICoreInit[] = {0x20, 0x01, 0x00};
    bool gRfSettingsRestored_flag = false;

#if (NXP_TVDD_CONF | NXP_RF_CONF)
    uint8_t *NxpNci_CONF;
    uint16_t NxpNci_CONF_size = 0;
#endif
#if (NXP_CORE_CONF_EXTN | NXP_CLK_CONF | NXP_TVDD_CONF | NXP_RF_CONF)
    uint8_t currentTS[32] = __TIMESTAMP__;
    uint8_t NCIReadTS[] = {0x20, 0x03, 0x03, 0x01, 0xA0, 0x14};
    uint8_t NCIWriteTS[7 + 32] = {0x20, 0x02, 0x24, 0x01, 0xA0, 0x14, 0x20};
#endif
    bool isResetRequired = false;

    /* Apply settings */
#if NXP_CORE_CONF
    if (sizeof(NxpNci_CORE_CONF) != 0)
    {
        isResetRequired = true;
        (void)writeData(NxpNci_CORE_CONF, sizeof(NxpNci_CORE_CONF));
        getMessage();
        if ((rxBuffer[0] != 0x40) || (rxBuffer[1] != 0x02) || (rxBuffer[3] != 0x00) || (rxBuffer[4] != 0x00))
        {
#ifdef SerialUSB
            Serial.println("NxpNci_CORE_CONF");
#endif
            return ERROR;
        }
    }
#endif

#if NXP_CORE_STANDBY
    if (sizeof(NxpNci_CORE_STANDBY) != 0)
    {

        (void)(writeData(NxpNci_CORE_STANDBY, sizeof(NxpNci_CORE_STANDBY)));
        getMessage();
        if ((rxBuffer[0] != 0x4F) || (rxBuffer[1] != 0x00) || (rxBuffer[3] != 0x00))
        {
#ifdef SerialUSB
            Serial.println("NxpNci_CORE_STANDBY");
#endif
            return ERROR;
        }
    }
#endif

    /* All further settings are not versatile, so configuration only applied if there are changes (application build timestamp) 
       or in case of PN7150B0HN/C11004 Anti-tearing recovery procedure inducing RF setings were restored to their default value */
#if (NXP_CORE_CONF_EXTN | NXP_CLK_CONF | NXP_TVDD_CONF | NXP_RF_CONF)
    /* First read timestamp stored in NFC Controller */
    if (gNfcController_generation == 1)
        NCIReadTS[5] = 0x0F;
    (void)writeData(NCIReadTS, sizeof(NCIReadTS));
    getMessage();
    if ((rxBuffer[0] != 0x40) || (rxBuffer[1] != 0x03) || (rxBuffer[3] != 0x00))
    {
#ifdef SerialUSB
        Serial.println("read timestamp ");
#endif
        return ERROR;
    }
    /* Then compare with current build timestamp, and check RF setting restauration flag */
    /*if(!memcmp(&rxBuffer[8], currentTS, sizeof(currentTS)) && (gRfSettingsRestored_flag == false))
    {
        // No change, nothing to do
    }
    else
    {
        */
    /* Apply settings */
#if NXP_CORE_CONF_EXTN
    if (sizeof(NxpNci_CORE_CONF_EXTN) != 0)
    {
        (void)writeData(NxpNci_CORE_CONF_EXTN, sizeof(NxpNci_CORE_CONF_EXTN));
        getMessage();
        if ((rxBuffer[0] != 0x40) || (rxBuffer[1] != 0x02) || (rxBuffer[3] != 0x00) || (rxBuffer[4] != 0x00))
        {
#ifdef SerialUSB
            Serial.println("NxpNci_CORE_CONF_EXTN");
#endif
            return ERROR;
        }
    }
#endif

#if NXP_CLK_CONF
    if (sizeof(NxpNci_CLK_CONF) != 0)
    {
        isResetRequired = true;

        (void)writeData(NxpNci_CLK_CONF, sizeof(NxpNci_CLK_CONF));
        getMessage();
        //NxpNci_HostTransceive(NxpNci_CLK_CONF, sizeof(NxpNci_CLK_CONF), Answer, sizeof(Answer), &AnswerSize);
        if ((rxBuffer[0] != 0x40) || (rxBuffer[1] != 0x02) || (rxBuffer[3] != 0x00) || (rxBuffer[4] != 0x00))
        {
#ifdef SerialUSB
            Serial.println("NxpNci_CLK_CONF");
#endif
            return ERROR;
        }
    }
#endif

#if NXP_TVDD_CONF
    if (NxpNci_CONF_size != 0)
    {

        (void)writeData(NxpNci_TVDD_CONF_2ndGen, sizeof(NxpNci_TVDD_CONF_2ndGen));
        getMessage();
        if ((rxBuffer[0] != 0x40) || (rxBuffer[1] != 0x02) || (rxBuffer[3] != 0x00) || (rxBuffer[4] != 0x00))
        {
#ifdef SerialUSB
            Serial.println("NxpNci_CONF_size");
#endif
            return ERROR;
        }
    }
#endif

#if NXP_RF_CONF
    if (NxpNci_CONF_size != 0)
    {

        (void)writeData(NxpNci_RF_CONF_2ndGen, sizeof(NxpNci_RF_CONF_2ndGen));
        getMessage();
        if ((rxBuffer[0] != 0x40) || (rxBuffer[1] != 0x02) || (rxBuffer[3] != 0x00) || (rxBuffer[4] != 0x00))
        {
#ifdef SerialUSB
            Serial.println("NxpNci_CONF_size");
#endif
            return ERROR;
        }
    }
#endif
    /* Store curent timestamp to NFC Controller memory for further checks */
    if (gNfcController_generation == 1)
        NCIWriteTS[5] = 0x0F;
    memcpy(&NCIWriteTS[7], currentTS, sizeof(currentTS));
    (void)writeData(NCIWriteTS, sizeof(NCIWriteTS));
    getMessage();
    if ((rxBuffer[0] != 0x40) || (rxBuffer[1] != 0x02) || (rxBuffer[3] != 0x00) || (rxBuffer[4] != 0x00))
    {
#ifdef SerialUSB
        Serial.println("NFC Controller memory");
#endif
        return ERROR;
    }
    //}
#endif

    if (isResetRequired)
    {
        /* Reset the NFC Controller to insure new settings apply */
        (void)writeData(NCICoreReset, sizeof(NCICoreReset));
        getMessage();
        if ((rxBuffer[0] != 0x40) || (rxBuffer[1] != 0x00) || (rxBuffer[3] != 0x00))
        {
#ifdef SerialUSB
            Serial.println("insure new settings apply");
#endif
            return ERROR;
        }

        (void)writeData(NCICoreInit, sizeof(NCICoreInit));
        getMessage();
        if ((rxBuffer[0] != 0x40) || (rxBuffer[1] != 0x01) || (rxBuffer[3] != 0x00))
        {
#ifdef SerialUSB
            Serial.println("insure new settings apply 2");
#endif
            return ERROR;
        }
    }
    return SUCCESS;
}

//#if defined P2P_SUPPORT || defined RW_SUPPORT
void Electroniccats_PN7150::NdefPull_Cb(unsigned char *pNdefMessage, unsigned short NdefMessageSize)
{
    unsigned char *pNdefRecord = pNdefMessage;
    NdefRecord_t NdefRecord;
    unsigned char save;

    if (pNdefMessage == NULL)
    {
        Serial.println("--- Provisioned buffer size too small or NDEF message empty");
        return;
    }

    while (pNdefRecord != NULL)
    {
        Serial.println("--- NDEF record received:");

        NdefRecord = DetectNdefRecordType(pNdefRecord);

        switch (NdefRecord.recordType)
        {
        case MEDIA_VCARD:
        {
            save = NdefRecord.recordPayload[NdefRecord.recordPayloadSize];
            NdefRecord.recordPayload[NdefRecord.recordPayloadSize] = '\0';
            Serial.print("vCard:");
            //Serial.println(NdefRecord.recordPayload);
            NdefRecord.recordPayload[NdefRecord.recordPayloadSize] = save;
        }
        break;

        case WELL_KNOWN_SIMPLE_TEXT:
        {
            save = NdefRecord.recordPayload[NdefRecord.recordPayloadSize];
            NdefRecord.recordPayload[NdefRecord.recordPayloadSize] = '\0';
            Serial.print("Text record:");
            //Serial.println(&NdefRecord.recordPayload[NdefRecord.recordPayload[0]+1]);
            NdefRecord.recordPayload[NdefRecord.recordPayloadSize] = save;
        }
        break;

        case WELL_KNOWN_SIMPLE_URI:
        {
            save = NdefRecord.recordPayload[NdefRecord.recordPayloadSize];
            NdefRecord.recordPayload[NdefRecord.recordPayloadSize] = '\0';
            Serial.print("URI record: ");
            //Serial.println(ndef_helper_UriHead(NdefRecord.recordPayload[0]), &NdefRecord.recordPayload[1]);
            NdefRecord.recordPayload[NdefRecord.recordPayloadSize] = save;
        }
        break;

        case MEDIA_HANDOVER_WIFI:
        {
            unsigned char index = 0, i;

            Serial.println("--- Received WIFI credentials:");
            if ((NdefRecord.recordPayload[index] == 0x10) && (NdefRecord.recordPayload[index + 1] == 0x0e))
                index += 4;
            while (index < NdefRecord.recordPayloadSize)
            {
                if (NdefRecord.recordPayload[index] == 0x10)
                {
                    if (NdefRecord.recordPayload[index + 1] == 0x45)
                    {
                        Serial.print("- SSID = ");
                        for (i = 0; i < NdefRecord.recordPayload[index + 3]; i++)
                            Serial.print(NdefRecord.recordPayload[index + 4 + i]);
                        Serial.println("");
                    }
                    else if (NdefRecord.recordPayload[index + 1] == 0x03)
                    {
                        Serial.print("- Authenticate Type = ");
                        Serial.println(ndef_helper_WifiAuth(NdefRecord.recordPayload[index + 5]));
                    }
                    else if (NdefRecord.recordPayload[index + 1] == 0x0f)
                    {
                        Serial.print("- Encryption Type = ");
                        Serial.println(ndef_helper_WifiEnc(NdefRecord.recordPayload[index + 5]));
                    }
                    else if (NdefRecord.recordPayload[index + 1] == 0x27)
                    {
                        Serial.print("- Network key = ");
                        for (i = 0; i < NdefRecord.recordPayload[index + 3]; i++)
                            Serial.print("#");
                        Serial.println("");
                    }
                    index += 4 + NdefRecord.recordPayload[index + 3];
                }
                else
                    continue;
            }
        }
        break;

        case WELL_KNOWN_HANDOVER_SELECT:
            Serial.print("Handover select version ");
            Serial.print(NdefRecord.recordPayload[0] >> 4);
            Serial.println(NdefRecord.recordPayload[0] & 0xF);
            break;

        case WELL_KNOWN_HANDOVER_REQUEST:
            Serial.print("Handover request version ");
            Serial.print(NdefRecord.recordPayload[0] >> 4);
            Serial.println(NdefRecord.recordPayload[0] & 0xF);
            break;

        case MEDIA_HANDOVER_BT:
            Serial.print("BT Handover payload = ");
            //Serial.print(NdefRecord.recordPayload);
            //Serial.println(NdefRecord.recordPayloadSize);
            break;

        case MEDIA_HANDOVER_BLE:
            Serial.print("BLE Handover payload = ");
            //Serial.print(NdefRecord.recordPayload);
            //Serial.println(NdefRecord.recordPayloadSize);
            break;

        case MEDIA_HANDOVER_BLE_SECURE:
            Serial.print("   BLE secure Handover payload = ");
            //Serial.println(NdefRecord.recordPayload, NdefRecord.recordPayloadSize);
            break;

        default:
            Serial.println("Unsupported NDEF record, cannot parse");
            break;
        }
        pNdefRecord = GetNextRecord(pNdefRecord);
    }

    Serial.println("");
}
//#endif // if defined P2P_SUPPORT || defined RW_SUPPORT

//#if defined P2P_SUPPORT || defined CARDEMU_SUPPORT
const char NDEF_MESSAGE[] = {0xD1,     // MB/ME/CF/1/IL/TNF
                             0x01,     // TYPE LENGTH
                             0x07,     // PAYLOAD LENTGH
                             'T',      // TYPE
                             0x02,     // Status
                             'e', 'n', // Language
                             'T', 'e', 's', 't'};

void Electroniccats_PN7150::NdefPush_Cb(unsigned char *pNdefRecord, unsigned short NdefRecordSize)
{
    Serial.println("--- NDEF Record sent");
}
//#endif // if defined P2P_SUPPORT || defined CARDEMU_SUPPORT

bool Electroniccats_PN7150::NxpNci_FactoryTest_Prbs(NxpNci_TechType_t type, NxpNci_Bitrate_t bitrate)
{
    uint8_t NCIPrbs_1stGen[] = {0x2F, 0x30, 0x04, 0x00, 0x00, 0x01, 0x01};
    uint8_t NCIPrbs_2ndGen[] = {0x2F, 0x30, 0x06, 0x00, 0x00, 0x00, 0x00, 0x01, 0x01};
    uint8_t *NxpNci_cmd;
    uint16_t NxpNci_cmd_size = 0;

    if (gNfcController_generation == 1)
    {
        NxpNci_cmd = NCIPrbs_1stGen;
        NxpNci_cmd_size = sizeof(NCIPrbs_1stGen);
        NxpNci_cmd[3] = type;
        NxpNci_cmd[4] = bitrate;
    }
    else if (gNfcController_generation == 2)
    {
        NxpNci_cmd = NCIPrbs_2ndGen;
        NxpNci_cmd_size = sizeof(NCIPrbs_2ndGen);
        NxpNci_cmd[5] = type;
        NxpNci_cmd[6] = bitrate;
    }

    if (NxpNci_cmd_size != 0)
    {
        (void)writeData(NxpNci_cmd, sizeof(NxpNci_cmd));
        getMessage();
        if ((rxBuffer[0] != 0x4F) || (rxBuffer[1] != 0x30) || (rxBuffer[3] != 0x00))
            return ERROR;
    }
    else
    {
        return ERROR;
    }

    return SUCCESS;
}

bool Electroniccats_PN7150::NxpNci_FactoryTest_RfOn(void)
{
    uint8_t NCIRfOn[] = {0x2F, 0x3D, 0x02, 0x20, 0x01};

    (void)writeData(NCIRfOn, sizeof(NCIRfOn));
    getMessage();
    //NxpNci_HostTransceive(NCIRfOn, sizeof(NCIRfOn), Answer, sizeof(Answer), &AnswerSize);
    if ((rxBuffer[0] != 0x4F) || (rxBuffer[1] != 0x3D) || (rxBuffer[3] != 0x00))
        return ERROR;

    return SUCCESS;
}