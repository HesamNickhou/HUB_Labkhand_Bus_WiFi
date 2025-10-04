#include "key.h"
#include "sys.h" 
#include "delay.h"
//////////////////////////////////////////////////////////////////////////////////	 
//ECA 2014
//All rights reserved									  
////////////////////////////////////////////////////////////////////////////////// 
								    
// Initialization function keys
void KEY_Init(void) //IO initialization
{ 
 	GPIO_InitTypeDef GPIO_InitStructure;
	// Initialize KEYS
 	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA|RCC_APB2Periph_GPIOE,ENABLE);//enable PORTA, PORTE clock

	GPIO_InitStructure.GPIO_Pin  = GPIO_Pin_3|GPIO_Pin_4|GPIO_Pin_6;//PE3~5
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IPU; //set pullup input
 	GPIO_Init(GPIOE, &GPIO_InitStructure);//initialize GPIOE 3,4,5,6
	
	// Initialize  GPIOE.6 drop down input
	GPIO_InitStructure.GPIO_Pin  = GPIO_Pin_5;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IPD; //PA0 is set to enter the default drop-down 
	GPIO_Init(GPIOE, &GPIO_InitStructure);//initialize GPIOE.6

	// Initialize WK_UP -> GPIOA.0 drop down input
	GPIO_InitStructure.GPIO_Pin  = GPIO_Pin_0;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IPD; //PA0 is set to enter the default drop-down 
	GPIO_Init(GPIOA, &GPIO_InitStructure);//initialize GPIOA.0
	
	

}
u8 KEY_Scan(u8 mode)
{	 
	static u8 key_up=1;//button press release flag
	if(mode)key_up=1;  //support double-click		  
	if(key_up&&(KEY1==1||KEY2==0||KEY3==0||KEY4==0||KEY5==1))
	{
		delay_ms(10);//debounce
		key_up=0;
		if(KEY1==1)return 1;
		else if(KEY2==0)return 2;
		else if(KEY3==0)return 3;
		else if(KEY4==0)return 4;
		else if(KEY5==1)return 5;
	}else if(KEY1==0&&KEY2==1&&KEY3==1&&KEY4==1&&KEY5==0)key_up=1; 	    
 	return 0;// no button is pressed
}
