#include "led.h"
//////////////////////////////////////////////////////////////////////////////////	 
//ECA 2014
//All rights reserved									  
////////////////////////////////////////////////////////////////////////////////// 
void LED_Init(void)
{
 
 GPIO_InitTypeDef  GPIO_InitStructure;
 	
 RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB|RCC_APB2Periph_GPIOC|RCC_APB2Periph_GPIOF|RCC_APB2Periph_GPIOG, ENABLE);	
	


 GPIO_InitStructure.GPIO_Pin = GPIO_Pin_6|GPIO_Pin_11;	 
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP; 	
 GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;	
 GPIO_Init(GPIOF, &GPIO_InitStructure);	  		
 //GPIO_SetBits(GPIOF,GPIO_Pin_6|GPIO_Pin_11); 						 

	
 GPIO_InitStructure.GPIO_Pin = GPIO_Pin_7;	 
 GPIO_Init(GPIOG, &GPIO_InitStructure);	  		
 //GPIO_SetBits(GPIOG,GPIO_Pin_7); 						 
	
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_0;	 
 GPIO_Init(GPIOB, &GPIO_InitStructure);	  		
 //GPIO_SetBits(GPIOB,GPIO_Pin_0); 			
}
 
