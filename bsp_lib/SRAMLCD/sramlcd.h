#ifndef __SRAMLCD_H
#define	__SRAMLCD_H	   
#include "sys.h"
//////////////////////////////////////////////////////////////////////////////////	 
//本程序只供学习使用，未经作者许可，不得用于其它任何用途
//ALIENTEK战舰STM32开发板
//SRAM-LCD 代码	   
//正点原子@ALIENTEK
//技术论坛:www.openedv.com
//修改日期:2012/9/2
//版本：V1.0
//版权所有，盗版必究。
//Copyright(C) 广州市星翼电子科技有限公司 2009-2019
//All rights reserved									  
////////////////////////////////////////////////////////////////////////////////// 	   


////////////////////////////////////////////////////////////////
 extern u16 LCD_GRAM_BUF[480][320];
////////////////////////////////////////////////////////////////

//SRAMLCD 控制器
typedef struct _m_sramlcd_dev
{															    
 	void (*show)(u16); 				//显示帧 
 	void (*draw_point)(u16,u16,u16);//画点		
	void (*clear)(u16);				//清帧 
	void (*move)(u8,u8,u16);		//移动   
	u8 pos;                        	//当前帧的位置(x坐标)
	u8 frame;				 		//当前帧(0/1)
}m_sramlcd_dev;
extern m_sramlcd_dev SLCD;  

u16  slcd_read_point(u16 x,u16 y);
void slcd_draw_point(u16 x,u16 y,u16 color);
void slcd_frame_show(u16 x);			 
void slcd_frame_clear(u16 color);
void slcd_frame_move(u8 dir,u8 skips,u16 pos);
#endif






























