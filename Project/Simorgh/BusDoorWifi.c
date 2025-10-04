#include "Config.h"


#if (DeviceType == BUSDOOR_WIFI)

#include <stddef.h>
#include <stdio.h>
#include <math.h>
#include "app_cfg.h"
#include "key/Keypad.h"
#include "GUI.h"
#include "menu3d.h"
#include "gps/gps.h"
#include "crc/crc16.h"
#include "fatfs/ff.h"
#include "rtc/rtc.h"
#include "usart/usart.h"
#include "beep/beep.h"
#include "touch/touch.h"
#include "nrf24l01/24l01.h"
#include "flash/flash.h"
#include "ecard/ecard.h"
#include "ecardKashan.h"
#include <wm.h>
#include <rdlib/types/RdLib.h>
#include <LCDConf_stm3210e_eval.h>
#include "esp8266/esp8266.h"
#include "BusDoorWifi.h"
#define MAX_TRANSACTIONS  32500 
#define LEN_TRANSACTIONS  128
#define GlobalBufferLen   1200

//extern unsigned char WIFI_GCF(unsigned char *c);

//==============================================================================
//==============================================================================
//                        Data Flash Routines
//==============================================================================
//==============================================================================
unsigned int 
	trHead,
	trTail,
	loHead,
	loTail,
	BinSize								 = 0,
	_PreEtebar 						 = 20000,
	__ID 									 = 0,
	FirmwareLength 				 = 0,
	BlackList_Count 			 = 0,
	DataSendInterval 			 = 0,
	BlackList_LastIdx 		 = 0,
	BlackList_Index 			 = 0,
	FirmwareReqTimer 			 = 10,
	StopTime 							 = 0, 
	MotionTime 						 = 0,
	WaitForAddress 				 = 0,
	GWaitForAddress 			 = 0,
	PassengerTimerDistance = 0,
	DeltaSpeed 						 = 0,
	ManualStop 						 = 0,
	DeviceInfoSendInterval = 0,
	BufferLen 						 = 0,
	OldGPSSec 						 = 0,
	ServiceID 						 = 0,
	TransactionID 				 = 0;
	
unsigned char 
	OldMin										 = 0, 
	OldHour 									 = 0, 
	fCounter 									 = 0, 
	fState										 = 0,
	STATUSES 									 = 0xFF,   // MSB  8>LCD 7>Memory 6>RTC 5>Reader 4>OneWayTicket 3> 2> 1>   LSB
	OldDisplayedHour 					 = 0,
	OldDisplayedMin 					 = 0,
	OldDisplayedSec 					 = 0,
	Connected2BCU 						 = 0,
	SendAliveInterval 				 = 0,
	TouchPage 								 = 1,
	DataCurrentlySend 				 = 1,
	Theme 										 = 0,
	NeedToConnect 						 = 0,
	NeedToConnectLastTry 			 = 0,
	IncommingCall 						 = 0,
	CurrentEnhancedItem 			 = 0,
	NightMode 								 = 0,
	SaveALocation 						 = 0,
	Alarm 										 = 0,
	MapView 									 = 0,
	PaymentPage 							 = 0,
	Guest 										 = 0,
	MapViewByGPS 							 = 1,
	DeviceStatus 							 = 0xFF,
	GearBoxSensor 						 = 0,
	MapAvailable 							 = 1,
	NumGPSSent 								 = 0,
	cntDriverInfo 						 = 30,
	LEDOption 								 = 0, 
	ActivePort 								 = 1,
	OldSec 										 = 0,
	BatteryLevel 							 = 0,
	RequestType 							 = 0,
	DisplayEnhancedInformation = 0,
	LEDCONState 							 = 0,
	GPSReinit 								 = 1,
	Passenger 								 = 0,
	LEDDisplaySetting 				 = 0,
	ServiceRequest 						 = 0,
	CheckFirmware 						 = 0x81,
	ServiceRequestTimeout 		 = 0,
	GlobalBuffer[GlobalBufferLen],
	DriverName[102],
	LastCardingDateTime[10],
	PayInfo[30],
	LastCardSec, 
	GPSS, 
	l,
	LastCardSnr[10],
	Request_FirmwareNextPacketfromServer(void),
	IsBlocked(void);

unsigned short 
	MapPointer[16*16],
	PointLocation,
	SecLocation,
	FirmwareRequestNo,
	OldGPSSpeed,
	SaveInterval			 = 0,
	Brightness 				 = 0x80,
	DeltaTimes 				 = 0,
	COG 							 = 0,
	FirmwareRefquestNo = 0,
	RequestNo 				 = 0,
	PassengerTargetID  = 0;

int ConnectedToServer = 0;

char 
	InTestMode  = 0,
	CheckReader = 0;

struct {
	unsigned short COG;
	unsigned short Speed;
	float Distance;	
	unsigned long int Time;
} LastGPS;

struct CardInfo_Struct Card_Info[10];

//...........................................................

#include "Firmware.h"

extern FirmwareStruct       Firmware;

extern unsigned int 
	Downloaded_FirmwareLength,
	FirmwarePercent,
	Percent,
	rxd3_counter,
	rxd3_wr_index,
	rxd3_rd_index;

extern unsigned char 
	InFirmwareDownloading,
	IsFileForGprsDownload,
	Check_ServerForUpdateFile,
	CheckUpdateTry,
	WiFiStep,
	WIFI_GCF(unsigned char *c),
	WIFI_GCL(unsigned char *c);

extern unsigned short FirmwarePacketIndex;
extern u8 SPI_FLASH_BUF[4096];

void Request_UploadedFirmwaresPropertiesFromServer(void);

//........................................................... 
	
extern TGPSData GPSData;
void SendOfflines(void);

//unsigned char InCity=0;

float 
	OldLon = 0, 
	OldLat = 0,
	LatOld, 
	LongOld;
int InfoCounter = 0, PassengerTimer = 0;     
unsigned long int LivingTime = 0;
signed long int Charge = -1;

TDeviceInfo DeviceInfo;

static union {
  unsigned char Buf[100];
  TDeviceInfo DeviceInfo;
} TaxiDeviceInfoTransfer;


union {
  unsigned char Buf[5];
  float f;
}uFloat;


signed short int CurrentCourse = -1, OldCourse = -1;

const unsigned char 
	*strDays[7]    = {{"يکشنبه"}, {"دوشنبه"}, {"سه شنبه"}, {"چهارشنبه"}, {"پنج شنبه"}, {"جمعه"}, {"شنبه"}},
  *strMonths[13] = {{"فروردين"}, {"ارديبهشت"}, {"خرداد"}, 
										{"تير"},     {"مرداد"},    {"شهريور"}, 
									  {"مهر"}, 		 {"آبان"}, 		 {"آذر"}, 
										{"دي"}, 		 {"بهمن"}, 		 {"اسفند"}};
	
void ShowCardingResult(char Mode,unsigned int Pay,unsigned int _UC,unsigned int _ID,unsigned char Result,unsigned char *);
unsigned char SendALive();

void DisplayToolbar(unsigned char c) {}

void WifiCallback(char* data, char len) {
	char i;	
	/*unsigned int sum = 0;
	for (i=0; i<len; i++)
		sum += data[i];
	GUI_SetColor(GUI_RED);
	GUI_DispDecAt(sum, 10, 30, 3);*/
	//PlayBip(1);
	SendALive();
}
	
void SaveRingDetail(void){
	unsigned int i;
	unsigned short crc=0;
	unsigned char buf[20];
	
  buf[0]  =  trHead 			 & 0xFF;
  buf[1]  = (trHead >> 8)  & 0xFF;
  buf[2]  = (trHead >> 16) & 0xFF;
  buf[3]  = (trHead >> 24) & 0xFF;
  buf[4]  =  trTail 			 & 0xFF;
  buf[5]  = (trTail >> 8)  & 0xFF;
  buf[6]  = (trTail >> 16) & 0xFF;
  buf[7]  = (trTail >> 24) & 0xFF;
  buf[8]  =  loHead 			 & 0xFF;
  buf[9]  = (loHead >> 8)  & 0xFF;
  buf[10] = (loHead >> 16) & 0xFF;
  buf[11] = (loHead >> 24) & 0xFF;
  buf[12] =  loTail 			 & 0xFF;
  buf[13] = (loTail >> 8)  & 0xFF;
  buf[14] = (loTail >> 16) & 0xFF;
  buf[15] = (loTail >> 24) & 0xFF;
  crc = 0;
  for (i=0; i<16; i++)
    crc += buf[i];
  buf[16] =  crc & 0xFF;
  buf[17] = (crc>>8) & 0xFF;
  buf[18] = 'N';
  buf[19] = 'W';
 	FLASH_UNLOCK
  SaveFromRamToDF(addIndicators, 20, buf);
 	FLASH_LOCK
}

unsigned char LoadRecordsInfo(void) {
	unsigned int i;
	unsigned short crc=0;
	unsigned int OfflineStart, Address, Idx, Idx2;
	unsigned char bcc, buf[LEN_TRANSACTIONS];

  //printf("\n\r===================================");
  LoadFromDFToRam(addIndicators, 20, buf);


	BytesToInt(&trHead, &buf[0]);
  //trHead=buf[3]; trHead<<=8;
  //trHead+=buf[2]; trHead<<=8;
  //trHead+=buf[1]; trHead<<=8;
  //trHead+=buf[0];
	BytesToInt(&trTail, &buf[4]);
  //trTail=buf[7]; trTail<<=8;
  //trTail+=buf[6]; trTail<<=8;
  //trTail+=buf[5]; trTail<<=8;
  //trTail+=buf[4];
	//BytesToInt(&prHead, &buf[8]);
  //prHead=buf[11]; prHead<<=8;
  //prHead+=buf[10]; prHead<<=8;
  //prHead+=buf[9]; prHead<<=8;
  //prHead+=buf[8];
	
	crc = 0;
  for (i=0; i<16; i++)
    crc += buf[i];
  if ((buf[16] + (buf[17] * 256)) != crc)
    trHead = trTail = 0;	
	
  //printf("\n\r===================================");
  //printf("\n\rFirst Values: trHead: %d trTail: %d", trHead, trTail);
  if ((trHead > MAX_TRANSACTIONS) || (trTail > MAX_TRANSACTIONS))
    trHead = trTail = 0;
  Idx  = trHead; 
	Idx2 = trTail;

  for (i=0; i<MAX_TRANSACTIONS; i++) {   
    Address = addTransactions + ((Idx + 1) * LEN_TRANSACTIONS) - 2;
    LoadFromDFToRam(Address, 2, buf);
		//printf("\n\r%d,buf[0]=%x,buf[1]=%x,buf[2]=%x,buf[3]=%x",Idx,buf[0],buf[1],buf[2],buf[3]);
    if ((buf[0] == 0xff) && (buf[1] == 0xff)) { //3c 14
      //printf("\n\rFound");
      trHead = Idx;
      trTail = Idx2;
      break;
    }
    Idx++;
    if (Idx >= MAX_TRANSACTIONS) 
			Idx = 0;
    if (Idx == Idx2)
      if (++Idx2 >= MAX_TRANSACTIONS)
        Idx2 = 0;
  }
  //printf("\n\rFound Values: trHead: %d trTail: %d", trHead, trTail);
  if (i >= MAX_TRANSACTIONS) {
    trHead = 0;
    trTail = 0;
  }
  
  //-----------------------------
  if (trHead != trTail) {
    Idx = trHead;
    OfflineStart = trTail;
    if (Idx > 0)
      Idx--;
    else if (trTail != 0)
      Idx = MAX_TRANSACTIONS - 1;
    
    //printf("\n\rCheck Offline ");
    for (i=0; i<MAX_TRANSACTIONS; i++) {
      if (Idx == trTail)
        break;

      Address = addTransactions + (Idx * LEN_TRANSACTIONS); 
      LoadFromDFToRam(Address+LEN_TRANSACTIONS - 1, 1, buf);
      //printf("- %d, buf[0] =%x ",Idx,buf[0]);			
      if (buf[0] == 0x90) {
        //printf("\n\rOff Found");
        OfflineStart=Idx+1;
        if(OfflineStart>=MAX_TRANSACTIONS) OfflineStart=0;
        break;
      }
      if(Idx==0)
        Idx=MAX_TRANSACTIONS-1;
      else 
        Idx--;
    }
    Indicators.OffTransactions = (OfflineStart > trHead) ? 
			(MAX_TRANSACTIONS - OfflineStart) + trHead : trHead - OfflineStart;
  } else 
		Indicators.OffTransactions = 0;
  
  SaveRingDetail();
	
  //printf("\n\rIndicators.OffTransactions=%d ",Indicators.OffTransactions);
  LoadFromDFToRam(addBlackListInfo, 8, buf);
	i=buf[2]; i<<=8;
	i+=buf[1]; i<<=8;
	i+=buf[0]; 
	BlackList_Count=i;
	i=buf[6]; i<<=8;
	i+=buf[5]; i<<=8;
	i+=buf[4]; i<<=8;
	i+=buf[3]; 
	BlackList_LastIdx=i;
	bcc=0xBC^buf[0]^buf[1]^buf[2]^buf[3]^buf[4]^buf[5]^buf[6];
	if(bcc!=buf[7])
	{
		BlackList_Count=0;
		BlackList_LastIdx=0;
	}

}


void IncTrHead(void) {
  if (++trHead >= MAX_TRANSACTIONS)
    trHead = 0;
  if (trHead == trTail)
    if (++trTail >= MAX_TRANSACTIONS)
      trTail = 0; 
	if (trTail>trHead)	{
		//if Head and Tail come in same sector tail must jump to next sector
		if ((trHead / (4096 / LEN_TRANSACTIONS)) == (trTail / (4096 / LEN_TRANSACTIONS)))
			trTail += (4096 / LEN_TRANSACTIONS) - (trTail % (4096 / LEN_TRANSACTIONS));
    if (++trTail >= MAX_TRANSACTIONS)
      trTail = 0;
	}
}

unsigned char SaveTransaction(unsigned char *buf) {
	unsigned int address;	
	unsigned char TBuff[512];
	unsigned short BCC, i, Idx = 0, len = 0;

	len = LEN_TRANSACTIONS;
	for (i = 0; i < len; i++)
    TBuff[i] = buf[i];

	TBuff[len - 1] = 0x50;	
	TBuff[len    ] = 0x3C;
  TBuff[len + 1] = 0x14;
  TBuff[len + 2] = 0xB1;

  address = addTransactions + (trHead * len);
  if (SaveFromRamToDF(address, len + 3, TBuff))
		if (SaveFromRamToDF(address, len + 3, TBuff)) {
			IncTrHead();
      address = addTransactions + (trHead * len);
		  if (SaveFromRamToDF(address, len + 3, TBuff)) {}
		}

	IncTrHead();
	if (trHead >= MAX_TRANSACTIONS) {
    TBuff[0] = 0x3C;
    TBuff[1] = 0x14;
    TBuff[2] = 0xB1;
    SaveFromRamToDF(address, len + 3, TBuff);
    trHead = 0;
  }
} 

void MEMError(void) {
	unsigned char i;
}

unsigned char SendPacket(unsigned int Len) {
	sendData(Len, GlobalBuffer);
	
	
	/*unsigned int i;
	unsigned char Byte;	
	
	SetRX485();
  EmptyRXBuffer();
  for(i=0; i<10; i++) if(!USART_GCF(&Byte)) break; 
  if(i>=10) 
		return(1);
	
  if(i>1) GUI_Delay(Config.DeviceID*5);  //Wait to ensure to line is free
  if(USART_GCF(&Byte)!=0) 
		return(2); //Line locked for other device communication
           
  SetTX485();
  //GUI_Delay(1);

  for(i=0; i<Len; i++)
    SendByte(GlobalBuffer[i]);
	GUI_Delay(5);
	SetRX485();*/

	return(0);

}

/*
	Check if the read UC code from card is exists in the valid UCs?
@param UC: the UC of read card
@return index of valid UC, return 0xFF if not exists in the valid UCs
*/
unsigned char GetUCMap(unsigned int UC) {
	unsigned int i;
  for (i = 0; i < 255; i++)
    if (UCMap[i] == UC)
      return i;
	return 0xFF;
}


void ShowCardHoldMessage(void){}

unsigned int uInt(unsigned char *data){
	unsigned int i;
  i  = data[3]; i <<= 8;
  i += data[2]; i <<= 8;
  i += data[1]; i <<= 8;
  i += data[0];  
  return(i);
}

void DisplayText(unsigned char *str){}

void ShowPaymentBox2(void){
	unsigned char i,Key=0;
 char str[100];

 unsigned long int ID,CardID,Li;
 long int Credit=0;

static const GUI_POINT _aPointHexagon[] = {
	{ -50, 0 },
	{ -40, -15 },
	{ 40, -15 },
	{ 50, 0 },
};
static const GUI_POINT _aPointHexagon1[] = {
	{ -60, 0 },
	{ -40, -20 },
	{ 40, -20 },
	{ 60, 0 },
};

static const GUI_POINT _aPointHexagon2[] = {
	{ -100, 0 },
	{ -80, 25 },
	{ 80, 25 },
	{ 100, 0 },
};
	
static const GUI_POINT _aPolig1[] = {
	{ 0, 0 },
	{ 0, -30 },
	{ 50-10, -30 },
	{ 35-10, 0 },
};

static const GUI_POINT _aPolig3[] = {
	{ 0+40+1, 0+1 },
	{ 15+40+1, -30-1 },
	{ 210+50-1, -30-1 },
	{ 210+50-1, 0+1 },
};

 
 GUI_SetColor(0x008000FF);	
 GUI_DrawRoundedFrame(5, 2, 272-5,30,6,2);

 //........................................................................................
 GUI_DrawGradientRoundedV(5, 20+15, 272-5, 225+30+15-40,6,  0x00FF00FF,0x007F00FF);	
 GUI_DrawGradientRoundedV(5, 71, 272-5, 225+30+15-40,6,  GUI_WHITE,GUI_LIGHTGRAY);	
  
 GUI_SetColor(0x00FF00FF);	
 GUI_FillRect(5, 20+15+20, 272-5, 20+15+40);

 GUI_SetColor(0x008000FF);
 GUI_DrawRoundedFrame(5, 20+15, 272-5, 225+30+15-40,6,3); 

 GUI_SetColor(GUI_BLACK);
 GUI_DrawLine(5+3,20+15+40,272-8,20+15+40);
 
 GUI_SetColor(0x008000FF);
 GUI_FillPolygon(&_aPointHexagon1[0], 4, 135, 225+30+15-40);
 
 GUI_SetColor(0x000f0000);
 
 LoadFont(FontBNazanin200);
 sprintf(str,"%d",Config.GrouhPrice[0][0]);
 PutText(0, 50, 272, 350, str, GUI_TA_CENTER);
 
  LoadFont(FontBKoodak40);

	GUI_SetColor(GUI_BLACK);	
  PutText(0, 20+15, 272, 20+15+50, "نرخ بليت - واحد تومان مي باشد", GUI_TA_CENTER);

	GUI_SetColor(GUI_YELLOW);

  sprintf(str,"آفلاين :%d",Indicators.OffTransactions);
  PutText(100, -3, 230, 25, str, GUI_TA_RIGHT);
	
 GUI_SetColor(GUI_BLACK);
 GUI_FillRect(5, 225+10, 272, 265); 

 GUI_SetColor(0x008000FF);
 GUI_DrawRoundedFrame(5, 225+10, 272-5, 265,6,3); 

 GUI_SetColor(GUI_WHITE);  
  if((Month>=1)&&(Month<=12))
  {
    if(DayOfWeek<7)
      sprintf(str, "%s  %d  %s  %d ",strDays[DayOfWeek],Day,strMonths[Month-1],Year);
    else
      sprintf(str, "%d  %s  %d ",Day,strMonths[Month-1],Year);
 
		PutText(0, 233, 271, 263, str, GUI_TA_CENTER);
  }
}

void ShowPaymentBox(void) {
 unsigned char i,Key=0;
 char str[120];
	
 static const GUI_POINT _aPointHexagon1[] = {
	{ -60, 0 },
	{ -40, -20 },
	{ 40, -20 },
	{ 60, 0 },
 };	
 
 if(LCDType==LCD480_272)
 {
	 ShowPaymentBox2();
	 return;
 }
 //........................................................................................

 GUI_DrawGradientRoundedV(5, 20+15, 272-5+SCREENRESIZE, 225+30+15,6,  GUI_DARKBLUE,GUI_DARKBLUE);	
 GUI_DrawGradientRoundedV(5, 71, 272-5+SCREENRESIZE, 225+30+15,6,  GUI_WHITE,GUI_CYAN);	
 
 GUI_SetColor(GUI_DARKBLUE);	
 GUI_FillRect(5, 20+15+20, 272-5+SCREENRESIZE, 20+15+40);

 GUI_SetColor(GUI_BLUE);
 GUI_DrawRoundedFrame(5, 20+15, 272-5+SCREENRESIZE, 225+30+15,6,3); 
 GUI_DrawLine(5,20+15+40,272-5+SCREENRESIZE,20+15+40);
 
 GUI_SetColor(GUI_BLUE);
 GUI_FillPolygon(&_aPointHexagon1[0], 4, (272+SCREENRESIZE)/2, 225+30+15);	
 
 LoadFont(FontBNazanin200);
 GUI_SetColor(0x000f0000);
 sprintf(str,"%d",Config.GrouhPrice[0][0]);
 PutText(0, 70, 272+SCREENRESIZE, 400, str, GUI_TA_CENTER);
 LoadFont(FontBKoodak40);

 GUI_SetColor(GUI_WHITE);	
 PutText(0, 20+15, 272+SCREENRESIZE, 20+15+50, "نرخ بليت - واحد تومان", GUI_TA_CENTER);
 WDTR;


 GUI_DrawGradientRoundedV(5, 275+35, 272-38+SCREENRESIZE, 275+35+30,6,  GUI_WHITE,GUI_CYAN);	


 GUI_SetColor(GUI_DARKGREEN);
 GUI_DrawRoundedFrame(5, 275+35, 272-38+SCREENRESIZE, 275+35+30,6,3); 
 
 /*
 LoadFont(FontTahoma20);
	if(LastCardingDateTime[0]!=0)
	{
    GUI_SetColor(GUI_DARKRED); 
   sprintf(str,"%d/%d/%d  %d:%d:%02d",LastCardingDateTime[0],LastCardingDateTime[1],LastCardingDateTime[2],LastCardingDateTime[3],LastCardingDateTime[4],LastCardingDateTime[5]);
   //PutText(5, 275+30+10, 272-38+SCREENRESIZE, 275+30+30+10, str, GUI_TA_CENTER);
	}
	
 LoadFont(FontBKoodak40);
 */
 
}

void ShowCardingResult(char Mode,unsigned int Pay,unsigned int _UC,unsigned int _ID,unsigned char Result,unsigned char *Text) {
 unsigned char i,Key=0;
 char str[120];
 unsigned int HeaderColor=GUI_DARKGREEN,ButtomColor=0x007F8000,TopColor=GUI_GREEN,BoarderColor=GUI_YELLOW;	
 static const GUI_POINT _aPointHexagon1[] = {
	{ -60, 0 },
	{ -40, -20 },
	{ 40, -20 },
	{ 60, 0 },
 };	
 //........................................................................................
 if(Mode==0)
 {
	LEDERR=1;
  HeaderColor=0x00007000;
	ButtomColor=0x007F8000;
	TopColor=GUI_GREEN;		
	BoarderColor=GUI_YELLOW;
 }
 else if(Mode==1)
 {
	//LEDERR=1;
  HeaderColor=GUI_DARKRED;
	ButtomColor=GUI_LIGHTRED;
	TopColor=GUI_RED;		
	 BoarderColor=GUI_RED;
 }
 else if(Mode==2)
 {
	 //LEDERR=1;
  HeaderColor=GUI_DARKRED;
	ButtomColor=GUI_LIGHTRED;
	TopColor=GUI_RED;		
	BoarderColor=GUI_RED;
 } 
 else if(Mode==3)
 {
	//LEDERR=1; 
  HeaderColor=GUI_DARKRED;
	ButtomColor=GUI_LIGHTRED;
	TopColor=GUI_RED;		
	BoarderColor=GUI_RED;
 }  
 else if(Mode==4)  //sam error
 {
	//LEDERR=1; 
  HeaderColor=GUI_DARKRED;
	ButtomColor=GUI_LIGHTRED;
	TopColor=GUI_RED;		
	BoarderColor=GUI_RED;
 }   
 //.....................................................................................
 if(LCDType==LCD800_480)
 {
   GUI_DrawGradientRoundedV(5, 20+15, 272-5+SCREENRESIZE, 225+30+15,6,  HeaderColor,HeaderColor);	
   GUI_DrawGradientRoundedV(5, 71, 272-5+SCREENRESIZE, 225+30+15,6,  TopColor,ButtomColor);	
 
   GUI_SetColor(HeaderColor);	
   GUI_FillRect(5, 20+15+20, 272-5+SCREENRESIZE, 20+15+40);

   GUI_SetColor(BoarderColor);
   GUI_DrawRoundedFrame(5, 20+15, 272-5+SCREENRESIZE, 225+30+15,6,3); 
   GUI_DrawLine(5,20+15+40,272-5+SCREENRESIZE,20+15+40);
   GUI_FillPolygon(&_aPointHexagon1[0], 4, (272+SCREENRESIZE)/2, 225+30+15);	
 }else{
   GUI_SetColor(0x008000FF);	
   GUI_DrawRoundedFrame(5, 2, 272-5,30,6,2);

   GUI_DrawGradientRoundedV(5, 20+15, 272-5, 225+30+15-40,6,  HeaderColor,HeaderColor);	
   GUI_DrawGradientRoundedV(5, 71, 272-5, 225+30+15-40,6,  TopColor,ButtomColor);	
  
   GUI_SetColor(HeaderColor);	
   GUI_FillRect(5, 20+15+20, 272-5, 20+15+40);

   GUI_SetColor(BoarderColor);
   GUI_DrawRoundedFrame(5, 20+15, 272-5, 225+30+15-40,6,3); 

   GUI_SetColor(BoarderColor);
   GUI_DrawLine(5+3,20+15+40,272-8,20+15+40);
 
   GUI_SetColor(BoarderColor);
   GUI_FillPolygon(&_aPointHexagon1[0], 4, 135, 225+30+15-40);
 }
 //.....................................................................................
 if(Mode==0)
   {
    LoadFont(FontBNazanin200);
    GUI_SetColor(GUI_BLACK);
    sprintf(str,"%d",Pay);
		 PutText(0, 70, 272+SCREENRESIZE-(LCDType==LCD480_272?SCREENRESIZE:0), 400, str, GUI_TA_CENTER);
 
    LoadFont(FontBKoodak40);

    GUI_SetColor(GUI_WHITE);	
    PutText(0, 20+15, 272+SCREENRESIZE-(LCDType==LCD480_272?SCREENRESIZE:0), 20+15+50, "مانده اعتبار - واحد تومان", GUI_TA_CENTER);
		VOICEEN=1;
		
		CheckReader=1; 
		PlayVoice("08.wav");
		CheckReader=0;
		 
		VOICEEN=0; 
	 }
 else if(Mode==1)
   {
    LoadFont(FontBNazanin200);
    GUI_SetColor(GUI_BLACK);
    sprintf(str,"%d",Pay);
    PutText(0, 70, 272+SCREENRESIZE-(LCDType==LCD480_272?SCREENRESIZE:0), 400, str, GUI_TA_CENTER);
 
    LoadFont(FontBKoodak40);

    GUI_SetColor(GUI_WHITE);	
    PutText(0, 20+15, 272+SCREENRESIZE-(LCDType==LCD480_272?SCREENRESIZE:0), 20+15+50, "اعتبار کافي نمي باشد ", GUI_TA_CENTER);
		 
    GUI_SetColor(GUI_BLACK);	
    PutText(0, 225-25, 272+SCREENRESIZE-(LCDType==LCD480_272?SCREENRESIZE:0), 225+50, "تومان", GUI_TA_CENTER);
		 
		VOICEEN=1;
		CheckReader=1; 
		PlayVoice("04.wav");
		CheckReader=0; 
		VOICEEN=0; 		 
	 } 
 else if(Mode==2)
   {
    LoadFont(FontBNazanin140);
    GUI_SetColor(GUI_BLACK);
    sprintf(str,"%d",_UC);
    PutText(0, 60, 272+SCREENRESIZE-(LCDType==LCD480_272?SCREENRESIZE:0), 400, str, GUI_TA_CENTER);
 
    sprintf(str,"%d",_ID);
    PutText(0, 148, 272+SCREENRESIZE-(LCDType==LCD480_272?SCREENRESIZE:0), 400, str, GUI_TA_CENTER);
		 
    LoadFont(FontBKoodak40);

    GUI_SetColor(GUI_WHITE);	
    PutText(0, 20+15, 272+SCREENRESIZE-(LCDType==LCD480_272?SCREENRESIZE:0), 20+15+50, "کارت نامعتبر مي باشد.", GUI_TA_CENTER);
		 
		VOICEEN=1;
		PlayVoice("03.wav");
		VOICEEN=0; 		 
	 } 
 else if(Mode==3)
   {
    LoadFont(FontBNazanin200);
    GUI_SetColor(GUI_BLACK);
    sprintf(str,"%d",Result);
    PutText(0, 70, 272+SCREENRESIZE-(LCDType==LCD480_272?SCREENRESIZE:0), 400, str, GUI_TA_CENTER);
 
    LoadFont(FontBKoodak40);

    GUI_SetColor(GUI_WHITE);	
    PutText(0, 20+15, 272+SCREENRESIZE-(LCDType==LCD480_272?SCREENRESIZE:0), 20+15+50, "خطا در عمليات شارژ", GUI_TA_CENTER);
		 
		VOICEEN=1;
		CheckReader=1; 		 
		PlayVoice("03.wav");
		CheckReader=0; 		 
		VOICEEN=0; 		 
	 } 	 
  else if(Mode==4)
   {

    GUI_SetColor(GUI_BLACK);
    GUI_SetColor(GUI_WHITE);	
    PutText(20, 20+60, 272-20+SCREENRESIZE-(LCDType==LCD480_272?SCREENRESIZE:0), 20+15+150,Text, GUI_TA_CENTER);
		 
		VOICEEN=1;
		PlayVoice("03.wav");
		VOICEEN=0; 		 
	 } 
	 
 WDTR;
	 
 if(LCDType==LCD800_480)
 {
   GUI_SetColor(GUI_BLACK);
   GUI_FillRoundedRect(5, 275, 272-5+SCREENRESIZE-(LCDType==LCD480_272?SCREENRESIZE:0), 275+30,6);	

   GUI_SetColor(GUI_DARKGREEN);
   GUI_DrawRoundedFrame(5, 275, 272-5+SCREENRESIZE-(LCDType==LCD480_272?SCREENRESIZE:0), 275+30,6,3); 
 }
}


void ConnectionMark(GUI_COLOR color){		
	GUI_SetColor(color); 
	
  if(LCDType==LCD480_272)
	{
	  GUI_FillCircle(10+234,15+1,4);
    GUI_DrawLine(10+234,15+1,22+234,8+3);
    GUI_FillCircle(22+234, 8+3, 4);
    GUI_DrawLine(10+234, 15+1, 22+234, 20+2);
    GUI_FillCircle(22+234, 20+2, 4);
	}else{
    GUI_FillCircle(10+234+SCREENRESIZE,15+1,4);
	  GUI_DrawLine(10+234+SCREENRESIZE,15+1,22+234+SCREENRESIZE,8+3);
	  GUI_FillCircle(22+234+SCREENRESIZE, 8+3, 4);
	  GUI_DrawLine(10+234+SCREENRESIZE, 15+1, 22+234+SCREENRESIZE, 20+2);
	  GUI_FillCircle(22+234+SCREENRESIZE, 20+2, 4);
	}
	
}


void ShowPageAlef2(void) {
	unsigned char i,Key=0;
	char str[150];

	unsigned long int ID,CardID,Li;
	long int Credit=0;

	static const GUI_POINT _aPointHexagon[] = {
		{ -50, 0 },
		{ -40, -15 },
		{ 40, -15 },
		{ 50, 0 },
	};
	static const GUI_POINT _aPointHexagon1[] = {
		{ -60, 0 },
		{ -40, -20 },
		{ 40, -20 },
		{ 60, 0 },
	};

	static const GUI_POINT _aPointHexagon2[] = {
		{ -100, 0 },
		{ -80, 25 },
		{ 80, 25 },
		{ 100, 0 },
	};

		
	static const GUI_POINT _aPolig1[] = {
		{ 0, 0 },
		{ 0, -30 },
		{ 50-10, -30 },
		{ 35-10, 0 },
	};



	static const GUI_POINT _aPolig3[] = {
		{ 0+40+1, 0+1 },
		{ 15+40+1, -30-1 },
		{ 210+50-1, -30-1 },
		{ 210+50-1, 0+1 },
	};


 GUI_DrawGradientV(0, 0, 272, 480,  GUI_MAKE_ALPHA(0,0),GUI_MAKE_ALPHA(0,0));

 GUI_DrawGradientRoundedV(5, 3, 272-5, 30,6,  0x000F0000,0x00070000);	

 
 GUI_SetColor(0x008000FF);	
 GUI_DrawRoundedFrame(5, 2, 272-5,30,6,2);

 //........................................................................................
 GUI_DrawGradientRoundedV(5, 20+15, 272-5, 225+30+15-40,6,  0x00FF00FF,0x007F00FF);	
 GUI_DrawGradientRoundedV(5, 71, 272-5, 225+30+15-40,6,  GUI_WHITE,GUI_LIGHTGRAY);	
  
 GUI_SetColor(0x00FF00FF);	
 GUI_FillRect(5, 20+15+20, 272-5, 20+15+40);

 GUI_SetColor(0x008000FF);
 GUI_DrawRoundedFrame(5, 20+15, 272-5, 225+30+15-40,6,3); 

 GUI_SetColor(GUI_BLACK);
 GUI_DrawLine(5+3,20+15+40,272-8,20+15+40);
 
  GUI_SetColor(0x008000FF);
 GUI_FillPolygon(&_aPointHexagon1[0], 4, 135, 225+30+15-40);
 //........................................................................................
 GUI_SetColor(0x008000FF);
 GUI_DrawRoundedFrame(5, 225+10, 272-5, 265,6,3); 

 GUI_SetColor(GUI_WHITE); 
 GUI_DrawRoundedRect(4+234,2+3,30+233,27+1,3);	
	
	

 if(LCDType==LCD800_480)
 {
   GUI_FillRoundedRect(9, 270, 100, 400,6); 	
   GUI_DrawGradientRoundedH(90, 270, 272-5, 400,6,  GUI_WHITE,GUI_LIGHTGRAY);	
 }
 else
 {
   GUI_SetColor(GUI_LIGHTGRAY);
	 GUI_FillRoundedRect(9, 270, 272-5, 400,6);
 }
 
 GUI_SetColor(0x008000FF);
 GUI_FillPolygon(&_aPointHexagon2[0], 4, 140, 225+30+13);
 GUI_DrawRoundedFrame(5, 268, 272-5, 400,6,3); 		 
 //........................................................................................

 LoadFont(FontBNazanin140);

 sprintf(str,"%d%d ",Hour/10,Hour%10);
 PointLocation=GUI_GetStringDistX(str)+40-10;

 sprintf(str, "%d%d", Min / 10, Min % 10);

 SecLocation = GUI_GetStringDistX(str) + 10 + PointLocation;

 
 GUI_SetColor(GUI_WHITE);
 //GUI_FillCircle(PointLocation, 15+370, 5);
 //GUI_FillCircle(PointLocation, 40+370, 5);

 GUI_SetColor(GUI_WHITE);	
 
 sprintf(str,"%d%d",Hour/10,Hour%10);
 //PutText(25-10, 325, 272, 450, str, GUI_TA_LEFT);

 sprintf(str,"%d%d",Min/10,Min%10);
 //PutText(PointLocation+10, 325, 272, 450, str, GUI_TA_LEFT);

 OldDisplayedHour=Hour;
 OldDisplayedMin=Min;

 GUI_SetColor(0x000f0000);	 
	 
 LoadFont(FontBNazanin200);
 sprintf(str,"%d",Config.GrouhPrice[0][0]);
 PutText(0, 50, 272, 350, str, GUI_TA_CENTER);
 
 LoadFont(FontBKoodak40);		
		
  GUI_SetColor(GUI_BLACK);	
  PutText(0, 20+15, 272, 20+15+50, "نرخ بليت - واحد تومان مي باشد", GUI_TA_CENTER);
  GUI_SetColor(GUI_YELLOW); 

  sprintf(str,"آفلاين :%d",Indicators.OffTransactions);
  PutText(100, -3, 230, 25, str, GUI_TA_RIGHT);
	 
	GUI_SetColor(GUI_WHITE);	
       if (Config.DeviceID == 1) sprintf(str, "درب جلو");
  else if (Config.DeviceID == 2) sprintf(str, "درب عقب");
  else                           sprintf(str, "ناصحيح");
	PutText(5, 260, 270, 290, str, GUI_TA_CENTER);

	GUI_SetColor(GUI_BLACK);	
  Config.Title1[60]=Config.Title2[140]=0;
		
  PutText(10, 300, 260, 330, Config.Title1, GUI_TA_CENTER);
  PutText(10, 330, 260, 390, Config.Title2, GUI_TA_CENTER);
	
	
	GUI_SetColor(GUI_WHITE);  
  if((Month>=1)&&(Month<=12))
  {
    if(DayOfWeek<7)
      sprintf(str, "%s  %d  %s  %d ",strDays[DayOfWeek],Day,strMonths[Month-1],Year);
    else
      sprintf(str, "%d  %s  %d ",Day,strMonths[Month-1],Year);
 
		PutText(0, 233, 271, 263, str, GUI_TA_CENTER);

  }
	
	if(Connected2BCU)   ConnectionMark(GUI_DARKGREEN); 
	else                ConnectionMark(GUI_RED); 
	
}


void ShowPageAlef(void) {
 unsigned char i,Key=0;
 char str[150];

 unsigned long int ID,CardID,Li;
 long int Credit=0;

static const GUI_POINT _aPointHexagon[] = {
	{ -50, 0 },
	{ -40, -15 },
	{ 40, -15 },
	{ 50, 0 },
};
static const GUI_POINT _aPointHexagon1[] = {
	{ -60, 0 },
	{ -40, -20 },
	{ 40, -20 },
	{ 60, 0 },
};
	
static const GUI_POINT _aPolig1[] = {
	{ 0, 0 },
	{ 0, -30 },
	{ 50-10+SCREENRESIZE, -30 },
	{ 35-10+SCREENRESIZE, 0 },
};



static const GUI_POINT _aPolig3[] = {
	{ 0+40+1+SCREENRESIZE, 0+1 },
	{ 15+40+1+SCREENRESIZE, -30-1 },
	{ 210+50-1+SCREENRESIZE, -30-1 },
	{ 210+50-1+SCREENRESIZE, 0+1 },
};

static const GUI_POINT _aFooterPoli1[] = {
	{ 0, 50 },
	{ 400, 310 },
	{ 0, 310 },

};

static const GUI_POINT _aFooterPoli2[] = {
	{ 0, 0 },
	{ 200, 0 },
	{ 42, 75 },	
	{ 0, 50 },

};

static const GUI_POINT _aFooterPoli3[] = {
	{ 42, 75 },
	{ 140, 32 },
	{ 70, 95 },	

};


  if(LCDType==LCD480_272)
	{
    ShowPageAlef2();
		return;
	}

 GUI_DrawGradientV(0, 0, 272+SCREENRESIZE, 480+320,  GUI_MAKE_ALPHA(0,0),GUI_MAKE_ALPHA(0,0));

 GUI_DrawGradientRoundedV(5, 3, 272-5+SCREENRESIZE, 30,6,  0x000F0000,0x00070000);	

 
 GUI_SetColor(GUI_DARKBLUE);	
 GUI_DrawRoundedFrame(5, 2, 272-5+SCREENRESIZE,30,6,2);
 //........................................................................................

 GUI_DrawGradientRoundedV(5, 20+15, 272-5+SCREENRESIZE, 225+30+15,6,  GUI_DARKBLUE,GUI_DARKBLUE);	
 GUI_DrawGradientRoundedV(5, 71, 272-5+SCREENRESIZE, 225+30+15,6,  GUI_WHITE,GUI_CYAN);	
 

 
 GUI_SetColor(GUI_DARKBLUE);	
 GUI_FillRect(5, 20+15+20, 272-5+SCREENRESIZE, 20+15+40);

 GUI_SetColor(GUI_BLUE);
 GUI_DrawRoundedFrame(5, 20+15, 272-5+SCREENRESIZE, 225+30+15,6,3); 
 GUI_DrawLine(5,20+15+40,272-5+SCREENRESIZE,20+15+40);
 
 GUI_SetColor(GUI_BLUE);
 GUI_FillPolygon(&_aPointHexagon1[0], 4, (272+SCREENRESIZE)/2, 225+30+15);

 //........................................................................................
 GUI_SetColor(GUI_DARKGREEN);
 GUI_DrawRoundedFrame(5, 275, 272-5+SCREENRESIZE, 275+30,6,3); 


 //........................................................................................
 GUI_DrawGradientRoundedV(272-35+SCREENRESIZE, 275+35, 272-5+SCREENRESIZE, 275+35+30,6,  0x000f8000,0x007f8000);	
 
 GUI_SetColor(GUI_DARKGREEN);
 GUI_DrawRoundedFrame(272-35+SCREENRESIZE, 275+35, 272-5+SCREENRESIZE, 275+35+30,6,3); 

 GUI_DrawGradientRoundedV(5, 275+35, 272-38+SCREENRESIZE, 275+35+30,6,  GUI_WHITE,GUI_CYAN);	
 
 GUI_SetColor(GUI_DARKGREEN);
 GUI_DrawRoundedFrame(5, 275+35, 272-38+SCREENRESIZE, 275+35+30,6,3); 
 //........................................................................................

 GUI_DrawGradientRoundedV(5, 345, 272-5+SCREENRESIZE, 475,6,  GUI_BLACK,GUI_BLACK);	

 //........................................................................................
 GUI_SetColor(0x00FFF000);
 GUI_FillPolygon(&_aPolig1[0], 4, 5, 461);
 GUI_SetColor(0x00FFF000);
 GUI_FillPolygon(&_aPolig3[0], 4, 5, 461);

 
 GUI_SetColor(GUI_BLUE);
 GUI_DrawRoundedFrame(5, 345, 272-5+SCREENRESIZE, 475,6,3); 		
		
 GUI_SetColor(GUI_WHITE);
 //GUI_DrawLine(5,440,272-5,440);
 

 LoadFont(FontBNazanin140);

 sprintf(str,"%d%d ",Hour/10,Hour%10);
 PointLocation=GUI_GetStringDistX(str)+40-10;

 sprintf(str,"%d%d",Min/10,Min%10);

 SecLocation=GUI_GetStringDistX(str)+10+PointLocation;

 
 GUI_SetColor(GUI_WHITE);
 GUI_FillCircle(PointLocation, 15+370, 5);
 GUI_FillCircle(PointLocation, 40+370, 5);

 GUI_SetColor(GUI_WHITE);	
 
 sprintf(str,"%d%d",Hour/10,Hour%10);
 PutText(25-10, 325, 272, 450, str, GUI_TA_LEFT);

 sprintf(str,"%d%d",Min/10,Min%10);
 PutText(PointLocation+10, 325, 272, 450, str, GUI_TA_LEFT);

 OldDisplayedHour=Hour;
 OldDisplayedMin=Min;

 GUI_SetColor(0);
 //GUI_FillPolygon(&_aPointHexagon[0], 4, 272/2, 475);
	 
	 //Config.GrouhPrice[0][0]=0;
 LoadFont(FontBNazanin200);
 sprintf(str,"%d",Config.GrouhPrice[0][0]);
 PutText(0, 70, 272+SCREENRESIZE, 400, str, GUI_TA_CENTER);
 
 
  LoadFont(FontBKoodak40);
  GUI_SetColor(GUI_BLACK);   
  sprintf(str,"%d",12);
		
  GUI_SetColor(GUI_WHITE);	
  PutText(0, 20+15, 272+SCREENRESIZE, 20+15+50, "نرخ بليت - واحد تومان مي باشد", GUI_TA_CENTER);


  GUI_SetColor(GUI_WHITE); 
  sprintf(str,"آفلاين : %d",Indicators.OffTransactions);
  PutText(130+SCREENRESIZE, 0, 220+SCREENRESIZE, 25, str, GUI_TA_RIGHT);		
	 
	GUI_SetColor(GUI_DARKBLUE);  
  if((Month>=1)&&(Month<=12))
  {
    if(DayOfWeek<7)
      sprintf(str, "%s  %d  %s  %d ",strDays[DayOfWeek],Day,strMonths[Month-1],Year);
    else
      sprintf(str, "%d  %s  %d ",Day,strMonths[Month-1],Year);
 
		PutText(60+SCREENRESIZE, 430, 260+SCREENRESIZE, 480, str, GUI_TA_CENTER);

  }

	/*
	  LoadFont(FontTahoma20);
		 GUI_SetColor(GUI_DARKRED); 
	if(LastCardingDateTime[0]!=0)
	{
	sprintf(str,"%d/%d/%d  %d:%d:%02d",LastCardingDateTime[0],LastCardingDateTime[1],LastCardingDateTime[2],LastCardingDateTime[3],LastCardingDateTime[4],LastCardingDateTime[5]);
	//PutText(5, 275+30+10, 272-38+SCREENRESIZE, 275+30+30+10, str, GUI_TA_CENTER);
	}
	
	 	LoadFont(FontBKoodak40);
	*/
	
	
  GUI_SetColor(GUI_CYAN); 
	GUI_FillRoundedRect(4+234+SCREENRESIZE,2+3,30+233+SCREENRESIZE,27+1,3);	

	
	if(Connected2BCU)   ConnectionMark(GUI_DARKGREEN); 
	else                ConnectionMark(GUI_RED); 
			
 GUI_SetColor(GUI_WHITE); 
 GUI_DrawRoundedRect(4+234+SCREENRESIZE,2+3,30+233+SCREENRESIZE,27+1,3);	
	

 GUI_DrawGradientRoundedV(5, 485, 272-5+SCREENRESIZE, 795,6,  GUI_LIGHTGRAY,GUI_GRAY);	
	 

 if(InTestMode)
 {
	 GUI_DrawGradientRoundedV(5, 485, 272-5+SCREENRESIZE, 795,6,  GUI_RED,GUI_RED);	 
	 
 GUI_SetColor(GUI_BLACK);   
 PutText(0,495, 250+SCREENRESIZE,530, "حالت تست", GUI_TA_RIGHT);
 
 }
 else
 {

 GUI_SetColor(GUI_RED);
 GUI_FillPolygon(&_aFooterPoli1[0], 3, 8,485);
		
 GUI_SetColor(GUI_YELLOW);
 GUI_FillPolygon(&_aFooterPoli2[0], 4, 8,485);		
 
 GUI_SetColor(GUI_ORANGE);
 GUI_FillPolygon(&_aFooterPoli3[0], 3, 8,485);	 
 
 GUI_SetColor(GUI_BLUE);
 GUI_DrawRoundedFrame(5, 485, 272-5+SCREENRESIZE, 795,6,3); 		

 GUI_SetColor(GUI_BLACK);
	 
 LoadFont(FontBKoodak40);
 if(Config.DeviceID==1)      sprintf(str,"درب جلو");
 else if(Config.DeviceID==2) sprintf(str,"درب عقب");
 else sprintf(str,"ناصحيح");
	 
 PutText(10, 490, 100, 560, str, GUI_TA_CENTER);

 
 Config.Title1[60]=Config.Title2[140]=0;
		
 GUI_SetColor(GUI_BLACK);   
 PutText(0,495, 250+SCREENRESIZE,530, Config.Title1, GUI_TA_RIGHT);
 
 GUI_SetColor(GUI_BLACK);   
 PutText(0,570-10, 250+SCREENRESIZE,600, Config.Title2, GUI_TA_RIGHT);

 GUI_SetColor(GUI_BLACK);   
 //PutText(0,550+55-20, 250+SCREENRESIZE,580+50, "تعداد تراکنش امروز : -", GUI_TA_RIGHT);
 //PutText(0,550+110-30, 250+SCREENRESIZE,580+150, "تعداد تراکنش ديروز : -", GUI_TA_RIGHT);
 //PutText(0,550+165-40, 250+SCREENRESIZE,580+200, "تعداد تراکنش پريروز : -", GUI_TA_RIGHT);
 }
 
  //LoadFont(FontBNazanin60);
}


unsigned char SetDateAndTime(unsigned char Y, unsigned char M, unsigned char D, unsigned char h, unsigned char m, unsigned char s, unsigned char save) {
	unsigned long int time=0;

  if((s > 59) || (m > 59) || (h > 23))
    return 1;
  
  //if(GPSTimeAcquired)
  //  return(2);
  

	
	Year=Y+1300;
  Month=M;
  Day=D;  
  jalali_to_gregorian(Year,Month,Day,&CurrentDate);
  DayOfWeek=GetDayOfWeek(CurrentDate.day,CurrentDate.month,CurrentDate.year); 
	
	
  Hour=h;
  Min=m;
  Sec=s;
  OldSec=s;
	RTC_Set(CurrentDate.year,CurrentDate.month,CurrentDate.day,h,m,s);
	
  return(0);
}


unsigned char SendRequest(unsigned char type, unsigned short idx) {
	unsigned char i=0;
	unsigned short crc=0, indx;
	unsigned char SendBuf[32];

  //printf("\n\rSend Request [%d] %d, %d",ActivePort,type,idx);
 
  SendBuf[0]=STX;
  SendBuf[1]=14;  
  SendBuf[2]=Config.DeviceID%256;
  SendBuf[3]=Config.DeviceID/256;
  SendBuf[4]=3;//Len
  SendBuf[5]=0;//Len
  indx=6;
  if((BinPageSize==512)&&(type==Simorgh))
  {
    SendBuf[4]=4;//Len
    SendBuf[indx++]=99; 
  }
  SendBuf[indx++]=type; 
  SendBuf[indx++]=idx%256;
  SendBuf[indx++]=idx/256;
  crc=crc16(0,SendBuf+1, indx-1);
  SendBuf[indx++]=crc%256;      
  SendBuf[indx++]=crc/256;            
  SendBuf[indx++]=ETX; 
  
  switch(ActivePort)
  {
    case 0:
			SetTX485();GUI_Delay(1);
      for(i=0; i<indx; i++)
        SendByte(SendBuf[i]);
		
		GUI_Delay(5);
		SetRX485();
      break;
    case 1:
      //GPRSSend(indx++, SendBuf);
      break;  
		
		case 3:
      WIFISend(indx, SendBuf);   
      break;
  }
  //printf("\n\r%d:%d:%d Firmware Request",Hour,Min,Sec);
 
}


unsigned char CheckNewFirmware2(void) {
	unsigned long int Idx, i;
	unsigned int crc=0, page, Loc;
		
	unsigned short crc2=0;
	unsigned int pageSize;
  
  if(CheckFirmware==0) return(0);
  //SaveTaxiInfo();
  ShowMessageDlg(mtInformation, "بررسي نسخه جديد",1,0,0,0);  
  FirmwareRequestNo=0; 
  CheckFirmware=0;

  for(i=0; i<32; i++)
    GlobalBuffer[i]=0;
	FLASH_UNLOCK
  SaveFromRamToDF(addFirmwareInfo, 32, GlobalBuffer);
	FLASH_LOCK

  if(FirmwareLength!=Firmware.Length)
  {
    ShowMessageDlg(mtError, "خطا در حجم اطلاعات",1,0,0,0);   
    InfoCounter=3;
    return(1);
  }

  crc=0;
	Loc=1024;
  for(i=0; i<FirmwareLength; i++)
	{
		if(Loc==1024)
		{
			SPI_Flash_Read(GlobalBuffer, i+addFirmware, 1024);
			Loc=0;
		}
		crc+=GlobalBuffer[Loc];
		crc2+=GlobalBuffer[Loc];
		Loc++;
	}
	
  if(crc!=Firmware.CheckSum)
  {
    ShowMessageDlg(mtError, "خطا صحت اطلاعات",1,0,0,0); 
    InfoCounter=3;
    return(2);
  }
  
  GlobalBuffer[0]=0xC2;
  GlobalBuffer[1]=(crc2)&0xFF;
  GlobalBuffer[2]=(crc2>>8)&0xFF;
  GlobalBuffer[3]=(FirmwareLength)&0xFF;
  GlobalBuffer[4]=(FirmwareLength>>8)&0xFF;
  GlobalBuffer[5]=(FirmwareLength>>16)&0xFF;
  GlobalBuffer[6]=(FirmwareLength>>24)&0xFF;
  GlobalBuffer[7]=0x55;
	FLASH_UNLOCK
  SaveFromRamToDF(addFirmwareInfo, 32, GlobalBuffer);
	FLASH_LOCK

  ShowMessageDlg(mtInformation, "برنامه جديد دريافت شد",0,0,0,0);  
  ShowPageAlef();
  return 0;
  
}


unsigned char SendDataRequest(unsigned char Type, unsigned int Index) {
	unsigned char i, Byte; 
	unsigned short crc=0;
	unsigned char SendBuf[20];

   SendBuf[0]=STX;
   SendBuf[1]=14; //Request
   SendBuf[2]=Config.DeviceID%256;
   SendBuf[3]=Config.DeviceID/256;
   SendBuf[4]=5;
   SendBuf[5]=0;
   SendBuf[6]=Type;
   SendBuf[7]=Index&0xFF;
   SendBuf[8]=(Index>>8)&0xFF;
   SendBuf[9]=(Index>>16)&0xFF;
   SendBuf[10]=(Index>>24)&0xFF;
   crc=crc16(0, SendBuf+1, 10);
   SendBuf[11]=crc&0xFF;
   SendBuf[12]=crc>>8;
   SendBuf[13]=ETX;
   
	SetTX485();GUI_Delay(1);
   for(Byte=0; Byte<14; Byte++)
     SendByte(SendBuf[Byte]);
		 
		 GUI_Delay(5);
		SetRX485();
   return(0);
}



unsigned char SendCommandResult(unsigned char Type, unsigned int Status) {
	unsigned char i, Byte; 
	unsigned short crc=0;

   GlobalBuffer[0]=STX;
   GlobalBuffer[1]=11; //Result
   GlobalBuffer[2]=Config.DeviceID%256;
   GlobalBuffer[3]=Config.DeviceID/256;
   GlobalBuffer[4]=2;
   GlobalBuffer[5]=0;
   GlobalBuffer[6]=Type;
   GlobalBuffer[7]=Status;
   crc=crc16(0, GlobalBuffer+1, 7);
   GlobalBuffer[8]=crc&0xFF;
   GlobalBuffer[9]=crc>>8;
   GlobalBuffer[10]=ETX;
   
   switch(ActivePort)
   {
     case 0:
			 SetTX485();GUI_Delay(1);
       for(Byte=0; Byte<11; Byte++)
         SendByte(GlobalBuffer[Byte]);
		 
		 GUI_Delay(5);
		 SetRX485();
       break;
     case 1:
       //GPRSSend(11, GlobalBuffer);
       break;
		 case 3:
			 WIFISend(11, GlobalBuffer);
       break;
   }

   //printf("\n\r%d:%d:%d Send Command Result",Hour,Min,Sec);
   return(0);
}


unsigned int SendDeviceConfiguration(void) {
	unsigned short Idx, i, crc;

  Idx = 0;
  GlobalBuffer[Idx++] = STX;
  GlobalBuffer[Idx++] = 11; //Result
	
  GlobalBuffer[Idx++] = Config.DeviceID % 256;
  GlobalBuffer[Idx++] = Config.DeviceID / 256;
	
  GlobalBuffer[Idx++] = 0;
  GlobalBuffer[Idx++] = 0;
  GlobalBuffer[Idx++] = 81;
  GlobalBuffer[Idx++] = 0;  //Group
  GlobalBuffer[Idx++] = 0;
	
  GlobalBuffer[Idx++] = Config.UC % 256;
  GlobalBuffer[Idx++] = Config.UC / 256;
	
  GlobalBuffer[Idx++] = Indicators.Transactions % 256;
  GlobalBuffer[Idx++] = Indicators.Transactions / 256;
	
  GlobalBuffer[Idx++] = Indicators.OffTransactions % 256;
  GlobalBuffer[Idx++] = Indicators.OffTransactions / 256;
	
  GlobalBuffer[Idx++] = Indicators.Locations % 256;
  GlobalBuffer[Idx++] = Indicators.Locations / 256;
	
  GlobalBuffer[Idx++] = Indicators.OffLocations % 256;
  GlobalBuffer[Idx++] = Indicators.OffLocations / 256;
	
  GlobalBuffer[Idx++] = Config.TransactionsSendInterval % 256;
  GlobalBuffer[Idx++] = Config.TransactionsSendInterval / 256;
	
  GlobalBuffer[Idx++] =  Config.TrackingInterval 			 & 0xFF;
  GlobalBuffer[Idx++] = (Config.TrackingInterval >> 8) & 0xFF;
	
  GlobalBuffer[Idx++] = Year % 256;
  GlobalBuffer[Idx++] = Year / 256;
  GlobalBuffer[Idx++] = Month;
  GlobalBuffer[Idx++] = Day;
  GlobalBuffer[Idx++] = Hour;
  GlobalBuffer[Idx++] = Min;
  GlobalBuffer[Idx++] = Sec;
  GlobalBuffer[Idx++] = Ver % 256;
  GlobalBuffer[Idx++] = Ver / 256;
  GlobalBuffer[Idx++] = Release % 256;
  GlobalBuffer[Idx++] = Release / 256;
	
  GlobalBuffer[Idx++] =  Config.Password 				& 0xFF;
  GlobalBuffer[Idx++] = (Config.Password >> 8)  & 0xFF;
  GlobalBuffer[Idx++] = (Config.Password >> 16) & 0xFF;
  GlobalBuffer[Idx++] = (Config.Password >> 24) & 0xFF;
	
  GlobalBuffer[Idx++] =  Config.DriverID 			 & 0xFF;
  GlobalBuffer[Idx++] = (Config.DriverID >> 8) & 0xFF;
	
  GlobalBuffer[4] 		= (Idx - 6) % 256;
  GlobalBuffer[5] 		= (Idx - 6) / 256;
	
  crc = crc16(0, GlobalBuffer+1, Idx-1);
	
  GlobalBuffer[Idx++] = crc % 256;
  GlobalBuffer[Idx++] = crc / 256;
  GlobalBuffer[Idx++] = ETX;
  
  switch(ActivePort) {
  case 0:
		SetTX485();
		GUI_Delay(1);
    for(i=0; i<Idx; i++)
      SendByte(GlobalBuffer[i]);
		GUI_Delay(5);
		SetRX485();
    break;
  case 1:
    //GPRSSend(Idx, GlobalBuffer);
    break;
	case 3:
		WIFISend(Idx, GlobalBuffer);   
		break;
  }
}


void Delete_Records() {
	SPI_Flash_Erase_Sector(addTransactions / 4096);
	trHead = trTail = Indicators.OffTransactions = 0;
	SaveRingDetail();
	OkBip(1);
}

void ProcessCommands(void) {
	unsigned int 
		InvalidDateRecordFlag,
		InvalidDateRecord = 0,
		crc, 
		Idx, 
		i, 
		Y,
		Address,
		j;
	
	unsigned char 
		FromY, 
		FromM, 
		FromD, 
		Byte, 
		m,
		M,
		D,
		YE,
		Temp1,
		Temp2,
		read_buf[320];
	
	unsigned long int Sector, Temp, Limit;
	char str[20];

  switch(GlobalBuffer[6]) {//Command #
		case 1: //Connection Test
		{
			SendCommandResult(1, 1);
			OkBip(1);
			break;
		}
		case 2: //Set date and time
		{
			SetDateAndTime(
				GlobalBuffer[7], 
				GlobalBuffer[8], 
				GlobalBuffer[9], 
				GlobalBuffer[10], 
				GlobalBuffer[11], 
				GlobalBuffer[12], 
				1);
			SendCommandResult(2, 1);
			OkBip(1);
			break;
		}
		case 80: { // Receive Device Configuration
			Idx = 7;
			//Config.DeviceID=GlobalBuffer[Idx]+(GlobalBuffer[Idx+1]*256); 
			Idx += 2;   
			//Config.Grouh=GlobalBuffer[Idx]+(GlobalBuffer[Idx+1]*256);
			Idx += 2;   
			//Config.UC=DeviceUC;/*GlobalBuffer[Idx]+(GlobalBuffer[Idx+1]<<8);
			Idx += 2;
			Config.Password  = GlobalBuffer[Idx+3]; Config.Password <<= 8;
			Config.Password += GlobalBuffer[Idx+2]; Config.Password <<= 8; 
			Config.Password += GlobalBuffer[Idx+1]; Config.Password <<= 8;
			Config.Password += GlobalBuffer[Idx]; 
			Idx += 4;
			Config.TransactionsSendInterval = (GlobalBuffer[Idx]+(GlobalBuffer[Idx+1]<<8));  Idx+=2;
			Config.TrackingInterval = (GlobalBuffer[Idx]+(GlobalBuffer[Idx+1]<<8)); Idx+=2; 
			SaveConfiguration();
			SendCommandResult(80, 1);
			//G_RepaintArea(220,240);
			OkBip(1);
			break;
		}
		case 81: // Send Device Configuration
		{
			//G_DrawGradianRectangle(255,255,255,255,168,0,220,20,15,1);  
			//PutText(5,222,0,0, "Get device configuration", ALINE_LEFT);
			SendDeviceConfiguration();
			//G_RepaintArea(220,240);
			OkBip(1);
			break;
		}
		case 82: //Delete Records
		{
			//G_DrawGradianRectangle(255,255,255,255,168,0,220,20,15,1);  
			//PutText(5,222,0,0, "Delete Device Transactions", ALINE_LEFT);
			SPI_Flash_Erase_Sector(addTransactions / 4096);
			trHead = trTail = Indicators.OffTransactions = 0;
			SaveRingDetail();
			SendCommandResult(82, 1);
			//G_RepaintArea(220,240);
			OkBip(1); 
			break;
		}
		case 12: {//Set Transactions Indicator
			if (trHead == 0) 
				Temp = MAX_TRANSACTIONS - 1;
			else
				Temp = trHead - 1;

			i = 0;
			InvalidDateRecord = 0;
			while (i < MAX_TRANSACTIONS) {              
				i++;
				LoadFromDFToRam(addTransactions+(Temp*LEN_TRANSACTIONS), LEN_TRANSACTIONS, read_buf);
				if (i == 1) 
					Indicators.OffTransactions = 0;
				InvalidDateRecordFlag = 0;

			// printf("\n\rIndex=%03d ",Temp);
			//printf("%02d/%02d/%02d  %02d/%02d/%02d - %d",read_buf[5],read_buf[6],read_buf[7],GlobalBuffer[7],GlobalBuffer[8],GlobalBuffer[9],InvalidDateRecord);

				if (Temp == 0)
					Temp = MAX_TRANSACTIONS - 1;
				else
					Temp = Temp - 1;

				if (read_buf[5] < GlobalBuffer[7])
					InvalidDateRecordFlag = 1;
				else if (read_buf[5] == GlobalBuffer[7]) {
					if(read_buf[6] < GlobalBuffer[8])
						InvalidDateRecordFlag = 1;
					else if (read_buf[6] == GlobalBuffer[8])
						if (read_buf[7] < GlobalBuffer[9])
							InvalidDateRecordFlag = 1;
				}

				if ((read_buf[5] >= 200) || (read_buf[6] >= 200) || (read_buf[7] >= 200)) 
					InvalidDateRecordFlag = 1;


			//printf("** %d ",InvalidDateRecordFlag);

				Indicators.OffTransactions++;

				if(InvalidDateRecordFlag) {
					if(++InvalidDateRecord >= 30) 
						break;
				}
				else
					InvalidDateRecord = 0;

				if (Temp) Temp--;
				else      Temp = MAX_TRANSACTIONS - 1;

				if (i > MAX_TRANSACTIONS) 
					break;
			}

			SendCommandResult(12, 1);
			OkBip(0);
			break;
	  }
		case 85:  //Text Message
		{
			Idx = GlobalBuffer[4] + (GlobalBuffer[5] * 256);
			Idx--;
			for(i=0; (i < Idx) && (i < 300); i++)
				read_buf[i] = GlobalBuffer[i + 7];
			read_buf[i] = 0;
			DisplayText(read_buf); 
			InfoCounter = 20;
			SendCommandResult(85, 2);
			break; 
		}
		case 86:
		{
			//G_DrawGradianRectangle(255,255,255,255,168,0,220,20,15,1);  
			//PutText(5,222,0,0, "Get SIM card charge", ALINE_LEFT);
			//G_RepaintArea(220,240);
			//GetSimCardChargeAuto();//by hesam nickhou
			if(Charge > 0)
				DeviceInfo.Charge = Charge;
			break;
		}
		case 87: //Disabled to ignore serial communication
		{
			/*Idx = GlobalBuffer[4] + (GlobalBuffer[5] * 256); 
			Idx -= 2;
			//G_DrawGradianRectangle(255,255,255,255,168,0,220,20,15,1);  
			//PutText(5,222,0,0, "Recharge SIM card", ALINE_LEFT);
			for(i=0; (i < Idx) && (i < 40); i++)
				ChargeCode[i] = GlobalBuffer[i + 6];
			ChargeCode[i] = 0;  
			//G_RepaintArea(220,240);
			Byte = ChargeSimCardAuto();
			ChargeRequest = 0;
			break;*/
		}
		case 88: //Disabled to ignore serial communication
		{/*
			//G_DrawGradianRectangle(255,255,255,255,168,0,220,20,15,1);  
			//PutText(5,222,0,0, "EXEC USSD command", ALINE_LEFT);
			for (i=0; (i < (GlobalBuffer[4] + (GlobalBuffer[5] * 256)) - 1) && (i < 49); i++)
				ChargeCode[i] = GlobalBuffer[i + 7];
			ChargeCode[i] = 0;
			//G_RepaintArea(220,240);
			LoadFromDFToRam(addIndicators + 20 + 20, 50, read_buf);
			for (j=0; j<i; j++)
				if (ChargeCode[j] != read_buf[j]) 
					break;
			if (j == i) {
				LoadFromDFToRam(addIndicators + 20 + 20 + 50, 50, read_buf);
				GlobalBuffer[0] = STX;
				GlobalBuffer[1] = 11; //Result
				GlobalBuffer[2] = Config.DeviceID % 256;
				GlobalBuffer[3] = Config.DeviceID / 256;
				GlobalBuffer[4] = 0;
				GlobalBuffer[5] = 0;
				GlobalBuffer[6] = 32;
				j               = 7;
				for (i = 0; (read_buf[i] >= 32) && (read_buf[i] <= 200) && (i < 49); i++)
					GlobalBuffer[j++] = read_buf[i];
				GlobalBuffer[4] = (j - 6) % 256;
				GlobalBuffer[5] = (j - 6) / 256;
				crc = crc16(0, GlobalBuffer + 1, j - 1);
				GlobalBuffer[j++] = crc % 256;
				GlobalBuffer[j++] = crc / 256;
				GlobalBuffer[j++] = ETX;
				switch(ActivePort) {
					case 0:
						SetTX485();GUI_Delay(1);
						for(i=0; i<j; i++)
							SendByte(GlobalBuffer[i]);
						GUI_Delay(5);
						SetRX485();
						break;
					
					case 1:
						GPRSSend(j, GlobalBuffer);
						break;  
				}
				break;
			}
				
			crc = ExecuteCommand();
			if(crc > 10) {
				Charge = crc;
				SendALive(150);
			}
			break;
		*/}
		case 124:
		{
			FLASH_UNLOCK
			SendRequest(124, 0);
			RequestType = 124;
			RequestNo = 0;
			OkBip(1);
			break;
		}
		case 125:
		{
			FLASH_UNLOCK
			SendRequest(125, 0);
			RequestType = 125;
			RequestNo   = 0;
			OkBip(1);
			break;
		}
		case 222:
		{
			Idx = 7;
			for(i=0; i<10; i++) 
				for(j=0; j<6; j++)
					GlobalBuffer[Idx + (i * 6) + j] ^= 0x42;
			MFRC531_WriteMasterKey(0x00, &GlobalBuffer[Idx]);
			MFRC531_WriteMasterKey(0x02, &GlobalBuffer[Idx + 6]);
			MFRC531_WriteMasterKey(0x04, &GlobalBuffer[Idx + 12]);
			MFRC531_WriteMasterKey(0x05, &GlobalBuffer[Idx + 18]);
			MFRC531_WriteMasterKey(0x06, &GlobalBuffer[Idx + 24]);                       
			MFRC531_WriteMasterKey(0x07, &GlobalBuffer[Idx + 30]);
			MFRC531_WriteMasterKey(0x0A, &GlobalBuffer[Idx + 36]);                       
			MFRC531_WriteMasterKey(0x0B, &GlobalBuffer[Idx + 42]);                       
			MFRC531_WriteMasterKey(0x08, &GlobalBuffer[Idx + 48]);                       
			MFRC531_WriteMasterKey(0x09, &GlobalBuffer[Idx + 54]);            
			OkBip(1);
			break;
		}
		case 129: 
		{
			CheckFirmware = 0x81;
			break;
		}
		case 130: 
		{
			RequestType = 130;
			RequestNo   = 0;
			break;
		}
		case 165: //BlackList  
		{
			BlackList_Count    = GlobalBuffer[9]; BlackList_Count <<= 8;
			BlackList_Count   += GlobalBuffer[8]; BlackList_Count <<= 8;
			BlackList_Count   += GlobalBuffer[7];
			BlackList_LastIdx  = GlobalBuffer[13]; BlackList_LastIdx <<= 8;
			BlackList_LastIdx += GlobalBuffer[12]; BlackList_LastIdx <<= 8;
			BlackList_LastIdx += GlobalBuffer[11]; BlackList_LastIdx <<= 8;
			BlackList_LastIdx += GlobalBuffer[10];
			SPI_Flash_Erase_Sector(addBlackListInfo / 4096);
			BlackList_Index = 1;
			//SendCommandResult(165, 1);
			SendDataRequest(165, BlackList_Index);
			break; 
		}
  }
  ShowPageAlef();
}


void ProcessData(void) {
	unsigned short  Counter,DataIndex, DataLen;
	unsigned long int Data_Length,i,j, address, Idx;
	unsigned char str[150],k;
	unsigned short _FirmwareRelease;
	unsigned short _FirmwareVer;
	unsigned short New_Ver;
	unsigned short New_Release;
	unsigned int Len;	

 //............................................................................................................

	if(GlobalBuffer[6]==165) //BlackList
	{
    if(BlackList_Index==(GlobalBuffer[7]+GlobalBuffer[8]*256))
		{
      Len=(GlobalBuffer[4]+GlobalBuffer[5]*256)-3;
			if(Len/8>64)
				return;
			i=(BlackList_Index-1)*(64*8);
			i+=addBlackList;
      SaveFromRamToDF(i, Len, &GlobalBuffer[9]);
			
			if(Len<64*8)
			{
				GlobalBuffer[0]=BlackList_Count & 0xFF;
				GlobalBuffer[1]=(BlackList_Count>>8) & 0xFF;
				GlobalBuffer[2]=(BlackList_Count>>16) & 0xFF;
				GlobalBuffer[3]=BlackList_LastIdx & 0xFF;
				GlobalBuffer[4]=(BlackList_LastIdx>>8) & 0xFF;
				GlobalBuffer[5]=(BlackList_LastIdx>>16) & 0xFF;
				GlobalBuffer[6]=(BlackList_LastIdx>>24) & 0xFF;
				GlobalBuffer[7]=0xBC^GlobalBuffer[0]^GlobalBuffer[1]^GlobalBuffer[2]^GlobalBuffer[3]^GlobalBuffer[4]^GlobalBuffer[5]^GlobalBuffer[6];
        SaveFromRamToDF(addBlackListInfo, 8, GlobalBuffer);
			  BlackList_Index=0;
			}else
			  BlackList_Index++;
		}
		if(BlackList_Index)
			SendDataRequest(165, BlackList_Index);
		return;
	}

	if(GlobalBuffer[6]!=DeviceType) return;
 	 	
 WDTR;



 if((FirmwarePacketIndex==(GlobalBuffer[7]+GlobalBuffer[8]*256))||((GlobalBuffer[7]+GlobalBuffer[8]*256)==0))
    {

     if((GlobalBuffer[7]+GlobalBuffer[8]*256)==0)
       {
        FirmwarePacketIndex=0;
        Load_FirmwareProperties();
        Firmware.SavedVer=Ver;
        Firmware.SavedRelease=Release;
        Save_FirmwareProperties();

 		    
				 
        i=9;
        IsFileForGprsDownload=0;
        Firmware.HaveData=0;

        Counter=0;
        if(GlobalBuffer[6]==BUSDOOR)
          {
           k=0;
           Load_FirmwareProperties();
          
           New_Ver=GlobalBuffer[9]+GlobalBuffer[10]*256;
           New_Release=GlobalBuffer[11]+GlobalBuffer[12]*256;
						
           if((New_Ver>Ver)||((New_Release>Release)&&(New_Ver==Ver)))
             {
              Firmware.HaveData=0xAA;
              Firmware.Length=GlobalBuffer[16]; Firmware.Length<<=8;
              Firmware.Length+=GlobalBuffer[15]; Firmware.Length<<=8;
              Firmware.Length+=GlobalBuffer[14]; Firmware.Length<<=8;
              Firmware.Length+=GlobalBuffer[13];

					  	if(Firmware.Length==0) return;
								
              Firmware.CheckSum=GlobalBuffer[20]; Firmware.CheckSum<<=8;
              Firmware.CheckSum+=GlobalBuffer[19]; Firmware.CheckSum<<=8;
              Firmware.CheckSum+=GlobalBuffer[18]; Firmware.CheckSum<<=8;
              Firmware.CheckSum+=GlobalBuffer[17];
         
              Firmware.SavedVer=New_Ver;
              Firmware.SavedRelease=New_Release;
         
              for(j=0; j<30; j++)
                Firmware.FirmwareFileName[j]=GlobalBuffer[21+j];
              Firmware.FirmwareFileName[j]=0;
              FirmwarePacketIndex=1;

              FirmwarePercent=Firmware.Length/512;  
  
              IsFileForGprsDownload=1;
              FirmwarePercent++;
             }
            else
              {
               Firmware.HaveData=0;
              }
						
            }
  
           Check_ServerForUpdateFile=1;
           Downloaded_FirmwareLength=0;
           //SaveConfiguration();

          }
         else if(FirmwarePacketIndex)
                {
								//	GUI_SetColor(GUI_GRAY);       		 GUI_FillCircle(15,15,5);GUI_Delay(250);
                 if(BufferLen<8) return; 
                 if(GlobalBuffer[6]!=DeviceType)  return;  
                   
                 
                 Data_Length=(GlobalBuffer[4]+GlobalBuffer[5]*256)-3;
                 
                 if((Data_Length==0)&&(FirmwarePacketIndex==1))  {IsFileForGprsDownload=0; return; }
                 //.....................................................................................
                 if(Data_Length==0)
                   {

										CheckNewFirmware();

                    FirmwarePacketIndex=1;
                  
                    return; 
                   }    
                

  		           FLASH_UNLOCK
									 
								 address=addFirmware+((FirmwarePacketIndex-1)*512);
								 if(address+512<addConfig)
								 {
								   if(SaveFromRamToDF(address, 512, GlobalBuffer+9))
									 {
										 FLASH_LOCK											
									   return;
									 }									 

                   Downloaded_FirmwareLength+=Data_Length;
								 }
									 
                 FLASH_LOCK
										
										
                 if(1) 
                   {   
                    if(FirmwarePacketIndex)
                      {
                       if(FirmwarePercent) Percent=((FirmwarePacketIndex*100)/FirmwarePercent);
                       else Percent=0;
                      }
											 
              	 if(InfoCounter==0)
		               {
                    GUI_SetColor(GUI_BLACK);
                    GUI_FillRect(22, 6, 150, 28);
                    GUI_SetColor(GUI_CYAN);
        
		                sprintf(str,"برنامه : %d %%",(100*Downloaded_FirmwareLength)/Firmware.Length,Data_Length,Downloaded_FirmwareLength,Firmware.Length);
 		                PutText(23, -2, 150, 28, str, GUI_TA_LEFT);    
                  	}
                  } 
									 
										
                 if(Downloaded_FirmwareLength>=Firmware.Length)
                   {

										CheckNewFirmware();

                    FirmwarePacketIndex=1;
                  
                    return; 
                   }										
										
                  FirmwarePacketIndex++;  

                  Request_FirmwareNextPacketfromServer();
   
                  //Indicators.LastFirmwareDownloadedLength+=Data_Length;  
                  //Indicators.LastFirmwareFirmwarePacketIndex=FirmwarePacketIndex;                  
                  //SaveIndicators();                  
                }
         }
}



/*
	Calculate XOR of data elements in 1 byte. The initial value of 0x12 or 18 is 
		biased to final result.
	@param from: from which index of data?
	@param data: data array of calculaing CRC
	@param len: length of data which should be calculate
	@return : CRC of given data from index in len bytes
*/
char CRC8(unsigned char from, const char* data, unsigned char len) {
	unsigned char index = from;
	char crc = 18; //Initial Value
	while (index < len)
		crc ^= data[index++];
	return crc;
}

/*
	Send alive packets every seconds for SUN device
*/
char SendDeviceInfo(void) {
	//Connects to Driver Console and wait to receive Configuration data
}


unsigned char SendALive() {	
	unsigned short Idx = 0, crc = 0;
	unsigned char temp, i;

	GlobalBuffer[Idx++] = STX;
	
	//============ Alive Packet header byte ===========
	GlobalBuffer[Idx++] = 12; 
	
	//============ Size of packet =====================
	GlobalBuffer[Idx++] = 0;  //Will be calculated at the end of function
	
	//============ 1 byte Device Type =================
	GlobalBuffer[Idx++] = 2; //2 means front door , 3 means back door , 1 means driver console
	
	//============ 1 byte Need Date ===================
	GlobalBuffer[Idx++] = 0; //Need Date? 
	
	//============ 29 bytes Live Packet Data ==========
						// "LIVE PACKET RECEIVE CORRECTLY"
	GlobalBuffer[Idx++] = 'L';
	GlobalBuffer[Idx++] = 'I';
	GlobalBuffer[Idx++] = 'V';
	GlobalBuffer[Idx++] = 'E';
	GlobalBuffer[Idx++] = ' ';
	GlobalBuffer[Idx++] = 'P';
	GlobalBuffer[Idx++] = 'A';
	GlobalBuffer[Idx++] = 'C';
	GlobalBuffer[Idx++] = 'K';
	GlobalBuffer[Idx++] = 'E';
	GlobalBuffer[Idx++] = 'T';
	GlobalBuffer[Idx++] = ' ';
	GlobalBuffer[Idx++] = 'R';
	GlobalBuffer[Idx++] = 'E';
	GlobalBuffer[Idx++] = 'C';
	GlobalBuffer[Idx++] = 'E';
	GlobalBuffer[Idx++] = 'I';
	GlobalBuffer[Idx++] = 'V';
	GlobalBuffer[Idx++] = 'E';
	GlobalBuffer[Idx++] = ' ';
	GlobalBuffer[Idx++] = 'C';
	GlobalBuffer[Idx++] = 'O';
	GlobalBuffer[Idx++] = 'R';
	GlobalBuffer[Idx++] = 'R';
	GlobalBuffer[Idx++] = 'E';
	GlobalBuffer[Idx++] = 'C';
	GlobalBuffer[Idx++] = 'T';
	GlobalBuffer[Idx++] = 'L';
	GlobalBuffer[Idx++] = 'Y';
	
	//============ Data size in byte of 2 =============
	GlobalBuffer[2] = Idx - 3;
	
	//============ Calculate CRC for 1 byte ===========
		
	GlobalBuffer[Idx++] = CRC8(1, GlobalBuffer, Idx);
	
	GlobalBuffer[Idx++] = ETX;
	
	/*GUI_SetColor(GUI_RED);
	GUI_SetFont(&GUI_Font16_1);
	for (i=0; i<Idx; i++)
		GUI_DispDecAt(GlobalBuffer[i], 8 + (i % 10)*25, 100 + (i / 10) * 15, 3);*/
	
	WifiConnectionError = 0;
	//keyboard_Beep();
	
	sprintf(GlobalBuffer, "LIVE PACKET!\n");
	Idx = strlen(GlobalBuffer);
	
	return (SendPacket(Idx));
}

void DisplayPacket(void) {
	int i, j, k=0;
	GUI_SetFont(&GUI_Font10_1);
 	GUI_SetBkColor(GUI_BLACK);
 	GUI_Clear();
  GUI_SetColor(GUI_WHITE);
	for(i=0; (i<60) && (k<BufferLen); i++)
	  for(j=0; (j<14) && (k<BufferLen); j++)
		  GUI_DispDecAt(GlobalBuffer[k++], j*30, i*20, 4);
	
	//for(i=0; (i<BufferLen)&&(i<48); i++)
	//{
	//	GUI_DispDecAt(GlobalBuffer[i], 0, i*10, 4);
	//}
}


unsigned char ProcessPacket(unsigned char Port) {
	unsigned int  Len, i, DID; 
	unsigned long int Address, Sector;
	unsigned short crc   = 0;
	unsigned short Start = 0;
	unsigned char 
		Refresh 		= 0, 
		ChangePrice = 0,
		Byte, 		
		m, 
		Idx, 
		Err;
	
	char str[50];

  if(BufferLen < 9)
    return(1);

  if(BufferLen < (GlobalBuffer[4] + (GlobalBuffer[5] * 256)))
    return(2);
    
  if ((GlobalBuffer[0] != STX) || (GlobalBuffer[BufferLen - 1] != ETX))
    return(3);

  crc = crc16(0, GlobalBuffer + 1, BufferLen - 4);
  
  if (crc != (GlobalBuffer[BufferLen - 3] + (GlobalBuffer[BufferLen - 2] * 256)))
    return(4);
    
  DID = GlobalBuffer[3]; DID <<= 8;
  DID = DID+GlobalBuffer[2];
  if ((DID != Config.DeviceID) && (DID != 0xFFFF) && (GlobalBuffer[1] >= 10) && (GlobalBuffer[1] != 12))
    return(5);

  ActivePort = Port; 
	WifiConnectionError = 0;
  switch (GlobalBuffer[1]) {
    case 1:  //Detect Device    
      if ((GlobalBuffer[4] + (GlobalBuffer[5] * 256)) == 6)
        SetDateAndTime(GlobalBuffer[6], GlobalBuffer[7], GlobalBuffer[8], GlobalBuffer[9], GlobalBuffer[10], GlobalBuffer[11], 0);

			if(Config.WiFi != 103)
				if(WiFiStep != 10) {
					WiFiStep = 9;
					CheckWiFi();
				}
      ConnectedToServer = 1;				 
      SendDeviceInfo();
      break;   
			
    case 10:  //Command  
			ConnectedToServer = 1;
      ProcessCommands();  
      break;
    case 13: //Data Accepted
			ConnectedToServer = 1;
      switch(GlobalBuffer[6]) {
        case 8: //Transactions:				
          Address = GlobalBuffer[10];        Address <<= 8;
          Address = Address+GlobalBuffer[9]; Address <<= 8;
          Address = Address+GlobalBuffer[8]; Address <<= 8;
          Address = Address+GlobalBuffer[7];
				
          if (Address == WaitForAddress) {
            Sector = addTransactions + ((Address == 0 ? (MAX_TRANSACTIONS - 1) : Address - 1) * LEN_TRANSACTIONS);
            if(Address == trHead) {
              if(trHead == 0)
                Sector = addTransactions + ((MAX_TRANSACTIONS - 1) * LEN_TRANSACTIONS);
              else
                Sector = addTransactions + ((trHead - 1) * LEN_TRANSACTIONS);
            }
						
            //printf("\n\r Data Sent Add: %d [%d]",WaitForAddress, Sector);
            GlobalBuffer[0] = 0x90;
						SaveFromRamToDF(Sector + LEN_TRANSACTIONS - 1, 1, GlobalBuffer);
            Indicators.OffTransactions = (Address > trHead) ? (MAX_TRANSACTIONS - Address) + trHead : trHead - Address;
						
           	LoadFont(FontBKoodak40);
            GUI_DrawGradientV(100, 4, 230, 28,0x000F0000,0x00070000);							
		        GUI_SetColor(GUI_YELLOW); 
            sprintf(str,"Ø¢Ù :%d",Indicators.OffTransactions);
            PutText(100, -3, 230, 25, str, GUI_TA_RIGHT);						
          }
          WaitForAddress = 0;
          if(Indicators.OffTransactions) {
						DataCurrentlySend = 1;
					  SendOfflines();
					}
          break;
      }
      break;  
    case 15:  //Data
      ProcessData();  
      break;
   }//switch
  
  return(0);
}

unsigned char CommunicateWithHostByWiFi(void) {
	unsigned char Byte;
  BufferLen = 0;
  while (WIFI_GCF(&Byte))
    if (Byte == 2) {
      GlobalBuffer[BufferLen++] = Byte;
      break;
    }

  while ((WIFI_GCF(&Byte)) && (BufferLen < GlobalBufferLen)) 
    GlobalBuffer[BufferLen++] = Byte;
	
  ProcessPacket(3);
	
  return(1);
}

unsigned char CommunicateWithHost(void) {
	unsigned char Byte;
  BufferLen = 0;
		
  while (USART_GCF(&Byte))
    if (Byte == 2) {
      GlobalBuffer[BufferLen++] = Byte;
      break;
    }

  while ((USART_GCF(&Byte)) && (BufferLen < GlobalBufferLen)) 
    GlobalBuffer[BufferLen++] = Byte;
  ProcessPacket(0);    
  return(1);
}


unsigned char CheckCard(unsigned int UC, unsigned long int CardID) {
	char str[100];
	if ((UC == 251) && (CardID == 15010)) {
		if (Config.DeviceID != 1)
			Config.DeviceID = 1;
		else
			Config.DeviceID = 2;
		
  /*     
       if(Config.DeviceID==1)
          Show_TextInTFT(320,85,0,85,"ÏÑÈ Ìáæ",R2L,1);    
       
       if(Config.DeviceID==2)
          Show_TextInTFT(320,85,0,85,"ÏÑÈ ÚÞÈ",R2L,1);    
*/
            
       OkBip(2);
       
       SaveConfiguration();
       InfoCounter = 1;
       return(1);
     }


  if(UC != Config.UC) {
		/*
    ShowMessageDlg(mtError, "???? ????? ?? ??? ???? ????",0,0,0,0);  
    GUI_SetColor(GUI_DARKGRAY);
    sprintf(str, " ?? ?????? : %d   ?? ???? : %d ", UC, CardID);
    PutText(80,190,400,220,str,GUI_TA_CENTER);
		*/
		ShowCardingResult(2, 0, UC,CardID, S_InvalidCardUC, "");
    InfoCounter = 3;
    return(1);
  }
  
  return(0);
}


unsigned short ShowErrorKashan(unsigned int Status) {
  switch(Status)
  {
    case 0x9900: ShowCardingResult(4,0,Config.UC,0,0,"خطا در ارتباط کارت"); break;
    case 0x9905: ShowCardingResult(4,0,Config.UC,0,0,"رمز کارت فعال است"); break;
    case 0x9920: 
    case 0x9930: 
			Status=S_OutOfRange; break;
	  case 0x9904: ShowCardingResult(4,0,Config.UC,0,0,"تاريخ انقضاي کارت");  break;
	  case 0x9901: ShowCardingResult(4,0,Config.UC,0,0,"کارت غير فعال است");  break;
		//case 0x9930: ShowCardingResult(4,0,Config.UC,0,0,"مبلغ بيش از حد مجاز"); break;
	  case 33:     ShowCardingResult(4,0,Config.UC,0,0,"کارت غير فعال است"); break;
  	case 34:     ShowCardingResult(4,0,Config.UC,0,0,"کارت مسدود است"); break;
		case 0xABCD: ShowCardingResult(4,0,Config.UC,0,0,"کارت راننده"); break;
  	case 80:     ShowCardingResult(4,0,Config.UC,0,0,"محدوده زماني غير مجاز"); break;
  	case 81:     ShowCardingResult(4,0,Config.UC,0,0,"تعداد دفعات غير مجاز"); break;
  }
	return(Status);
}


unsigned int SearchForCard(unsigned long int CardID, unsigned int *LastTime){
	unsigned long int SendUsers;
	unsigned char MEMBuffer[LEN_TRANSACTIONS+5];
	unsigned int Idx=0, Num, i, j, InvalidDateRecord=0;
	unsigned char YY=Year-1300;	
	unsigned char bufCardID[4];

  bufCardID[0]=CardID & 0xFF;	
  bufCardID[1]=CardID>>8 & 0xFF;	
  bufCardID[2]=CardID>>16 & 0xFF;	
  bufCardID[3]=CardID>>24 & 0xFF;	
	
  //printf("\n\rCardID: %X %X %X %X",bufCardID[0],bufCardID[1],bufCardID[2],bufCardID[3]);

	*LastTime=0;
	Num=0;
	if(trHead==0)  SendUsers=MAX_TRANSACTIONS-1;
	else    SendUsers=trHead-1;

  while(1)  
  {
		if(++Idx>5000)
			break;
		if(SendUsers==trTail) 
			break;

    LoadFromDFToRam(addTransactions+(SendUsers*LEN_TRANSACTIONS), 32/*LEN_TRANSACTIONS*/, MEMBuffer);
    if(SendUsers==0)  SendUsers=MAX_TRANSACTIONS-1;
    else    					SendUsers--;	

		WDTR;
		//printf("\n\rSU:%d %d/%d/%d", SendUsers,MEMBuffer[5],MEMBuffer[6],MEMBuffer[7]);

		if((MEMBuffer[4]!=YY)||(MEMBuffer[5]!=Month)||(MEMBuffer[6]!=Day))
		{
      //if(InvalidDateRecord>=20) 
			//	break;
			return Num;
		}
     
		//printf(" CardID: %2X%2X%2X%2X",MEMBuffer[1],MEMBuffer[2],MEMBuffer[3],MEMBuffer[30]);

    if((MEMBuffer[0]!=bufCardID[0])||(MEMBuffer[1]!=bufCardID[1])||(MEMBuffer[2]!=bufCardID[2])||(MEMBuffer[30]!=bufCardID[3]))
			continue;
		
		
		Num++;
		if(Num==1)
		{
			i=(MEMBuffer[7]*3600)+(MEMBuffer[8]*60)+MEMBuffer[9];
			j=(Hour*3600)+(Min*60)+Sec;
			if(j<i)
  			*LastTime=0;
			else
			  *LastTime=j-i;
		}
  }//while  
  return Num;	
}


unsigned char IsBlocked(void) {
#ifdef WithSAMCARD
int First=0, Last=0, Address=0, OldAddress=-1;	
char i;
unsigned char buf[8];	
	
	if(BlackList_Count==0)
		return(0);
	
	//SetTX485();
	//printf("\n\rCard PAN: ");
	//for(i=0; i<8; i++) printf("%2X", PayInfo[7+i]);
 	//printf("\n\rCount: %d", BlackList_Count);
	
	Last=BlackList_Count-1;
	if(BlackList_Count==1)
		Last=1;
	while(First!=Last)
	{
	  Address=(First+Last)/2;
  	//printf("\n\rFirst: %d Last: %d Address: %d Old: %d>> ", First, Last, Address, OldAddress);
		if(OldAddress==Address)
		{
			Address++;
			First=Last=Address;
		}
    LoadFromDFToRam(addBlackList+(Address*8), 8, buf);
		
	  //for(i=0; i<8; i++) printf("%2X", buf[i]);
		for(i=0; i<8; i++)
		{
      //CardPAN in PayInfo from 7..14
		  if(PayInfo[7+i]>buf[i])
			{
				First=Address;
				break;
			}	
			else if(PayInfo[7+i]<buf[i])
			{
				Last=Address;
				break;
			}
		}
		if(i>=8)
			return(1); //Found
		OldAddress=Address;
		
	}
	return(0);
#endif	
}



/*
* @brief Get the maximum credit of cards on different sites
* @param UC: The UC code of the site
* @return The maximum allowable amount of credit
*/
unsigned long int getMaxCharge(unsigned int UC) {
	switch(UC) {
    case 6364: //گرگان
    case 6374: //خوي
    case 5750:
    case 6025: //ساوه
    case 6352: //مراغه
    case 6373: //آران
    case 6356: //زنجان      
    case 6348: //شاهرود
    case 6110: //خرمشهر
      return 10000;
		
    case 6351:      
    case 7712:  //اشراق
    case 6347:  //شهرضا
    case 6115:  //ماهشهر
    case 6393:  //فلاورجان
    case 2937: 
    case 6346:  //قم
    case 6398:  //سقز
    case 5780:  //شاهين شهر
    case 6310:  //بروجرد          
      return 20000;
		
    case 6395:  //فسا   
    case 6330:  //نيشابور
      return 40000;
		
    case 6345: //رفسنجان
    case 6358: //قوچان  
    case 1010: //لرستان UNI
    case 6391: //مهاباد
    case 5855: //کرمان uni
    case 6404: //شيراز uni
      return 50000;
		
    case 6350: 
    case 6355:
      return 15000;
		
    case 6378: //تربت
      return 25000;
		
    case 7090: //موزه ها
    case 6380: //اردبيل    
    case 6135: //مرودشت
      return 100000;
		
    case 6105: //سبزوار
      return 11000;
		
    case 6325:  //بجنورد
      return 30000;
		
    case 6412:  //عاليشهر
      return 100000;
		
		case 6415:
			return 20000;
		
    default:
      return 400000;  
  }
}

char ProcessCard(unsigned char MifareType, unsigned char *snr) {
	unsigned char 
		buf[50],
		TicketBuffer[100],
		Deselect 	= 1,
		MemRes 		= 0,
		EmptyCard = 0,
		_CRC,
		Loop,
		SerialChip,
		Key,
		Byte, 
		crc,
		LastOP,
		OPIndex;
	
	unsigned int 
 		i, 
		j,
		UC,
		Price,
		CardTimeout2,
		_ID;
	
	unsigned long int 
		CardID,
		ID = 0,
		Li,
		MaxCharge;
	
	unsigned short 
		Grouh = 0,
		Status,
		LastDevice;
	
	signed long int 
		Etebar, 
		PreEtebar,
		RemEtebar,
		MinCharge = 0,
		TempInt;
	
	char str[100], Try=0;
	static unsigned char RepeatedCard_Counter = 0;
	long int Credit = 0, Temp;
	signed long int OrgPayment = 0; 

  if (MifareType == 2) {
    #ifdef WithSAMCARD
		Status=0;
    if(ISO14443_LoginE2(0,0x10))
    {
      if(ISO14443_SingleTagSelect(snr))
        ISO14443_SingleTagSelect(snr);
      if(!ISO14443_LoginE2(0,0x10))
		    Status=1;
    }else 
		  Status=1;

		if(Status==1)
		{
      if(!ISO14443_ReadBlock(1, buf)) 
      {
        GenerateCardIdUC(buf,snr,&UC,&_ID);
        ID=_ID;
        if(CheckCard(UC, ID)==0)
				{
          ShowMessageDlg(mtInformation, "کارت مسدود شده است",1,0,0,0);  
					InfoCounter=2; 
				}
      }
			return(1);
    }
    #endif
  }
	 
  Price     = Config.GrouhPrice[0][0];
	MaxCharge = getMaxCharge(Config.UC);
	
  if (Config.UC == 6356)
    if ((ID >= 224076) && (ID <= 224250)) 
      MaxCharge = 40000;
  if (Config.UC == 6325)  //بجنورد
    MinCharge = -1200;
	
  ID = 0;
  UC = Config.UC; 
 
  for (i = 0; i < 30; i++) 
		PayInfo[i] = 0;

  if (MifareType == 2) {
    #ifdef WithSAMCARD
		
    {Price=Config.GrouhPrice[0][0]; 
    //SetTX485();//////////////////////
    Status=ReadCardIDKashanMifare(&ID, Price*10,  &Etebar, &PreEtebar);
    if((Status>=19)&&(Status<=29)) Status=ReadCardIDKashanMifare(&ID, Price*10,  &Etebar, &PreEtebar);
		Etebar/=10;
		PreEtebar/=10;
    //SetRX485();/////////////////

    WDTR;
    if(LCDType==LCD800_480)
		{
      GUI_SetColor(GUI_BLACK);
      GUI_FillRoundedRect(5, 275-(LCDType==LCD480_272?40:0), 272-5+SCREENRESIZE-(LCDType==LCD480_272?SCREENRESIZE:0), 275+30-(LCDType==LCD480_272?40:0),6);	

      GUI_SetColor(GUI_DARKGREEN);
      GUI_DrawRoundedFrame(5, 275-(LCDType==LCD480_272?40:0), 272-5+SCREENRESIZE-(LCDType==LCD480_272?SCREENRESIZE:0), 275+30-(LCDType==LCD480_272?40:0),6,3); 
		}
    InfoCounter=2;   
    if((Status>=0x6300)&&(Status<=0x63FF))
	    ShowCardingResult(4,Etebar,Config.UC,ID,0,"ناسازگاري کارت با ماژول امنيتي");
    else if((Status>=19)&&(Status<=29))
    {
      if(Status==19) 	 ShowCardingResult(4,Etebar,Config.UC,ID,0,"خطا در ماژول امنيتي");
      else      	     ShowCardingResult(4,Etebar,Config.UC,ID,0,"");
   	}
    else
    {
			if((Status!=0)&&(Status!=0x9714))
				Status=ShowErrorKashan(Status);
		}}
		#else
    Status = ExecTransaction(2, &ID, &UC, Price, MinCharge, MaxCharge, 
			Config.BusID, &Etebar, &PreEtebar, &LastDevice, &LastOP, &OPIndex);

    if (Status == 6) {//Is Old Format  
      ID = 0;                             
      UC = Config.UC;
      Status = Old_ExecTransaction(2, &ID, &UC, Price, MinCharge, MaxCharge, 
				Config.BusID, &Etebar, &PreEtebar, &LastDevice, &LastOP, &OPIndex);
      if (Status == 1) {//Is Old Format  
        ID = 0;                             
        UC = Config.UC;
        Status = Old_ExecTransaction(2, &ID, &UC, Price, MinCharge, MaxCharge, 
					Config.BusID, &Etebar, &PreEtebar, &LastDevice, &LastOP, &OPIndex);
      } 
    }
		#endif
  }
  else if (MifareType == 3) {
    #ifdef WithSAMCARD
    Price=Config.GrouhPrice[0][0]; 

    Status=ReadCardIDKashan(&ID, Price*10,  &Etebar, &PreEtebar);
    if((Status>=19)&&(Status<=29)) Status=ReadCardIDKashan(&ID, Price*10,  &Etebar, &PreEtebar);

    WDTR;
    if(LCDType==LCD800_480)
		{
      GUI_SetColor(GUI_BLACK);
      GUI_FillRoundedRect(5, 275-(LCDType==LCD480_272?40:0), 272-5+SCREENRESIZE-(LCDType==LCD480_272?SCREENRESIZE:0), 275+30-(LCDType==LCD480_272?40:0),6);	

      GUI_SetColor(GUI_DARKGREEN);
      GUI_DrawRoundedFrame(5, 275-(LCDType==LCD480_272?40:0), 272-5+SCREENRESIZE-(LCDType==LCD480_272?SCREENRESIZE:0), 275+30-(LCDType==LCD480_272?40:0),6,3); 
		}
    SetRX485();
    #endif

    InfoCounter = 2;   
    if ((Status >= 0x6300) && (Status <= 0x63FF))
	    ShowCardingResult(4, Etebar, Config.UC, ID, 0, "ناسازگاري کارت با ماژول امنيتي");
    else if ((Status >= 19) && (Status <= 29)) {
      if (Status == 19) ShowCardingResult(4, Etebar, Config.UC, ID, 0, "خطا در ماژول امنيتي");
      else      	      ShowCardingResult(4, Etebar, Config.UC, ID, 0, "");
      InfoCounter = 2;          
   	}
    else {			
			if ((Status != 0) && (Status != 0x9714))
				Status = ShowErrorKashan(Status);
			else {
        #ifdef CLRC_CHIP
        if((MifareType==3)&&(InTestMode==0))  phpalI14443p4_Deselect(&palI14443p4); 
        #endif
			}
  	}
  }
     
  if (Status == 11)
    if (CheckCard(UC, ID))
      return(1);

		/*if (PreEtebar < 0) {
			TempInt1  =  0x100000000 - PreEtebar;
			TempInt1 |= (0x800000);
		}
		else
			TempInt1 = PreEtebar;
		
		if (Etebar < 0) {
			TempInt2  = 0x100000000 - Etebar;
			TempInt2 |= (0x800000);
		}
		else
			TempInt2 = Etebar;
  
		sprintf(TicketBuffer, "{\"C\":1, \"ID\":%u, \"Type\":%d, \"UC\":%u, \""  \
			"Year\":%u, \"Month\":%u, \"Day\":%u, \"Hour\":%u, \"Minute\":%u, \""  \
			"Second\":%u, \"Grouh\":%u, \"Price\":%u, \"PreEtebar\":%u, \"Etebar"  \
   		"\":%u, \"BusID\":%u, \"OperatorID\":%u, \"LastOP\":%u, \"LastDevice"  \
			"\":%u}", 
			ID, DeviceType, UC, Year, Month, Day, Hour, Min, Sec, Grouh, Price,
			TempInt1, TempInt2, Config.BusID, Indicators.OperatorID, LastOP, 
			LastDevice);*/
		
	//HNA - saveing data to ram to send offline in the future
	/*TicketBuffer[0] =  ID 			 & 0xFF;
  TicketBuffer[1] = (ID >>  8) & 0xFF;
  TicketBuffer[2] = (ID >> 16) & 0xFF;

	TicketBuffer[3] = 2; //Device Type: 2 means front door, 3 means back door

	TicketBuffer[4] =  UC        & 0xFF;
	TicketBuffer[5] = (UC >> 8)  & 0xFF;
	TicketBuffer[6] = (UC >> 16) & 0xFF;
		
  TicketBuffer[4] = GetUCMap(UC);

  TicketBuffer[4] = Year - 1300;
  TicketBuffer[5] = Month;
  TicketBuffer[6] = Day;
  TicketBuffer[7] = Hour;
  TicketBuffer[8] = Min;
  TicketBuffer[9] = Sec;

  if (Status == 0xABCD)
 	  Grouh = 200;
				
  TicketBuffer[10] =  Grouh 			& 0xFF;
  TicketBuffer[11] = (Grouh >> 8) & 0xFF;
      
  TicketBuffer[12] = Price 				& 0xFF;
  TicketBuffer[13] = (Price >> 8) & 0xFF;
      
  if (PreEtebar < 0) {
    TempInt  = 0x100000000 - PreEtebar;
    TempInt |= (0x800000);
  }
  else
    TempInt = PreEtebar;
      
  TicketBuffer[14] =  TempInt 			 & 0xFF;
  TicketBuffer[15] = (TempInt >>  8) & 0xFF;
  TicketBuffer[16] = (TempInt >> 16) & 0xFF;
  TicketBuffer[17] = (TempInt >> 24) & 0xFF;
      
  if (Etebar < 0) {
    TempInt  = 0x100000000 - Etebar;
    TempInt |= (0x800000);
  }
  else
    TempInt = Etebar;     

  TicketBuffer[18] =  TempInt        & 0xFF;
  TicketBuffer[19] = (TempInt >>  8) & 0xFF;
  TicketBuffer[20] = (TempInt >> 16) & 0xFF;

  TicketBuffer[22] = 0xE9;
  TicketBuffer[23] = Config.BusID % 256;
  TicketBuffer[24] = Config.BusID / 256; 
  TicketBuffer[25] =  Indicators.OperatorID        & 0xFF;
  TicketBuffer[26] = (Indicators.OperatorID >>  8) & 0xFF;
  TicketBuffer[27] = (Indicators.OperatorID >> 16) & 0xFF;
  TicketBuffer[21] = LastOP;
  TicketBuffer[28] = LastDevice % 256;
  TicketBuffer[29] = LastDevice / 256;*/
	
	//==========================================================
	//====================Old Format============================
	//==========================================================
	
  TicketBuffer[0] =  ID 			 & 0xFF;
  TicketBuffer[1] = (ID >>  8) & 0xFF;
  TicketBuffer[2] = (ID >> 16) & 0xFF;
			
  TicketBuffer[3] = GetUCMap(UC);
			
  TicketBuffer[4] = Year - 1300;
  TicketBuffer[5] = Month;
  TicketBuffer[6] = Day;
  TicketBuffer[7] = Hour;
  TicketBuffer[8] = Min;
  TicketBuffer[9] = Sec;

  if (Status == 0xABCD)
 	  Grouh = 200;
				
  TicketBuffer[10] =  Grouh 			& 0xFF;
  TicketBuffer[11] = (Grouh >> 8) & 0xFF;
      
  TicketBuffer[12] = Price 				& 0xFF;
  TicketBuffer[13] = (Price >> 8) & 0xFF;
      
  if (PreEtebar < 0) {
    TempInt  = 0x100000000 - PreEtebar;
    TempInt |= (0x800000);
  }
  else
    TempInt = PreEtebar;
      
  TicketBuffer[14] =  TempInt 			 & 0xFF;
  TicketBuffer[15] = (TempInt >>  8) & 0xFF;
  TicketBuffer[16] = (TempInt >> 16) & 0xFF;
  TicketBuffer[17] = (TempInt >> 24) & 0xFF;
      
  if (Etebar < 0) {
    TempInt  = 0x100000000 - Etebar;
    TempInt |= (0x800000);
  }
  else
    TempInt = Etebar;     

  TicketBuffer[18] =  TempInt        & 0xFF;
  TicketBuffer[19] = (TempInt >>  8) & 0xFF;
  TicketBuffer[20] = (TempInt >> 16) & 0xFF;

  TicketBuffer[22] = 0xE9;
  TicketBuffer[23] = Config.BusID % 256;
  TicketBuffer[24] = Config.BusID / 256; 
  TicketBuffer[25] =  Indicators.OperatorID        & 0xFF;
  TicketBuffer[26] = (Indicators.OperatorID >>  8) & 0xFF;
  TicketBuffer[27] = (Indicators.OperatorID >> 16) & 0xFF;
  TicketBuffer[21] = LastOP;
  TicketBuffer[28] = LastDevice % 256;
  TicketBuffer[29] = LastDevice / 256;

  #ifdef WithSAMCARD
  TicketBuffer[21]=Config.BusID&0xFF;  
  TicketBuffer[28]=Indicators.TransactionUID%256;
  TicketBuffer[29]=Indicators.TransactionUID/256;
      
  TicketBuffer[30]=(ID>>24)&0xFF;
  for(i=0;i<30;i++) TicketBuffer[31+i]=PayInfo[i];
  #endif
 
  switch(Status) {
		case 0x9714: {
      Grouh = 250;
		  TicketBuffer[10] = Grouh & 0xFF;
      TicketBuffer[11] = (Grouh >> 8) & 0xFF;
      SaveTransaction(TicketBuffer);
      Indicators.OffTransactions++; 
    	if (Indicators.OffTransactions >= MAX_TRANSACTIONS) 
				Indicators.OffTransactions = MAX_TRANSACTIONS;
			Status = ShowErrorKashan(34);
		  return 0;
			break;
		}
    case S_Success: {
			#ifdef WithSAMCARD
			if(IsBlocked())
 			{
				Status=ShowErrorKashan(34);
	  		return 0;
		  }
			#endif
      LastCardingDateTime[0] = Year - 1300;
      LastCardingDateTime[1] = Month;
      LastCardingDateTime[2] = Day;
      LastCardingDateTime[3] = Hour;
      LastCardingDateTime[4] = Min;
      LastCardingDateTime[5] = Sec;
      if (LastCardSec > Sec)  
				CardTimeout2 = (60 - LastCardSec) + Sec;
      else                 
				CardTimeout2 = Sec - LastCardSec;

      if ((CardTimeout2 < 10) && ((snr[0] == LastCardSnr[0]) && 
					(snr[1] == LastCardSnr[1]) && (snr[2] == LastCardSnr[2]) && 
					(snr[3] == LastCardSnr[3])))
        RepeatedCard_Counter++;
      else
        RepeatedCard_Counter = 0;
          
      #ifdef WithSAMCARD
      if(MifareType==3)  {TicketBuffer[10]=14;TicketBuffer[11]=14;}
      #endif
					
      SaveTransaction(TicketBuffer);          
      Indicators.OffTransactions++; 
    	if (Indicators.OffTransactions >= MAX_TRANSACTIONS) 
				Indicators.OffTransactions = MAX_TRANSACTIONS;
      Indicators.TransactionUID++;
        
      if ((Indicators.TransactionUID & 0x000000FFFF) >= 65536) 
        Indicators.TransactionUID = 0;
					
      ///////////////////////SaveIndicators();
			WaitForAddress    = 0;
		  DataCurrentlySend = 1;	
      SendOfflines();

			ShowCardingResult(0, Etebar, Config.UC, ID, 0, "");
          
      LastCardSnr[0] = snr[0];
      LastCardSnr[1] = snr[1];
      LastCardSnr[2] = snr[2];
      LastCardSnr[3] = snr[3];
      LastCardSec    = Sec;
      Grouh          = 0; //Default
   
      if (LCDType == LCD480_272) {
				GUI_SetColor(GUI_BLACK);
				GUI_FillRect(100, 6, 230, 25);
        GUI_SetColor(GUI_YELLOW); 
        sprintf(str, "آفلاين :%d", Indicators.OffTransactions);
        PutText(100, -3, 230, 25, str, GUI_TA_RIGHT);
			}
			else {
        GUI_DrawGradientV(130 + SCREENRESIZE, 4, 220 + SCREENRESIZE, 28, 
					0x000F0000, 0x00070000);							
        GUI_SetColor(GUI_WHITE); 
        sprintf(str, "آفلاين : %d", Indicators.OffTransactions);
        PutText(130 + SCREENRESIZE, 0, 220 + SCREENRESIZE, 25, str, 
					GUI_TA_RIGHT);							
			}
      InfoCounter = 2;
      break;
		}
    case S_OutOfRange: {
      ShowCardingResult(1, PreEtebar, Config.UC, ID, Status, "");
      InfoCounter = 2;
      break;
		}
    case S_InvalidCardUC: {
      ShowCardingResult(2, Etebar, UC, ID, Status, "");
      InfoCounter = 2;
      break;
		}
    case 1:
      //ShowCardingResult(4,Etebar,UC,ID,Status,"کارت را نگه داريد .");
      //InfoCounter=2;
      break;
    default:
      //ShowCardingResult(3,Etebar,UC,ID,Status,"");
      InfoCounter = 2;
      break;
  }
}

void SendOfflines(void) {
	unsigned char Byte, NumRecords = 0;
	unsigned int   Idx = 0, i;
	unsigned short crc = 0;
	unsigned long int SendUsers;
	unsigned long int Start,Temp;
	unsigned char MEMBuffer[LEN_TRANSACTIONS];
	static char GG = 0;
	char str[512];
	
  if (Indicators.OffTransactions == 0) 
		return; 
	
  Start = (Indicators.OffTransactions > trHead) ? 
		(MAX_TRANSACTIONS - (Indicators.OffTransactions - trHead)) : 
		trHead - Indicators.OffTransactions;

  Idx = 11;
  SendUsers = Start;
  while (1) {
    if (SendUsers == trHead)
      break;
		
    LoadFromDFToRam(addTransactions + (SendUsers * LEN_TRANSACTIONS), 
			LEN_TRANSACTIONS, MEMBuffer);
		
		//================== Written by HNA ========================================
		//Send every record in JSON format
		sprintf(str, //-------------------------------------------------------------  Creating a JSON data to send for BCU via WIFI
			"{\"C\":1, \"ID\":%u, \"Type\":%u, \"UC\":%u, \"Year\":%u, "  \
			"\"Month\":%u, \"Day\":%u, \"Hour\":%u, \"Minute\":%u, \"Second\":%u, "  \
			"\"Grouh\":%u, \"Price\":%u, \"PreEtebar\":%u, \"Etebar\":%u, \"BusID\"" \
			":%u, \"OperatorID\":%u, \"LastOP\":%u, \"LastDevice\":%u}\n", 
			 MEMBuffer[0] 			+ 	  //ID
			(MEMBuffer[1] << 8) + 
			(MEMBuffer[2] << 16),
			 2,												// Type
 UCMap[MEMBuffer[3]], 					// UC
			 MEMBuffer[5] + 1300,			// Year
			 MEMBuffer[6], 						// Month
			 MEMBuffer[7], 						// Day
			 MEMBuffer[8], 						// Hour
			 MEMBuffer[9], 						// Minute
			 MEMBuffer[10], 					// Second
			 MEMBuffer[11] +					// Grouh
			(MEMBuffer[12] << 8),
			 MEMBuffer[13] + 					// Price
			(MEMBuffer[14] << 8),
			 MEMBuffer[15] +					// PreEtebar
			(MEMBuffer[16] << 8)  +
			(MEMBuffer[17] << 16) +
			(MEMBuffer[18] << 24),
			 MEMBuffer[19] 			 + 		// Etebar
			(MEMBuffer[20] << 8) +
			(MEMBuffer[21] << 16), 
			 MEMBuffer[22], 					// LastOP
			 MEMBuffer[23] +					// BusID
			(MEMBuffer[24] << 8),
			 MEMBuffer[25] + 					// Operator ID
			(MEMBuffer[26] << 8) + 
			(MEMBuffer[27] << 16),
			 MEMBuffer[28] + 					// LastDevice
			(MEMBuffer[29] << 8)
		);
		WIFISend(strlen(str), str);
		
		sprintf(str, "%02X%02X%02X%02X%02X%02X%02X%02X%02X%02X%02X%02X%02X%02X%02X" \
			"%02X%02X%02X%02X%02X%02X%02X%02X%02X%02X%02X%02X%02X%02X%02X\n",
			MEMBuffer[0],MEMBuffer[1],MEMBuffer[2],MEMBuffer[3],MEMBuffer[4],MEMBuffer[5],
			MEMBuffer[6],MEMBuffer[7],MEMBuffer[8],MEMBuffer[9],MEMBuffer[10],MEMBuffer[11],
			MEMBuffer[12],MEMBuffer[13],MEMBuffer[14],MEMBuffer[15],MEMBuffer[16],MEMBuffer[17],
			MEMBuffer[18],MEMBuffer[19],MEMBuffer[20],MEMBuffer[21],MEMBuffer[22],MEMBuffer[23],
			MEMBuffer[24],MEMBuffer[25],MEMBuffer[26],MEMBuffer[27],MEMBuffer[28],MEMBuffer[29],
			MEMBuffer[30]);
			
		WIFISend(strlen(str), str);
		

    SendUsers++;
    if (SendUsers >= MAX_TRANSACTIONS)
      SendUsers = 0;
    if (++NumRecords >= 5) 
			break;
  }
	Delete_Records();
  	
	/*unsigned char  HNA
		Byte, 
		MEMBuffer[LEN_TRANSACTIONS],
		NumRecords = 0;
	
	unsigned long int 
		SendUsers,
		Start,
		Temp;
	
	unsigned int Idx   = 0, i;
	unsigned short crc = 0;	
	static char GG     = 0;
	
  if (Indicators.OffTransactions == 0) 
		return; 
	
  Start = (Indicators.OffTransactions > trHead) ? 
		(MAX_TRANSACTIONS - (Indicators.OffTransactions - trHead)) : 
		trHead - Indicators.OffTransactions;

  Idx       = 11;
  SendUsers = Start;
  while (1) {
    if (SendUsers == trHead)
      break;
		
    LoadFromDFToRam(addTransactions + (SendUsers * LEN_TRANSACTIONS), 
			LEN_TRANSACTIONS, MEMBuffer);
    Byte          = MEMBuffer[30];
    MEMBuffer[30] = MEMBuffer[29];
    MEMBuffer[29] = MEMBuffer[28];
    MEMBuffer[28] = MEMBuffer[21];
    MEMBuffer[21] = 0;   

    for (i=0; i<31; i++)
      GlobalBuffer[Idx++] = MEMBuffer[i];
      
    #ifdef WithSAMCARD
    GlobalBuffer[Idx++]=Byte;//30 ID>>4
    for(i=0; i<30; i++)
      GlobalBuffer[Idx++]=MEMBuffer[31+i];//PayInfo
    #endif		
    
    SendUsers++;
    if (SendUsers >= MAX_TRANSACTIONS)
      SendUsers = 0;
    if (++NumRecords >= 5) 
			break;
  }                 
  
  Temp = SendUsers;
	
	Idx = 0; HNA
	GlobalBuffer[Idx++] = STX;
  GlobalBuffer[Idx++] = 10; //Header Transaction
	GlobalBuffer[Idx++] = 45; //Size of data in front of...
	
	//================= Device ID =======================
  GlobalBuffer[Idx++] =  Config.DeviceID 			  & 0xFF;
  GlobalBuffer[Idx++] = (Config.DeviceID >>  8) & 0xFF;
  GlobalBuffer[Idx++] = (Config.DeviceID >> 16) & 0xFF;
	
	//================= Device Type =====================
  GlobalBuffer[Idx++] = Config.deviceType;
  
	//================= UC ==============================
	GlobalBuffer[Idx++] =  Config.UC 			  & 0xFF;
  GlobalBuffer[Idx++] = (Config.UC >>  8) & 0xFF;
  GlobalBuffer[Idx++] = (Config.UC >> 16) & 0xFF;
	
	//================= Operator ID =====================
	GlobalBuffer[Idx++] =  Config.DriverID 			  & 0xFF;
  GlobalBuffer[Idx++] = (Config.DriverID >>  8) & 0xFF;
  GlobalBuffer[Idx++] = (Config.DriverID >> 16) & 0xFF;
	
	//================= Line ID =========================
	GlobalBuffer[Idx++] =  Config.LineCode 				& 0xFF;
  GlobalBuffer[Idx++] = (Config.LineCode >>  8) & 0xFF;
  GlobalBuffer[Idx++] = (Config.LineCode >> 16) & 0xFF;
	
	//================= Card ID =========================
	GlobalBuffer[Idx++] =  Config.LineCode & 0xFF;
  GlobalBuffer[Idx++] = (Config.LineCode>>  8) & 0xFF;
  GlobalBuffer[Idx++] = (Config.LineCode >> 16) & 0xFF;
	
	//================= preCredit Card ==================
	GlobalBuffer[Idx++] = Config.
	//================= Price ===========================
	//================= Card Credit =====================
	//================= Date ============================
	//================= QR Id ===========================
	//================= CRC & ETX =======================
	
  ActivePort = 3;	HNA
	WIFISend(Idx, GlobalBuffer);*/
}

void ProcessPeriodicTasks(void) {
	unsigned int i;
	int percent;
	unsigned char 
		Last_WifiConnectionStep = 0,
		LWiFiStep 							= 0,
		elapsed 								= 0,
		str[50];

	++Sec;	
  if (OldSec > Sec)
    OldSec = (60 - OldSec) + Sec;
  else
    OldSec = Sec - OldSec;
	
  if ((Sec % 2) == 0) {
		RTC_Get_Time();
		OldSec = Sec;
	}

  //..........................................................................................................
  if (Config.WiFi != 103) {
		LWiFiStep = WiFiStep;
		CheckWiFi();
		
		if (WiFiStep == 10)
			if (LWiFiStep == 9) {//If connected to server for the first time
				ActivePort = 3; 
				SendDeviceInfo(); 
				ConnectedToServer = 1; 
			}
			else 								//If connected previously
				SendALive();		
	}
  //..........................................................................................................
   
	if(WiFiStep < 7)
		ConnectedToServer = 0;

	if(Min != OldMin) {
		RTC_Get();
		//ShowPageAlef();
		GUI_SetColor(0);
		GUI_FillRoundedRect(8, 348 + (LCDType == LCD480_272 ? 55 : 0), 
													 272 - 8 + SCREENRESIZE - (LCDType == LCD480_272 ? SCREENRESIZE : 0), 
													 430 + (LCDType == LCD480_272 ? 50 : 0), 6);

		LoadFont(FontBNazanin140);

		GUI_SetColor(0);
		sprintf(str,"%d%d ", OldDisplayedHour / 10, OldDisplayedHour % 10);
		PutText(25 - 10, 325 + (LCDType == LCD480_272 ? 45 : 0), 272, 450 + (LCDType == LCD480_272 ? 30 : 0), str, GUI_TA_LEFT);		
		PointLocation = GUI_GetStringDistX(str) + 40 - 10;
		sprintf(str, "%d%d", OldDisplayedMin / 10, OldDisplayedMin % 10);
		SecLocation = GUI_GetStringDistX(str) + 10 + PointLocation;
		PutText(PointLocation + 10, 325 + (LCDType == LCD480_272 ? 45 : 0), 272, 450 + (LCDType == LCD480_272 ? 30 : 0), str, GUI_TA_LEFT);

		GUI_FillCircle(PointLocation, 15 + 370 + (LCDType == LCD480_272 ? 55 : 0), 5);
		GUI_FillCircle(PointLocation, 40 + 370 + (LCDType == LCD480_272 ? 55 : 0), 5);		

		OldMin = Min;

		GUI_SetColor(0);
		if (LCDType == LCD800_480)
			GUI_FillRect(SecLocation, 350, 263, 390);

		GUI_SetColor(GUI_WHITE);	
		sprintf(str, "%d%d ", Hour / 10, Hour % 10);
		PutText(25 - 10, 325 + (LCDType == LCD480_272 ? 45 : 0), 272, 450 + (LCDType == LCD480_272 ? 30 : 0), str, GUI_TA_LEFT);		
		PointLocation = GUI_GetStringDistX(str) + 40 - 10;
		sprintf(str, "%d%d", Min / 10, Min % 10);
		SecLocation = GUI_GetStringDistX(str) + 10 + PointLocation;
		PutText(PointLocation+10, 325 + (LCDType == LCD480_272 ? 45 : 0), 272, 450 + (LCDType == LCD480_272 ? 30 : 0), str, GUI_TA_LEFT);
		OldDisplayedHour = Hour;
		OldDisplayedMin  = Min;
	}
	
  if(InfoCounter == 0) {
		l = 1 - l;
		GUI_SetColor(RGB(l * 255, l * 255, l * 255));  
    LoadFont(FontBNazanin60);		
    GUI_FillCircle(PointLocation, 15 + 370 + (LCDType == LCD480_272 ? 55 : 0), 5);
    GUI_FillCircle(PointLocation, 40 + 370 + (LCDType == LCD480_272 ? 55 : 0), 5);
    GUI_SetColor(0);
    GUI_FillRect(SecLocation, 350 + (LCDType == LCD480_272 ? 55 : 0), 263, 390 + (LCDType == LCD480_272 ? 65 : 0));
    GUI_SetColor(GUI_WHITE);
		GUI_DispDecAt(Sec, SecLocation + 2, 340 + (LCDType == LCD480_272 ? 55 : 0), 2);  
  }

	if (InfoCounter)
    if (--InfoCounter == 0) {
      Guest       = 0;
      PaymentPage = 0;
			LEDOK       = 0;
			LEDERR      = 0;
      ShowPaymentBox();
    }
	
  DataSendInterval++;
	if (ConnectedToServer) {
		if ((++DataSendInterval > Config.TransactionsSendInterval) && (Config.TransactionsSendInterval)) {
			if (Indicators.OffTransactions) {
				DataSendInterval 			 = 0;
				DeviceInfoSendInterval = 0;
				SendOfflines();
			}
			else if (!Check_ServerForUpdateFile) {//&&((ActivePort!=0)))
				DataSendInterval 			 = 0;
				DeviceInfoSendInterval = 0;
				CheckUpdateTry++;
				Download_NewFirmwares();
			}
			else if (Download_NewFirmwares()) {
				DataSendInterval 			 = 0;
				DeviceInfoSendInterval = 0;
			}

			if (++DeviceInfoSendInterval > Config.DeviceInfoSendInterval) {
				DeviceInfoSendInterval    = 0;
				if ((ActivePort == 3) && (Check_ServerForUpdateFile) && (!IsFileForGprsDownload)) 
					Check_ServerForUpdateFile = 0;
				if (!Download_NewFirmwares())
					SendALive(150);
			}
		}
	}
}

void InitializeAlef(void) {
	unsigned long int j, i, address=0;
	unsigned char BCC=0, buf[LEN_TRANSACTIONS+10];
	unsigned char b[8];
	unsigned int x=0;
	unsigned short _FirmwareRelease;
	unsigned short _FirmwareVer;
	
  PowerMode=1;
  Config.Beep=1;
	LastGPS.Time=OS_TimeMS;
	
  Config.ConnectionRetry=3;

  Config.DeviceInfoSendInterval=10; ///////////////////////////90
	LoadRecordsInfo();
  Config.LNG   = 0;
  GPSValidData = 0;  

  //DeleteAllMessages(); by Hesam Nickhou. not used in wifi model
  
	RTC_Get();	
  OldSec=Sec;
 
  LoadFromDFToRam(addFirmwareInfo, 32, GlobalBuffer);
  if((GlobalBuffer[0]==0xE2)&&(GlobalBuffer[7]==0xE9))
  {
    FirmwareRequestNo=GlobalBuffer[2]; FirmwareRequestNo<<=8;
    FirmwareRequestNo+=GlobalBuffer[1];
    FirmwareLength=GlobalBuffer[6]; FirmwareLength<<=8;
    FirmwareLength+=GlobalBuffer[5]; FirmwareLength<<=8;
    FirmwareLength+=GlobalBuffer[4]; FirmwareLength<<=8;
    FirmwareLength+=GlobalBuffer[3];
    _FirmwareVer=GlobalBuffer[9]+(GlobalBuffer[10]*256);
    _FirmwareRelease=GlobalBuffer[11]+(GlobalBuffer[12]*256);
    if((FirmwareLength>512*1024)||(_FirmwareVer!=Ver)||((_FirmwareVer==Ver)&&(_FirmwareRelease<=Release)))
    {
      FirmwareRequestNo=0; 
      FirmwareLength=0;
      for(i=0; i<32; i++)
        GlobalBuffer[i]=0;
     	FLASH_UNLOCK
      SaveFromRamToDF(addFirmwareInfo, 32, &GlobalBuffer[0]);
     	FLASH_LOCK
    }else{  
      Firmware.Length=GlobalBuffer[16]; Firmware.Length<<=8;
      Firmware.Length+=GlobalBuffer[15]; Firmware.Length<<=8;
      Firmware.Length+=GlobalBuffer[14]; Firmware.Length<<=8;
      Firmware.Length+=GlobalBuffer[13];
      Firmware.CheckSum=GlobalBuffer[20]; Firmware.CheckSum<<=8;
      Firmware.CheckSum+=GlobalBuffer[19]; Firmware.CheckSum<<=8;
      Firmware.CheckSum+=GlobalBuffer[18]; Firmware.CheckSum<<=8;
      Firmware.CheckSum+=GlobalBuffer[17];  
      for(i=0; i<15; i++)
        Firmware.FirmwareFileName[i]=GlobalBuffer[i+21];
    }
  }
	else if((GlobalBuffer[0]==0xC2)&&(GlobalBuffer[7]==0x55))
	{
    for(i=0; i<32; i++)
      GlobalBuffer[i]=0;
   	FLASH_UNLOCK
    SaveFromRamToDF(addFirmwareInfo, 32, &GlobalBuffer[0]);
   	FLASH_LOCK
	}
	
}

unsigned char GetDateRange(unsigned short *SYear, unsigned char *SMonth, unsigned char *SDay, unsigned short *EYear, 
	unsigned char *EMonth, unsigned char *EDay) {

	unsigned short CurX, CurY;
	unsigned char Refresh, Key;
	unsigned char Loc=0;
	unsigned char Stream[]="0000000000000000";

  
  Stream[0]=(*SYear/10)%10;
  Stream[1]=*SYear%10;
  Stream[2]=*SMonth/10;
  Stream[3]=*SMonth%10;
  Stream[4]=*SDay/10;
  Stream[5]=*SDay%10;
  
  Stream[6]=(*EYear/10)%10;
  Stream[7]=*EYear%10;
  Stream[8]=*EMonth/10;
  Stream[9]=*EMonth%10;
  Stream[10]=*EDay/10;
  Stream[11]=*EDay%10;

  GUI_DrawGradientV(0, TFT_START_Y+0, 272, TFT_START_Y+272, GUI_MAKE_ALPHA(0x00, 0xA02020), GUI_MAKE_ALPHA(0x00, 0x000000));
  GUI_SetColor(RGB(237,237,237));
  GUI_FillRect(20, TFT_START_Y+40, 272-20, TFT_START_Y+200);
  GUI_SetColor(RGB(33,31,32));
  GUI_FillRect(25, TFT_START_Y+45, 272-25, TFT_START_Y+195);
  //LoadFont(FontBKoodak40);
  GUI_SetColor(GUI_WHITE);
  PutText(50,45,220,80,"?? ?????",GUI_TA_CENTER);
  PutText(50,115,220,150,"?? ?????",GUI_TA_CENTER);
  G_LoadBMP(128,80,"0:desktop/InputBox.bmp",0);
  G_LoadBMP(128,150,"0:desktop/InputBox.bmp",0);
  DisplayKeys("??????", "...", "?????");

  GUI_SetColor(GUI_BLACK);
	GUI_DispCharAt(Stream[0]+'0', 140, 80);
	GUI_DispCharAt(Stream[1]+'0', 160, 80);
	GUI_DispCharAt('/',           180, 80);
	GUI_DispCharAt(Stream[2]+'0', 200, 80);
	GUI_DispCharAt(Stream[3]+'0', 220, 80);
	GUI_DispCharAt('/',           240, 80);
	GUI_DispCharAt(Stream[4]+'0', 260, 80);
	GUI_DispCharAt(Stream[5]+'0', 280, 80);
	
	GUI_DispCharAt(Stream[6]+'0', 140, 150);
	GUI_DispCharAt(Stream[7]+'0', 160, 150);
	GUI_DispCharAt('/',           180, 150);
	GUI_DispCharAt(Stream[8]+'0', 200, 150);
	GUI_DispCharAt(Stream[9]+'0', 220, 150);
	GUI_DispCharAt('/',           240, 150);
	GUI_DispCharAt(Stream[10]+'0', 260, 150);
	GUI_DispCharAt(Stream[11]+'0', 280, 150);

  CurX=140;
	CurY=80;
  while(Loc<=11)
  {
    switch(Key=ScanKeyboard())
    {
       case BUP:
         if(Stream[Loc]<9)
         {
           GUI_SetColor(GUI_WHITE);
           GUI_FillRect(CurX, CurY+8, CurX+15, CurY+25);
	         Stream[Loc]++;
           GUI_SetColor(GUI_ORANGE);
           GUI_DispCharAt(Stream[Loc]+'0', CurX, CurY);
         }
      	 break;
       case BDOWN : 
         if(Stream[Loc]>0)
         {
           GUI_SetColor(GUI_WHITE);
           GUI_FillRect(CurX, CurY+8, CurX+15, CurY+25);
	         Stream[Loc]--;
           GUI_SetColor(GUI_ORANGE);
           GUI_DispCharAt(Stream[Loc]+'0', CurX, CurY);
         }
      	 break;
       case BRIGHT:
         if(Loc<11)
         {
           GUI_SetColor(GUI_BLACK);
           GUI_DispCharAt(Stream[Loc]+'0', CurX, CurY);
           Loc++;
					 Refresh=1;
         }
         break;
       case BLEFT:
         if(Loc>0)
         {
           GUI_SetColor(GUI_BLACK);
           GUI_DispCharAt(Stream[Loc]+'0', CurX, CurY);
           Loc--;
					 Refresh=1;
         }
         break;
       case BF3 : 
         GUI_SetColor(GUI_BLACK);
         GUI_DispCharAt(Stream[Loc]+'0', CurX, CurY);
         Loc++;
         Refresh=1;
         if((Loc>11)||(Key==BF3))  
         {
           Loc=12;
           *SYear=(Stream[0]*10)+Stream[1];
           *SYear+=1300;
           *SMonth=(Stream[2]*10)+Stream[3];
           *SDay=(Stream[4]*10)+Stream[5];
           *EYear=(Stream[6]*10)+Stream[7];
           *EYear+=1300;
           *EMonth=(Stream[8]*10)+Stream[9];
           *EDay=(Stream[10]*10)+Stream[11];
           break;
         }  
	       break;
       case BF1:
         return(1);
			 case BOK:
				 Loc=12;
				 break;
       case BCANCEL:
         return(1);
    }
		if (Refresh) {
			Refresh = 0;
      if (Loc >= 6) CurY = 150;
			else       		CurY = 80;  
			switch(Loc % 6) {
			  case 0: CurX = 140; break;
				case 1: CurX = 160; break;
				case 2: CurX = 200; break;
				case 3: CurX = 220; break;
				case 4: CurX = 260; break;
				case 5: CurX = 280; break;
			}
		  GUI_SetColor(GUI_ORANGE);
      GUI_DispCharAt(Stream[Loc] + '0', CurX, CurY);
    }
  }
  return(0);
}


void ProcessKeysAlef(unsigned int Key) {
  char  str[50];

  switch(Key) {
    case BOK:
			Setup();
			ShowPageAlef();
      break;
    case BA1:
      break;
    case BA2:
      break;
    case BA3:
      break;
		case BA4:
    //frmTransactionsList();
    		break;
    case BA5:
      break;
    case BA6:
      break;
    case BA7:
      break;
    case BA8:
      break;
    case BA9: 
      break;
    case BA10: 
      break;

    case BDOWN:
      break;
    case BUP: 
      break;	
    case BLEFT:
      break;
    case BRIGHT: 
      break;
  }  
}


unsigned int DumpMem(void) {
	unsigned long int Counter=0;
	unsigned long int SendUsers;
	unsigned char MEMBuffer[LEN_TRANSACTIONS+5];
	unsigned int Li;
	
	if(trHead==0)  
		SendUsers=MAX_TRANSACTIONS-1;
	else    
		SendUsers=trHead-1;
  Counter=0;
	SetTX485();
  while(1)  
  {
		if(SendUsers==trTail) break;

    LoadFromDFToRam(addTransactions+(SendUsers*LEN_TRANSACTIONS), LEN_TRANSACTIONS, MEMBuffer);
	  if(SendUsers==0)  SendUsers=MAX_TRANSACTIONS-1;
	  else    SendUsers=SendUsers-1;		

		WDTR;
		printf("\n\rid: %d",SendUsers);
		Li  = MEMBuffer[30]; Li<<=8;
		Li += MEMBuffer[2]; Li<<=8;
		Li += MEMBuffer[1]; Li<<=8;
		Li += MEMBuffer[0]; 
		printf(" ID: %ld", Li);
		printf(" %d/%d/%d %d:%d:%d", MEMBuffer[4]+1300,MEMBuffer[5],MEMBuffer[6],MEMBuffer[7],MEMBuffer[8],MEMBuffer[9]);
		printf(" Pay: %d",MEMBuffer[12]+MEMBuffer[13]*256);
		printf(" tid: %d", MEMBuffer[28]+MEMBuffer[29]*256);
		
			
  }//while     
	   
  return(Counter);
}


void DumpArea() {
	int i, j, idx=0;
  
	LoadFromDFToRam(addConfig2, 4096, SPI_FLASH_BUF);
  for(i=0; i<256; i++)
	{
	  printf("\n\r");
		for(j=0; j<16; j++)
      printf("%02X ",SPI_FLASH_BUF[idx+j]);
		printf(" > ");
		for(j=0; j<16; j++)
      printf("%c",SPI_FLASH_BUF[idx+j]);
		idx+=16;
	}
}

void MainAlef(void) {
	unsigned char lastsnr[5];
	unsigned char TestCounter,Byte, snr[20];
	unsigned int i,MM=0;
	unsigned long int v;
	unsigned int Key;
	unsigned int Dist;
	
	//DumpArea();
	//while(1) WDTR;
	
	Config.WiFi = 1;
	
	#if(DeviceType == BUSDOOR)
	if ((Config.DeviceID != 1) && (Config.DeviceID != 2)) Config.DeviceID = 1;
	#endif

	setCallback(WifiCallback);

	if ((Config.LocalIP[0] < '0') || (Config.LocalIP[0] > '9'))
		sprintf(Config.LocalIP, "192.168.001.178");

	if ((Config.SubnetIP[0] < '0') || (Config.SubnetIP[0] > '9'))
		sprintf(Config.SubnetIP, "255.255.255.000");

	if ((Config.RoutIpAddress[0] < '0') || (Config.RoutIpAddress[0] > '9')) {
		Config.RoutIpAddress[0] = 192;
		Config.RoutIpAddress[1] = 168;
		Config.RoutIpAddress[2] = 110;
		Config.RoutIpAddress[3] = 1;
		sprintf(Config.RoutIpAddress, "192.168.001.001");			
	}

	if ((Config.ServerIP[0] < '0') || (Config.ServerIP[0] > '9'))
		sprintf(Config.ServerIP, "192.168.001.123");

	if ((Indicators.TransactionUID & 0x000000FFFF) >= 65536) 
		Indicators.TransactionUID = 0;

	RTC_Get();

  strcpy(Config.MainPassword, "111111");  
	InitializeAlef();
	ShowPageAlef();		
 	LEDERR = 0;
	Config.PriceType = 0;

	//DumpMem();/////////////////////////

	while(1) {		
    #ifdef WithSAMCARD
	  CardEtebarKashan=0;

		#ifdef CLRC_CHIP
    Byte=ISO14443_SingleTagSelect2(snr);
		#else
    if(!ISO14443_SingleTagSelect(snr))
			Byte=2;
		else
			Byte=0;
		#endif	
    if(Byte) {
      if((lastsnr[0]!=snr[0])||(lastsnr[1]!=snr[1])||(lastsnr[2]!=snr[2])||(lastsnr[3]!=snr[3]))
        if(ProcessCard(Byte,snr)) {
					lastsnr[0] = snr[0]; 
					lastsnr[1] = snr[1];
					lastsnr[2] = snr[2];
					lastsnr[3] = snr[3];
					lastsnr[4] = 2;
				}
    }
    #else
    if (!ISO14443_SingleTagSelect(snr)) {
      if ((lastsnr[0] != snr[0]) || (lastsnr[1] != snr[1]) || (lastsnr[2] != snr[2]) || (lastsnr[3] != snr[3]))
        if (ProcessCard(2, snr)) {
					lastsnr[0] = snr[0];
					lastsnr[1] = snr[1];
					lastsnr[2] = snr[2];
					lastsnr[3] = snr[3];
					lastsnr[4] = 2;
				}
    } 
    #endif		
		 
		if (Tick >= 1000) {
			if (lastsnr[4])
				if (--lastsnr[4] == 0) {
					lastsnr[0] = 0;
					lastsnr[1] = 0;
					lastsnr[2] = 0;
					lastsnr[3] = 0;
				}
      SetRX485();
	    ProcessPeriodicTasks();
			SetRX485();
			Tick = 0;
		}

		Key = ScanKeyboard();
    if (Key)
			ProcessKeysAlef(Key);
		
		if (rxd3_counter)
			if (Config.WiFi != 103)
				CommunicateWithHostByWiFi();
		
    if (rxd_counter) {
      CommunicateWithHost();
      EmptyRXBuffer();
    }
  }
}
#endif

