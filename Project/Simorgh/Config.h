#ifndef __CONFIG_H
#define __CONFIG_H	 

#include <includes.h>

/*
	USART1 => RS485 Serial
	USART2 => SAM CARD
	USART3 => ESP8266 or GSM
*/

//Labkhand Bus Door 
#define DeviceType   BUSDOOR_WIFI
#define DisplayType  TFT50 

/*
Faradis
--------------

add CLRC_CHIP >= in C++ Configuration
#define WithSAMCARD
#define ReaderType   NewRoutine

JahanGostar
--------------
//#define WithSAMCARD
#define ReaderType   RC522Routine
remove CLRC_CHIP >= in C++ Configuration
*/


//Labkhand Shahre Bazi
//#define DeviceType   APARK

#define Ver          50
#define Release      262

#define Simorgh         160
#define APARK           160
#define BUSDOOR         164
#define BUSDOOR_WIFI    165


#if (DeviceType == BUSDOOR)

	/*
	CLRC_CHIP

	براي ريدر 632 به پيکربندي اضافه شود
	*/


	//#define WithSAMCARD

	//#define ReaderType   NewRoutine
	#define ReaderType   RC522Routine
	
	#define SCREENRESIZE  201
	#define CoderType   ctNone
#endif

#if (DeviceType == BUSDOOR_WIFI)
	//#define WithSAMCARD
	//#define ReaderType   NewRoutine
	#define ReaderType   RC522Routine
	#define SCREENRESIZE  0
	#define DisplayType  TFT50 
	#define CoderType   ctNone
#endif

#if (DeviceType==APARK)
	#define SCREENRESIZE  0
	#define ReaderType   RC522Routine
	#define CoderType   ctSABA
#endif


#define LCD480_272  0
#define LCD800_480  1

#define ctNone     0
#define ctSABA     1
#define ctTaxi     2
#define ctParyaft  3



//#define OldRoutine     0
#define NewRoutine     1 //BUS
#define RC522Routine   2 //RC522

#define TFT22 1
#define TFT35 2
#define TFT24 4   
#define TFT5  5   

#define STX  2
#define ETX  3


#define Log(str)

/*

             [ MEMORY STRUCTURE ]
							     W25Q64
									 
     START			END         DESCRIPTION
    --------  --------		-------------------- 
		0x00000   0x80000     Firmware area
    0x80000	  0x100000    Configuration data	
		0x100000  0x200000		Resources area							 
		0x200000  0x300000    Transactions records
		0x300000  0x400000    Locations records
		0x400000  0x500000    Transaction backup
    0x500000  0x600000    Travel info
    0x600000  0x800000    Free memory 		

*/

//400Kb binary size
#define addFirmwareInfo       0  
#define addFirmware           512
#define addConfig             0x80000
#define addIndicators         addConfig + 4096

#define addVoices             addIndicators + 4096
#define addResources          0x100000

#define addTransactions       0x200000

#define addBlackListInfo      0x380000
#define addBlackList          0x381000

#define addAdvSch             0x600000
#define addAdvInfo            0x601000

#define addConfig2             0x700000


#define WDTR  IWDG->KR  = 0xAAAA

#define DATA_SIZE 2 * 1024
extern unsigned char _acBuffer[];
extern unsigned char LCDType;

/*
extern GUI_CONST_STORAGE GUI_FONT GUI_FontTahoma20;
extern GUI_CONST_STORAGE GUI_FONT GUI_FontBNazaniN140;
extern GUI_CONST_STORAGE GUI_FONT GUI_FontBNazanin100;
extern GUI_CONST_STORAGE GUI_FONT GUI_FontBNazanin60;
extern GUI_CONST_STORAGE GUI_FONT GUI_FontBKoodak40;
*/
extern unsigned char  TFT_START_Y; 
extern unsigned short TFT_HEIGHT;  

#ifdef Simorgh50N
extern unsigned char *FontBKoodak40;
extern unsigned char *FontBNazanin200;
extern unsigned char *FontBNazanin60;
extern unsigned char *FontTahoma20;
extern unsigned char *FontBNazanin140;
#endif
#ifdef Torgheh
#define FontBKoodak40 0
#define FontBNazanin200 1
#define FontBNazanin60 2
#define FontTahoma20 3
#define FontBNazanin140 4

#define RGB888_RGB565(color) ((((color) >> 19) & 0x1f) << 11) | ((((color) >> 10) & 0x3f) << 5) | (((color) >> 3) & 0x1f)
extern void GUI_DrawRect1(int X_1,int Y_1,int X_2,int Y_2);
extern unsigned short BkColor;

#endif

extern GUI_FONT ExtFont;

extern const unsigned int CRC16_table[256];
extern const unsigned int UCMap[255];
extern const unsigned char KeyTable[256];

extern unsigned char SDCardPresent;

extern __IO int32_t OS_TimeMS;

#define ON    1
#define OFF   2
#define SLEEP 3
extern unsigned char PowerMode;


#define  VI   0
#define  VII  1
extern unsigned char BOARD_VER;

//extern unsigned char TempArea[];
extern GUI_CONST_STORAGE GUI_FONT GUI_FontTahoma20;

typedef struct {
  unsigned int Password;
  unsigned long int Permissions; 
} Operator;

typedef struct {
  unsigned int  DeviceID;
	unsigned char deviceType;
  unsigned int  UC;
  unsigned int  Type;
  unsigned char BaudRate;
  unsigned char ConnectionType;
  unsigned char LNG;
  unsigned int LineNumber;
  unsigned char OperationMode;
  unsigned int TransactionsSendInterval;
  unsigned int DeviceInfoSendInterval;
  unsigned int TrackingInterval;
  unsigned int ConnectionRetry;
  char HostIP[15];
  char HostPort[10];
  char MainPassword[7];
  char Voice;
  char Beep; 
  unsigned int  Password;
  unsigned char RelayOption;
  unsigned char PrinterType;
  unsigned short Volume, Brightness; 
  unsigned int DriverID;
  unsigned int LineCode;

  unsigned char WorkModes;

  unsigned char DriverName[50];
  unsigned char CarNo[8];
  unsigned char LineTitle[50];

  unsigned char WorkMode;

  unsigned int  DriversID[4];
  unsigned char DriversName[4][50];

  unsigned char LocalIP[16];
  unsigned char ServerIP[16];  
  unsigned char RoutIpAddress[16];  
  unsigned char SubnetIP[16];  
  unsigned int  ConnectionPort;
  char SubNetMask[16];

	unsigned char TFTType;
	unsigned char SIMCardType;
	unsigned char SIMCardNumber[10];
  
  unsigned char PriceType;   
  unsigned char GrouhLoaded;   
  unsigned int  DefaultPrice;
  unsigned int  BusID;
  unsigned int  GrouhPrice[20][5]; 
	unsigned int  Price, G_GameTimer, Relay_Timeout;
	unsigned char SSID[50], WiFiPassword[30];
	unsigned char WiFi, TerminalID[10];
  unsigned char Title1[121], Title2[281];
	unsigned char X10;
  
} TConfig;

extern TConfig Config;

typedef struct {
  unsigned long int Transactions;
  unsigned long int Locations;
  unsigned long int OffTransactions;
  unsigned long int OffLocations;
  unsigned long int RegUsers;
  unsigned long int OfflineCounter;
  unsigned short Log;
  unsigned char ActivityTime_Year;
  unsigned char ActivityTime_Month;
  unsigned char ActivityTime_Day;
  unsigned char ActivityTime_Hour;
  unsigned char ActivityTime_Min;
  unsigned char ActivityTime_Sec;

  unsigned long int OperatorID;
	unsigned long int TransactionUID;
} TIndicators;

extern TConfig Config;
extern TIndicators Indicators;


typedef const struct {
	const char * Title;
	const unsigned int StartAddress;
	const unsigned int Len;
} TResource;

#if (DeviceType == Basket)
typedef struct {
  unsigned int  ID;
	unsigned char Members;
	unsigned char FName[50];
	unsigned char LName[50];
	unsigned char Quarters[50];
}TPerson;
extern TPerson Person;


typedef struct {
	unsigned short ID;
	unsigned short KalaCode;
	unsigned short Amount[15];
	unsigned char ExpirationDate[3];
	unsigned char Status;
}TSeries;

extern TSeries Series[];

typedef struct {
	unsigned short ID;
	unsigned char KalaType;
	unsigned char KalaUnit;
	unsigned char Title[23];
}TKalas;

extern TKalas Kalas[];
#endif


#ifdef Simorgh50N
void LoadFont(const unsigned char* Data);
#endif
#ifdef Torgheh
void LoadFont(unsigned char FileType);
#endif
unsigned char SaveFromRamToDF(unsigned int memoryOffset, unsigned int sizeToCopy, unsigned char *pDest);
unsigned char LoadFromDFToRam(unsigned int memoryOffset, unsigned int sizeToCopy, unsigned char *pDest);
unsigned char SaveConfiguration(void);
unsigned char LoadConfiguration(void);
unsigned char SaveIndicators(void);
unsigned char LoadIndicators(void);
unsigned char GetResourceAddress(char *title, unsigned int *StartAddress, unsigned int *Len);
void PutText(unsigned int left, unsigned int top, unsigned int right, unsigned int bottom, const unsigned char *str, unsigned char style);
extern s8 ConvertUTF8TOWin1256(u8 *StringUTF8, u8 *BuffStrWin1256);
extern s8 ConvertWin1256TOUTF8(u8 *StringWin1256, u8 *BuffStrUTF8);

extern void GenerateCardIdUC(unsigned char *buf,unsigned char *snr,unsigned int *_UC,unsigned int *_CardID);
#endif
