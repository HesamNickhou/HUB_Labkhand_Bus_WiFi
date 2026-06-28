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
#define DEBUG_HNA     //To log the important data
#define DEBUG_WIFI  	//For sending and receiving data
#define DEBUG_WIFI2	//For details AT Commands

extern unsigned char wifiBuffer[1200];
extern unsigned char DataIsReady;

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
	WIFIReceiveBuf[1024],
	RXD3Buffer[BUFFER3_SIZE],
							WIFI_GCF(unsigned char *c),
							WIFI_GCL(unsigned char *c),
	GPS_CheckSystemTime2(unsigned char GPSTimeChecked),
	 GPS_CheckSystemTime(void);

unsigned int
	WifiConnectionError = 0,
	rxd3_counter 				= 0,
	rxd3_wr_index 			= 0,
	rxd3_rd_index 			= 0,
	read_index          = 0,
	DataSize 						= 0;
	
unsigned short WIFIReceiveBufLen;

char DebugX = 0, started = 0;

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

typedef void (*WIFIListener)();
WIFIListener callback;

void setCallback(WIFIListener interface);

//HNA
void debug(const char* str_out, char reset) {
#ifdef DEBUG_HNA
	if (reset == 1)
		SetTX485();
	for (unsigned int k=0; k<strlen(str_out); k++)
		SendByte(str_out[k]);
#endif
}

void setCallback(WIFIListener interface) { callback = interface; }
void USART_SendStr(USART_TypeDef* USARTx, char *str) {
	
	#ifdef DEBUG_WIFI2
		debug("\n>> ", 0);
		debug(str, 0);
	#else
		GUI_X_Delay(100);
	#endif
	
	int i = 0;
  while (str[i] != 0) {
    switch (str[i]) {
      case '\n': USART_SendData(USARTx, 13); break;
      case '\r': USART_SendData(USARTx, 10); break;
      case '\"': USART_SendData(USARTx, 34); break;
      default  : USART_SendData(USARTx, str[i]);
    }
    i++;
  }
}
unsigned char WIFI_GCL(unsigned char *c) {
	int i = OS_TimeMS;
	while (1) {
  	if (i > OS_TimeMS) 
			return 0;
	  if ((OS_TimeMS - i) >= 3000) //3 sec
			return 0;
    if (rxd3_counter != 0) 
      break;
		GUI_Delay(1);
		WDTR;
	}
  *c = RXD3Buffer[rxd3_rd_index];
	//SendByte(RXD3Buffer[rxd3_rd_index]);
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
	//SendByte(RXD3Buffer[rxd3_rd_index]);
  if (++rxd3_rd_index >= BUFFER3_SIZE) 
		rxd3_rd_index = 0;
  if (rxd3_counter) 
		rxd3_counter--;
  return 1;
}

void EmptyWIFIRXBuffer(void) { 
	unsigned short i;
	#ifdef DEBUG_WIFI
	debug("<< ", 1);
	debug(RXD3Buffer, 0);
	#endif
	
	rxd3_rd_index = rxd3_counter = rxd3_wr_index = 0;
	memset(RXD3Buffer, 0, BUFFER3_SIZE);
	//for (i=0; i<BUFFER3_SIZE; i++)
		//RXD3Buffer[i] = 0;
}
unsigned char GetWiFiResponse(const char* Response) {
	int i, j;
	unsigned int now;
	unsigned char Byte, str[100];

	 // printf("\n\rResponse[%d](%s) : \n\r",WIFIReceiveBufLen,Response);/////////////
  WIFIReceiveBufLen = 0;
	while (WIFIReceiveBufLen == 0) {
    if (WIFI_GCL(&Byte)) {
      if ((Byte != 13) && (Byte != 10))
		    WIFIReceiveBuf[WIFIReceiveBufLen++] = Byte;  
    }
		else {
      return 1;
		}
  }
	
  while ((WIFI_GCF(&Byte)) && (WIFIReceiveBufLen < 1020))
    WIFIReceiveBuf[WIFIReceiveBufLen++] = Byte;
  WIFIReceiveBuf[WIFIReceiveBufLen] = 0;

  j = 0;
  for (i = 0; i<WIFIReceiveBufLen; i++) {
    if (Response[j] == '\0') {
			return 0;
		}
    else if (Response[j] == WIFIReceiveBuf[i]) 
			j++;
		else 
			j = 0;
  }
  return 2;
}


unsigned char safe_rilz(const char *ip, char* result) {
	//static char result[16];  // Static buffer - no malloc needed
	unsigned int octets[4];
	
	if (!ip) return 0;
	
	// Parse and validate the IP
	if (sscanf(ip, "%u.%u.%u.%u", 
					 &octets[0], &octets[1], &octets[2], &octets[3]) != 4)
		return 0;  // Invalid format
	
	// Validate each octet (0-255)
	for (int i = 0; i < 4; i++)
		if (octets[i] > 255) return 0;
	
	// Format without leading zeros
	sprintf(result, "%u.%u.%u.%u", 
					octets[0], octets[1], octets[2], octets[3]);
	return 1;
}
char* rilz(const char *ip_with_zeros) {
	if (!ip_with_zeros) return NULL;
	
	char input_copy[16];  // Max IP length is 15 chars + null terminator
	strncpy(input_copy, ip_with_zeros, sizeof(input_copy) - 1);
	input_copy[sizeof(input_copy) - 1] = '\0';
	
	char *octet;
	char *saveptr;
	int octet_count = 0;
	unsigned int octets[4];
	
	// Parse the IP address by splitting on dots
	octet = strtok_r(input_copy, ".", &saveptr);
	
	while (octet != NULL && octet_count < 4) {
			// Check if octet contains only digits
			for (char *c = octet; *c; c++) {
					if (!isdigit(*c)) {
							return NULL;  // Invalid character
					}
			}
			
			// Convert to integer to remove leading zeros
			unsigned int value = (unsigned int)atoi(octet);
			
			// Validate octet range (0-255)
			if (value > 255) {
					return NULL;  // Invalid value
			}
			
			octets[octet_count] = value;
			octet_count++;
			
			octet = strtok_r(NULL, ".", &saveptr);
	}
	
	// Valid IP must have exactly 4 octets and no extra data
	if (octet_count != 4 || octet != NULL) {
			return NULL;
	}
	
	// Allocate memory for result (max "255.255.255.255" = 15 chars + null = 16)
	char *result = (char*)malloc(16 * sizeof(char));
	if (!result) return NULL;
	
	// Format without leading zeros
	sprintf(result, "%u.%u.%u.%u", octets[0], octets[1], octets[2], octets[3]);
	
	return result;
}

char CheckWiFi(void) {
	unsigned char str[200];	
	char *ptr;
	if (WiFiStep < 10)
		EmptyWIFIRXBuffer();
	
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
		LoadFont(FontBKoodak40);
		GUI_SetColor(GUI_DARKGRAY);
		GUI_FillRoundedRect(4 + 234, 2 + 3, 30 + 233, 27 + 1, 3);	
		GUI_SetColor(GUI_LIGHTGREEN);
		GUI_DispDecAt(WiFiStep, 240, 0, 2);
  }
     
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
      USART_SendStr(USART3, "AT+CWMODE=1\n\r");  //client		
			GUI_Delay(500);
      USART_SendStr(USART3, "AT+CIPMODE=1\n\r");
			GUI_Delay(500);
			//USART_SendStr(USART3, "AT+CWDHCP=1,1\n\r"); //Enable DHCP Auto IP
     //  GetWiFiResponse("OK");  
      WiFiStep++;
      break;
		}
    case 4:	{ //Configure IP instead of DHCP
			char ip1[16], ip2[16], ip3[16];
			if (!safe_rilz(Config.LocalIP, ip1))
				sprintf(ip1, "123.456.789.0");
			
			if (!safe_rilz(Config.RoutIpAddress, ip2))
				sprintf(ip2, "123.456.789.0");
			
			if (!safe_rilz(Config.SubnetIP, ip3))
				sprintf(ip3, "255.255.255.0");
			
      sprintf(str, "AT+CIPSTA=\"%s\",\"%s\",\"%s\"\n\r", ip1, ip2, ip3);
			//sprintf(str, "AT+CIPSTA=\"192.168.1.40\",\"192.168.1.1\",\"255.255.255.0\"\n\r");		

      USART_SendStr(USART3, str);
		  //GetWiFiResponse("OK");
      WiFiStep++;
			
      break;
		}
    case 5: { //Connect to SSID and Password
      sprintf(str, "AT+CWJAP=\"%s\",\"%s\"\n\r", Config.SSID, Config.WiFiPassword); 
      USART_SendStr(USART3, str);
			WiFiStep++;
      ConnectionRetry = 0;
      break;
		}
    case 6: { //Check the connection status
      //EmptyWIFIRXBuffer(); 
      WIFIReceiveBuf[0] = 0;
      USART_SendStr(USART3, "AT+CIPSTATUS\n\r");
      // STATUS:2 = Got IP (AP connected, no TCP yet) -> proceed to TCP connect (step 7)
      // STATUS:3 = TCP already connected             -> skip TCP connect, go straight to step 8
      if (GetWiFiResponse("STATUS:2") == 0) {
				ConnectionRetry = 0;
        WiFiStep++;      // go to step 7 to open TCP connection
			}
      else if (GetWiFiResponse("STATUS:3") == 0) {
        // TCP already established (e.g. retry from step 8): skip CIPSTART
        ConnectionRetry = 0;
        WiFiStep = 8;    // jump straight to waiting for data
      }
      else {
        // Save a clean copy of the buffer before any pointer manipulation
        char *bufStart = (char *)WIFIReceiveBuf;
        int isBusy   = (strstr(bufStart, "bus")    != NULL);
        int hasStatus= (strstr(bufStart, "STATUS:") != NULL);

        if (isBusy || hasStatus) {
          if (++ConnectionRetry > 10) {
            if (isBusy) {
              // "busy p" from module: flush and retry status check, don't full-reset
              EmptyWIFIRXBuffer();
              WIFIReceiveBuf[0] = 0;
              USART_SendStr(USART3, "AT+CIPSTATUS\n\r");
              ConnectionRetry = 0;
              // stay on step 6
            } else {
              // Non-busy failure after many retries: full reset
              WiFiStep        = 0;
              ConnectionRetry = 0;
            }
          }
          // else: still within retry limit, stay on step 6
        }
        else if (++ConnectionRetry > 10) {
          // No recognisable status at all after many retries
          WiFiStep        = 0;
          ConnectionRetry = 0;
        }
      }
      break;
		}
    case 7: { //Establishing the connection
      //EmptyWIFIRXBuffer();
			/*sprintf(str, "AT+CIFSR\n\r");
			USART_SendStr(USART3, str);
			GUI_Delay(1000);*/
			
      sprintf(str, "AT+CIPSTART=\"TCP\",\"%s\",%d\n\r", rilz(Config.ServerIP), Config.ConnectionPort);
			//sprintf(str, "AT+CIPSTART=\"TCP\",\"192.168.1.52\",1212\n\r");
			//AT+CIPSTART="TCP","192.168.1.5",1212\n\r
      USART_SendStr(USART3, str);
      WiFiStep++;			
      //break;
		}
    case 8: { //Wait to get CONNECT phrase 
			if (GetWiFiResponse("CONNECT") == 0) {
        //USART_SendStr(USART3, "AT+CWMODE=1\n\r");
				DebugX   = 0;
		    WiFiStep = 9;
			}
			else {
				debug(".", 0);
				if (++ConnectionRetry > 10)
					WiFiStep = 6;
			}	
      break;
		}
    case 9: {
			//EmptyWIFIRXBuffer(); 
      //USART_SendStr(USART3, "AT+CIPMODE=1\n\r");
      //GetWiFiResponse("OK");
			
			//======= Enable TCP Server on Port 1212 =============
			//USART_SendStr(USART3, "AT+CIPSERVER=1,1212");
			
		/*
		 EmptyWIFIRXBuffer(); 
		      USART_SendStr(USART3, "AT+CIPMODE?\n\r");
		GUI_Delay(2000);
		GetWiFiResponse("$");
				 LoadFont(FontTahoma20);
         GUI_SetColor(GUI_RED); 
         PutText(5, 80, 272-38, 275+30+30+10, WIFIReceiveBuf, GUI_TA_CENTER);
		*/
		
      /*EmptyWIFIRXBuffer();
      WIFIReceiveBuf[0] = 0;
      USART_SendStr(USART3, "AT+CIPSEND\n\r");
      if (GetWiFiResponse(">"))  {}*/
      WiFiStep++;
      WifiConnectionError = 0;
      break;
		}
		case 10: {
  		if (++WifiConnectionError > 10) {
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
	//USART_SendStr(USART3, "AT+CIPSEND\n\r");
	GUI_Delay(1);
	#ifdef DEBUG_WIFI
	debug("\n>> ", 1);
	#endif
	while (i < len) {
		USART_SendData(USART3, DataToSendRec[i++]);
		#ifdef 	DEBUG_WIFI
		SendByte(DataToSendRec[i-1]);
		#endif
	}
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
		RXD3Buffer[rxd3_wr_index++] = Byte;

		if (WiFiStep > 8) {
			if (Byte == '{') {
				if (DataIsReady == 0)
					read_index = 0;
				started = 1;
				wifiBuffer[read_index++] = Byte;
			}
			else if ((started > 0) && (Byte == '}')) {
				wifiBuffer[read_index++] = Byte;
				wifiBuffer[read_index] = '\0';
				#ifdef DEBUG_WIFI
					debug("\n<< ", 1);
					for (unsigned int i=0; i<read_index; i++)
						SendByte(wifiBuffer[i]);
				#endif
				started = 0;
				callback();
			}
			else if (started > 0)
				wifiBuffer[read_index++] = Byte;
		}

		if (rxd3_wr_index >= BUFFER3_SIZE) 
			rxd3_wr_index = 0;
    if (++rxd3_counter >= BUFFER3_SIZE)
      rxd3_rd_index = rxd3_counter = rxd3_wr_index = 0;
  }
} 
