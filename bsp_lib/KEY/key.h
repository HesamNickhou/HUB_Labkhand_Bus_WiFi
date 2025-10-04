#ifndef __KEY_H
#define __KEY_H	 
#include "sys.h"
//////////////////////////////////////////////////////////////////////////////////	 
//ECA 2014
//All rights reserved									  
////////////////////////////////////////////////////////////////////////////////// 

#define KEY5  GPIO_ReadInputDataBit(GPIOE,GPIO_Pin_5)
#define KEY4  GPIO_ReadInputDataBit(GPIOE,GPIO_Pin_4)
#define KEY3  GPIO_ReadInputDataBit(GPIOE,GPIO_Pin_3)
#define KEY2  GPIO_ReadInputDataBit(GPIOE,GPIO_Pin_6)
#define KEY1  GPIO_ReadInputDataBit(GPIOA,GPIO_Pin_0)//(WK_UP) 

#define KEY_5 5
#define KEY_4 4
#define KEY_3	3
#define KEY_2	2
#define KEY_1	1

void KEY_Init(void);//IO initialization
u8 KEY_Scan(u8);  	//key scan function				    
#endif
