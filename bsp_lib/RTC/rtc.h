#ifndef __RTC_H
#define __RTC_H


#include <includes.h>

typedef struct struct_rDate{
	unsigned char day;
	unsigned char month;
	unsigned int year;
} rDateType;  

extern rDateType CurrentDate;
extern unsigned short  Year;
extern unsigned short MYear;
extern unsigned char MMonth;
extern unsigned char MDay;
extern unsigned char Month;
extern unsigned char Day;
extern unsigned char Hour;
extern unsigned char Min;
extern unsigned char Sec;
extern unsigned char DayOfWeek;

extern void gregorian_to_jalali(int grgYear, char grgMonth, char grgDay, rDateType *gDate);
extern void jalali_to_gregorian(int hshYear, char hshMonth, char hshDay, rDateType *gDate);

extern unsigned char GetDayOfWeek(unsigned int Year,unsigned int Month,unsigned int Day);
extern unsigned short GetDayOfYear(void); 

void Disp_Time(u8 x,u8 y,u8 size);		
void Disp_Week(u8 x,u8 y,u8 size,u8 lang);	
u8 RTC_Init(void); 
u8 Is_Leap_Year(u16 year);
u8 RTC_Get(void);         
u8 RTC_Get_Week(u16 year,u8 month,u8 day);
u8 RTC_Set(u16 syear,u8 smon,u8 sday,u8 hour,u8 min,u8 sec);

#endif
