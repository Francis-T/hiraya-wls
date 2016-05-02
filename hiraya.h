#ifndef __HIRAYA_H__
#define __HIRAYA_H__

#define USE_ARDUINO

#ifndef USE_ARDUINO
#include <stdio.h>
#include <string.h>
#include "Arduino.h"
#include "HardwareSerial.h"
#include "Stub_SGSM_Controller.h"
#endif
#include "EEPROM.h"
#include "SGSM.h"

#define STATUS_WARN     2
#define STATUS_OK       1
#define STATUS_ERROR    -1

#define NOT_MATCHED     0
#define MATCHED         1

#define D_OFFS_NODE_NAME    0
#define D_OFFS_NODE_OWNER   16
#define D_OFFS_READ_WAIT    32
#define D_OFFS_TARG_MOBILE  36

#define D_BLEN_NODE_NAME    16
#define D_BLEN_NODE_OWNER   16
#define D_BLEN_READ_WAIT    4
#define D_BLEN_TARG_MOBILE  10

#define D_CMD_LIST_SZ       9
#define D_MAX_RECV_BUF_LEN  255

/* Type definitions */
typedef enum {
    NODE_NAME, 
    NODE_OWNER, 
    READ_WAIT,
    TARGET_MOBILE
} eProp_t;

typedef struct
{
    eProp_t ePropId;
    int iOffs;
    int iLen;
} prop_t;

typedef int (*fCommandFunc_t) (char* pCmd, int iLen, int iPayloadOffs);

typedef struct
{
    char aCmdStr[22];
    int iLen;
    fCommandFunc_t fProcess;

} command_t;

/**********************/
/** Global variables **/
/**********************/
#ifndef USE_ARDUINO
HardwareSerial Serial;
EEPROM EEPROM;
StubGsmController StubGsmController;
#endif
SGSM SGSM;

prop_t _aProps[] =
{
    { NODE_NAME, D_OFFS_NODE_NAME, D_BLEN_NODE_NAME },
    { NODE_OWNER, D_OFFS_NODE_OWNER, D_BLEN_NODE_OWNER },
    { READ_WAIT, D_OFFS_READ_WAIT, D_BLEN_READ_WAIT },
    { TARGET_MOBILE, D_OFFS_TARG_MOBILE, D_BLEN_TARG_MOBILE },
};
int _iPropsLen = sizeof(_aProps)/sizeof(_aProps[0]);

char _aRecvBuf[D_MAX_RECV_BUF_LEN];
int _iRecvBufLen = 0;

/************************/
/* Function definitions */
/************************/
void setup();
void loop();
void processSerialInput();

int comm_displayRecvData();
int comm_discardData();
int comm_recvData();

int proc_processMessage(char* pMsg, int iMsgLen);
int proc_getOwner(char* pMsg, char* pOwner);
int proc_getContents(char* pMsg, char* pContents);

int gsm_sendMessage(const char* aMsg, int iLen);

prop_t* eeprom_getProperty(eProp_t ePropId);
void eeprom_saveProperty(eProp_t ePropId, char* pVal, int iLen);
void eeprom_loadProperty(eProp_t ePropId, char* pStore, int iMaxLen);

int utl_matches(const char* s1, const char* s2, int iLen);
int utl_getField(char* s1, char* s2, int iTgtField, char cDelim);

int proc_initialize(char* pCmd, int iLen, int iPayloadOffs);
int proc_setName(char* pCmd, int iLen, int iPayloadOffs);
int proc_setReadInterval(char* pCmd, int iLen, int iPayloadOffs);
int proc_setTargetMobile(char* pCmd, int iLen, int iPayloadOffs);
int proc_sendLastReading(char* pCmd, int iLen, int iPayloadOffs);
int proc_sendOldReadings(char* pCmd, int iLen, int iPayloadOffs);
int proc_sendPowerStatus(char* pCmd, int iLen, int iPayloadOffs);
int proc_sendSignalStatus(char* pCmd, int iLen, int iPayloadOffs);
int proc_sendDeviceInfo(char* pCmd, int iLen, int iPayloadOffs);

/* Command Table */
command_t _aCmd[D_CMD_LIST_SZ] =
{
    { "Initialize as ",         14, proc_initialize },
    { "Set name to ",           12, proc_setName },
    { "Set read interval to ",  21, proc_setReadInterval },
    { "Set target mobile to ",  21, proc_setTargetMobile },
    { "Send last reading",      17, proc_sendLastReading },
    { "Send old readings",      17, proc_sendOldReadings },
    { "Send power status",      17, proc_sendPowerStatus },
    { "Send signal status",     18, proc_sendSignalStatus },
    { "Send device info",       16, proc_sendDeviceInfo }
};

#endif /* __HIRAYA_H__ */

