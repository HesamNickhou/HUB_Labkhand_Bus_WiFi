#include <includes.h>
#include <math.h>
#include <GUI.h>
#include "gps.h"
#include "Config.h"
#include "menu3d.h"
#include "usart/usart.h"
#include "rtc/rtc.h"


TGPSData GPSData;

unsigned char GPS_CheckSystemTime2(unsigned char GPSTimeChecked);

unsigned char GPS_CheckSystemTime(void);

unsigned int GPSValidData=0;
unsigned int HDOP=0;
unsigned int SattelitesInView=0;
unsigned char PositionFix=0;
unsigned char GPSDataWrite = 0;
unsigned long int GPSDataWaitTimeout=0;
unsigned char GPSTimeAcquired=0;
unsigned int GPSSpeed=0;

unsigned int rxd2_counter = 0;
unsigned int rxd2_wr_index = 0;
unsigned int rxd2_rd_index = 0;

#define BUFFER2_SIZE         250
unsigned char RXD2Buffer[BUFFER2_SIZE];

const char *GPRMC="GPRMC";
const unsigned char GPRMCLen=5;
unsigned char GPRMCLoc=0;
const char *GPGGA="GPGGA";
const unsigned char GPGGALen=5;
unsigned char GPGGALoc=0;
unsigned char GGAReady=0;
unsigned char GPRMCReady=0;

unsigned char GGAMessage[150];
unsigned char GPRMCMessage[150];

//=============================================================================
extern unsigned char GPSReinit;
void USART2_IRQHandler(void) 
{
volatile unsigned int IIR;
u8 Byte;
	
  IIR = USART2->SR;
  if (IIR & USART_FLAG_ORE) 
	{        
    USART2->SR &= ~USART_FLAG_ORE;	          // clear interrupt
  }

  if (IIR & USART_FLAG_TXE) 
	{        
    USART2->SR &= ~USART_FLAG_TXE;	          // clear interrupt
  }

  if (IIR & USART_FLAG_RXNE) 
	{        
    USART2->SR &= ~USART_FLAG_RXNE;	          // clear interrupt
		Byte=(USART2->DR & 0x1FF);
		RXD2Buffer[rxd2_wr_index]=Byte;
    //USART_SendData(USART1, Byte);/////////
		//printf("[%d-%d]%X,", rxd2_wr_index,rxd2_counter,Byte);
		
		if(GPSReinit==0)
		{
      if(Byte=='$')
      {
  			if(GPGGALoc==GPGGALen)
				{
					for(Byte=0; (Byte<rxd2_counter) && (Byte<150); Byte++)
				  {
    				GGAMessage[Byte]=RXD2Buffer[rxd2_rd_index];
            if(++rxd2_rd_index >= BUFFER2_SIZE) rxd2_rd_index=0;
					}	
					GGAMessage[Byte]=0;
          ProcessGPGGAMessage();
					GGAReady=1;
				}
        if(GPRMCLoc==GPRMCLen)
				{
					for(Byte=0; (Byte<rxd2_counter) && (Byte<150); Byte++)
				  {
    				GPRMCMessage[Byte]=RXD2Buffer[rxd2_rd_index];
            if(++rxd2_rd_index >= BUFFER2_SIZE) rxd2_rd_index=0;
					}
					GPRMCMessage[Byte]=0;
          ProcessGPRMCMessage();
					GPRMCReady=1;
				}					
      
        GPRMCLoc=0;
        GPGGALoc=0;
        rxd2_rd_index=rxd2_counter=rxd2_wr_index=0;
				return;
      }
      else
      {
        
        if(GPRMCLoc<GPRMCLen)
          GPRMCLoc=(Byte==GPRMC[GPRMCLoc])?GPRMCLoc+1:0;
        if(GPGGALoc<GPGGALen)
          GPGGALoc=(Byte==GPGGA[GPGGALoc])?GPGGALoc+1:0;
      }
		}

		if(++rxd2_wr_index == BUFFER2_SIZE) rxd2_wr_index=0;
    if (++rxd2_counter == BUFFER2_SIZE)
    {
      rxd2_rd_index=rxd2_counter=rxd2_wr_index=0;
    }
  } 
	
} 


//------------------------------------------------------------------------------
void GPSReset(void)
{
}

//------------------------------------------------------------------------------
int GPSInitialize(void)
{
  GPSReset();
  
  //delay_ms(500);

  USART_SendData(USART2, 0xA0);
  USART_SendData(USART2, 0xA1);
  USART_SendData(USART2, 0x00);
  USART_SendData(USART2, 0x09);
  USART_SendData(USART2, 0x08);
  USART_SendData(USART2, 0x01);
  USART_SendData(USART2, 0x00);
  USART_SendData(USART2, 0x00);
  USART_SendData(USART2, 0x00);
  USART_SendData(USART2, 0x01);
  USART_SendData(USART2, 0x00);
  USART_SendData(USART2, 0x00);
  USART_SendData(USART2, 0x01);
  USART_SendData(USART2, 0x09);
  USART_SendData(USART2, 0x0D);
  USART_SendData(USART2, 0x0A);  
  
  return(0);
}

//------------------------------------------------------------------------------
unsigned char Hex2Byte(unsigned char h1, unsigned char h2)
{
  if(h1>'A') h1=h1-'A'+10;
  else       h1=h1-'0';
  if(h2>'A') h2=h2-'A'+10;
  else       h2=h2-'0';
  return((h1<<4)+h2);
}

//------------------------------------------------------------------------------
void ProcessGPRMCMessage(void)
{
unsigned char Buffer[155];	
int i, Item=0, Loc=0;
unsigned char CheckSum='G'^'P'^'R'^'M'^'C'^',';
unsigned char PacketCheckSum=0;

	if(GPSValidData) return; //Last Data not processed yet
  GPSValidData=0;
  for(i=0; (GPRMCMessage[i]!=0) && (i<150); i++)
	  Buffer[i]=GPRMCMessage[i];
  Buffer[i]=0;
    
	//printf("\n\rData: ");
	for(i=6; (Buffer[i]!=0) && (i<150); i++)
  {
    //printf("%c", Buffer[i]);
    if(Buffer[i]=='*')
    {
      PacketCheckSum=Hex2Byte(Buffer[i+1],Buffer[i+2]);
      if((Buffer[i+3]!=13)||(Buffer[i+4]!=10))
        return;
      if(PacketCheckSum!=CheckSum)
        return;
    }
    CheckSum^=Buffer[i];
    if(Buffer[i]==',')
    {
      Item++;
      Loc=0;
      continue;
    }

    switch(Item)
    {
      case 0:
        GPSData.UTC[Loc++]=Buffer[i];
        GPSData.UTC[Loc]=0;
        break;
      case 1:
        GPSData.Status[Loc++]=Buffer[i];
        GPSData.Status[Loc]=0;
        break;
      case 2:
        GPSData.Lat[Loc++]=Buffer[i];
        GPSData.Lat[Loc]=0;
        break;
      case 3:
        GPSData.NS[Loc++]=Buffer[i];
        GPSData.NS[Loc]=0;
        break;
      case 4:
        GPSData.Lon[Loc++]=Buffer[i];
        GPSData.Lon[Loc]=0;
        break;
      case 5:
        GPSData.EW[Loc++]=Buffer[i];
        GPSData.EW[Loc]=0;
        break;
      case 6:
        GPSData.Speed[Loc++]=Buffer[i];
        GPSData.Speed[Loc]=0;
        break;
      case 7:
        GPSData.COG[Loc++]=Buffer[i];
        GPSData.COG[Loc]=0;
        break;
      case 8:
        GPSData.DATE[Loc++]=Buffer[i];
        GPSData.DATE[Loc]=0;
        break;        
    }//switch
		if(Loc>=19)
			Loc=0;
  }//for
  
	//printf("\n\rStatus: %s UTC: %s NS: %s LAT: %s EW: %s LON: %s Speed: %s COG: %s", GPSData.Status, GPSData.UTC, GPSData.NS, GPSData.Lat, GPSData.EW, GPSData.Lon, GPSData.Speed, GPSData.COG);
  //printf("\n\rUTC: %s Date: %s", GPSData.UTC, GPSData.DATE);
  GPSValidData=1;

}

//------------------------------------------------------------------------------
void ProcessGPTXTMessage(char  Status)
{
  GPSData.ANTSTATUS=Status; 
}

//------------------------------------------------------------------------------
unsigned char Is_LeapYaer(unsigned int year)
{
  unsigned char Jalali[8]={1,5,9,13,17,22,26,30};
  unsigned char Counter;
  
  for(Counter=0;Counter<8;Counter++)
  {
   if((year%33)==Jalali[Counter]) return(1);
  }
 
  return(0);
}

//------------------------------------------------------------------------------
unsigned char Is_LeapYaerMiladi(unsigned int year)
{
  
  if(!(year%4)&&(year%100))  return(1);
  if(!(year%400))             return(1);  
 
  return(0);
}

//------------------------------------------------------------------------------
unsigned char GPS_CheckSystemTime(void)
{
unsigned char Leap_YearFlag=0;
unsigned char Calc_Hour=0, Calc_Minute=0, Calc_Second=0, Gps_Month=0, Gps_Day=0; 
unsigned int  Gps_Year=0;
unsigned short y, m, d;	
  
  //if(SattelitesInView==0)
  //  return(2);
  
  Calc_Hour=(GPSData.UTC[0]-48)*10+(GPSData.UTC[1]-48);
  Calc_Minute=(GPSData.UTC[2]-48)*10+(GPSData.UTC[3]-48);
  Calc_Second=(GPSData.UTC[4]-48)*10+(GPSData.UTC[5]-48);
  Gps_Year=(GPSData.DATE[4]-48)*10+(GPSData.DATE[5]-48); 
  Gps_Year+=2000;
  Gps_Month=(GPSData.DATE[2]-48)*10+(GPSData.DATE[3]-48);
  Gps_Day=(GPSData.DATE[0]-48)*10+(GPSData.DATE[1]-48); 
  if((Gps_Year<2014)||(Gps_Year>2020))
    return(2);  
  y=Gps_Year;
	m=Gps_Month;
	d=Gps_Day;
  gregorian_to_jalali(Gps_Year,Gps_Month,Gps_Day,&CurrentDate);
  Gps_Year=CurrentDate.year;
  Gps_Month=CurrentDate.month;
  Gps_Day=CurrentDate.day;
  Leap_YearFlag=Is_LeapYaer(Gps_Year);

  if((Gps_Month>=1)&&(Gps_Month<=6))
    Calc_Hour+=4;
  else
    Calc_Hour+=3;
  
  if((Gps_Month==6)&&(Gps_Day==31)) Calc_Hour--;
  
  Calc_Minute+=30;
  if(Calc_Minute>=60)
  {
    Calc_Minute%=60;
    Calc_Hour++;
  }  
  if(Calc_Hour>=24)
  {
    Calc_Hour%=24;
    if((Gps_Day<31)&&(Gps_Month<=6)&&(Gps_Month>=1))           Gps_Day++;
    else if((Gps_Day<30)&&(Gps_Month<=11)&&(Gps_Month>=7))     Gps_Day++;     
    else if((Gps_Day<30)&&(Leap_YearFlag)&&(Gps_Month==12))    Gps_Day++;          
    else if((Gps_Day<29)&&(!Leap_YearFlag)&&(Gps_Month==12))   Gps_Day++;               
    else if((Gps_Day==31)&&(Gps_Month<=6)&&(Gps_Month>=1))     {Gps_Day=1;  Gps_Month++;}    
    else if((Gps_Day==30)&&(Gps_Month<=11)&&(Gps_Month>=7))    {Gps_Day=1;  Gps_Month++;}    
    else if((Gps_Day==30)&&(Leap_YearFlag)&&(Gps_Month==12))   {Gps_Day=1;  Gps_Month=1; Gps_Year++;}             
    else if((Gps_Day==29)&&(!Leap_YearFlag)&&(Gps_Month==12))  {Gps_Day=1;  Gps_Month=1; Gps_Year++;} 
  }

  Year=Gps_Year;
  Month=Gps_Month;
  Day=Gps_Day;
  Hour=Calc_Hour;
  Min=Calc_Minute;
  Sec=Calc_Second;
  DayOfWeek = GetDayOfWeek(Year,Month,Day);
	RTC_Set(y,m,d,Calc_Hour,Calc_Minute,Calc_Second);

  return(0);
}

//------------------------------------------------------------------------------
unsigned char GPS_CheckSystemTime2(unsigned char GPSTimeChecked)
{
unsigned char Gps_Hour, Gps_Minute, Gps_Second; 
unsigned short i;

  //if(SattelitesInView==0)
  //  return(2);
  i=2000;
  i+=(GPSData.DATE[4]-48)*10+(GPSData.DATE[5]-48); 
  if((i<2014)||(i>2020))
    return(2);
  
  Gps_Hour=(GPSData.UTC[0]-48)*10+(GPSData.UTC[1]-48);
  Gps_Minute=(GPSData.UTC[2]-48)*10+(GPSData.UTC[3]-48);
  Gps_Second=(GPSData.UTC[4]-48)*10+(GPSData.UTC[5]-48);
  Gps_Minute=Gps_Minute+30;
  if(Gps_Minute>=60)
  {
    Gps_Minute%=60;
    Gps_Hour++;
  }
  if(Month<=6)
    Gps_Hour+=4;
  else
    Gps_Hour+=3;
  if(Gps_Hour>23)
    Gps_Hour%=24;
  
  GPSTimeAcquired=10;
  if((Hour!=Gps_Hour)||(GPSTimeChecked==0))
  {
    GPS_CheckSystemTime();
  }else
  {
    Min=Gps_Minute;
    Sec=Gps_Second;
  }
  return(0);
}

//------------------------------------------------------------------------------
void ProcessGPGGAMessage(void)
{
unsigned char Buffer[155];
int i, Item=0, Loc=0;
char s[20];
char s1[20];
char s2[20];
unsigned char CheckSum='G'^'P'^'G'^'G'^'A'^',';
unsigned char PacketCheckSum=0;

  for(i=0; (GGAMessage[i]!=0) && (i<150); i++)
	  Buffer[i]=GGAMessage[i];
	Buffer[i]=0;

  s[0]=0;
  s1[0]=0;
  s2[0]=0;
  for(i=6; (Buffer[i]!=0) && (i<150); i++)
  {
    //printf("%c", Buffer[i]);
    if(Buffer[i]=='*')
    {
      PacketCheckSum=Hex2Byte(Buffer[i+1],Buffer[i+2]);
      if((Buffer[i+3]!=13)||(Buffer[i+4]!=10))
        return;
      if(PacketCheckSum!=CheckSum)
        return;
    }
    CheckSum^=Buffer[i];
    if(Buffer[i]==',')
    {
      Item++;
      Loc=0;
      continue;
    }
    switch(Item)
    {
      case 5:
        s2[Loc++]=Buffer[i];
        s2[Loc]=0;
        break;
      case 6:
        s[Loc++]=Buffer[i];
        s[Loc]=0;
        break;
      case 7:
        s1[Loc++]=Buffer[i];
        s1[Loc]=0;
        break;
    }//switch
		if(Loc>=19)
			Loc=0;
  }//for
  PositionFix=atoi(s2);
	i=SattelitesInView;
  SattelitesInView=atoi(s);
		
  HDOP=floor(atof(s1));
}

//==============================================================================
void DisplayLocation(unsigned char ForceShow)
{
static unsigned char LastGPSState='Z';
	
	#ifdef Simorgh50N
  if((LastGPSState!=GPSData.Status[0])||(ForceShow))
  {
    LastGPSState=GPSData.Status[0];
    switch(GPSData.Status[0])
    {
      case 'A':
        G_LoadBMP(305, 0, "0:desktop/gps2.bmp", 1);  
        break;
      case 'V':
        G_LoadBMP(305, 0, "0:desktop/gps1.bmp", 1);
        break;
      default:
        G_LoadBMP(305, 0, "0:desktop/gps0.bmp", 1);
    }
    GUI_SetColor(GUI_GREEN);
    if(SattelitesInView>9)
      GUI_DispDecAt(9, 330, TFT_START_Y+0, 1);
    else
      GUI_DispDecAt(SattelitesInView, 330, TFT_START_Y+0, 1);
  }
	#endif
  return;
}

//=========================================================================
float _fmod(float f, int i)
{
  return(f/i);
}

//=========================================================================
float GpsDist(float Lat1, float Long1, float Lat2, float Long2)
{
float min, dLat, dLong, a, c, d, pi;
    
    pi = 3.14159265;
    
    min  = fmod(Lat1,100);   
    Lat1 -= min;
    Lat1 /= 100;
    min = min / 60;
    Lat1 += min;
    Lat1 = Lat1 / 180 * pi;
                
    min  = fmod(Lat2,100);
    Lat2 -= min;
    Lat2 /= 100;
    min = min / 60;
    Lat2 += min;
    Lat2 = Lat2 / 180 * pi;
    
    min  = fmod(Long1,100);
    Long1 -= min;
    Long1 /= 100;
    min = min / 60;
    Long1 += min;
    Long1 = Long1 / 180 * pi;

    min  = fmod(Long2,100);
    Long2 -= min;
    Long2 /= 100;
    min = min / 60;
    Long2 += min;
    Long2 = Long2 / 180 * pi;
    
    if(Lat2>Lat1)
        dLat = Lat2 - Lat1;
    else
        dLat = Lat1 - Lat2;
    if(Long2>Long1)
        dLong = Long2 - Long1;
    else
        dLong = Long1 - Long2;
            
    a = sin(dLat/2) * sin(dLat/2) + cos(Lat1) * cos(Lat2) * sin(dLong/2) * sin(dLong/2);
    
    c = 2 * atan2(sqrt(a),sqrt(1-a));
    
    d = 6371 * c * 1000; 
    
    return(d);
}

