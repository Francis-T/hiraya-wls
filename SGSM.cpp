#ifndef __SGSM_CPP__
#define __SGSM_CPP__

#include "SGSM.h"

SGSM::SGSM()
{
    return;
}

void SGSM::initialize(HardwareSerial* pSerial)
{
    _pSerial = pSerial;
    SGSM::sendAtCmd(AT_CMD_ECHO_OFF, strlen(AT_CMD_ECHO_OFF), TRUE);
    SGSM::sendAtCmd(AT_CMD_TEST, strlen(AT_CMD_TEST), TRUE);
    SGSM::sendAtCmd(AT_CMD_ENC_GSM, strlen(AT_CMD_ENC_GSM), TRUE);
    SGSM::sendAtCmd(AT_CMD_EXT_ERRORS, strlen(AT_CMD_EXT_ERRORS), TRUE);
    SGSM::sendAtCmd(AT_CMD_TEXT_MODE, strlen(AT_CMD_TEXT_MODE), TRUE);
    SGSM::sendAtCmd(AT_CMD_SMS_TST, strlen(AT_CMD_SMS_TST), TRUE);
    SGSM::sendAtCmd(AT_CMD_SVC_CEN_TST, strlen(AT_CMD_SVC_CEN_TST), TRUE);
    SGSM::sendAtCmd(AT_CMD_SVC_CENTER, strlen(AT_CMD_SVC_CENTER), TRUE);
    SGSM::sendAtCmd(AT_CMD_SET_MSG_IND, strlen(AT_CMD_SET_MSG_IND), TRUE);
    SGSM::sendAtCmd(AT_CMD_SEND_MSG_TST, strlen(AT_CMD_SEND_MSG_TST), TRUE);
    delay(3000);

    return;
}

void SGSM::send(char* aMobileNo, char* aMsg, int iLen)
{
    char* pStatus = NULL;
    int iBytesReceived = 0;
    char aAtCmdPart[32];
    char aMsgPart[160];
    char aResp[128];
    int iRetries = D_MAX_RETRIES;

    memset(aAtCmdPart, 0, 32);
    memset(aMsgPart, 0, 160);

    sprintf(aAtCmdPart, AT_CMD_SEND_MSG, aMobileNo);
    sprintf(aMsgPart, "%s%c", aMsg, CTRL_Z);

    for (int iIdx = 0; iIdx < 5; iIdx++)
    {
      sendAtCmd(AT_CMD_RSSI, strlen(AT_CMD_RSSI), TRUE);
    }

    sendAtCmd(aAtCmdPart, strlen(aAtCmdPart), FALSE);
    delay(300);

    while ((pStatus == NULL) && (iRetries > 0))
    {
      iBytesReceived = SGSM::receiveResp(aResp, 128);
      if (iBytesReceived <= 0)
      {
        delay(90);
        continue;
      }
      
      pStatus = strstr(aResp, ">");
      break;
    }

    if (pStatus == NULL)
    {
      return;
    }

    _pSerial->write(aMsgPart);
    delay(300);

    pStatus = NULL;
    iRetries = D_MAX_RETRIES;
    while ((pStatus == NULL) && (iRetries > 0))
    {
      iBytesReceived = SGSM::receiveResp(aResp, 128);
      if (iBytesReceived <= 0)
      {
        delay(90);
        continue;
      }

//      _pSerial->print("#   ");
//      _pSerial->println(aResp);

      pStatus = strstr(aResp, "\r\nOK\r\n");
      if (pStatus != NULL)
      {
        break;
      }
      
      pStatus = strstr(aResp, "\r\nERROR\r\n");
      if (pStatus != NULL)
      {
        break;
      }
    }
    
    discardData();
    return;
}

void SGSM::sendAtCmd(const char* aCmd, int iLen, bool_t bAwaitResp)
{
    char* pStatus = NULL;
    int iBytesReceived = 0;
    int iRetries = D_MAX_RETRIES;
    char aResp[128];

    while ((pStatus == NULL) && (iRetries > 0))
    {
        _pSerial->write(aCmd, iLen);
        delay(90);

        if (bAwaitResp == FALSE)
        {
            return;
        }

        iBytesReceived = SGSM::receiveResp(aResp, 128);
        if (iBytesReceived > 0)
        {
            pStatus = strstr(aResp, "OK");
        }

        if (pStatus != NULL)
        {
//            _pSerial->println("Response received.");
//            _pSerial->print("#   ");
//            _pSerial->println(aResp);
            break;
        }

        iRetries--;
    }

    discardData();

    return;
}

int SGSM::receiveResp(char* pBuf, int iMaxBufLen)
{
    int iByteCount = 0;

    /* Clear the buffer first */
    memset(pBuf, 0, iMaxBufLen);

    while (_pSerial->available() > 0)
    {
        if (iByteCount >= iMaxBufLen)
        {
            break;
        }

        pBuf[iByteCount] = _pSerial->read();
        iByteCount++;
        delay(50);
    }

    return iByteCount;
}

void SGSM::discardData()
{
    while (_pSerial->available() > 0)
    {
      _pSerial->read();
      delay(90);
    }

    return;
}

#endif /* __SGSM_CPP__ */

