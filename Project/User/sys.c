#include "sys.h" 
//////////////////////////////////////////////////////////////////////////////////	 
//本程序只供学习使用，未经作者许可，不得用于其它任何用途
//ALIENTEK STM32开发板
//系统时钟初始化		   
//正点原子@ALIENTEK
//技术论坛:www.openedv.com
//修改日期:2012/8/18
//版本：V1.7
//版权所有，盗版必究。
//Copyright(C) 广州市星翼电子科技有限公司 2009-2019
//All rights reserved
//********************************************************************************
//V1.4修改说明
//把NVIC KO了,没有使用任何库文件!
//加入了JTAG_Set函数
//V1.5 20120322
//增加void INTX_DISABLE(void)和void INTX_ENABLE(void)两个函数
//V1.6 20120412
//1,增加MSR_MSP函数												    
//2,修改VECT_TAB_RAM的默认偏移,设置为0X6000.
//V1.7 20120818
//1,添加ucos支持配置宏SYSTEM_SUPPORT_UCOS
//2,修改了注释
//3,去掉了不常用函数BKP_Write
////////////////////////////////////////////////////////////////////////////////// 	  
//THUMB指令不支持汇编内联
//采用如下方法实现执行汇编指令WFI  
__asm void WFI_SET(void)
{
	WFI;		  
}
//关闭所有中断
__asm void INTX_DISABLE(void)
{
	CPSID I;		  
}
//开启所有中断
__asm void INTX_ENABLE(void)
{
	CPSIE I;		  
}
//设置栈顶地址
//addr:栈顶地址
__asm void MSR_MSP(u32 addr) 
{
    MSR MSP, r0 			//set Main Stack value
    BX r14
}

 //系统软复位   
void Sys_Soft_Reset(void)
{  
 NVIC_SystemReset();	  
}    

 void NVIC_Configuration(void)
{

    NVIC_InitTypeDef NVIC_InitStructure;

  /* Configure the NVIC Preemption Priority Bits */
  NVIC_PriorityGroupConfig(NVIC_PriorityGroup_1);

  NVIC_InitStructure.NVIC_IRQChannel = SDIO_IRQn;
  NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0;
  NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;
  NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
  NVIC_Init(&NVIC_InitStructure);


}



























