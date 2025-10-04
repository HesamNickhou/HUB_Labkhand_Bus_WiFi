
#include "config.h"

#include "futil.h"
#include <key/keypad.h>
#include <GUI.h>
#include <WM.h>

int KeyMap[10][8]=
{
  {'1',0,0,0,0,0,0,0},	          //1
  {1576,1662,1578,1579,'2',0,0,0},	  //2
  {1575,1570,1574,'3',0,0,0,0},  	  //3
  {1587,1588,1589,1590,'4',0,0,0},	  //4
  {1583,1584,1585,1586,1688,'5',0,0},	//5
  {1670,1580,1581,1582,'6',0,0,0},	  //6
  {1606,1608,1607,1740,'7',0,0,0},  	//7
  {1601,1602,1705,1711,1604,1605,'8',0},//8  
  {1591,1592,1593,1594,'9',0,0,0}, 	  //9
  {' ','0',0,0,0,0,0,0} 	  //0
};

unsigned char NumericInput=1;
unsigned char Released=0;
WM_HWIN DisplayIO;
extern unsigned short Tick;

void HandleForm(WM_HWIN hWin, char (*func)(unsigned char Key))
{
unsigned char Key;
unsigned char LastKey=0;
unsigned char cntKey=0;
unsigned long int Timeout;
  WM_HWIN hItem;
	
	Timeout=OS_TimeMS;
	while(1) 
	{  
		
		 if(Released==1)
		 {
			  WM_DeleteWindow(hWin);
			  WM_InvalidateWindow(WM_HBKWIN);
			  WM_Exec();
			  //GUI_Clear();
			  Released=0;
			  break;
		 }
		
		 Key=ScanKeyboard();
		 if((Key)&&(func!=0))
         if((*func)(Key)==0xAA)
           Key=0;					 
     switch(Key) 
     {
			 #if(DeviceType==Basket)
			 case BP1: GUI_StoreKeyMsg('4', 1); break;
			 case BP2: GUI_StoreKeyMsg('3', 1); break;
			 case BP3: GUI_StoreKeyMsg('2', 1); break;
			 case BP4: GUI_StoreKeyMsg('1', 1); break;
			 case BHELP: GUI_StoreKeyMsg('5', 1); break;
			 case BF3: GUI_StoreKeyMsg('6', 1); break;
			 case BMAP: GUI_StoreKeyMsg('7', 1); break;
			 case BPAYMENT2: GUI_StoreKeyMsg('8', 1); break;
			 case BMUTE: GUI_StoreKeyMsg('9', 1); break;
			 case BF2: GUI_StoreKeyMsg('0', 1); break;
			 #else
			 #ifdef Simorgh50N
			 case BA4: GUI_StoreKeyMsg('4', 1); break;
			 case BA3: GUI_StoreKeyMsg('3', 1); break;
			 case BA2: GUI_StoreKeyMsg('2', 1); break;
			 case BA1: GUI_StoreKeyMsg('1', 1); break;
			 case BA5: GUI_StoreKeyMsg('5', 1); break;
			 case BA6: GUI_StoreKeyMsg('6', 1); break;
			 case BA7: GUI_StoreKeyMsg('7', 1); break;
			 case BA8: GUI_StoreKeyMsg('8', 1); break;
			 case BA9: GUI_StoreKeyMsg('9', 1); break;
			 case BA10: GUI_StoreKeyMsg('0', 1); break;
			 #endif
			 #endif
			 /*
			 case 1: case 2: case 3:
			 case 4: case 5: case 6:
			 case 7: case 8: case 9:
			 case 10:
				 if(NumericInput)
				 {
					 if(Key==10)
  	  		   GUI_StoreKeyMsg('0', 1);
					 else
  	  		   GUI_StoreKeyMsg(Key+'0', 1);
				 }else{
  				 if((Tick<1000)&&(LastKey==Key))
	  			 {
		  			 GUI_StoreKeyMsg(GUI_KEY_BACKSPACE, 1);
			  		 if(KeyMap[Key-1][cntKey]==0)
				  		   cntKey=0;
  				 }else
	  			   cntKey=0;
				 
  	  		 GUI_StoreKeyMsg(KeyMap[Key-1][cntKey], 1);
  		  	 cntKey++;
			   }
				 break;
			*/
       case BDOWN : 
				 GUI_StoreKeyMsg(GUI_KEY_DOWN, 1);
         break;
       case BUP:
				 GUI_StoreKeyMsg(GUI_KEY_UP, 1);
         break;
       case BRIGHT: 
				 GUI_StoreKeyMsg(GUI_KEY_TAB, 1);
         break;
       case BOK: 
				 GUI_StoreKeyMsg(GUI_KEY_ENTER, 1);
         break;
       case BCANCEL: 
				 GUI_StoreKeyMsg(GUI_KEY_ESCAPE, 1);
         break;
       case BLEFT:
				 //GUI_StoreKeyMsg(GUI_KEY_LEFT, 1);
				 GUI_StoreKeyMsg(GUI_KEY_BACKSPACE, 1);
			   break;
			 //case BBACKSPACE:
			//	 GUI_StoreKeyMsg(GUI_KEY_BACKSPACE, 1);
			//	 break;
     }
		 if(Key)
		 {
     	 Timeout=OS_TimeMS;
			 LastKey=Key;
			 Tick=0;
		 }
  	 if(OS_TimeMS-Timeout>=30000) 
			 Released=1;
		 GUI_Delay(10);
	}
	NumericInput=1;
}


