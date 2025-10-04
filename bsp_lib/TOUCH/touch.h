#ifndef __TOUCH_H__
#define __TOUCH_H__
#include "sys.h"
#define TP_PRES_DOWN 0x80  
#define TP_CATH_PRES 0x40  
										    
typedef struct
{
	u8 (*init)(void);			
	u8 (*scan)(u8);				
	void (*adjust)(void);		
	u16 x0;						
	u16 y0;
	u16 x; 						
	u16 y;						   	    
	u8  sta;					
								
	float xfac;					
	float yfac;
	short xoff;
	short yoff;	   

//touchtype=0
//touchtype=1
	u8 touchtype;
}_m_tp_dev;

extern _m_tp_dev tp_dev;	

#define PEN  	PFin(10) 
#define DOUT 	PFin(8)  
#define TDIN 	PFout(9) 
#define TCLK 	PBout(1) 
#define TCS  	PCout(13) 
     

void TP_Write_Byte(u8 num);			
u16 TP_Read_AD(u8 CMD);				
u16 TP_Read_XOY(u8 xy);				
u8 TP_Read_XY(u16 *x,u16 *y);		
u8 TP_Read_XY2(u16 *x,u16 *y);		
void TP_Drow_Touch_Point(u16 x,u16 y,u16 color);
void TP_Draw_Big_Point(u16 x,u16 y,u16 color);	
u8 TP_Scan(u8 tp);								
void TP_Save_Adjdata(void);						
u8 TP_Get_Adjdata(void);						
void TP_Adjust(void);							
u8 TP_Init(void);								
																 
void TP_Adj_Info_Show(u16 x0,u16 y0,u16 x1,u16 y1,u16 x2,u16 y2,u16 x3,u16 y3,u16 fac);
 		  
#endif

