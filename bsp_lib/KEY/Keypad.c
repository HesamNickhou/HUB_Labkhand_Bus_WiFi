#include <includes.h>
#include "Keypad.h"
#include "config.h"
#include "beep/beep.h"
#include "touch/touch.h"


unsigned char Key_Pressed=0;
unsigned char OldKey_Pressed=0;
unsigned int ReleasedTime=0;

//===============================================================================
//===============================================================================
//===============================================================================
#ifdef Torgheh
const unsigned char KeyboardTable[2][4] =
{
{11  ,10  ,9  ,4  },
{1   ,2  ,3  ,6  }
};


//===============================================================================
unsigned char  AppTaskKbd (void)
{
static unsigned char VolPressed;
unsigned long tmp;
unsigned char i,j;
unsigned int k, l;
unsigned char Key=0,OldKey=0;
u8 tick=0;
	
    Key=0;

    for (i=0; i < 2; i++)
    {   
  	  switch (i)
	    {
			  case 0: ROW0=0;	ROW1=1;	break;
  			case 1: ROW0=1;	ROW1=0;	break;
  		}
			
			__NOP;
			__NOP;
			__NOP;
			__NOP;
			__NOP;

      for (j=0; j < 4; j++)
      {  
			  switch (j)
  			{
	  			case 0: tmp=COL0; break;
		  		case 1: tmp=COL1; break;
			  	case 2: tmp=COL2; break;
				  case 3: tmp=COL3; break;
  			}
        if ((Key==0) && (tmp==0))
				{
          Key_Pressed=KeyboardTable[i][j];
				}
      }
    }

		if ((Key_Pressed) && (OldKey_Pressed==0))
		{
			if ((OS_TimeMS-ReleasedTime)>100)
			{
  			//printf("\n\rPressed: %d", Key_Pressed);
    		OldKey_Pressed=Key_Pressed;
   	    keyboard_Beep();
  		  Key_Pressed=0;
  		  return OldKey_Pressed;
			}
		}else{
			if ((Key_Pressed==0) && (OldKey_Pressed != 0))
			{
 			  //printf("\n\Released: %d", OldKey_Pressed);
				ReleasedTime=OS_TimeMS;
		    OldKey_Pressed=0;
			}
		}	
 		Key_Pressed=0;
 		return 0;

}

#endif

//===============================================================================
//===============================================================================
//===============================================================================
#ifdef Simorgh50N
const unsigned char KeyboardTable[5][4] =
{
{1  ,2  ,3  ,4  },
{5  ,6  ,7  ,8  },
{9  ,10 ,11 ,12 },
{13 ,14 ,15 ,16 },
{17 ,18 ,19 ,120}
};


const unsigned char KeyboardTableVII[4][4] =
{
{BA4  ,BA7 ,BA2 ,BA10    },
{BA9  ,BA6 ,BA1,BA5 },
{BOK  ,BA8,BA3,BDOWN  },
{BUP ,14,15,BCANCEL    }


/*
{  , ,    },
{       ,       , },
{     ,BRIGHT    ,BLEFT  },
{       ,       ,    },
{BMUTE     ,     ,    },
{       ,BMAP      ,    }
*/
};

//===============================================================================
unsigned char  AppTaskKbd (void)
{
static unsigned char VolPressed;
unsigned long tmp;
unsigned char i,j;
unsigned int k, l;
unsigned char Key=0,OldKey=0;
u8 tick=0;
	
    Key=0;

    for (i=0; i < 5; i++)
    {  
  	  switch (i)
	    {
			  case 0: ROW0=0;	ROW1=1;	ROW2=1;	ROW3=1;	ROW4=1; break;
  			case 1: ROW0=1;	ROW1=0;	ROW2=1;	ROW3=1;	ROW4=1; break;
	  		case 2: ROW0=1;	ROW1=1;	ROW2=0;	ROW3=1;	ROW4=1; break;
		  	case 3: ROW0=1;	ROW1=1;	ROW2=1;	ROW3=0;	ROW4=1; break;
			  case 4: ROW0=1;	ROW1=1;	ROW2=1;	ROW3=1;	ROW4=0; break;
  		}
			
      for (j=0; j < 500 ; j++)
	  		__NOP;

      for (j=0; j < 4; j++)
      {  
			  switch (j)
  			{
	  			case 0: tmp=COL0; break;
		  		case 1: tmp=COL1; break;
			  	case 2: tmp=COL2; break;
				  case 3: tmp=COL3; break;
  			}
        if ((Key==0) && (tmp==0))
				{
          Key_Pressed=KeyboardTable[i][j];
				}
      }
    }

		if ((Key_Pressed) && (OldKey_Pressed==0))
		{
			if ((OS_TimeMS-ReleasedTime)>100)
			{
  			//printf("\n\rPressed: %d", Key_Pressed);
    		OldKey_Pressed=Key_Pressed;
   	    keyboard_Beep();
  		  Key_Pressed=0;
  		  return OldKey_Pressed;
			}
		}else{
			if ((Key_Pressed==0) && (OldKey_Pressed != 0))
			{
 			  //printf("\n\Released: %d", OldKey_Pressed);
				ReleasedTime=OS_TimeMS;
		    OldKey_Pressed=0;
			}
		}	
 		Key_Pressed=0;
 		return 0;

}

//===============================================================================
unsigned char  AppTaskKbdVII (void) {
	static unsigned char VolPressed;
	unsigned long tmp;
	unsigned char i,j;
	unsigned int k, l;
	unsigned char Key = 0, OldKey = 0;
	u8 tick = 0;
	
	
	Key = 0;
	for (i=0; i < 4; i++) {
		switch (i) {
			case 0: ROW0_VII = 0;	ROW1_VII = 1;	ROW2_VII = 1;	ROW3_VII = 1;	break;
			case 1: ROW0_VII = 1;	ROW1_VII = 0;	ROW2_VII = 1;	ROW3_VII = 1;	break;
			case 2: ROW0_VII = 1;	ROW1_VII = 1;	ROW2_VII = 0;	ROW3_VII = 1;	break;
			case 3: ROW0_VII = 1;	ROW1_VII = 1;	ROW2_VII = 1;	ROW3_VII = 0;	break;
		}

		for (j=0; j < 10; j++)
		__NOP;

		for (j=0; j < 4; j++) {
			switch (j) {
				case 0: tmp = COL0_VII; break;
				case 1: tmp = COL1_VII; break;
				case 2: tmp = COL2_VII; break;
				case 3: tmp = COL3_VII; break;
			}
			if ((Key == 0) && (tmp == 0))
				Key_Pressed = KeyboardTableVII[i][j];
		}
	}

		/*
		if ((Key_Pressed) && (OldKey_Pressed==0))
		{
			if ((OS_TimeMS-ReleasedTime)>100)
			{
  			//printf("\n\rPressed: %d", Key_Pressed);
    		OldKey_Pressed=Key_Pressed;
   	    keyboard_Beep();
  		  Key_Pressed=0;
  		  return OldKey_Pressed;
			}
		}else{
			if ((Key_Pressed==0) && (OldKey_Pressed != 0))
			{
 			  //printf("\n\Released: %d", OldKey_Pressed);
				ReleasedTime=OS_TimeMS;
		    OldKey_Pressed=0;
			}
		}	
		*/
		if ((Key_Pressed) && (OldKey_Pressed == 0)) {
			if ((OS_TimeMS - ReleasedTime ) > 100)
    		OldKey_Pressed = Key_Pressed;
		}
		else {
			/*
			if ((Key_Pressed != 0) && (OldKey_Pressed==Key_Pressed))
			{
				if (OS_TimeMS-ReleasedTime>1000)
				{
					if (Key_Pressed==BOK)
					{
    				ReleasedTime=OS_TimeMS;
		        OldKey_Pressed=0;
		        Key_Pressed=0;
       	    keyboard_Beep();
  	    	  return BRIGHT;
					}
				}
			}
			*/
			if ((Key_Pressed == 0) && (OldKey_Pressed != 0)) {
 			  //printf("\n\Released: %d", OldKey_Pressed);
				if (OS_TimeMS - ReleasedTime > 1000) {
					if (OldKey_Pressed == BUP)
		        OldKey_Pressed = BRIGHT;
					if (OldKey_Pressed == BDOWN)
		        OldKey_Pressed = BLEFT;
				}
				ReleasedTime 	 = OS_TimeMS;
  		  Key_Pressed 	 = 0;
				Key 					 = OldKey_Pressed;
		    OldKey_Pressed = 0;
   	    keyboard_Beep();
  		  return Key;
			}
			if ((Key_Pressed == 0) && (OldKey_Pressed == 0))
				ReleasedTime = OS_TimeMS;
		}	
 		Key_Pressed = 0;
 		return 0;
}
#endif

//===============================================================================
unsigned char ScanKeyboard(void) {
	unsigned char temp;
  /*
  if (Volume != ' ')
  {
    keyboard_Beep();
    VolumeTurned=VT;
    VT=0;
    temp=Volume;
    Volume=' ';
    return temp;
  } 
  */	
  
  WDTR;
	/*
	tp_dev.scan(0); 		 
	if (tp_dev.sta&TP_PRES_DOWN)			
	{	
	 	if (tp_dev.x<480&&tp_dev.y<272)
		{	
			//TP_Draw_Big_Point(tp_dev.x,tp_dev.y,GUI_RED);					   
			return 200;
		}
	}	
	*/
	
	#ifdef Torgheh
  temp=AppTaskKbd();
	#endif
	#ifdef Simorgh50N
	if (BOARD_VER == VII)
    temp = AppTaskKbdVII();
  else		
    temp = AppTaskKbd();
	#endif
  return temp;
  
}

