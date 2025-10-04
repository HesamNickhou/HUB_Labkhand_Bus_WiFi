#include "sramlcd.h"
#include "dma.h"
#include "lcd.h"
#include "delay.h"
#include "malloc.h"
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


//2帧的缓存,分配在malloc内存及管理表之后	 
u16 LCD_GRAM_BUF[480][320] __attribute__ ((at(0X68000000+MEM2_MAX_SIZE+MEM2_ALLOC_TABLE_SIZE*2)));//LCD缓存开在MALLOC之后.
m_sramlcd_dev SLCD=
{
	slcd_frame_show,
	slcd_draw_point,
	slcd_frame_clear,
	slcd_frame_move,
	0,
	0,
};	
//在指定位置画点.
//x,y:在当前帧(SLCD.frame)的坐标.
//color:颜色.
void slcd_draw_point(u16 x,u16 y,u16 color)
{							   
	LCD_GRAM_BUF[x+240*SLCD.frame][y]=color;
}

//读取指定位置点的颜色值
//x,y:在当前帧(SLCD.frame)的坐标
//返回值:颜色
u16 slcd_read_point(u16 x,u16 y)
{
	return LCD_GRAM_BUF[x+240*SLCD.frame][y];
}	  
//显示帧
//x:帧的x坐标.
//color:颜色.
void slcd_frame_show(u16 x)
{							   
	LCD_Scan_Dir(U2D_L2R);		//设置扫描方向
	LCD_SetCursor(0x00,0x0000);	//设置光标位置 
	LCD_WriteRAM_Prepare();     //开始写入GRAM	 	  
	LCD_DMA_Enable(x);			//启动一次DMA传输
	LCD_Scan_Dir(DFT_SCAN_DIR);	//恢复默认方向
}
//清空帧.
//清空当前帧(SLCD.frame)
//color:颜色.
void slcd_frame_clear(u16 color)
{		
	u16 i,j;
	for(i=0;i<240;i++)
	{
		for(j=0;j<320;j++)
		LCD_GRAM_BUF[i+240*SLCD.frame][j]=color;
	}											    
}
//移动屏幕
//dir:方向,0:左移;1,右移
//skips:每次跳跃列数
//pos:起始位置
void slcd_frame_move(u8 dir,u8 skips,u16 pos)
{
	u16 x ;				 
	if(dir)//屏幕右移
	{
		for(x=pos;x>0;)
		{
			if(x>skips)x-=skips;
			else x=0;
			slcd_frame_show(x);
 	 	}
		SLCD.frame=0;		
	}else //屏幕左移
	{
		for(x=pos;x<240;)
		{
			x+=skips;
			if(x>240)x=240;
			slcd_frame_show(x);
 	 	}
		SLCD.frame=1;		
	} 
}










