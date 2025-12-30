
#ifndef __ESP8266_H
#define __ESP8266_H

extern unsigned char DataBuffer[2048];
extern unsigned int DataBufferLen;

extern unsigned char WIFIDataIsAvailable;
extern unsigned char WifiConnectionStep,NeedTowifiInit;
extern char CheckWiFi(void);
extern char WIFISend(unsigned int len, unsigned char *DataToSendRec);
extern char sendData(unsigned int len, unsigned char *data);

extern unsigned int WifiConnectionError;
#endif