
#ifndef __ESP8266_H
#define __ESP8266_H


extern unsigned char WIFIDataIsAvailable;
extern unsigned char WifiConnectionStep,NeedTowifiInit;
extern char CheckWiFi(void);
extern char WIFISend(unsigned int len, unsigned char *DataToSendRec);
extern char sendData(unsigned int len, unsigned char *data);
extern void debug(const char* log, char reset);

extern unsigned int WifiConnectionError;
#endif