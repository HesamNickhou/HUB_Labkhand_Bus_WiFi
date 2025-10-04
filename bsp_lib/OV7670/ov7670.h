	#ifndef _OV7670_H
#define _OV7670_H
#include "sys.h"
#include "sccb.h"


#define OV7670_VSYNC  PGin(6)			
#define OV7670_WRST		PDout(6)		
#define OV7670_WREN		PBout(3)		
#define OV7670_RCK		PBout(4)		
#define OV7670_RRST		PGout(14)  		
#define OV7670_CS		PGout(15)  		
															  					 
#define OV7670_DATA   GPIO_ReadInputData(GPIOC,0x00FF) 					
//GPIOC->IDR&0x00FF 
/////////////////////////////////////////
#define CHANGE_REG_NUM 							171			
extern const u8 ov7670_init_reg_tbl[CHANGE_REG_NUM][2];		
	    				 
u8   OV7670_Init(void);		  	   		 
void OV7670_Light_Mode(u8 mode);
void OV7670_Color_Saturation(u8 sat);
void OV7670_Brightness(u8 bright);
void OV7670_Contrast(u8 contrast);
void OV7670_Special_Effects(u8 eft);
void OV7670_Window_Set(u16 sx,u16 sy,u16 width,u16 height);


#endif

