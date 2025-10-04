
#ifndef GSM_H
#define GSM_H

#define Disconnected  0
#define Connected     1
#define Connecting    2
#define Idle          3
#define ChargeQuering 4

extern unsigned char ActivePort;
extern unsigned char CUSDReady;
extern unsigned char GPRSStatus;
extern unsigned char CallReceived;
extern unsigned char CallReady;
extern unsigned char SMSReceived;
extern unsigned char HaveNotSIMCARD;
extern unsigned char GetBtsName;
extern unsigned char DataSent;
extern unsigned char NoCarrier;
extern unsigned char GPRSConnectOK;
extern unsigned char GPRSConnecting;
extern unsigned char GPRSClose;
extern unsigned char GPRSDeact;
extern unsigned char GPRSRetry;
extern unsigned char GPRSCHECK;
extern unsigned char DataInBuffer;
extern unsigned char CSQ;

extern unsigned int ConnectionError;

extern unsigned char DataBuffer[];
extern unsigned int DataBufferLen;
extern unsigned char DataIsAvailable;

extern unsigned char CSQValue;


extern unsigned char CurrentConnectionStep;

extern unsigned char ChargeCode[];

extern unsigned char ChargeRequest;

void GSMHandler(void *p_arg);

extern void PasssGPRSConnectionSteps(void);
extern int GSMInitialize(unsigned char Reset);
extern char SendSMS(char *TelNum,char *DataToSend);
extern unsigned char GPRSConnect(void);
extern char GPRSSend(unsigned int len, unsigned char *DataToSendRec);
extern unsigned char DeleteAllMessages(void);
extern unsigned char ProcessSMS(void);
extern signed long int GetSimCardChargeInt(unsigned char background);
extern unsigned char GSMSetTime(void);
extern void GetGPRSStatus(void);
extern unsigned char GSMGetTime(void);
extern unsigned char GetGSMResponse(const char *Response);
extern unsigned char _IsPrintable(unsigned char ch);
extern unsigned char Get_GPRSStatusText(unsigned char *Status);
extern unsigned char GetSimCardChargeAuto(void);
extern unsigned int  ExecuteCommand(void);
extern unsigned char ChargeSimCardAuto(void);
extern unsigned char ChargeSimCard(unsigned char JustChargeQuery);
extern unsigned char GetSimCardCharge(void);
extern void GSM_PowerOFF(void);
extern void CheckGPRS(void);

#endif
