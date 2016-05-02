#ifndef __SGSM_H__
#define __SGSM_H__

#define USE_ARDUINO

#ifndef USE_ARDUINO
#include <stdio.h>
#include <string>
#endif
#include "Arduino.h"
#include "HardwareSerial.h"

#define TRUE    1
#define FALSE   0

#define D_MAX_RETRIES   20

#define AT_CMD_TEST         "AT\r\n"
#define AT_CMD_ECHO_OFF     "ATE0\r\n"
#define AT_CMD_TEXT_MODE    "AT+CMGF=1\r\n"
#define AT_CMD_EXT_ERRORS   "AT+CMEE=1\r\n"
#define AT_CMD_ENC_GSM      "AT+CSCS=\"GSM\"\r\n"
#define AT_CMD_SMS_TST      "AT+CSMS=?\r\n"
#define AT_CMD_SVC_CEN_TST  "AT+CSCA=?\r\n"
#define AT_CMD_SVC_CENTER   "AT+CSCA?\r\n"
#define AT_CMD_SEND_MSG_TST "AT+CMGS=?\r\n"
#define AT_CMD_SEND_MSG     "AT+CMGS=\"0%s\"\r\n"
#define AT_CMD_RSSI         "AT+CSQ\r\n"
#define AT_CMD_SET_MSG_IND  "AT+CNMI=1,2,0,0,0\r\n"
#define CTRL_Z              0x1A

typedef int bool_t;

class SGSM
{
    public:
        SGSM();
        void initialize(HardwareSerial* pSerial);
        void send(char* aMobileNo, char* aMsg, int iLen);
    private:
        HardwareSerial* _pSerial;

        void sendAtCmd(const char* aCmd, int iLen, bool_t bAwaitResp);
        int receiveResp(char* pBuf, int iMaxBufLen);
        void discardData();
};

#endif /* __SGSM_H__ */

