#include <includes.h>
#include "rtc.h"
#include "sys.h"
#include <math.h>

rDateType CurrentDate;
#define MinYear 2014

u16 daycnt=0;
unsigned short MYear=2015;
unsigned char MMonth=1;
unsigned char MDay=1;
unsigned short Year=1389;
unsigned char Month=4;
unsigned char Day=27;
unsigned char Hour=0;
unsigned char Min=0;
unsigned char Sec=0;
unsigned char DayOfWeek=1;

const int grgSumOfDays[2][13] = {
                              {0, 31, 59, 90, 120, 151, 181, 212, 243, 273, 304, 334, 365},
                              {0, 31, 60, 91, 121, 152, 182, 213, 244, 274, 305, 335, 366}
                            };
const int hshSumOfDays[2][13] = {
                              {0, 31, 62, 93, 124, 155, 186, 216, 246, 276, 306, 336, 365},
                              {0, 31, 62, 93, 124, 155, 186, 216, 246, 276, 306, 336, 366}
                            };
 
#define div(a,b) ((a) / (b))
#define FALSE	0
#define TRUE	-1
														
#define SECONDS_PER_MINUTE  60
#define SECONDS_PER_HOUR    3600
#define SECONDS_PER_DAY     86400
#define DAYS_PER_YEAR       365
#define DAYS_PER_LEAP_YEAR  366

/* Epoch reference: 1970-01-01 00:00:00 UTC */
#define EPOCH_YEAR 1970

//=========================================================================
unsigned char GetDayOfWeek(unsigned int Year,unsigned int Month,unsigned int Day) {
	unsigned int days, i, m;

  days=Day-1;
  
  for (i=1385; i<Year; i++)
    if ((i-1383)%4) days += 365;
    else              days += 366;
  
  m=Month;
  if (m>6) {
    days += ((m-7)*30);
    m=7;
  }    
  
  days += ((m-1)*31);
  
  return (days + 3%7); 
 
}

//=========================================================================
unsigned short GetDayOfYear(void) {
	unsigned short days;
 
  days=Day;
  
  if (Month<=6)
    days += (Month-1)*31;
  else if (Month==12)
    days += (6*31)+(5*30);
  else
    days += (6*31)+((Month-7)*30);

  return days; 
 
}

//=========================================================================
unsigned char grgIsLeap(int Year) {
  return ((Year % 4) == 0 && ((Year % 100) != 0 || (Year % 400) == 0));
}
 
//=========================================================================
unsigned char hshIsLeap(int Year) {
  Year = (Year - 474) % 128;
  Year = ((Year >= 30) ? 0 : 29) + Year;
  Year = Year - floor((double)Year/33) - 1;
  return ((Year % 4) == 0);
}

//=========================================================================
void gregorian_to_jalali(int gy, char gm, char gd, rDateType *gDate) {
  long jy, gy2, days, g_d_m[12] = {0, 31, 59, 90, 120, 151, 181, 212, 243, 273, 304, 334};
  gy2 = (gm > 2) ? (gy + 1) : gy;
  days = 355666 + (365 * gy) + ((int)((gy2 + 3) / 4)) - 
		((int)((gy2 + 99) / 100)) + ((int)((gy2 + 399) / 400)) + gd + 
		g_d_m[gm - 1];
		
  jy = -1595 + (33 * ((int)(days / 12053)));
  days %= 12053;
  jy += 4 * ((int)(days / 1461));
  days %= 1461;
  if (days > 365) {
    jy += (int)((days - 1) / 365);
    days = (days - 1) % 365;
  }
  gDate->year = jy;
  if (days < 186) {
    gDate->month = 1 + (int)(days / 31);
    gDate->day  = 1 + (days % 31);
  } else {
    gDate->month = 7 + (int)((days - 186) / 30);
    gDate->day   = 1 + ((days - 186) % 30);
  }
}


unsigned long long toTimeStamp(unsigned short year, 
												 unsigned char month, 
												 unsigned char day,
                         unsigned char hour, 
												 unsigned char minute, 
												 unsigned char second) {
    /* Days in each month (non-leap year) */
    static const uint16_t days_in_month[] = {
        0, 31, 59, 90, 120, 151, 181, 212, 243, 273, 304, 334
    };

    /* Days since Unix epoch (1970-01-01) up to start of given year */
    uint32_t y = year - 1970;
    uint32_t leap_days = (y + 1) / 4;   /* Leap years since 1970 */

    uint32_t days = y * 365 + leap_days;

    /* Add days for months elapsed this year */
    days += days_in_month[month - 1];

    /* Add leap day for current year if past February */
    if (month > 2 && ((year % 4 == 0 && year % 100 != 0) || (year % 400 == 0)))
        days += 1;

    /* Add days elapsed this month (zero-indexed) */
    days += day - 1;

    /* Convert to seconds and add time */
		unsigned long long result = (unsigned long) days * 86400UL;
		result += (unsigned long) hour   * 3600UL;
		result += (unsigned long) minute * 60UL;
		result += (unsigned long) second;
		
    return result;
}

/*unsigned long toTimeStamp(unsigned short year, unsigned char month,  unsigned char day, 
													unsigned char hour,  unsigned char minute, unsigned char second) {
	unsigned long timestamp = 0;
    
	// Basic validation 
	if (year < EPOCH_YEAR || month < 1 || month > 12 || 
		day < 1 || day > 31 || hour > 23 || minute > 59 || second > 59) {
		return -1;
	}
	
	// 1. Calculate days since epoch using algorithm 
	// Formula based on days since March 1 (to handle leap years more easily) 
	
	// Adjust month/year for March-based year 
	uint16_t m = month;
	uint16_t y = year;
	
	if (m <= 2) {
		m += 12;
		y -= 1;
	}
	
	// Convert years to days 
	// Days from 1970-03-01 to year-03-01 
	uint64_t days = (uint64_t)y * 365 + y/4 - y/100 + y/400;
	days -= (uint64_t)EPOCH_YEAR * 365 + EPOCH_YEAR/4 - EPOCH_YEAR/100 + EPOCH_YEAR/400;
	
	// Add days for months 
	days += (uint64_t)(m * 153 + 8) / 5;
	
	// Adjust for day of month 
	days += day - 1;
	
	// Subtract days from Jan 1 to Mar 1 (59 for non-leap, 60 for leap) 
	if (grgIsLeap(year) && month <= 2)
		days -= 60;  // Leap year adjustment 
	else
		days -= 59;
	
	// 2. Convert days to seconds and add time 
	timestamp  = days 						* SECONDS_PER_DAY;
	timestamp += (uint64_t)hour   * SECONDS_PER_HOUR;
	timestamp += (uint64_t)minute * SECONDS_PER_MINUTE;
	timestamp += second;
	
	return timestamp;
}*/
/*
			OLD function!
void gregorian_to_jalali(int grgYear, char grgMonth, char grgDay, rDateType *gDate) {
	unsigned char grgLeap = 0, hshLeap = 0;
	int hshYear, hshMonth, hshDay, i, XmasToNorooz;
	long int hshElapsed, grgElapsed;
        
  hshYear = grgYear - 621;
  grgLeap = grgIsLeap(grgYear);
  hshLeap = hshIsLeap(hshYear - 1);
  grgElapsed 	 = grgSumOfDays[(grgLeap ? 1 : 0)][grgMonth - 1] + grgDay;
  XmasToNorooz = (hshLeap && grgLeap) ? 80 : 79;
	if (grgElapsed <= XmasToNorooz) {
		hshElapsed = grgElapsed + 286;
		hshYear--;
		if (hshLeap && !grgLeap)
			hshElapsed++;
	}
	else {
		hshElapsed = grgElapsed - XmasToNorooz;
		hshLeap 	 = hshIsLeap(hshYear);
	}
 
	for (i=1; i <= 12 ; i++) {
		if (hshSumOfDays [(hshLeap ? 1:0)][i] >= hshElapsed) {
			hshMonth = i;
			hshDay 	 = hshElapsed - hshSumOfDays [(hshLeap ? 1 : 0)][i - 1];
			break;
		}
	}
 
	gDate->day   = hshDay;
	gDate->month = hshMonth;
	gDate->year  = hshYear;   
}*/
 
//=========================================================================
void jalali_to_gregorian(int hshYear, char hshMonth, char hshDay, rDateType *gDate) {
	int 					grgYear, 		grgMonth, 	grgDay, i;
	unsigned char hshLeap, 		grgLeap;
	long int 			hshElapsed, grgElapsed;

	grgYear 	 = hshYear+621;
	hshElapsed = hshSumOfDays [hshLeap ? 1 : 0][hshMonth - 1] + hshDay;
	hshLeap 	 = hshIsLeap(hshYear);
	grgLeap 	 = grgIsLeap(grgYear);
 
	if (hshMonth > 10 || (hshMonth == 10 && hshElapsed > 286 + (grgLeap ? 1 : 0))) {
		grgElapsed = hshElapsed - (286 + (grgLeap ? 1:0));
		grgLeap = grgIsLeap (++grgYear);
	}
	else {
		hshLeap = hshIsLeap (hshYear-1);
		grgElapsed = hshElapsed + 79 + (hshLeap ? 1:0) - (grgIsLeap(grgYear-1) ? 1 : 0);
	}
 
	for (i=1; i <= 12; i++) {
		if (grgSumOfDays [grgLeap ? 1 : 0][i] >= grgElapsed) {
			grgMonth = i;
			grgDay = grgElapsed - grgSumOfDays [grgLeap ? 1 : 0][i - 1];
			break;
		}
	}
 
	gDate->day   = grgDay;
	gDate->month = grgMonth;
	gDate->year  = grgYear;   
}


//=========================================================================
static void RTC_NVIC_Config(void) {	
  NVIC_InitTypeDef NVIC_InitStructure;
	NVIC_InitStructure.NVIC_IRQChannel = RTC_IRQn;		//RTC????
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0;	//?????1?,????3?
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;	//?????0?,????4?
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;		//???????
	NVIC_Init(&NVIC_InitStructure);		//??NVIC_InitStruct???????????NVIC???
}

//=========================================================================
unsigned char RTC_STEP=0;
u8 RTC_Wait4Init(void) {
	if (RTC_STEP == 1) {
		if (RCC_GetFlagStatus(RCC_FLAG_LSERDY) == RESET)	
			return 1;

		RCC_RTCCLKConfig(RCC_RTCCLKSource_LSE);
		RCC_RTCCLKCmd(ENABLE);
		RTC_WaitForLastTask();
		RTC_WaitForSynchro();	
		RTC_ITConfig(RTC_IT_SEC, DISABLE);
		RTC_WaitForLastTask();
		RTC_EnterConfigMode();
		RTC_SetPrescaler(32767);
		RTC_WaitForLastTask();
		RTC_Set(MinYear,5,7,10,0,55);
		RTC_ExitConfigMode(); 
		BKP_WriteBackupRegister(BKP_DR1, 0X5050);
		RTC_Get();
  }

  if (RTC_STEP == 2) {
		RTC->CRL &= (uint16_t)~RTC_FLAG_RSF;
		/* Loop until RSF flag is set */
		if ((RTC->CRL & RTC_FLAG_RSF) == (uint16_t)RESET)
			return 1;
		RTC_ITConfig(RTC_IT_SEC, DISABLE);
    if ((RTC->CRL & RTC_FLAG_RTOFF) == (uint16_t)RESET)
			return 1;
		RTC_Get();
	}

	return 0;

}

//=========================================================================
u8 RTC_Init(void)
{
u8 temp=0;
 
	if (BKP_ReadBackupRegister(BKP_DR1) != 0x5050)	
	{	 			
		RCC_APB1PeriphClockCmd(RCC_APB1Periph_PWR | RCC_APB1Periph_BKP, ENABLE);	
		PWR_BackupAccessCmd(ENABLE);
		BKP_DeInit();
		RCC_LSEConfig(RCC_LSE_ON);
		RTC_STEP=1;
		
    #if (DeviceType==APARK)
		return 0;
		#endif

		while (RCC_GetFlagStatus(RCC_FLAG_LSERDY) == RESET)	
		{
			temp++;
			GUI_Delay(10);
			if (temp>=250) break;
		}
		RCC_RTCCLKConfig(RCC_RTCCLKSource_LSE);
		RCC_RTCCLKCmd(ENABLE);
		RTC_WaitForLastTask();
		RTC_WaitForSynchro();	
		RTC_ITConfig(RTC_IT_SEC, DISABLE);
		RTC_WaitForLastTask();
		RTC_EnterConfigMode();
		RTC_SetPrescaler(32767);
		RTC_WaitForLastTask();
		RTC_Set(MinYear,5,7,10,0,55);
		RTC_ExitConfigMode(); 
		BKP_WriteBackupRegister(BKP_DR1, 0X5050);
	}
	else
	{
		RTC_STEP=2;
    #if (DeviceType==APARK)
		return 0;
		#endif
		RTC_WaitForSynchro();	
		RTC_ITConfig(RTC_IT_SEC, DISABLE);
		RTC_WaitForLastTask();	
	}
	//RTC_NVIC_Config();
	RTC_Get();
	return 0; 

}		 				    

//=========================================================================
void RTC_IRQHandler(void)
{		 
	if (RTC_GetITStatus(RTC_IT_SEC) != RESET)
		RTC_Get();
	if (RTC_GetITStatus(RTC_IT_ALR) != RESET)
		RTC_ClearITPendingBit(RTC_IT_ALR);		
	RTC_ClearITPendingBit(RTC_IT_SEC|RTC_IT_OW);	
	RTC_WaitForLastTask();	  	    						 	   	 
}

//=========================================================================
u8 Is_Leap_Year(u16 year)
{		
	if (year<2000)
		year += 2000;
	if (year%4==0) 
	{ 
		if (year%100==0) 
		{ 
			if (year%400==0)return 1;
			else return 0;   
		}else return 1;   
	}else return 0;	
}	 			   

//=========================================================================
u8 const table_week[12] ={0,3,3,6,1,4,6,2,5,0,3,5}; 
const u8 mon_table[12] ={31,28,31,30,31,30,31,31,30,31,30,31};
u8 RTC_Set(u16 syear,u8 smon,u8 sday,u8 hour,u8 min,u8 sec) {
	u16 t;
	u32 seccount=0;
	
	daycnt=0;
	if (syear<1970||syear>2099)return 1;	   
	for (t=MinYear;t<syear;t++)	
	{
		if (Is_Leap_Year(t))seccount += 31622400;
		else seccount += 31536000;			 
	}
	smon -= 1;
	if (smon>11)return -2;	
	for (t=0;t<smon;t++)	  
	{
		seccount += (u32)mon_table[t]*86400;
		if (Is_Leap_Year(syear)&&t==1)seccount += 86400;
	}
	if (sday>31)return -3;	
	seccount += (u32)(sday-1)*86400;								//
	if (hour>23)return -4;	
	seccount += (u32)hour*3600;									//
	if (min>59)return -5;	
	seccount += (u32)min*60;	 									//
	if (sec>59)return -6;	
	seccount += sec;													//

	RTC_WaitForLastTask();	
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_PWR | RCC_APB1Periph_BKP, ENABLE);	//??PWR?BKP????  
	PWR_BackupAccessCmd(ENABLE);
	RTC_SetCounter(seccount);	
	RTC_WaitForLastTask();	
  RTC_Get();
	return 0;	    
}

//=========================================================================
u8 RTC_Get(void)
{
u32 timecount=0; 
u32 temp=0;
u16 temp1=0;
  
	timecount=RTC->CNTH;
  timecount <<= 16;
	timecount|=RTC->CNTL;	

 	temp=timecount/86400;
	if (daycnt != temp)
	{	  
		daycnt=temp;
		temp1=MinYear; //1970;	
		while (temp>=365)
		{				 
			if (Is_Leap_Year(temp1))
			{
				if (temp>=366)temp -= 366;
				else {/*temp1++;*/break;}  
			}
			else temp -= 365;	 
			temp1++;  
		}   
		Year=temp1;
		temp1=0;
		while (temp>=28)
		{
			if (Is_Leap_Year(Year)&&temp1==1)
			{
				if (temp>=29)temp -= 29;
				else break; 
			}
			else 
			{
				if (temp>=mon_table[temp1])temp -= mon_table[temp1];
				else break;
			}
			temp1++;  
		}
		Month=temp1+1;
		Day=temp+1; 
  	DayOfWeek=RTC_Get_Week(Year,Month,Day);
		MYear=Year;
		MMonth=Month;
		MDay=Day;
	  //printf("\n\r%u>%u,%u,%u",timecount,Year,Month,Day);
    gregorian_to_jalali(Year,Month,Day,&CurrentDate);
    Year=CurrentDate.year;
    Month=CurrentDate.month;
    Day=CurrentDate.day;
	}
	temp=timecount%86400;
	Hour=temp/3600;     	
	Min=(temp%3600)/60; 	
	Sec=(temp%3600)%60; 	
	return 0;
}	 

//=========================================================================
u8 RTC_Get_Time(void) {
	unsigned int timecount = 0; 
	unsigned int temp = 0;

	timecount = RTC->CNTH;
	timecount <<= 16;
	timecount += RTC->CNTL;			 

	temp = timecount % 86400;
	Hour = temp / 3600;     	
	Min  = (temp % 3600) / 60; 	
	Sec  = (temp % 3600) % 60; 	
	return 0;
}	 

//=========================================================================
u8 RTC_Get_Week(u16 year,u8 month,u8 day)
{	
	u16 temp2;
	u8 yearH,yearL;
	
	yearH=year/100;	yearL=year%100; 
	if (yearH>19)yearL += 100;
	temp2=yearL+yearL/4;
	temp2=temp2%7; 
	temp2=temp2+day+table_week[month-1];
	if (yearL%4==0&&month<3)temp2--;
	return temp2%7;
}			  

