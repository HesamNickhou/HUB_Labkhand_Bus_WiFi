#include <includes.h>
#include <math.h>
#include <GUI.h>
#include "esp8266.h"
#include "Config.h"
#include "menu3d.h"
#include "usart/usart.h"
#include "rtc/rtc.h"
#include <key/keypad.h>

extern int ConnectedToServer;
#define BUFFER3_SIZE         1200

unsigned char 
	getReady            = 0,
	WiFiStep        		= 0,
	ConnectionRetry 		= 0,
	WifiConnected 			= 0,
	WifiDisConnected 		= 0,
	WifiClosed 					= 0,
	WifiDataSent 				= 0,
	WifiBusyP 					= 0,
	WIFIDataIsAvailable = 0,
	WIFICHECK 					= 0,
	WIFICurSocket 			= 1,
	NeedTowifiInit			= 0,
	IPDLoc							= 0,
	DataBuffer[2048],
	WIFIReceiveBuf[1024],
	RXD3Buffer[BUFFER3_SIZE],
							WIFI_GCF(unsigned char *c),
							WIFI_GCL(unsigned char *c),
	GPS_CheckSystemTime2(unsigned char GPSTimeChecked),
	 GPS_CheckSystemTime(void);

unsigned int
	WifiConnectionError = 0,
	DataBufferLen 			= 0,
	rxd3_counter 				= 0,
	rxd3_wr_index 			= 0,
	rxd3_rd_index 			= 0,
	DataSize 						= 0;
	
unsigned short WIFIReceiveBufLen;

const unsigned char 
	IPD[6] = "+IPD,",
	IPDLen = 5;

char DebugX = 0;

struct TUSARTMessage{
  char *Message;
  unsigned char Len;
  unsigned char CurLoc;
  unsigned char *Received;
  char *Data;
};
typedef struct TUSARTMessage USARTMessage;
USARTMessage USART_2[5] = {
  {"WIFI CONNECTED\0",    14, 0, &WifiConnected,    ""},
  {"WIFI DISCONNECTED\0", 17, 0, &WifiDisConnected, ""},
  {"CLOSED\0",             6, 0, &WifiClosed,       ""},  
  {"SEND OK\0",            7, 0, &WifiDataSent,     ""},  
  {"busy p\0",             6, 0, &WifiBusyP,        ""},
};

typedef void (*WIFIListener)(char* data, char len);
WIFIListener callback;

unsigned char GetWiFiResponse2(const char *Response1, const char* Response2);
void setCallback(WIFIListener interface);

void setCallback(WIFIListener interface) { callback = interface; }
void USART_SendStr(USART_TypeDef* USARTx, char *str) {
	int i = 0;
  while (str[i] != 0) {
    switch (str[i]) {
      case '\n': USART_SendData(USARTx, 13); break;
      case '\r': USART_SendData(USARTx, 10); break;
      case '\"': USART_SendData(USARTx, 34); break;
        default: USART_SendData(USARTx, str[i]);
    }
    i++;
  }
}
unsigned char WIFI_GCL(unsigned char *c) {
	int i = OS_TimeMS;
	while (1) {
  	if (i > OS_TimeMS) 
			return 0;
	  if ((OS_TimeMS - i) >= 1000) //3 sec
			return 0;
    if (rxd3_counter != 0) 
      break;
		GUI_Delay(1);
		WDTR;
	}
  *c = RXD3Buffer[rxd3_rd_index];
  if (++rxd3_rd_index >= BUFFER3_SIZE) 
		rxd3_rd_index = 0;
  if (rxd3_counter)
		rxd3_counter--;
  return 1;
}
unsigned char WIFI_GCF(unsigned char *c) {
	unsigned int i = OS_TimeMS;
	while (1) {
  	if (i > OS_TimeMS) 
			return 0;
	  if ((OS_TimeMS - i) >= 200)
			return 0;
    if (rxd3_counter != 0) 
      break;
		GUI_Delay(1);
		WDTR;
	}		
  *c = RXD3Buffer[rxd3_rd_index];
  if (++rxd3_rd_index >= BUFFER3_SIZE) 
		rxd3_rd_index = 0;
  if (rxd3_counter) 
		rxd3_counter--;
  return 1;
}
unsigned char WIFI_GCVF(unsigned char *c) {
	unsigned int i = OS_TimeMS;
	while (1) {
  	if (i > OS_TimeMS) 
			return 0;
	  if ((OS_TimeMS - i) >= 50)
			return 0;
    if (rxd3_counter != 0) 
      break;
		GUI_Delay(1);
		WDTR;
	}		
  *c = RXD3Buffer[rxd3_rd_index];
  if (++rxd3_rd_index >= BUFFER3_SIZE) 
		rxd3_rd_index = 0;
  if (rxd3_counter) 
		rxd3_counter--;
  return 1;
}	 
void EmptyWIFIRXBuffer(void) { 
	unsigned short i;
	rxd3_rd_index = rxd3_counter = rxd3_wr_index = 0; 
	for (i=0; i<BUFFER3_SIZE; i++)
		RXD3Buffer[i] = 0;
}
unsigned char GetWiFiResponse(const char* Response) {
	int i, j;
	unsigned char Byte;

	 // printf("\n\rResponse[%d](%s) : \n\r",WIFIReceiveBufLen,Response);/////////////

  WIFIReceiveBufLen = 0;
	while (WIFIReceiveBufLen == 0) {
    if (WIFI_GCL(&Byte)) {
      if ((Byte != 13) && (Byte != 10))
		    WIFIReceiveBuf[WIFIReceiveBufLen++] = Byte;  
    } 
		else 
      return 1;
  }

  while ((WIFI_GCF(&Byte)) && (WIFIReceiveBufLen < 1020)) 
    WIFIReceiveBuf[WIFIReceiveBufLen++] = Byte;
  WIFIReceiveBuf[WIFIReceiveBufLen] = 0;
  
  j = 0;
  for (i = 0; i<WIFIReceiveBufLen; i++) {
    if (Response[j] == '\0')
			return 0;
    if (Response[j] == WIFIReceiveBuf[i]) 
			j++;
		else 
			j = 0;
  }          
  return 2;
}

unsigned char GetWiFiResponse2(const char *Response1, const char* Response2) {
	int i, j, k;
	unsigned char Byte;

	 // printf("\n\rResponse[%d](%s) : \n\r",WIFIReceiveBufLen,Response);/////////////

  WIFIReceiveBufLen = 0;
	while (WIFIReceiveBufLen == 0) {
    if (WIFI_GCL(&Byte)) {
      if ((Byte != 13) && (Byte != 10))
		    WIFIReceiveBuf[WIFIReceiveBufLen++] = Byte;  
    } 
		else 
      return 1;
  }

  while ((WIFI_GCF(&Byte)) && (WIFIReceiveBufLen < 1020)) 
    WIFIReceiveBuf[WIFIReceiveBufLen++] = Byte;
  WIFIReceiveBuf[WIFIReceiveBufLen] = 0;
  
  k = j = 0;
  for (i = 0; i<WIFIReceiveBufLen; i++) {
    if ((Response1[j] == '\0') || (Response2[k] == '\0'))
			return 0;
    if (Response1[j] == WIFIReceiveBuf[i]) 
			j++;
		else 
			j = 0;
		if (Response2[k] == WIFIReceiveBuf[i])
			k++;
    else 
			k = 0;
  }          
  return 2;
}
char CheckWiFi(void) {
	unsigned char str[200];	
	char *ptr;
	
	if (WiFiStep == 10) {
		GUI_SetColor(GUI_CYAN);
		GUI_FillRoundedRect(4 + 234, 2 + 3, 30 + 233, 27 + 1, 3);		
		if (ConnectedToServer)   GUI_SetColor(GUI_DARKGREEN); 
		else                    GUI_SetColor(GUI_RED); 	
		GUI_FillCircle(10 + 234, 15 + 1, 4);
		GUI_DrawLine  (10 + 234, 15 + 1, 22 + 234, 8 + 3);
		GUI_FillCircle(22 + 234, 8  + 3, 4);
		GUI_DrawLine  (10 + 234, 15 + 1, 22 + 234, 20 + 2);
		GUI_FillCircle(22 + 234, 20 + 2, 4);
	} 
  else {	 
		LoadFont(FontTahoma20);
		GUI_SetColor(GUI_DARKGRAY);
		GUI_FillRoundedRect(4 + 234, 2 + 3, 30 + 233, 27 + 1, 3);	
		GUI_SetColor(GUI_LIGHTGREEN);
		GUI_DispDecAt(WiFiStep, 240, 6, 2);
  }
 
    //  SendByte(WiFiStep+'0');
  
  //printf("\n\rWiFiStep=%d ",WiFiStep);
  
  Config.ServerIP[15]      = 0;
	Config.LocalIP[15]       = 0;
	Config.RoutIpAddress[15] = 0;
	Config.SubnetIP[15]      = 0;
	
  switch (WiFiStep)  {
    case 0: { //Hardware reset the WIFI module
			#ifdef STM32F2XX
      WIFIRST(0);
      GUI_Delay(100);
      WIFIRST(1);  
      #else
				WIFIRST = 0;
				GUI_Delay(100);
				WIFIRST = 1;  
      #endif
    
      WiFiStep++;
      break;
		}
    case 1: { //Start comminucating with WIFI module       
      USART_SendStr(USART3, "ATE0\n\r");
		  GetWiFiResponse("OK"); 
		  WiFiStep++;
      break;
		}		
    case 2: { //Set just one single connection at a time
      USART_SendStr(USART3, "AT+CIPMUX=0\n\r");		
			if (GetWiFiResponse("OK") == 0)
				WiFiStep++;
		  else 
				WiFiStep = 0;
      break;
		}
    case 3: { //Enable client mode and transparent mode
      EmptyWIFIRXBuffer(); 
      USART_SendStr(USART3, "AT+CWMODE=1\n\r");  //client		
      EmptyWIFIRXBuffer(); 
      USART_SendStr(USART3, "AT+CIPMODE=1\n\r");
     //  GetWiFiResponse("OK");  
      WiFiStep++;
      break;
		}
    case 4:	{ //Configure IP instead of DHCP
		  EmptyWIFIRXBuffer(); 
      sprintf(str, "AT+CIPSTA=\"%s\",\"%s\",\"%s\"\n\r", Config.LocalIP, 
				Config.RoutIpAddress, Config.SubnetIP);
			//sprintf(str, "AT+CIPSTA=\"192.168.1.40\",\"192.168.1.1\",\"255.255.255.0\"\n\r");
			//sprintf(str, "AT+CWDHCP=1,1"); Enable DHCP Auto IP
      USART_SendStr(USART3, str);
		    //GetWiFiResponse("OK");
      WiFiStep++;
      break;
		}
    case 5: { //Connect to SSID and Password
			EmptyWIFIRXBuffer(); 
      sprintf(str, "AT+CWJAP=\"%s\",\"%s\"\n\r", Config.SSID, Config.WiFiPassword); 
      USART_SendStr(USART3, str);
			WiFiStep++;
      ConnectionRetry = 0;
      break;
		}
    case 6: { //Check the connection status
      EmptyWIFIRXBuffer(); 
      WIFIReceiveBuf[0] = 0;
      USART_SendStr(USART3, "AT+CIPSTATUS\n\r");
      if (GetWiFiResponse("STATUS:2") == 0) {
				ConnectionRetry = 0;
        WiFiStep++;
			}
      else {
				ptr = (char *)WIFIReceiveBuf;
				if ((ptr = strstr(ptr, "STATUS:")) || (ptr = strstr(ptr, "bus"))) {
					if (++ConnectionRetry > 10) {
						if (strstr(ptr, "bus")) {
							EmptyWIFIRXBuffer(); 
							WIFIReceiveBuf[0] = 0;
							USART_SendStr(USART3, "AT+CIPSTATUS\n\r");
							ConnectionRetry = 0;
						}
						WiFiStep = 0;
					}
				}
        else if (++ConnectionRetry > 3) {
					WiFiStep 				= 9;
					ConnectionRetry = 0;
				}
      }
      break;
		}
    case 7: { //Establishing the connection
      EmptyWIFIRXBuffer();
      Config.ConnectionPort = 1212;
      sprintf(str, "AT+CIPSTART=\"TCP\",\"%s\",%d\n\r", Config.ServerIP, Config.ConnectionPort);
			sprintf(str, "AT+CIPSTART=\"TCP\",\"192.168.1.12\",1212\n\r");
			//AT+CIPSTART="TCP","192.168.1.5",1212\n\r
      USART_SendStr(USART3, str);
      WiFiStep++;
      break;
		}
    case 8: { //Wait to get CONNECT phrase
      //if (GetWiFiResponse2("CONNECT", "OK") == 0) {
			if (GetWiFiResponse("CONNECT") == 0) {
        USART_SendStr(USART3, "AT+CWMODE=1\n\r");
		    WiFiStep = 9;
			}
			else {
				if (++ConnectionRetry > 0)
					WiFiStep = 6;
			}	
      break;
		}
    case 9: {
			EmptyWIFIRXBuffer(); 
      USART_SendStr(USART3, "AT+CIPMODE=1\n\r");
      GetWiFiResponse("OK");
			
			//======= Enable TCP Server on Port 1212 =============
			USART_SendStr(USART3, "AT+CIPSERVER=1,1212");
			
		/*
		 EmptyWIFIRXBuffer(); 
		      USART_SendStr(USART3, "AT+CIPMODE?\n\r");
		GUI_Delay(2000);
		GetWiFiResponse("$");
				 LoadFont(FontTahoma20);
         GUI_SetColor(GUI_RED); 
         PutText(5, 80, 272-38, 275+30+30+10, WIFIReceiveBuf, GUI_TA_CENTER);
		*/
		
      EmptyWIFIRXBuffer();
      WIFIReceiveBuf[0] = 0;
      USART_SendStr(USART3, "AT+CIPSEND\n\r");
      if (GetWiFiResponse(">"))  {}
      WiFiStep++;
      WifiConnectionError = 0;
      break;
		}
		case 10: {
			DebugX = 1;
  		if (++WifiConnectionError > 36) {
				DebugX   = 0;
				WiFiStep = 0;
			}
		  break;
		}
  }
}
void DisplaySSIDList(unsigned char List[20][30], unsigned char OldMenuItem, unsigned char MenuItem, unsigned char RefreshAllItems) {
	unsigned char FirstItem;
	int i, Loc;
  
  if ((MenuItem >= 12) || ((MenuItem == 0) && (OldMenuItem >= 12)) || ((MenuItem == 11) && (OldMenuItem == 12)))
		RefreshAllItems = 1;
	
	if (MenuItem < 12)
		FirstItem = 0;
	else
		FirstItem = MenuItem - 11;
	
  Loc = 0;
  for (i = 0; (i < 12) && (List[i+FirstItem][0] != '\0'); i++) {
    Loc += 35;
    if ((RefreshAllItems != 0) || ((FirstItem + i) == MenuItem) || ((FirstItem + i) == OldMenuItem)) {
      if ((FirstItem + i) == MenuItem)
      	GUI_DrawGradientV(0, Loc, 272, Loc + 36, GUI_MAKE_ALPHA(0x00, RGB(247, 96, 1)), GUI_MAKE_ALPHA(0x00, RGB(255, 172, 65)));
  	  else
      	GUI_DrawGradientV(0, Loc, 272, Loc + 36, GUI_MAKE_ALPHA(0x00, RGB(218, 223, 228)), GUI_MAKE_ALPHA(0x00, RGB(255, 255, 255)));
			GUI_SetColor(GUI_BLACK);
      PutText(0, Loc, 270, Loc + 35, List[i + FirstItem], GUI_TA_LEFT | GUI_TA_VCENTER);
	  }
  }//for
}
unsigned char ExecSSIDList(unsigned char List[20][30]) {
	unsigned char Key, MenuCurPos = 0;
	unsigned int Timeout;	

	GUI_Clear();
 	GUI_DrawGradientV(0, 0, 272, 35, GUI_MAKE_ALPHA(0x00, RGB(0xe3, 0xe6, 0xea)), GUI_MAKE_ALPHA(0x00, RGB(10, 10, 10)));
  DisplaySSIDList(List, 0, MenuCurPos, 1);
	Timeout = OS_TimeMS;
  while (1) {
  	if (OS_TimeMS-Timeout >= 15000) 
		  return 255;
    Key = ScanKeyboard();
		if (Key)
			Timeout = OS_TimeMS;
    switch (Key) {
      case BDOWN:
      case 'R': 
	      if (List[MenuCurPos+1][0] == '\0') {
 	        DisplaySSIDList(List, MenuCurPos, 0, 0);
          MenuCurPos = 0;
        }
				else{
      	  DisplaySSIDList(List, MenuCurPos, MenuCurPos + 1, 0);
          MenuCurPos++;
        }
    	  break;
      case BOK: 				
        List[MenuCurPos][29] = 0;
        strcpy((char *)Config.SSID, (const  char *)List[MenuCurPos]);
        Config.SSID[29] = 0;
        return MenuCurPos;
			case BLEFT:	
			case BCANCEL:	
        return 255;
    }//switch
  }//while
}
unsigned char SearchAndSelectSSID(void) {
	unsigned char FirstFlag = 0, Byte;
	unsigned char SSID_List[20][30], Count = 0, Index = 0;
	unsigned long int j, Li;
	unsigned short i;
	char *ptr;
	
	//DERE=1;
  GUI_SetColor(GUI_DARKBLUE);
  GUI_FillRoundedRect(5, 70, 267, 145, 0);
  GUI_SetColor(GUI_DARKMAGENTA);
  GUI_FillRoundedRect(5, 47, 267, 75, 0);	
  GUI_SetColor(GUI_WHITE);  
 
  PutText(0, 45, 270,100, "لطفا صبر کنيد", GUI_TA_CENTER);
	
  EmptyWIFIRXBuffer();
  USART_SendStr(USART3, "AT\n\r");
  GetWiFiResponse("OK");
   
  USART_SendStr(USART3, "AT+CWLAP\n\r");
	//DERE=1;
	Li = OS_TimeMS;
  GUI_SetColor(GUI_DARKBLUE);
  GUI_FillRoundedRect(5, 70, 267, 145,0);
  GUI_SetColor(GUI_YELLOW);
	while (1) {
		if (WIFI_GCF(&Byte))
			if (Byte == '+')
				break;
		j  = 210 * (OS_TimeMS - Li);
		j /= 8000;			
		GUI_FillRoundedRect(5, 70, 5+j, 145,0);
		if ((OS_TimeMS - Li) > 8000)
			return 0;
	}
 
  GetWiFiResponse("OK");
  //printf("\n\rWIFIReceiveBuf=%s,strlen(WIFIReceiveBuf)=%d\n\r",WIFIReceiveBuf,strlen((char *)WIFIReceiveBuf)); 
  WIFIReceiveBuf[800] = 0;
 
  Count     = 0;
  FirstFlag = 0;
  ptr = (char *)WIFIReceiveBuf;
  //printf("\n\r+++>%s ",ptr);
  i = 0;
  while (1) {
    if (Count >= 14)  break;
    if (ptr = strstr(ptr, "CWLAP:")) {
      i = 0;
      ptr = ptr + 5;
      //printf("\n\r==>Count=%d : %s",Count,ptr);
      
      while (1) {
        if (ptr[i] == 0)  break;
        if (ptr[i] == '"') {
          if (FirstFlag == 0) 
						FirstFlag = 1;
          else {
						FirstFlag = 0; 
            Count++; 
					  Index = 0; 
					  break;
					}
        }
				else {
          if (FirstFlag)
            if (Index < 29) {
              SSID_List[Count][Index++] = ptr[i];
              //SendByte(SSID_List[Count][Index]);                    
              SSID_List[Count][Index] = 0;
            }
        }
        i++;
      }
    }
    else 
			break;
  }

  SSID_List[Count + 1][0] = 0;
  GUI_SetFont(&GUI_Font20B_1);
	ExecSSIDList(SSID_List);
 	LoadFont(FontBKoodak40);
  return 3;
}

char WIFISend(unsigned int len, unsigned char *DataToSendRec) {
	unsigned int i = 0;
  //EmptyWIFIRXBuffer();
	while (i < len)
		USART_SendData(USART3, DataToSendRec[i++]);
  return 1;
}

char sendData(unsigned int len, unsigned char *data) {
	return WIFISend(len, data);
	/*return;
	
	char str[100];
	unsigned int now;
	
	EmptyWIFIRXBuffer();
	sprintf(str, "AT+CIPSEND=%d\n\r", len);
	WIFISend(strlen(str), str);
	
		{
		getReady = 0;
		WIFISend(len, data);
		sprintf(str, "\n\r");
		WIFISend(2, str);
	}*/
}
void USART3_IRQHandler(void) {
	int dataLen;
	volatile unsigned int IIR;
	unsigned char Byte;
	
  IIR = USART3->SR;
  if (IIR & USART_FLAG_ORE)        
    USART3->SR &= ~USART_FLAG_ORE;	          // clear interrupt

  if (IIR & USART_FLAG_TXE) 
    USART3->SR &= ~USART_FLAG_TXE;	          // clear interrupt

  if (IIR & USART_FLAG_RXNE) {
    USART3->SR &= ~USART_FLAG_RXNE;	          // clear interrupt
		Byte = (USART3->DR & 0x1FF);
		printf("%c", Byte);
		RXD3Buffer[rxd3_wr_index] = Byte;
		
		//Written by HNA
		//if \r\n is detected on the last of phrase
		//then check is it the +IPD response? (+IPD is received when a data is come from TCP socket)
		//The +IPD,Size:[data block in given size]
		if ((RXD3Buffer[rxd3_wr_index - 1] == '\r') && (RXD3Buffer[rxd3_wr_index] == '\n')) {
			char* result = strstr(RXD3Buffer, "+IPD");
			if (result != NULL) {
				char colon = strchr(result, ':') - result;
				char strLen[10] = {0};
				char data[100];
				memcpy(strLen, result + 5, colon - 5);
				dataLen = atoi(strLen);				
				memcpy(data, result + colon + 1, dataLen);
				EmptyWIFIRXBuffer();
				callback(data, dataLen);				
			}
		}

		if (++rxd3_wr_index == BUFFER3_SIZE) 
			rxd3_wr_index = 0;
    if (++rxd3_counter == BUFFER3_SIZE)
      rxd3_rd_index = rxd3_counter = rxd3_wr_index = 0;
  }
} 
