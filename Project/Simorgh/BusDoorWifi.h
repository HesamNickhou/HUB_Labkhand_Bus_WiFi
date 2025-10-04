
#ifndef __BUSDOORWIFI_H
#define __BUSDOORWIFI_H	 

#include "GUI.h"

#define modTaxiMeter    0
#define modFixedLine    1
#define modTaxiService  2
#define modOutOfService 3

#define EXOP_INUSE      19
#define EXOP_WDTR       40
#define EXOP_STATUSA    42
#define EXOP_STATUSV    43
#define EXOP_STATUSU    44
#define EXOP_BADHDOP    45
#define EXOP_GOODHDOP   46
#define EXOP_INMENU     47
#define EXOP_OUTMENU    48
#define EXOP_INREP      49
#define EXOP_OUTREP     50
#define EXOP_CLEARPASSN 51

#define TRANSACTIONS_START_MEM   10     //1000000 Transactions
#define LOCATIONS_START_MEM      62510  //1000000 Locations
#define PERSONS_START_MEM        93760

#define mrYes          1
#define mrNo           2

typedef struct {
	unsigned int OperatorID;
  unsigned char Day;
  unsigned long int Cash;
  unsigned long int DirectCash;
  unsigned long int CardCash;
  unsigned short int NumOfPassengers;
  float Distance;
  unsigned long int MotionTime;
  unsigned long int StopTime;
  unsigned int DeltaSpeed;
  unsigned short DeltaTimes;
  unsigned short FuelDistance;
	signed int Charge;
} TDeviceInfo;

struct CardInfo_Struct {
	unsigned char Empty;
	unsigned int  DID;
	unsigned char Hour;
	unsigned char Minutes;
	unsigned char Year;
	unsigned char Month;
	unsigned char Day;
	unsigned char OP;
	unsigned int  Price;
	unsigned int Etebar;  
	unsigned int RemEtebar;
};

#define BinPageSize 512

#define MAX_COURSES  5
typedef struct{
  unsigned int LineCode;
  unsigned char Order;
  unsigned int Price;
  float lat1;
  float lon1;
  float lat2;
  float lon2;
  float K1;
  float K2;
  float K3;
  float OldDistance;
  float Distance;
  unsigned char Title[50];
} Course;

extern unsigned short Tick;
extern TDeviceInfo DeviceInfo;

extern unsigned char Theme;
extern unsigned char MapView;
extern int InfoCounter;     

extern unsigned short COG;
extern unsigned char DisplayEnhancedInformation;
extern unsigned char TouchPage;
extern unsigned char DriverName[];
extern signed short int CurrentCourse, OldCourse;
extern int CurMapX0, CurMapX1;
extern int CurMapY0, CurMapY1;
extern GUI_RECT MapArea;
extern unsigned char NightMode;
extern unsigned char cntDriverInfo; 
extern unsigned int  ManualStop;
extern float CLat, CLon;
extern unsigned char MapAvailable;
extern unsigned char CurrentEnhancedItem;
extern signed long int Charge;
extern unsigned int DeltaSpeed;
extern unsigned short DeltaTimes;

void MainSimorgh(void);// *p_arg); 

extern const unsigned char* strDays[7];
extern const unsigned char* strMonths[13];

void AddStopTime(unsigned char idx, unsigned short st);
void ShowPageTaxiService(void);
void ShowEnhancedInformation(unsigned char FullOrText);
void DisplayFooter(void);
void DisplayToolbar(unsigned char forceview);
void DisplayDriverInfo(void);
unsigned char SaveTaxiInfo(void);
void ProcessKeysSimorgh(unsigned int Key);
unsigned char CheckCard(unsigned int UC, unsigned long int CardID);
void ProcessPassenger(unsigned int CardID, unsigned char ByCard);
void ShowPageAlef(void);
void SaveExtraOP(unsigned char op);
unsigned char SendCommandResult(unsigned char Type, unsigned int Status);
void RefreshPassengers(unsigned char PassengerID);
extern unsigned char MEDSdcard_DeInitialize(void);
unsigned char SendALive();
void ChargeCard(unsigned int CardID, unsigned int Amount, unsigned int FKCharge);

#endif
