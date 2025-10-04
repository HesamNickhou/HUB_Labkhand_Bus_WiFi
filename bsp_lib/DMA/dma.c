#include "dma.h"
#include "sramlcd.h"
																	   	  
//////////////////////////////////////////////////////////////////////////////////	 
//本程序只供学习使用，未经作者许可，不得用于其它任何用途
//ALIENTEK战舰STM32开发板
//DMA 代码	   
//正点原子@ALIENTEK
//技术论坛:www.openedv.com
//修改日期:2012/9/8
//版本：V1.0
//版权所有，盗版必究。
//Copyright(C) 广州市星翼电子科技有限公司 2009-2019
//All rights reserved									  
//////////////////////////////////////////////////////////////////////////////////

DMA_InitTypeDef DMA_InitStructure;

u16 DMA1_MEM_LEN;//保存DMA每次数据传送的长度 		    
//DMA1的各通道配置
//这里的传输形式是固定的,这点要根据不同的情况来修改
//从存储器->外设模式/8位数据宽度/存储器增量模式
//DMA_CHx:DMA通道CHx
//cpar:外设地址
//cmar:存储器地址
//cndtr:数据传输量  
void MYDMA_Config(DMA_Channel_TypeDef*DMA_CHx,u32 cpar,u32 cmar,u16 cndtr)
{
 	RCC_AHBPeriphClockCmd(RCC_AHBPeriph_DMA1, ENABLE);	//使能DMA传输
	
    DMA_DeInit(DMA_CHx);   //将DMA的通道1寄存器重设为缺省值
	DMA1_MEM_LEN=cndtr;
	DMA_InitStructure.DMA_PeripheralBaseAddr = cpar;  //DMA外设ADC基地址
	DMA_InitStructure.DMA_MemoryBaseAddr = cmar;  //DMA内存基地址
	DMA_InitStructure.DMA_DIR = DMA_DIR_PeripheralDST;  //数据传输方向，从内存读取发送到外设
	DMA_InitStructure.DMA_BufferSize = cndtr;  //DMA通道的DMA缓存的大小
	DMA_InitStructure.DMA_PeripheralInc = DMA_PeripheralInc_Disable;  //外设地址寄存器不变
	DMA_InitStructure.DMA_MemoryInc = DMA_MemoryInc_Enable;  //内存地址寄存器递增
	DMA_InitStructure.DMA_PeripheralDataSize = DMA_PeripheralDataSize_Byte;  //数据宽度为8位
	DMA_InitStructure.DMA_MemoryDataSize = DMA_MemoryDataSize_Byte; //数据宽度为8位
	DMA_InitStructure.DMA_Mode = DMA_Mode_Normal;  //工作在正常缓存模式
	DMA_InitStructure.DMA_Priority = DMA_Priority_Medium; //DMA通道 x拥有中优先级 
	DMA_InitStructure.DMA_M2M = DMA_M2M_Disable;  //DMA通道x没有设置为内存到内存传输
	DMA_Init(DMA_CHx, &DMA_InitStructure);  //根据DMA_InitStruct中指定的参数初始化DMA的通道USART1_Tx_DMA_Channel所标识的寄存器	  	
} 
//开启一次DMA传输
void MYDMA_Enable(DMA_Channel_TypeDef*DMA_CHx)
{

	DMA_Cmd(DMA_CHx, DISABLE );  //关闭USART1 TX DMA1 所指示的通道      
 	DMA_SetCurrDataCounter(DMA1_Channel4,DMA1_MEM_LEN);//DMA通道的DMA缓存的大小
 	DMA_Cmd(DMA_CHx, ENABLE);  //使能USART1 TX DMA1 所指示的通道 
}	  
////////////////////////////////////////////////////////////////////////////////////
//LCD DMA初始化部分
u16 LCD_CUR_LINE;
u32 DMA2_MEM_LEN=320*240/2;//保存DMA每次数据传送的长度 半屏幕   	  
void LCD_DMA_Coinfig(void)
{												    

	RCC_AHBPeriphClockCmd(RCC_AHBPeriph_DMA2, ENABLE);	//使能DMA传输
	
    DMA_DeInit(DMA2_Channel5);   //将DMA的通道1寄存器重设为缺省值
	DMA_InitStructure.DMA_PeripheralBaseAddr = (u32)LCD_GRAM_BUF;  //DMA外设ADC基地址
	DMA_InitStructure.DMA_MemoryBaseAddr = (u32)0X6C000800;  //DMA内存基地址
	DMA_InitStructure.DMA_DIR = DMA_DIR_PeripheralSRC;  //数据传输方向，从内存读取发送到外设
	DMA_InitStructure.DMA_BufferSize = DMA2_MEM_LEN;  //DMA通道的DMA缓存的大小
	DMA_InitStructure.DMA_PeripheralInc = DMA_PeripheralInc_Enable;  //外设地址寄存器不变
	DMA_InitStructure.DMA_MemoryInc = DMA_MemoryInc_Disable;  //内存地址寄存器递增
	DMA_InitStructure.DMA_PeripheralDataSize = DMA_PeripheralDataSize_HalfWord;  //数据宽度为8位
	DMA_InitStructure.DMA_MemoryDataSize = DMA_MemoryDataSize_HalfWord; //数据宽度为8位
	DMA_InitStructure.DMA_Mode = DMA_Mode_Normal;  //工作在正常缓存模式
	DMA_InitStructure.DMA_Priority = DMA_Priority_Medium; //DMA通道 x拥有中优先级 
	DMA_InitStructure.DMA_M2M = DMA_M2M_Enable;  //DMA通道x没有设置为内存到内存传输
	DMA_Init(DMA2_Channel5, &DMA_InitStructure);  //根据DMA_InitStruct中指定的参数初始化DMA的通道USART1_Tx_DMA_Channel所标识的寄存器	  
									 
}

//开启一次LCD DMA传输
//x:其实传输地址编号(0~240)
void LCD_DMA_Enable(u16 x)
{	
	//因为DMA每次传输数据有限,故分两次传输.
	//第一次传输												    	  
 	DMA_Cmd(DMA2_Channel5,DISABLE);	//关闭DMA传输
 	DMA_ClearFlag(DMA2_FLAG_TC5);	//清除上次的传输完成标记 
	LCD_CUR_LINE=x;
	DMA2_Channel5->CNDTR=DMA2_MEM_LEN; 
	DMA2_Channel5->CPAR=(u32)LCD_GRAM_BUF[LCD_CUR_LINE]; //DMA2,改变存储器地址
	DMA_Cmd(DMA2_Channel5, ENABLE);//开启DMA传输
    while(DMA_GetFlagStatus(DMA2_FLAG_TC5)==RESET);	//等待
	//第二次传输
	DMA_ClearFlag(DMA2_FLAG_TC5);	//清除上次的传输完成标记
	DMA2_Channel5->CPAR=(u32)LCD_GRAM_BUF[LCD_CUR_LINE+120]; //DMA1,改变存储器地址

	DMA_Cmd(DMA2_Channel5,DISABLE);	//关闭DMA传输
	DMA2_Channel5->CNDTR=DMA2_MEM_LEN; 
	DMA_Cmd(DMA2_Channel5, ENABLE);//开启DMA传输
	while(DMA_GetFlagStatus(DMA2_FLAG_TC5)==RESET);	//等待


}
 

























