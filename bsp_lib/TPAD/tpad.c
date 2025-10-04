#include "tpad.h"
#include "delay.h"		    
#include "usart.h"
#include "key.h"


#define TPAD_ARR_MAX_VAL 	0XFFFF	//最大的ARR值
vu16 tpad_default_val=0;//空载的时候(没有手按下),计数器需要的时间

u8 TPAD_Scan(u8 mode)
{
	
	u8 t,res=0;
	t=KEY_Scan(0);		
 	if(t)		   
		if (t==KEY_5)res=1; 							   		     	    					   
	return res;
}	 

