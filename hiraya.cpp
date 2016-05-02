#ifndef __HIRAYA_CPP__
#define __HIRAYA_CPP__

#include "hiraya.h"

/**********************/
/** Main Source Code **/
/**********************/
void setup()
{
    char aPropData[16];

    Serial.begin(38400);
    comm_discardData();

    SGSM.initialize(&Serial);

    delay(1000);
    
    comm_discardData();
    
    return;
}

void loop()
{
    processSerialInput();
    delay(100);
    return;
}

void processSerialInput()
{
    int cRead = '\0';
    
    /* If data is available on the Serial, receive it */
    if ((Serial.available() > 0) && (_iRecvBufLen <= 0))
    {
        /* Attempt to receive data */
        if (comm_recvData() < STATUS_OK)
        {
            return;
        }
    }
    
    /* Process the received data if there are bytes read */
    if (_iRecvBufLen > 0)
    {
      if (proc_processMessage(_aRecvBuf, _iRecvBufLen) < STATUS_OK)
      {
          return;
      }
      
      #ifndef USE_ARDUINO
      StubGsmController.processInput(_aRecvBuf, _iRecvBufLen);
      #endif
    }

    /* Clear our receive buffer */
    memset(_aRecvBuf, 0, sizeof(_aRecvBuf));

    return;
}

/** SecXX: Proc Functions **/
int proc_processMessage(char* pMsg, int iMsgLen)
{
    int iIdx;
    int iCmdLen;
    char aContents[128];
    
    memset(aContents, 0, sizeof(aContents));

    /* Extract Content from the message */
    if (proc_getContents(pMsg, aContents) < STATUS_OK)
    {
        return STATUS_ERROR;
    }

    /* Process the message contents */
    for (iIdx = 0; iIdx < D_CMD_LIST_SZ; iIdx++)
    {
        iCmdLen = _aCmd[iIdx].iLen;
        if (utl_matches(_aCmd[iIdx].aCmdStr, aContents, iCmdLen))
        {
            _aCmd[iIdx].fProcess(aContents, strlen(aContents), iCmdLen);
        }
    }

    return STATUS_OK;
}

int proc_initialize(char* pCmd, int iLen, int iPayloadOffs)
{
    char aValue[32];
    char* pArgs = pCmd + iPayloadOffs;
    
    /* Extract and save the node owner */
    memset(aValue, 0, 32);
    utl_getField(pArgs, aValue, 1, '.');
    eeprom_saveProperty(NODE_OWNER, aValue, D_BLEN_NODE_OWNER);

    /* Extract and save the node name */
    memset(aValue, 0, 32);
    utl_getField(pArgs, aValue, 2, '.');
    eeprom_saveProperty(NODE_NAME, aValue, D_BLEN_NODE_NAME);

    /* Set the default value for the read wait */
    memset(aValue, 0, 32);
    strcpy(aValue, "0000");
    eeprom_saveProperty(READ_WAIT, aValue, D_BLEN_READ_WAIT);

    gsm_sendMessage("OK", 2);

    return STATUS_OK;
}

int proc_setName(char* pCmd, int iLen, int iPayloadOffs)
{
    char aValue[16];
    char* pArgs = pCmd + iPayloadOffs;
    
    /* Extract and save the node name */
    memset(aValue, 0, sizeof(aValue));
    eeprom_saveProperty(NODE_NAME, pArgs, 16);

    gsm_sendMessage("OK", 2);

    return STATUS_OK;
}

int proc_setReadInterval(char* pCmd, int iLen, int iPayloadOffs)
{
    char aValue[4];
    char* pArgs = pCmd + iPayloadOffs;
    
    /* Extract and save the read interval */
    memset(aValue, 0, sizeof(aValue));
    eeprom_saveProperty(READ_WAIT, pArgs, 4);

    gsm_sendMessage("OK", 2);

    return STATUS_OK;
}

int proc_setTargetMobile(char* pCmd, int iLen, int iPayloadOffs)
{
    char aValue[10];
    char* pArgs = pCmd + iPayloadOffs;

    /* Drop the leading zeroes */
    if (*pArgs == '0')
    {
        pArgs++;
    }
    
    /* Extract and save the target mobile number */
    memset(aValue, 0, sizeof(aValue));
    eeprom_saveProperty(TARGET_MOBILE, pArgs, 10);

    gsm_sendMessage("OK", 2);

    return STATUS_OK;
}

int proc_sendLastReading(char* pCmd, int iLen, int iPayloadOffs)
{
    /* TODO */
    return STATUS_OK;
}

int proc_sendOldReadings(char* pCmd, int iLen, int iPayloadOffs)
{
    /* TODO */
    return STATUS_OK;
}

int proc_sendPowerStatus(char* pCmd, int iLen, int iPayloadOffs)
{
    /* TODO */
    return STATUS_OK;
}

int proc_sendSignalStatus(char* pCmd, int iLen, int iPayloadOffs)
{
    /* TODO */
    return STATUS_OK;
}

int proc_sendDeviceInfo(char* pCmd, int iLen, int iPayloadOffs)
{
    char aMessage[160];
    char aPropData[16];

    memset(aMessage, 0, 160);
    /* Load data from EEPROM */
    eeprom_loadProperty(NODE_OWNER, aPropData, 16);
    strcat(aMessage, "Owner = ");
    strcat(aMessage, aPropData);
    strcat(aMessage, ", ");

    eeprom_loadProperty(READ_WAIT, aPropData, 16);
    strcat(aMessage, "Read Interval = ");
    strcat(aMessage, aPropData);
    strcat(aMessage, ", ");

    eeprom_loadProperty(TARGET_MOBILE, aPropData, 16);
    strcat(aMessage, "Target Mobile = ");
    strcat(aMessage, aPropData);

    gsm_sendMessage(aMessage, strlen(aMessage));

    return STATUS_OK;
}

int proc_getOwner(char* pMsg, char* pOwner)
{
    return utl_getField(pMsg, pOwner, 1, ':');
}

int proc_getContents(char* pMsg, char* pContents)
{
    char* pContentStart = strchr(pMsg, ':');
    
    /* Skip to first char after the ':' */
    pContentStart++;

    /* Skip spaces */
    while (*pContentStart == ' ')
    {
        pContentStart++;
    }

    strcpy(pContents, pContentStart);

    return STATUS_OK;
}

/** SecXX: Comm Functions **/
int comm_displayRecvData()
{
    int iByteCount = 0;
    Serial.print("# ");
    for (int i = 0; i < D_MAX_RECV_BUF_LEN; i++)
    {
      if (_aRecvBuf[i] == 0)
      {
        /* Skip over NULL chars */
        continue;
      }
      else if ((_aRecvBuf[i] >= 32) && (_aRecvBuf[i] <= 126))
      {
        Serial.print(" ");
        Serial.print(_aRecvBuf[i]);
      }
      else
      {
        Serial.print(" ");
        Serial.print((int)(_aRecvBuf[i]));
        Serial.print("*");
      }
      
      if (((iByteCount+1) % 40) == 0)
      {
        Serial.println();
      }
      iByteCount++;
    }
    Serial.println();

    return STATUS_OK;
}

int comm_discardData()
{
    comm_recvData();
    //comm_displayRecvData();
    
    while (Serial.available() > 0)
    {
      Serial.read();
    }
    return STATUS_OK;
}

int comm_recvData()
{
    int cRead = '\0';
    int iBytesRead = 0;

    while (Serial.available() > 0)
    {
        if (iBytesRead >= D_MAX_RECV_BUF_LEN)
        {
            break;
        }

        cRead = Serial.read();
//        if ((cRead < 32) || (cRead > 126)) {
//          /* Quietly discard non-printables */
//          cRead = '?';
//        }

        if ((cRead == '\n') || (cRead == '\r'))
        {
          break;
        }
        
        _aRecvBuf[iBytesRead] = cRead;

        iBytesRead++;
        delay(50);
    }

    /* Set the received buffer length */
    _iRecvBufLen = iBytesRead;

    /* Check if we've received all chars */
    if (Serial.available() > 0)
    {
        return STATUS_WARN;
    }

    return STATUS_OK;
}

/** SecXX: GSM Functions **/
int gsm_sendMessage(const char* aMsg, int iLen)
{
    char aCompleteMsg[160];
    char aPropData[16];

    memset(aCompleteMsg, 0, 160);
    memset(aPropData, 0, 16);

    eeprom_loadProperty(NODE_NAME, aPropData, 16);
    strcat(aCompleteMsg, aPropData);
    strcat(aCompleteMsg, ": ");
    strcat(aCompleteMsg, aMsg);

    eeprom_loadProperty(TARGET_MOBILE, aPropData, 16);

    SGSM.send(aPropData, aCompleteMsg, strlen(aCompleteMsg));
   
    return STATUS_OK;
}

/** SecXX: EEPROM Functions **/
prop_t* eeprom_getProperty(eProp_t ePropId)
{
    int iIdx;

    /* Match the given property id to a known property */
    for (iIdx = 0; iIdx < _iPropsLen; iIdx++)
    {
        if (_aProps[iIdx].ePropId == ePropId)
        {
            return &_aProps[iIdx];
        }
    }

    return NULL;
}

void eeprom_saveProperty(eProp_t ePropId, char* pVal, int iLen)
{
    int iIdx;
    prop_t* pProp;

    /* Retrieve the property */
    pProp = eeprom_getProperty(ePropId);
    if (pProp == NULL)
    {
        return;
    }

    for (iIdx = 0; iIdx < iLen; iIdx++)
    {
        EEPROM.write(pProp->iOffs + iIdx, pVal[iIdx]);
    }

    return;
}

void eeprom_loadProperty(eProp_t ePropId, char* pStore, int iMaxLen)
{
    int iIdx;
    char bRead;
    prop_t* pProp;

    /* Clear the buffer first */
    memset(pStore, 0, iMaxLen);

    /* Retrieve the property */
    pProp = eeprom_getProperty(ePropId);
    if (pProp == NULL)
    {
        return;
    }

    /* Read from the EEPROM, copying the property value */
    for (iIdx = 0; iIdx < pProp->iLen; iIdx++)
    {
        bRead = (char) EEPROM.read(iIdx + pProp->iOffs);
        pStore[iIdx] = bRead;
    }

    return;
}

/** SecXX: Util Functions **/
int utl_matches(const char* s1, const char* s2, int iLen)
{
    if (strncmp(s1, s2, iLen) == 0)
    {
        return MATCHED;
    }

    return NOT_MATCHED;
}

/*
 * @function    utl_getField()
 * @description Extracts a specific 'field' given a delimited string
 * @params      s1        - pointer to the string to be searched
 *              s2        - pointer to the string to write results to
 *              iTgtField - which field to extract
 *              cDelim    - delimiter for extracting fields
 * @returns     An integer status code 
 * @usage       Given the string testStr = "CALIB SENSOR PH 8.00" and an
 *               empty char array, outStr:
 *
 *              utl_getField(testStr, outStr, 4, ' ');    // returns "8.00" in outStr
 *              utl_getField(testStr, outStr, 3, ' ');    // returns "PH"   in outStr
 *              utl_getField("READ ALL", outStr, 2, ' '); // returns "ALL"  in outStr
 *
 * @note        For safety, the pointer s2 should point to a char array with
 *               a large _enough_ size to contain the result; otherwise,
 *               memory issues might occur
 */
int utl_getField(char* s1, char* s2, int iTgtField, char cDelim)
{
    int iCurrField = 1;

    if (s1 == NULL) {
        return STATUS_ERROR;
    }

    char* pStart = s1;
    char* pEnd = strchr(pStart, cDelim);
    if (pEnd == NULL)
    {
        pEnd = pStart + strlen(pStart);
    }

    while (iCurrField != iTgtField)
    {
        pStart = pEnd + 1;
        pEnd = strchr(pStart, cDelim);
        if (pEnd == NULL)
        {
            pEnd = pStart + strlen(pStart);
            break;
        }
        iTgtField++;
    }

    /* Copy the matching field */
    strncpy(s2, pStart, (pEnd - pStart));

    return STATUS_OK;
}

#ifndef USE_ARDUINO
int main()
{
    Serial.setCallback(&serialEvent);
    StubGsmController.initialize(Serial);

    setup();

    while (1)
    {
        if (Serial.readInput() < 0) 
        {
            break;
        }
        loop();
    }

    return 0;
}
#endif

#endif /* __HIRAYA_CPP__ */

