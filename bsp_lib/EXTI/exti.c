#include "exti.h"
#include "led.h"
#include "key.h"
#include "delay.h"
#include "usart.h"
#include "ov7670.h"
//////////////////////////////////////////////////////////////////////////////////	 
//ECA 2014
//All rights reserved	
//www.eca.ir
//////////////////////////////////////////////////////////////////////////////////
void EXTIX_Init(void)
{
 
 	EXTI_InitTypeDef EXTI_InitStructure;
 	NVIC_InitTypeDef NVIC_InitStructure;

    KEY_Init();	

  	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA|RCC_APB2Periph_GPIOE,ENABLE);

  
  	GPIO_EXTILineConfig(GPIO_PortSourceGPIOE,GPIO_PinSource2);

  	EXTI_InitStructure.EXTI_Line=EXTI_Line2;
  	EXTI_InitStructure.EXTI_Mode = EXTI_Mode_Interrupt;	
  	EXTI_InitStructure.EXTI_Trigger = EXTI_Trigger_Falling;
  	EXTI_InitStructure.EXTI_LineCmd = ENABLE;
  	EXTI_Init(&EXTI_InitStructure);	 

  	GPIO_EXTILineConfig(GPIO_PortSourceGPIOE,GPIO_PinSource3);

  	EXTI_InitStructure.EXTI_Line=EXTI_Line3;
  	EXTI_InitStructure.EXTI_Mode = EXTI_Mode_Interrupt;	
  	EXTI_InitStructure.EXTI_Trigger = EXTI_Trigger_Falling;
  	EXTI_InitStructure.EXTI_LineCmd = ENABLE;
  	EXTI_Init(&EXTI_InitStructure);	  	

 
  	GPIO_EXTILineConfig(GPIO_PortSourceGPIOE,GPIO_PinSource4);

  	EXTI_InitStructure.EXTI_Line=EXTI_Line4;
  	EXTI_InitStructure.EXTI_Mode = EXTI_Mode_Interrupt;	
  	EXTI_InitStructure.EXTI_Trigger = EXTI_Trigger_Falling;
  	EXTI_InitStructure.EXTI_LineCmd = ENABLE;
  	EXTI_Init(&EXTI_InitStructure);	

    NVIC_InitStructure.NVIC_IRQChannel = EXTI2_IRQn;			
  	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0x02;
  	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0x02;		
  	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;				
  	NVIC_Init(&NVIC_InitStructure);


  	NVIC_InitStructure.NVIC_IRQChannel = EXTI3_IRQn;			
  	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0x02;
  	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0x01;		
  	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;				
  	NVIC_Init(&NVIC_InitStructure);  	

	NVIC_InitStructure.NVIC_IRQChannel = EXTI4_IRQn;			
  	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0x02;
  	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0x00;		
  	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;				
  	NVIC_Init(&NVIC_InitStructure);  	
 
}
extern u8 ov_sta;
void EXTI2_IRQHandler(void)
{
	delay_ms(10);
	if(KEY2==0)	 
	{
		LED1=!LED1;	 
	}		 
	EXTI_ClearITPendingBit(EXTI_Line2);  
}

void EXTI3_IRQHandler(void)
{
	delay_ms(10);
	if(KEY1==0)	 
	{				 
		LED2=!LED2;	
	}		 
	EXTI_ClearITPendingBit(EXTI_Line3);  
}

void EXTI4_IRQHandler(void)
{
	delay_ms(10);
	if(KEY1==0)	 
	{
		LED1=!LED1;	 
	}		 
	EXTI_ClearITPendingBit(EXTI_Line4);  
}
 
void EXTI0_IRQHandler(void)
{		 		
 
	if(EXTI_GetITStatus(EXTI_Line0)==SET)
	{     
		if(ov_sta<2)
		{
			if(ov_sta==0)
			{
				OV7670_WRST=0;	 
				OV7670_WRST=1;	
				OV7670_WREN=1;	
			}else 
			{
				OV7670_WREN=0;	
				OV7670_WRST=0;	
				OV7670_WRST=1;	
			}
			ov_sta++;
		}
	}
 
	EXTI_ClearITPendingBit(EXTI_Line0);  
} 
void EXTI9_5_IRQHandler(void)
{		 		
 
	if(EXTI_GetITStatus(EXTI_Line6)==SET)
	{     
		if(ov_sta<2)
		{
			if(ov_sta==0)
			{
				OV7670_WRST=0;	 
				OV7670_WRST=1;	
				OV7670_WREN=1;	
			}else 
			{
				OV7670_WREN=0;
				OV7670_WRST=0;
				OV7670_WRST=1;	
			}
			ov_sta++;
		}
	}
 
	EXTI_ClearITPendingBit(EXTI_Line6);  
} 

void EXTI8_Init(void)
{   
	EXTI_InitTypeDef EXTI_InitStructure; 
	NVIC_InitTypeDef NVIC_InitStructure;
	
	 GPIO_EXTILineConfig(GPIO_PortSourceGPIOA,GPIO_PinSource0);

 	EXTI_InitStructure.EXTI_Line=EXTI_Line0;
  	EXTI_InitStructure.EXTI_Mode = EXTI_Mode_Interrupt;	
  	EXTI_InitStructure.EXTI_Trigger = EXTI_Trigger_Rising;
  	EXTI_InitStructure.EXTI_LineCmd = ENABLE;
  	EXTI_Init(&EXTI_InitStructure);		

	
  	NVIC_InitStructure.NVIC_IRQChannel = EXTI0_IRQn;			
  	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0x02;	
  	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0x03;					
  	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;								
  	NVIC_Init(&NVIC_InitStructure); 

	
  	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOG,ENABLE);

 	GPIO_EXTILineConfig(GPIO_PortSourceGPIOG,GPIO_PinSource6);
 	   	 
	EXTI_InitStructure.EXTI_Line=EXTI_Line6;
  	EXTI_InitStructure.EXTI_Mode = EXTI_Mode_Interrupt;	
  	EXTI_InitStructure.EXTI_Trigger = EXTI_Trigger_Rising;
  	EXTI_InitStructure.EXTI_LineCmd = ENABLE;
  	EXTI_Init(&EXTI_InitStructure);		
	
	NVIC_InitStructure.NVIC_IRQChannel = EXTI9_5_IRQn;			
  	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0;	
  	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;			
  	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;				
  	NVIC_Init(&NVIC_InitStructure);  
}


