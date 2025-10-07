#include "menu3d.h"
#include "Config.h"
#include <key/keypad.h>
#include <rtc/rtc.h>
#include <gsm/gsm.h>
#include <fatfs/ff.h>
#include <flash/flash.h>
#include <beep/beep.h>
#include <GUI.h>
#include <LCDConf_stm3210e_eval.h>
#include "forms/futil.h"

#ifdef Torgheh
extern unsigned short BkColor;
extern void LcdWriteReg(U16 Data);
extern void LcdWriteData(U16 Data);
#endif
//=========================================================================
//                                Structures
//=========================================================================
extern FATFS fs;

unsigned char SetTestMode(void);
extern char InTestMode;
		
	
#if (DeviceType==BUSDOOR)
extern void ShowPageAlef(void);
#endif

unsigned char ChangeRelayTimeout(void);
unsigned char SetGameTime(void);

unsigned char SearchAndSelectSSID(void);
unsigned char ChangeWifiPassword(void);
unsigned char ChangeWifi(void);
unsigned char ShowCommunicationMenu(void);

unsigned char Option=0;

extern void DisplayToolbar(unsigned char forceview);

unsigned int GetNumber(const unsigned char *msg, unsigned int Number, unsigned char Password);
unsigned char LEDTest(void);
unsigned char ChangePassword(void);
unsigned char SetLocalIP(void);
unsigned char SetGatewayIP(void);
unsigned char SetSubnetIP(void);

unsigned char ShowSettingMenu(void);
unsigned char ShowToolsMenu(void);
unsigned char SetLanguage(void);
unsigned char SetDateTime(void);
unsigned char SetDeviceID(void);
unsigned char SetDeviceUC(void);
unsigned char SetPrinter(void);
unsigned char SetServerIP(void);
unsigned char SetTransactionsSendInterval(void);
unsigned char SetTrakingInterval(void);
unsigned char DoClockCalibration(void);
unsigned char MemoryTest(void);
unsigned char SetTFTType(void);
unsigned char SetSIMCardType(void);
unsigned char KeypadTest(void);
unsigned char SetX10(void);



  MenuStruct const MainMenu[] =
  {
		#ifdef Simorgh50N
    //{{"0:icons/back.bmp", ""}, {"", ""}, {"برگشت", ""}, 0, 0},
		#endif
    //{{"0:icons/M0001/000.bmp", ""}, {"0:1.bmp", ""}, {"Users", ""}, 0, 0},
    {{"0:icons/M0001/001.bmp", ""}, {"0:1.bmp", ""}, {"تنظيمات", ""}, ShowSettingMenu, 0},
    {{"0:icons/M0001/002.bmp", ""}, {"0:1.bmp", ""}, {"امکانات", ""}, ShowToolsMenu, 0},
    {{"", ""}, {"", ""}, {"", ""}, 0, 0},
  };     

  MenuStruct const SettingMenu[] =
  {
		#ifdef Simorgh50N
    //{{"0:icons/back.bmp", ""}, {"", ""}, {"برگشت", ""}, 0, 0},
		#endif
    //{{"0:icons/M0002/000.bmp", ""}, {"0:1.bmp", ""}, {"زمان", ""}, SetDateTime, 0},
    {{"0:icons/M0002/001.bmp", ""}, {"0:1.bmp", ""}, {"کد دستگاه", ""}, SetDeviceID, 0},
    {{"0:icons/M0002/002.bmp", ""}, {"0:1.bmp", ""}, {"کد سازمان", ""}, SetDeviceUC, 0},

		#if (DeviceType != BUSDOOR)
    {{"0:icons/M0002/002.bmp", ""}, {"0:1.bmp", ""}, {"ارتباط", ""}, ShowCommunicationMenu, 0},
		#endif
		#ifdef Simorgh50N
    //{{"0:icons/M0004/000.bmp", ""}, {"0:1.bmp", ""}, {"نمايشگر", ""}, SetTFTType, 0},
		#endif
    //{{"0:icons/M0002/004.bmp", ""}, {"0:1.bmp", ""}, {"Language", ""}, SetLanguage, 0},
    //{{"0:icons/M0002/005.bmp", ""}, {"0:1.bmp", ""}, {"Printer", ""}, SetPrinter, 0},
    {{"", ""}, {"", ""}, {"", ""}, 0, 0},
  };     

  MenuStruct const ToolsMenu[] = 
 {
		#ifdef Simorgh50N
    //{{"0:icons/back.bmp", ""}, {"", ""}, {"برگشت", ""}, 0, 0},
		#endif
   // {{"0:icons/M0003/000.bmp", ""}, {"0:1.bmp", ""}, {"CreditTest", ""}, 0, 0},
    //{{"0:icons/M0003/001.bmp", ""}, {"0:1.bmp", ""}, {"MemTest", ""}, MemoryTest, 0},
    {{"0:icons/M0003/002.bmp", ""}, {"0:1.bmp", ""}, {"رمز عبور", ""}, ChangePassword, 0},
		#if (DeviceType != BUSDOOR)
    {{"0:icons/M0003/003.bmp", ""}, {"0:1.bmp", ""}, {"زمان بازي", ""}, SetGameTime, 0},		
    {{"0:icons/M0003/003.bmp", ""}, {"0:1.bmp", ""}, {"زمان رله", ""}, ChangeRelayTimeout, 0},
    {{"0:icons/M0002/002.bmp", ""}, {"0:1.bmp", ""}, {"نرخ ويژه", ""}, SetX10, 0},
    #endif		
    //{{"0:icons/M0002/002.bmp", ""}, {"0:1.bmp", ""}, {"نوع سيم کارت", ""}, SetSIMCardType, 0},
    {{"0:icons/M0002/002.bmp", ""}, {"0:1.bmp", ""}, {"بروزرساني صداي دستگاه", ""}, DoClockCalibration, 0},
    {{"0:icons/M0002/002.bmp", ""}, {"0:1.bmp", ""}, {"تست صفحه کليد", ""}, KeypadTest, 0},
    {{"0:icons/M0002/002.bmp", ""}, {"0:1.bmp", ""}, {"تست ال اي دي", ""}, LEDTest, 0},		
    {{"0:icons/M0002/002.bmp", ""}, {"0:1.bmp", ""}, {"حالت تست", ""}, SetTestMode, 0},				
		
		
    {{"", ""}, {"", ""}, {"", ""}, 0, 0},
  };

  MenuStruct const CommunicationMenu[] = 
  {
		#ifdef WithWIFI
    {{"0:icons/M0002/003.bmp", ""}, {"0:1.bmp", ""}, {"جستجوي وايفاي", ""}, SearchAndSelectSSID, 0},		
    {{"0:icons/M0002/003.bmp", ""}, {"0:1.bmp", ""}, {"رمز وايفاي", ""}, ChangeWifiPassword, 0},				
    {{"0:icons/M0002/003.bmp", ""}, {"0:1.bmp", ""}, {"آي پي دستگاه", ""}, SetLocalIP, 0},
    {{"0:icons/M0002/003.bmp", ""}, {"0:1.bmp", ""}, {"آي پي سرور", ""}, SetServerIP, 0},
    {{"0:icons/M0002/003.bmp", ""}, {"0:1.bmp", ""}, {"ساب نت", ""}, SetSubnetIP, 0},		
    {{"0:icons/M0002/003.bmp", ""}, {"0:1.bmp", ""}, {"آي پي دروازه", ""}, SetGatewayIP, 0},
    {{"0:icons/M0002/002.bmp", ""}, {"0:1.bmp", ""}, {"زمانبندي", ""}, SetTransactionsSendInterval, 0},
		#endif
    {{"", ""}, {"", ""}, {"", ""}, 0, 0},
  }; 
  
  MenuStruct const SIMCardTypeMenu[] =
  {
    {{"-", ""}, {"", ""}, {"ايرانسل", ""}, 0, 1},
    {{"-", ""}, {"", ""}, {"همراه اول", ""}, 0, 2},
    {{"-", ""}, {"", ""}, {"رايتل", ""}, 0, 3},
    {{"", ""}, {"", ""}, {"", ""}, 0, 0},
  };     

  MenuStruct const LanguageRadioMenu[] =
  {
    {{"-", ""}, {"", ""}, {"Persian", ""}, 0, 1},
    {{"-", ""}, {"", ""}, {"English", ""}, 0, 2},
    {{"-", ""}, {"", ""}, {"German", ""}, 0, 3},
    {{"", ""}, {"", ""}, {"", ""}, 0, 0},
  };     
   MenuStruct const EnDeMenu[] =
  {
    {{"-", ""}, {"", ""}, {"غير فعال", ""}, 0, 1},
    {{"-", ""}, {"", ""}, {"فعال", ""}, 0, 2},
    {{"", ""}, {"", ""}, {"", ""}, 0, 0},
  }; 
  
/*

//==============================================================================
int APP_GetData(void * p, const U8 * * ppData, unsigned NumBytesReq, U32 Off) 
{
unsigned int ByteRead;
FIL *File;
DWORD NumBytesRead;
File = (FIL *)p;

  if (NumBytesReq > DATA_SIZE) 
    NumBytesReq = DATA_SIZE;

  f_read(File, _acBuffer, NumBytesReq, &ByteRead);

  *ppData = _acBuffer;
  return ByteRead;
}

//==============================================================================
unsigned char G_LoadBMP(unsigned int X,unsigned int Y, const char* FileName, char LoadTransparent)
{
FRESULT res;	
FIL File;
	
	res = f_open(&File, FileName, FA_OPEN_EXISTING|FA_READ);
	if (res==FR_OK)
	{
  	GUI_BMP_DrawEx(APP_GetData, &File, X, Y);
	}
  f_close(&File);
}

//==============================================================================
void DispBMPAt(const char *filename, int x, int y)
{
FRESULT res;	
FIL File;
	
	res = f_open(&File, filename, FA_OPEN_EXISTING|FA_READ);
	if (res==FR_OK)
	{
  	GUI_BMP_DrawEx(APP_GetData, &File, x, y);
	}
  f_close(&File);
}
	
//==============================================================================
void DispPNGAt(const char *filename, int x, int y)
{
FRESULT res;	
FIL File;
	
	res = f_open(&File, filename, FA_OPEN_EXISTING|FA_READ);
	if (res==FR_OK)
	{
  	GUI_PNG_DrawEx(APP_GetData, &File, x, y);
	}
  f_close(&File);
}

*/


//==============================================================================
#define RGB2(r,g,b)	((unsigned int)( (( r >> 3 ) << 11 ) | \
								(( g >> 2 ) << 5  ) | \
								( b  >> 3 )))
#define RGB3(r,g,b)	((unsigned int)( (( b >> 3 ) << 11 ) | \
								(( g >> 2 ) << 5  ) | \
								( r  >> 3 )))

//==============================================================================
unsigned char G_LoadBMPPart(unsigned int X,unsigned int Y, unsigned short x1, unsigned short y1, unsigned short x2, unsigned short y2, const char* FileName)
{
unsigned int i,j;
unsigned short Color=0;
unsigned short Loc, yPos=Y;
unsigned char LastColor=0, FirstSector=1, R, G, B;
unsigned int ByteRead;
FRESULT res;
DIR dirs;
FIL FileObject;
unsigned int biWidth, biHeight;
unsigned short CurCol=0;
unsigned short CurRow=0;
unsigned char NeedRelocate=0;
unsigned short LastRow=0;
unsigned short b=0;

	  Y += TFT_START_Y;
    // Open the file
    res = f_open(&FileObject, FileName, FA_OPEN_EXISTING|FA_READ);
    if ( res != FR_OK ) 
		{
  	  if (res==5)
    	{
        memset(&fs, 0, sizeof(FATFS));      // Clear file system object
	      f_mount(&fs, "", 0); 		 		
	    }
      return 1;
		}
    while (1)
    {
      res = f_read(&FileObject, _acBuffer, DATA_SIZE, &ByteRead);
      if (res==FR_OK) 
      {
        Loc=0;
        if (FirstSector)
        {
          FirstSector=0;
          biWidth=_acBuffer[21]; biWidth<<=8;
          biWidth += _acBuffer[20]; biWidth<<=8;
          biWidth += _acBuffer[19]; biWidth<<=8;
          biWidth += _acBuffer[18];
          yPos=y2-y1+Y;
					#ifdef Simorgh50N
          LCD_REG_ADDRESS=0x002b;	
          LCD_DATA_ADDRESS=yPos>>8;	    
          LCD_DATA_ADDRESS=yPos&0x00ff;
      	  LCD_DATA_ADDRESS=271>>8;	    
      	  LCD_DATA_ADDRESS=271&0x00ff;
          LCD_REG_ADDRESS=0x002A;	
	        LCD_DATA_ADDRESS=X>>8;	    
	        LCD_DATA_ADDRESS=X&0x00ff;
	        LCD_DATA_ADDRESS=(X+biWidth)>>8;	    
	        LCD_DATA_ADDRESS=(X+biWidth)&0x00ff;
          LCD_REG_ADDRESS = 0x002C;
					#endif
					#ifdef Torgheh
					LcdWriteReg(0x0020);	
					LcdWriteData(yPos);
					LcdWriteReg(0x0021);	
					LcdWriteData((YSIZE_PHYS-1)-X);
					LcdWriteReg (0x0022);
					#endif
          Loc=54;
        }

        for (;Loc<ByteRead;)
        {
          if (LastColor==0)
            R=_acBuffer[Loc++];
          if (LastColor<=1)
          {
            if (Loc>=ByteRead) { LastColor=1; break; }
            G=_acBuffer[Loc++];
          }
          if (Loc>=ByteRead) { LastColor=2; break; }
          B=_acBuffer[Loc++];
          LastColor=0;
          Color=RGB2(B,G,R);
          if ((CurCol<x1) || (CurCol>x2) || ((CurRow)<y1) || ((CurRow)>y2))
          {
            
          }else{
            if ((CurRow != LastRow) && (LastRow != 0))
            {
              yPos--;
              NeedRelocate=1;
            }
            if (NeedRelocate)
            {
							#ifdef Simorgh50N
              LCD_REG_ADDRESS=0x002b;	
	            LCD_DATA_ADDRESS=yPos>>8;	    
	            LCD_DATA_ADDRESS=yPos&0x00ff;
	            LCD_DATA_ADDRESS=271>>8;	    
	            LCD_DATA_ADDRESS=271&0x00ff;
  	          LCD_REG_ADDRESS = 0x002C;
							#endif
							#ifdef Torgheh
							LcdWriteReg(0x0020);	
							LcdWriteData(yPos);
							LcdWriteReg(0x0021);	
							LcdWriteData((YSIZE_PHYS-1)-X);						
							LcdWriteReg (0x0022);
							#endif
              NeedRelocate=0;
            }
            LastRow=CurRow;

						#ifdef Simorgh50N
						LCD_DATA_ADDRESS = Color;
						#endif
						#ifdef Torgheh
						LcdWriteData(Color);
						#endif
          }  
          CurCol++;
          if (CurCol>=biWidth)
          {
            NeedRelocate=1;
            CurCol=0;
            CurRow++;
          }  
        }
      }
      if (ByteRead<DATA_SIZE)
        break;
    }

 
    // Close the file
    res = f_close(&FileObject);
    if ( res != FR_OK )
        return 10;
}

//==============================================================================
unsigned char G_LoadBMP(unsigned int X,unsigned int Y, const char* FileName, char LoadTransparent)
{
unsigned int i,j;
unsigned short Color=0;
unsigned short Loc, yPos=Y;
unsigned char LastColor=0, FirstSector=1, R, G, B;
unsigned int ByteRead;
unsigned int biWidth, biHeight;
unsigned short CurCol=0;
unsigned short CurRow=0;
unsigned char NeedRelocate=0;
unsigned short LastRow=0;
unsigned short b=0;
unsigned int StartAddress, Len;
   
	  if (GetResourceAddress((char *)FileName, &StartAddress, &Len))
			return 1;
		
		Y += TFT_START_Y;
		
    Len += StartAddress;
    while (1)
    {
 			ByteRead=DATA_SIZE;
			if (StartAddress+DATA_SIZE>Len)
  			ByteRead=Len-StartAddress;
      LoadFromDFToRam(StartAddress, ByteRead, _acBuffer);
      if (1) 
      {
        Loc=0;
        if (FirstSector)
        {
					if ((_acBuffer[0] != 'B') || (_acBuffer[1] != 'M'))
						return 1;
          FirstSector=0;
          biWidth=_acBuffer[21]; biWidth<<=8;
          biWidth += _acBuffer[20]; biWidth<<=8;
          biWidth += _acBuffer[19]; biWidth<<=8;
          biWidth += _acBuffer[18];
          biHeight=_acBuffer[25]; biHeight<<=8;
          biHeight += _acBuffer[24]; biHeight<<=8;
          biHeight += _acBuffer[23]; biHeight<<=8;
          biHeight += _acBuffer[22];
					if ((biHeight>480) || (biWidth>800))
						return 3;
          yPos=Y+biHeight;
 					#ifdef Simorgh50N
          LCD_REG_ADDRESS=0x002b;	
          LCD_DATA_ADDRESS=yPos>>8;	    
          LCD_DATA_ADDRESS=yPos&0x00ff;
      	  LCD_DATA_ADDRESS=271>>8;	    
      	  LCD_DATA_ADDRESS=271&0x00ff;
          LCD_REG_ADDRESS=0x002A;	
	        LCD_DATA_ADDRESS=X>>8;	    
	        LCD_DATA_ADDRESS=X&0x00ff;
	        LCD_DATA_ADDRESS=479>>8;	    
	        LCD_DATA_ADDRESS=479&0x00ff;
          LCD_REG_ADDRESS = 0x002C;
					#endif
					#ifdef Torgheh
					//LcdWriteReg(0x0020);	
					//LcdWriteData(yPos);
					//LcdWriteReg(0x0021);	
					//LcdWriteData((YSIZE_PHYS-1)-X);
					//LcdWriteReg (0x0022);
					#endif
          Loc=54;
        }

        for (;Loc<ByteRead;)
        {
          if (LastColor==0)
            B=_acBuffer[Loc++];
          if (LastColor<=1)
          {
            if (Loc>=ByteRead) { LastColor=1; break; }
            G=_acBuffer[Loc++];
          }
          if (Loc>=ByteRead) { LastColor=2; break; }
          R=_acBuffer[Loc++];
          LastColor=0;
					#ifdef Simorgh50N
          Color=RGB2(R,G,B);
					#endif
					#ifdef Torgheh
          Color=RGB3(B,G,R);
					#endif
          if ((CurRow != LastRow) && (LastRow != 0))
          {
            yPos--;
            NeedRelocate=1;
          }
          if (NeedRelocate)
          {
  					#ifdef Simorgh50N
            LCD_REG_ADDRESS=0x002b;	
	          LCD_DATA_ADDRESS=yPos>>8;	    
	          LCD_DATA_ADDRESS=yPos&0x00ff;
	          LCD_DATA_ADDRESS=271>>8;	    
	          LCD_DATA_ADDRESS=271&0x00ff;
         	  LCD_REG_ADDRESS=0x002A;	
	          LCD_DATA_ADDRESS=X>>8;	    
	          LCD_DATA_ADDRESS=X&0x00ff;
	          LCD_DATA_ADDRESS=479>>8;	    
	          LCD_DATA_ADDRESS=479&0x00ff;							
            LCD_REG_ADDRESS = 0x002C;
						#endif
  					#ifdef Torgheh
						//LcdWriteReg(0x0020);	
						//LcdWriteData(yPos);
						//LcdWriteReg(0x0021);	
						//LcdWriteData((YSIZE_PHYS-1)-X);						
						//LcdWriteReg (0x0022);
						#endif
            NeedRelocate=0;
          }
          LastRow=CurRow;

					CurCol++;
          if ((LoadTransparent) && ((R==255) && (G==0) && (B==0)))
          {
  					#ifdef Simorgh50N
         	  LCD_REG_ADDRESS=0x002A;	
	          LCD_DATA_ADDRESS=(X+CurCol)>>8;	    
	          LCD_DATA_ADDRESS=(X+CurCol)&0x00ff;
	          LCD_DATA_ADDRESS=479>>8;	    
	          LCD_DATA_ADDRESS=479&0x00ff;							
            LCD_REG_ADDRESS = 0x002C;
						#endif
						#ifdef Torgheh
						//LcdWriteReg(0x0021);	
						//LcdWriteData((YSIZE_PHYS-1)-(X+CurCol));						
						//LcdWriteReg (0x0022);
						#endif
          }else{  
  					#ifdef Simorgh50N
    				LCD_DATA_ADDRESS = Color;
						#endif
						#ifdef Torgheh
						LcdWriteReg(0x0020);	
						LcdWriteData(yPos);
						LcdWriteReg(0x0021);	
						LcdWriteData((YSIZE_PHYS-1)-(X+CurCol));						
						LcdWriteReg (0x0022);
						LcdWriteData(Color);
						#endif
					}
          if (CurCol>=biWidth)
          {
            NeedRelocate=1;
            CurCol=0;
            CurRow++;
          }  
        }
      }
			StartAddress += ByteRead;
      if (StartAddress>=Len)
        break;
    }

  return 0;
}
//==============================================================================
unsigned char G_LoadMapPart(unsigned int X,unsigned int Y, unsigned short x1, unsigned short y1, unsigned short x2, unsigned short y2, const char* FileName)
{
unsigned int i,j;
unsigned short Color=0;
unsigned short Loc, yPos=Y;
unsigned char LastColor=0, FirstSector=1, R, G, B;
unsigned int ByteRead;
FRESULT res;
DIR dirs;
FIL FileObject;
unsigned int biWidth, biHeight;
unsigned short CurCol=0;
unsigned short CurRow=0;
unsigned char NeedRelocate=0;
unsigned short LastRow=0;
unsigned short b=0;

		Y += TFT_START_Y;

  	// Open the file
    res = f_open(&FileObject, FileName, FA_OPEN_EXISTING|FA_READ);
    if (res != FR_OK) 
		{
 			GUI_SetColor(GUI_LIGHTGRAY);
      GUI_FillRect(X, Y, X+x2, Y+y2);
			if (res==5)
    	{
        memset(&fs, 0, sizeof(FATFS));      // Clear file system object
	      f_mount(&fs, "", 0); 		 		
        return 1;
	    }else{
		  	return 2;
			}
		}
		
    while (1)
    {
      res = f_read(&FileObject, _acBuffer, DATA_SIZE, &ByteRead);
      if (res==FR_OK) 
      {
        Loc=0;
        if (FirstSector)
        {
          FirstSector=0;
          biWidth=_acBuffer[21]; biWidth<<=8;
          biWidth += _acBuffer[20]; biWidth<<=8;
          biWidth += _acBuffer[19]; biWidth<<=8;
          biWidth += _acBuffer[18];
          yPos=y2-y1+Y;
					/*
          LCD_REG_ADDRESS=0x002b;	
          LCD_DATA_ADDRESS=yPos>>8;	    
          LCD_DATA_ADDRESS=yPos&0x00ff;
      	  LCD_DATA_ADDRESS=271>>8;	    
      	  LCD_DATA_ADDRESS=271&0x00ff;
          LCD_REG_ADDRESS=0x002A;	
	        LCD_DATA_ADDRESS=X>>8;	    
	        LCD_DATA_ADDRESS=X&0x00ff;
	        LCD_DATA_ADDRESS=479>>8;	    
	        LCD_DATA_ADDRESS=479&0x00ff;
          LCD_REG_ADDRESS = 0x002C;
					*/
          LCD_REG_ADDRESS=0x002b;	
          LCD_DATA_ADDRESS=yPos>>8;	    
          LCD_DATA_ADDRESS=yPos&0x00ff;
      	  LCD_DATA_ADDRESS=271>>8;	    
      	  LCD_DATA_ADDRESS=271&0x00ff;
          LCD_REG_ADDRESS=0x002A;	
	        LCD_DATA_ADDRESS=X>>8;	    
	        LCD_DATA_ADDRESS=X&0x00ff;
	        LCD_DATA_ADDRESS=(X+biWidth)>>8;	    
	        LCD_DATA_ADDRESS=(X+biWidth)&0x00ff;
          LCD_REG_ADDRESS = 0x002C;
          Loc=54;
        }

        for (;Loc<ByteRead;)
        {
          if (LastColor==0)
            R=_acBuffer[Loc++];
          if (LastColor<=1)
          {
            if (Loc>=ByteRead) { LastColor=1; break; }
            G=_acBuffer[Loc++];
          }
          if (Loc>=ByteRead) { LastColor=2; break; }
          B=_acBuffer[Loc++];
          LastColor=0;
          Color=RGB2(B,G,R);
          if ((CurCol<x1) || (CurCol>x2) || ((CurRow)<y1) || ((CurRow)>y2))
          {
            
          }else{
            if ((CurRow != LastRow) && (LastRow != 0))
            {
              yPos--;
              NeedRelocate=1;
            }
            if (NeedRelocate)
            {
							/*
              LCD_REG_ADDRESS=0x002b;	
	            LCD_DATA_ADDRESS=yPos>>8;	    
	            LCD_DATA_ADDRESS=yPos&0x00ff;
	            LCD_DATA_ADDRESS=271>>8;	    
	            LCD_DATA_ADDRESS=271&0x00ff;
           	  LCD_REG_ADDRESS=0x002A;	
	            LCD_DATA_ADDRESS=X>>8;	    
	            LCD_DATA_ADDRESS=X&0x00ff;
	            LCD_DATA_ADDRESS=479>>8;	          
	            LCD_DATA_ADDRESS=479&0x00ff;							
  	          LCD_REG_ADDRESS = 0x002C;
							*/
              LCD_REG_ADDRESS=0x002b;	
	            LCD_DATA_ADDRESS=yPos>>8;	    
	            LCD_DATA_ADDRESS=yPos&0x00ff;
	            LCD_DATA_ADDRESS=271>>8;	    
	            LCD_DATA_ADDRESS=271&0x00ff;
  	          LCD_REG_ADDRESS = 0x002C;
              NeedRelocate=0;
            }
            LastRow=CurRow;

						LCD_DATA_ADDRESS = Color;
          }  
          CurCol++;
          if (CurCol>=biWidth)
          {
            NeedRelocate=1;
            CurCol=0;
            CurRow++;
          }  
        }
      }else return 1;
      if (ByteRead<DATA_SIZE)
        break;
    }

 
    // Close the file
    res = f_close(&FileObject);
    if ( res != FR_OK )
        return 10;
}

//==============================================================================
unsigned char G_LoadMap(unsigned int X,unsigned int Y, const char* FileName, char LoadTransparent)
{
unsigned int i,j;
unsigned short Color=0;
unsigned short Loc, yPos=Y;
unsigned char LastColor=0, FirstSector=1, R, G, B;
unsigned int ByteRead;
FRESULT res;
DIR dirs;
FIL FileObject;
unsigned int biWidth, biHeight;
unsigned short CurCol=0;
unsigned short CurRow=0;
unsigned char NeedRelocate=0;
unsigned short LastRow=0;
unsigned short b=0;
   
    // Open the file
    res = f_open(&FileObject, FileName, FA_OPEN_EXISTING|FA_READ);
    if ( res != FR_OK ) 
		{
  	  if (res==5)
    	{
        memset(&fs, 0, sizeof(FATFS));      // Clear file system object
	      f_mount(&fs, "", 0); 		 		
	    }
      return 1;
		}		

	  Y += TFT_START_Y;

    while (1)
    {
      res = f_read(&FileObject, _acBuffer, DATA_SIZE, &ByteRead);
      if (res==FR_OK) 
      {
        Loc=0;
        if (FirstSector)
        {
          FirstSector=0;
          biWidth=_acBuffer[21]; biWidth<<=8;
          biWidth += _acBuffer[20]; biWidth<<=8;
          biWidth += _acBuffer[19]; biWidth<<=8;
          biWidth += _acBuffer[18];
          biHeight=_acBuffer[25]; biHeight<<=8;
          biHeight += _acBuffer[24]; biHeight<<=8;
          biHeight += _acBuffer[23]; biHeight<<=8;
          biHeight += _acBuffer[22];
          yPos=Y+biHeight;
          LCD_REG_ADDRESS=0x002b;	
          LCD_DATA_ADDRESS=yPos>>8;	    
          LCD_DATA_ADDRESS=yPos&0x00ff;
      	  LCD_DATA_ADDRESS=271>>8;	    
      	  LCD_DATA_ADDRESS=271&0x00ff;
          LCD_REG_ADDRESS=0x002A;	
	        LCD_DATA_ADDRESS=X>>8;	    
	        LCD_DATA_ADDRESS=X&0x00ff;
	        LCD_DATA_ADDRESS=479>>8;	    
	        LCD_DATA_ADDRESS=479&0x00ff;
          LCD_REG_ADDRESS = 0x002C;
          Loc=54;
        }

        for (;Loc<ByteRead;)
        {
          if (LastColor==0)
            B=_acBuffer[Loc++];
          if (LastColor<=1)
          {
            if (Loc>=ByteRead) { LastColor=1; break; }
            G=_acBuffer[Loc++];
          }
          if (Loc>=ByteRead) { LastColor=2; break; }
          R=_acBuffer[Loc++];
          LastColor=0;
          Color=RGB2(R,G,B);
          if ((CurRow != LastRow) && (LastRow != 0))
          {
            yPos--;
            NeedRelocate=1;
          }
          if (NeedRelocate)
          {
            LCD_REG_ADDRESS=0x002b;	
	          LCD_DATA_ADDRESS=yPos>>8;	    
	          LCD_DATA_ADDRESS=yPos&0x00ff;
	          LCD_DATA_ADDRESS=271>>8;	    
	          LCD_DATA_ADDRESS=271&0x00ff;
         	  LCD_REG_ADDRESS=0x002A;	
	          LCD_DATA_ADDRESS=X>>8;	    
	          LCD_DATA_ADDRESS=X&0x00ff;
	          LCD_DATA_ADDRESS=479>>8;	    
	          LCD_DATA_ADDRESS=479&0x00ff;							
            LCD_REG_ADDRESS = 0x002C;
            NeedRelocate=0;
          }
          LastRow=CurRow;

					CurCol++;
          if ((LoadTransparent) && ((R==255) && (G==0) && (B==0)))
          {
         	  LCD_REG_ADDRESS=0x002A;	
	          LCD_DATA_ADDRESS=(X+CurCol)>>8;	    
	          LCD_DATA_ADDRESS=(X+CurCol)&0x00ff;
	          LCD_DATA_ADDRESS=479>>8;	    
	          LCD_DATA_ADDRESS=479&0x00ff;							
            LCD_REG_ADDRESS = 0x002C;
          }else{  
    				LCD_DATA_ADDRESS = Color;
					}
          if (CurCol>=biWidth)
          {
            NeedRelocate=1;
            CurCol=0;
            CurRow++;
          }  
        }
      }
      if (ByteRead<DATA_SIZE)
        break;
    }

 
    // Close the file
    res = f_close(&FileObject);
    if ( res != FR_OK )
        return 10;
}

//==============================================================================
void PutText(unsigned int left, unsigned int top, unsigned int right, unsigned int bottom, const unsigned char *str, unsigned char style) {
GUI_RECT Rect;	

	Rect.x0=left;
	Rect.x1=right;
	Rect.y0=TFT_START_Y+top;
	Rect.y1=TFT_START_Y+bottom;
	
  GUI_DispStringInRectWrap(str, &Rect, style, GUI_WRAPMODE_WORD); 
	
}

//==============================================================================
void DisplayKeys(char *Key1, char *Key2, char *Key3) {
  //GUI_DrawGradientV(0, TFT_START_Y+245, 479, TFT_START_Y+271, GUI_MAKE_ALPHA(0x00, RGB(120,120,120)), GUI_MAKE_ALPHA(0x00, RGB(40,40,40)));
  if (BOARD_VER == VI) 	{
		//LoadFont(FontBKoodak40);
    GUI_SetColor(RGB(11,108,0));
    GUI_FillRoundedRect(325, TFT_START_Y+250, 475, TFT_START_Y+271, 3);
    GUI_SetColor(GUI_YELLOW);
    GUI_DrawRoundedRect(325, TFT_START_Y+250, 475, TFT_START_Y+271, 3);
    GUI_SetColor(GUI_WHITE);
    PutText(325, 240, 475, 271, (unsigned char *)Key3, GUI_TA_CENTER);

    GUI_SetColor(RGB(11,108,0));
    GUI_FillRoundedRect(165, TFT_START_Y+250, 315, TFT_START_Y+271, 3);
    GUI_SetColor(GUI_YELLOW);
    GUI_DrawRoundedRect(165, TFT_START_Y+250, 315, TFT_START_Y+271, 3);
    GUI_SetColor(GUI_WHITE);
    PutText(165, 240, 315, 271, (unsigned char *)Key2, GUI_TA_CENTER);

  	GUI_SetColor(RGB(11,108,0));
    GUI_FillRoundedRect(5, TFT_START_Y+250, 155, TFT_START_Y+271, 3);
    GUI_SetColor(GUI_YELLOW);
    GUI_DrawRoundedRect(5, TFT_START_Y+250, 155, TFT_START_Y+271, 3);
    GUI_SetColor(GUI_WHITE);
    PutText(5, 240, 155, 271, (unsigned char *)Key1, GUI_TA_CENTER);
  }
}

//==============================================================================
void DisplayKeysTorgheh(char *Key1, char *Key2, char *Key3, char *Key4) {
	short i;
	char j;	
	
	/*
	  GUI_SetColor(GUI_BLACK);
	  GUI_FillCircle(27, 165, 25);
	  GUI_FillCircle(82, 165, 25);
	  GUI_FillCircle(137, 165, 25);
	  GUI_FillCircle(192, 165, 25);
	*/
	for (j=0; j<4; j++)
	  for (i=0; i<5; i++)
	  {
	    GUI_SetColor(RGB(210+(i*10), 210+(i*10), 0));
	    GUI_DrawCircle(27+(j*55), 165, 25-i);
  	}
	for (j=0; j<4; j++)
	  for (i=0; i<21; i++)
	  {
	    GUI_SetColor(RGB(100+(i*5), 100+(i*5), 100+(i*5)));
	    GUI_DrawCircle(27+(j*55), 165, 20-i);
  	}
	
  G_LoadBMP(137-10, 165-12, "0:navi/7.bmp", 1);
  G_LoadBMP(192-6, 165-12, "0:navi/3.bmp", 1);
	  /*
		GUI_DrawLine(192-5,165-5,192+5,165);
	  GUI_DrawLine(192-5,165+5,192+5,165);

	  GUI_DrawLine(137+5,165-5,137-5,165);
	  GUI_DrawLine(137+5,165+5,137-5,165);
		*/
}

//==============================================================================
void ShowMessageDlg(unsigned char mType, unsigned char *mMessage, unsigned char repaint, unsigned char showpage, unsigned int delay, unsigned char alarm) {
  GUI_RECT   Rect = {0};
  int        r;

	#ifdef Simorgh50N
  r = 6;
  Rect.x0 = 20;
  Rect.y0 = TFT_START_Y+50;
  Rect.x1 = 250+SCREENRESIZE;
  Rect.y1 = TFT_START_Y+220;
  GUI_DrawGradientRoundedV(Rect.x0, Rect.y0, Rect.x1, Rect.y1, r, GUI_MAKE_ALPHA(0x00, 0xA02020), GUI_MAKE_ALPHA(0x00, 0x000000));
  GUI_SetColor(GUI_WHITE);
  GUI_DrawRoundedRect(Rect.x0, Rect.y0, Rect.x1, Rect.y1, r);
	#endif
	#ifdef Torgheh
	BkColor=0;
  Rect.x0 = 10;
  Rect.y0 = 30;
  Rect.x1 = 210;
  Rect.y1 = 160;
  GUI_DrawGradientRoundedV(Rect.x0, Rect.y0, Rect.x1, Rect.y1, 6, GUI_MAKE_ALPHA(0x00, 0xA02020), GUI_MAKE_ALPHA(0x00, 0x000000));
  GUI_SetColor(GUI_WHITE);
  GUI_DrawRoundedRect(Rect.x0, Rect.y0, Rect.x1, Rect.y1, 6);
	#endif
	
	LoadFont(FontBKoodak40);
  GUI_SetColor(0xFFFFFF);
  GUI_DispStringInRectWrap((const char *)mMessage, &Rect, GUI_TA_HCENTER | GUI_TA_VCENTER, GUI_WRAPMODE_WORD);
	
	
/*	
  switch (mType)
  {
    case mtError: G_LoadBMPPart(220,140,0,0,320,240,"0:desktop/EDlg.bmp"); break;
    case mtWarning: G_LoadBMPPart(220,140,0,0,320,240,"0:desktop/WDlg.bmp"); break;
    case mtInformation: G_LoadBMPPart(220,140,0,0,320,240,"0:desktop/IDlg.bmp"); break;
  }
*/

  switch (alarm)
  {
    case 1: OkBip(1); break;
    case 3: ErrorBip(0); break;
    case 4: ErrorBip(1); break;
  }
    
  if (delay)
    GUI_Delay(delay);
  #if (DeviceType==BUSDOOR)
	if (showpage)
    ShowPageAlef();
	#endif
}

//==============================================================================
void DisplayMenu(MenuStruct const *Menu, unsigned char OldMenuItem, unsigned char MenuItem, unsigned char RefreshAllItems) {
#ifdef Simorgh50N
	unsigned char NeedRefresh=0;
	unsigned char FirstItem;
	int i, Loc;
	unsigned char Y; 
	unsigned char str[18];
	unsigned short x;
  
  if ((MenuItem>=7) || ((MenuItem==0) && (OldMenuItem>=7)) || ((MenuItem==6) && (OldMenuItem==7)))
		RefreshAllItems=1;
	
	if (MenuItem<7)
		FirstItem=0;
	else
		FirstItem=MenuItem-6;
  Loc=0;
  for (i=0; (i<7) && (Menu[i+FirstItem].ICON[0][0] != '\0'); i++)
  {
    Loc += 34;
    if ((RefreshAllItems != 0) || (FirstItem+i==MenuItem) || (FirstItem+i==OldMenuItem))
		{
      if (FirstItem+i==MenuItem)
			{
    		GUI_DrawGradientRoundedV(0, Loc+2, 270+SCREENRESIZE, Loc+33,6,GUI_MAKE_ALPHA(0, RGB(220,220,0)),GUI_MAKE_ALPHA(0, RGB(40,40,0)));
      	//GUI_DrawGradientV(0, Loc, 479, Loc+35, GUI_MAKE_ALPHA(0x00, RGB(247,96,1)), GUI_MAKE_ALPHA(0x00, RGB(255,172,65)));
			  GUI_SetColor(GUI_YELLOW);
			}
  	  else {
			  GUI_SetColor(GUI_BLACK);
    		GUI_FillRoundedRect(0, Loc+3, 270+SCREENRESIZE, Loc+32,6);
			  GUI_SetColor(GUI_WHITE);
    		GUI_DrawRoundedRect(0, Loc+2, 270+SCREENRESIZE, Loc+33,6);
			  GUI_SetColor(GUI_GREEN);
      	//GUI_DrawGradientV(0, Loc, 479, Loc+35, GUI_MAKE_ALPHA(0x00, RGB(218,223,228)), GUI_MAKE_ALPHA(0x00, RGB(255,255,255)));
			  //GUI_SetColor(GUI_DARKGREEN);
			}
		  if (Menu[i+FirstItem].Select != 0) {
				if (Menu[i+FirstItem].Select>=100) {
				  x=1<<(Menu[i+FirstItem].Select-100);
					if ((Option&x)==x)
		        G_LoadBMP(230,Loc+6,"011",1); 
				  else
		        G_LoadBMP(230,Loc+6,"010",1); 
				}
				else {
				  if (Menu[i + FirstItem].Select == Option)
		        G_LoadBMP(230,Loc+6,"015",1); 
				  else
		        G_LoadBMP(230,Loc+6,"014",1); 
				}
			}//else
		  //G_LoadBMP(450,Loc,Menu[i+FirstItem].ICON[0],1);    
      PutText(0, Loc-TFT_START_Y, 260+SCREENRESIZE, Loc+35-TFT_START_Y, (unsigned char *)Menu[i+FirstItem].Title[0], GUI_TA_RIGHT);
	  }
  }//for
#endif
#ifdef Torgheh	
unsigned char NeedRefresh=0;
unsigned char FirstItem;
int i, Loc;
unsigned char Y; 
unsigned char str[18];
unsigned short x;
  
  if ((MenuItem>=4) || ((MenuItem==0) && (OldMenuItem>=4)) || ((MenuItem==3) && (OldMenuItem==4)))
		RefreshAllItems=1;
	
	if (MenuItem<4)
		FirstItem=0;
	else
		FirstItem=MenuItem-3;
  Loc=0;
  for (i=0; (i<4) && (Menu[i+FirstItem].ICON[0][0] != '\0'); i++)
  {
    Loc += 35;
    if ((RefreshAllItems != 0) || (FirstItem+i==MenuItem) || (FirstItem+i==OldMenuItem))
		{
      if (FirstItem+i==MenuItem)
      	GUI_DrawGradientV(0, Loc, 219, Loc+36, GUI_MAKE_ALPHA(0x00, RGB(247,96,1)), GUI_MAKE_ALPHA(0x00, RGB(255,172,65)));
  	  else
      	GUI_DrawGradientV(0, Loc, 219, Loc+36, GUI_MAKE_ALPHA(0x00, RGB(218,223,228)), GUI_MAKE_ALPHA(0x00, RGB(255,255,255)));
			GUI_SetColor(GUI_BLACK);
		  if (Menu[i+FirstItem].Select != 0)
			{
				if (Menu[i+FirstItem].Select>=100)
				{
				  x=1<<(Menu[i+FirstItem].Select-100);
					if ((Option&x)==x)
		        G_LoadBMP(190,Loc+6,"011",1); 
				  else
		        G_LoadBMP(190,Loc+6,"010",1); 
				}else{
				  if (Menu[i+FirstItem].Select==Option)
		        G_LoadBMP(190,Loc+6,"015",1); 
				  else
		        G_LoadBMP(190,Loc+6,"014",1); 
				}
			}//else
		  G_LoadBMP(190,Loc,Menu[i+FirstItem].ICON[0],1);    
      PutText(0, Loc, 190, Loc+35, (unsigned char *)Menu[i+FirstItem].Title[0], GUI_TA_RIGHT);
	  }
  }//for
#endif
}

//==============================================================================
unsigned char ExecMenu(MenuStruct const *Menu) {
	unsigned char Key, MenuCurPos=0;
	unsigned char Option=0;
	unsigned short i, j;
	unsigned int Timeout;	

  //GUI_DrawGradientV(0, 25, 480, 240, GUI_MAKE_ALPHA(0x00, 0xA02020), GUI_MAKE_ALPHA(0x00, 0x000000));
#ifdef Simorgh50N	
	GUI_Clear();
  //LoadFont(FontBKoodak40);
 	GUI_DrawGradientV(0, TFT_START_Y+0, 272, TFT_START_Y+35, GUI_MAKE_ALPHA(0x00, RGB(0xe3,0xe6,0xea)), GUI_MAKE_ALPHA(0x00, RGB(10,10,10)));
  DisplayMenu(Menu, 0, MenuCurPos, 1);
	Timeout = OS_TimeMS;
  while (1) {
  	if (OS_TimeMS-Timeout >= 20000) 
		  return Option;
    Key = ScanKeyboard();
		if (Key)
			Timeout = OS_TimeMS;
    switch (Key) {
      case BUP://ssssssssssssssssssssssssssssssssssssssssssssssss
      case 'L':
	      if (Menu[MenuCurPos + 1].ICON[0][0] == '\0') {
 	        DisplayMenu(Menu, MenuCurPos, 0, 0);
          MenuCurPos = 0;
        }
				else {
      	  DisplayMenu(Menu, MenuCurPos, MenuCurPos + 1, 0);
          MenuCurPos++;
        }
    	  break;
      case 'R':
      case BDOWN: 
      	if (MenuCurPos == 0) {
          for (i=0; Menu[i].ICON[0][0] != '\0'; i++);
  	      DisplayMenu(Menu, MenuCurPos, i - 1, 0);
          MenuCurPos = i - 1;
        }
				else {
      	  DisplayMenu(Menu, MenuCurPos, MenuCurPos-1, 0);
          MenuCurPos--;
        }  
      	break;
      case 120:           
      case BOK : 
        if (Menu[MenuCurPos].Select) {
          if (Menu[MenuCurPos].Select >= 100) {
						i = 1 << (Menu[MenuCurPos].Select - 100);
						if ((Option & i) == i)
						  Option &= (i ^ 0xFFFF);
						else
						  Option |= i;
					} else
					  Option = Menu[MenuCurPos].Select;
        }
				else {
          if (Menu[MenuCurPos].fun != 0) {  
  	        if ((*Menu[MenuCurPos].fun)() == 100) return 0; 
            ScanKeyboard();  
            //LoadFont(FontBKoodak40);
          	GUI_Clear();
 	          GUI_DrawGradientV(0, 0, 272, 35, GUI_MAKE_ALPHA(0x00, RGB(0xe3,0xe6,0xea)), GUI_MAKE_ALPHA(0x00, RGB(10, 10, 10)));
          }  
        }  
  			Timeout = OS_TimeMS;
				DisplayMenu(Menu, MenuCurPos, MenuCurPos, 1);
        break;
			case BLEFT:	
			case BCANCEL:	
			#ifdef Simorgh50N
			case BF2:
			#endif
        return Option;
    }//switch
  }//while
#endif	
#ifdef Torgheh	
	GUI_Clear();
  //LoadFont(FontBKoodak40);
 	GUI_DrawGradientV(0, 0, 220, 35, GUI_MAKE_ALPHA(0x00, RGB(0xe3,0xe6,0xea)), GUI_MAKE_ALPHA(0x00, RGB(10,10,10)));
  DisplayMenu(Menu, 0, MenuCurPos, 1);
	Timeout=OS_TimeMS;
  while (1)
  {
  	if (OS_TimeMS-Timeout>=15000) 
		  return Option;
    Key=ScanKeyboard();
		if (Key)
			Timeout=OS_TimeMS;
    switch (Key) {
      case BDOWN:
      case 'R':
	      if (Menu[MenuCurPos+1].ICON[0][0] =='\0')
        {
 	        DisplayMenu(Menu, MenuCurPos, 0, 0);
          MenuCurPos=0;
        }else{
      	  DisplayMenu(Menu, MenuCurPos, MenuCurPos+1, 0);
          MenuCurPos++;
        }
    	  break;
      case 'L':
      case BUP: 
      	if (MenuCurPos==0)
        {
          for (i=0; Menu[i].ICON[0][0] != '\0'; i++);
  	      DisplayMenu(Menu, MenuCurPos, i-1, 0);
          MenuCurPos=i-1;
        }else{
      	  DisplayMenu(Menu, MenuCurPos, MenuCurPos-1, 0);
          MenuCurPos--;
        }  
      	break;
      case 120:           
      case BOK : 
        if (Menu[MenuCurPos].Select)
				{
          if (Menu[MenuCurPos].Select>=100)
					{
						i=1<<(Menu[MenuCurPos].Select-100);
						if ((Option&i)==i)
						  Option&=(i^0xFFFF);
						else
						  Option|=i;
					}else
					  Option=Menu[MenuCurPos].Select;
        }
				else
        {
          if (Menu[MenuCurPos].fun != 0) 
          {  
  	        if ((*Menu[MenuCurPos].fun)()==100) return 0; 
            ScanKeyboard();  
            //LoadFont(FontBKoodak40);
          	GUI_Clear();
 	          GUI_DrawGradientV(0, 0, 220, 35, GUI_MAKE_ALPHA(0x00, RGB(0xe3,0xe6,0xea)), GUI_MAKE_ALPHA(0x00, RGB(10,10,10)));
          }  
        }  
  			Timeout=OS_TimeMS;
				DisplayMenu(Menu, MenuCurPos, MenuCurPos, 1);
        break;
			case BCANCEL:	
        return Option;
    }//switch
  }//while
#endif	
}

//==============================================================================
void DisplayStringList(unsigned char List[20][50], unsigned char OldMenuItem, unsigned char MenuItem, unsigned char RefreshAllItems) {
#ifdef Simorgh50N
	unsigned char NeedRefresh=0;
	unsigned char FirstItem;
	int i, Loc;
	unsigned char Y; 
	unsigned short x;
  
  if ((MenuItem>=7) || ((MenuItem==0) && (OldMenuItem>=7)) || ((MenuItem==6) && (OldMenuItem==7)))
		RefreshAllItems=1;
	
	if (MenuItem<7)
		FirstItem=0;
	else
		FirstItem=MenuItem-6;
  Loc=0;
  for (i=0; (i<7) && (List[i+FirstItem][0] != '\0'); i++) {
    Loc += 34;
    if ((RefreshAllItems != 0) || (FirstItem+i==MenuItem) || (FirstItem+i==OldMenuItem)) {
      if (FirstItem+i==MenuItem) {
    		GUI_DrawGradientRoundedV(0, Loc+2, 479, Loc+33,6,GUI_MAKE_ALPHA(0, RGB(220,220,0)),GUI_MAKE_ALPHA(0, RGB(40,40,0)));
      	//GUI_DrawGradientV(0, Loc, 479, Loc+35, GUI_MAKE_ALPHA(0x00, RGB(247,96,1)), GUI_MAKE_ALPHA(0x00, RGB(255,172,65)));
			  GUI_SetColor(GUI_YELLOW);
			}
  	  else {
			  GUI_SetColor(GUI_BLACK);
    		GUI_FillRoundedRect(0, Loc+3, 479, Loc+32,6);
			  GUI_SetColor(GUI_WHITE);
    		GUI_DrawRoundedRect(0, Loc+2, 479, Loc+33,6);
			  GUI_SetColor(GUI_GREEN);
      	//GUI_DrawGradientV(0, Loc, 479, Loc+35, GUI_MAKE_ALPHA(0x00, RGB(218,223,228)), GUI_MAKE_ALPHA(0x00, RGB(255,255,255)));
			  //GUI_SetColor(GUI_DARKGREEN);
			}
      PutText(0, Loc-TFT_START_Y, 450, Loc+35-TFT_START_Y, List[i+FirstItem], GUI_TA_RIGHT);
	  }
  }//for
#endif
#ifdef Torgheh	
unsigned char NeedRefresh=0;
unsigned char FirstItem;
int i, Loc;
unsigned char Y; 
unsigned short x;
  
  if ((MenuItem>=4) || ((MenuItem==0) && (OldMenuItem>=4)) || ((MenuItem==3) && (OldMenuItem==4)))
		RefreshAllItems=1;
	
	if (MenuItem<4)
		FirstItem=0;
	else
		FirstItem=MenuItem-3;
  Loc=0;
  for (i=0; (i<4) && (List[i+FirstItem][0] != '\0'); i++)
  {
    Loc += 35;
    if ((RefreshAllItems != 0) || (FirstItem+i==MenuItem) || (FirstItem+i==OldMenuItem))
		{
      if (FirstItem+i==MenuItem)
      	GUI_DrawGradientV(0, Loc, 219, Loc+36, GUI_MAKE_ALPHA(0x00, RGB(247,96,1)), GUI_MAKE_ALPHA(0x00, RGB(255,172,65)));
  	  else
      	GUI_DrawGradientV(0, Loc, 219, Loc+36, GUI_MAKE_ALPHA(0x00, RGB(218,223,228)), GUI_MAKE_ALPHA(0x00, RGB(255,255,255)));
			GUI_SetColor(GUI_BLACK);
      PutText(0, Loc, 190, Loc+35, List[i+FirstItem], GUI_TA_RIGHT);
	  }
  }//for
#endif
}

//==============================================================================
unsigned char ExecStringList(unsigned char List[20][50]) {
	unsigned char Key, MenuCurPos=0;
	unsigned char Option=0;
	unsigned short i, j;
	unsigned int Timeout;	

#ifdef Simorgh50N	
	GUI_Clear();
  //LoadFont(FontBKoodak40);
 	GUI_DrawGradientV(0, TFT_START_Y+0, 480, TFT_START_Y+35, GUI_MAKE_ALPHA(0x00, RGB(0xe3,0xe6,0xea)), GUI_MAKE_ALPHA(0x00, RGB(10,10,10)));
#endif
#ifdef Torgheh	
	GUI_Clear();
  //LoadFont(FontBKoodak40);
 	GUI_DrawGradientV(0, 0, 220, 35, GUI_MAKE_ALPHA(0x00, RGB(0xe3,0xe6,0xea)), GUI_MAKE_ALPHA(0x00, RGB(10,10,10)));
#endif	
	
  DisplayStringList(List, 0, MenuCurPos, 1);
	Timeout=OS_TimeMS;
  while (1)
  {
  	if (OS_TimeMS-Timeout>=15000) 
		  return 255;
    Key=ScanKeyboard();
		if (Key)
			Timeout=OS_TimeMS;
    switch (Key) {
      case BUP://ssssssssssssssssssssssssssssssssssssssssss
      case 'L':
	      if (List[MenuCurPos+1][0] =='\0')
        {
 	        DisplayStringList(List, MenuCurPos, 0, 0);
          MenuCurPos=0;
        }else{
      	  DisplayStringList(List, MenuCurPos, MenuCurPos+1, 0);
          MenuCurPos++;
        }
    	  break;
      case 'R':
      case BDOWN: 
      	if (MenuCurPos==0)
        {
          for (i=0; List[i][0] != '\0'; i++);
  	      DisplayStringList(List, MenuCurPos, i-1, 0);
          MenuCurPos=i-1;
        }else{
      	  DisplayStringList(List, MenuCurPos, MenuCurPos-1, 0);
          MenuCurPos--;
        }  
      	break;
      case 120:           
      case BOK : 
        return MenuCurPos;
        break;
			case BLEFT:	
			case BCANCEL:	
			#ifdef Simorgh50N
			case BF2:
			#endif
        return 255;
    }//switch
  }//while
}

//==============================================================================
void DisplayRadioMenu(MenuStruct const *Menu, unsigned char OldMenuItem, unsigned char MenuItem) 
{
unsigned char NeedRefresh=0;
unsigned char FirstItem=0;
int i;
unsigned char Y=35; 
	
	#ifdef Simorgh50N
  for (i=0; i<8; i++)
  {
    if (Menu[i+FirstItem].ICON[0][0] =='\0') break;
    if (FirstItem+i==MenuItem)
		{
     	GUI_DrawGradientV(0, Y, 479, Y+35, GUI_MAKE_ALPHA(0x00, RGB(247,96,1)), GUI_MAKE_ALPHA(0x00, RGB(255,172,65)));
		  GUI_SetColor(GUI_YELLOW);
		}
 	  else
		{
     	GUI_DrawGradientV(0, Y, 479, Y+35, GUI_MAKE_ALPHA(0x00, RGB(218,223,228)), GUI_MAKE_ALPHA(0x00, RGB(255,255,255)));
		  GUI_SetColor(GUI_DARKGREEN);
		}
		if (Menu[i+FirstItem].Select==Option)
      G_LoadBMP(455, Y+5, "0:icons/r1.bmp", 1);
    PutText(0, Y, 450, Y+35, (unsigned char *)Menu[i+FirstItem].Title[0], GUI_TA_RIGHT);
    Y += 35;
  }
	#endif
	
	#ifdef Torgheh
  for (i=0; i<8; i++)
  {
    if (Menu[i+FirstItem].ICON[0][0] =='\0') break;
    if (FirstItem+i==MenuItem)
     	GUI_DrawGradientV(0, Y, 219, Y+36, GUI_MAKE_ALPHA(0x00, RGB(247,96,1)), GUI_MAKE_ALPHA(0x00, RGB(255,172,65)));
 	  else
     	GUI_DrawGradientV(0, Y, 219, Y+36, GUI_MAKE_ALPHA(0x00, RGB(218,223,228)), GUI_MAKE_ALPHA(0x00, RGB(255,255,255)));
		GUI_SetColor(GUI_BLACK);

		if (Menu[i+FirstItem].Select==Option)
      G_LoadBMP(190, Y, "0:icons/r1.bmp", 1);
    PutText(0, Y, 185, Y+30, (char *)Menu[i+FirstItem].Title[0], GUI_TA_RIGHT);
    Y += 30;
  }
	#endif
}

//==============================================================================
unsigned short ExecRadioMenu(MenuStruct const *Menu, unsigned short Option) 
{
unsigned char Key, MenuCurPos=0;

	GUI_Clear();
  //LoadFont(FontBKoodak40);
	#ifdef Simorgh50N
 	GUI_DrawGradientV(0, TFT_START_Y+0, 480, TFT_START_Y+35, GUI_MAKE_ALPHA(0x00, RGB(0xe3,0xe6,0xea)), GUI_MAKE_ALPHA(0x00, RGB(10,10,10)));
  #endif
	#ifdef Torgheh
 	GUI_DrawGradientV(0, 0, 219, 25, GUI_MAKE_ALPHA(0x00, RGB(0xe3,0xe6,0xea)), GUI_MAKE_ALPHA(0x00, RGB(10,10,10)));
  #endif
	DisplayRadioMenu(Menu, 0, MenuCurPos);
	
  while (1)
  {
    Key=ScanKeyboard();
    switch (Key) 
		{
      case BUP://sssssssssssssssssssssssssssssssssssss
      case 'L':
	      if (Menu[MenuCurPos+1].ICON[0][0] =='\0')
	        break;
	      DisplayRadioMenu(Menu, MenuCurPos, MenuCurPos+1);
        MenuCurPos++;
	      break;
      case 'R':
      case BDOWN : 
	      if (MenuCurPos==0)
	        break;
	      DisplayRadioMenu(Menu, MenuCurPos, MenuCurPos-1);
        MenuCurPos--;
	      break;
      case 120:           
      case BOK : 
        return Menu[MenuCurPos].Select;
	      break;
      case BLEFT:
      case BCANCEL:
        return Option;
    }//switch
  }//while
	
}

//==============================================================================
unsigned char Setup(void) {
	unsigned int Pass = 0;
	unsigned int DefPass = Hour + Min;	

  DefPass = (Hour + Min) ^ Day; 
	
	
  Pass = GetNumber("رمز را وارد نماييد", 0, 1);
  if ((Pass == Config.Password) || (Pass == DefPass)) {
    //G_DrawGradianRectangle(255,255,255,84,75,80,215,25,20,1);
    ExecMenu(MainMenu);
    SaveConfiguration();
  }
  else if (Pass != 0)
    ShowMessageDlg(mtError, "رمز اشتباه است", 1, 0, 1000, 0);
}

//==============================================================================
unsigned char ShowSettingMenu(void) { ExecMenu(SettingMenu); }  
//==============================================================================
unsigned char ShowToolsMenu(void) { ExecMenu(ToolsMenu); }
//==============================================================================
unsigned char ShowCommunicationMenu(void) { ExecMenu(CommunicationMenu); }
//==============================================================================
unsigned char SetLanguage(void) { ExecRadioMenu(LanguageRadioMenu, 1); }
//==============================================================================
unsigned char SetTestMode(void) {
	#if (DeviceType==BUSDOOR)
	switch (InTestMode)
	{
		case 0:
      Option=1;
      break; 		
		default:
      Option=2;
      break; 		
	}
  InTestMode=ExecRadioMenu(EnDeMenu, Option);	
#endif
}

//==============================================================================
unsigned char SetSIMCardType(void) {
	switch (Config.SIMCardType) {
		case 2:
		case 20:
      Option = 2;
      break; 		
		case 3:
      Option = 3;
      break; 		
		default:
      Option = 1;
      break; 		
	}
  Config.SIMCardType = ExecRadioMenu(SIMCardTypeMenu, Option);
}

//==============================================================================
char CalcWeek(int year, int month, int day) {
  char week;
  
  if (month == 1 || month == 2) {
    month += 12;
    --year;
  }
  
  week = (day + 2 * month + 3 * (month + 1) / 5 + year + year / 4 - year / 100 + year / 400) % 7;
	++week;
  
  return week;
}

//==============================================================================
unsigned char SetDateTime(void) {
/*	
unsigned short CurX, y;
unsigned char Refresh,Key, newWeek, m, d, hh, mm, ss;
unsigned char Loc=0;
unsigned char Stream[] ="0000000000";
char FileName[] ="0:desktop/clock/0.bmp";
unsigned int previousTime;
unsigned long int time=0;
unsigned char buf[32];

  G_LoadBMPPart(0, 0, 0, 0, 320, 240-25, "0:desktop/0000.bmp");
  DisplayKeys("", "انصراف", "تاييد");
  LcdFont(Nazanin19);
  SetFgColor(WHITE);
  
  Stream[0] =(Year/10)%10;
  Stream[1] =Year%10;
  Stream[2] =Month/10;
  Stream[3] =Month%10;
  Stream[4] =Day/10;
  Stream[5] =Day%10;
  Stream[6] =Hour/10;
  Stream[7] =Hour%10;
  Stream[8] =Min/10;
  Stream[9] =Min%10;

  G_DrawGradianRectangle(255,255,255,84,75,80,150,36,30,1);  
  SetCursorY(155);

  SetCursorX(90);  PutChar(Stream[0]+'0');
  SetCursorX(110); PutChar(Stream[1]+'0');
  SetCursorX(130); PutChar('/');	
  SetCursorX(150); PutChar(Stream[2]+'0');
  SetCursorX(170); PutChar(Stream[3]+'0');
  SetCursorX(190); PutChar('/');	
  SetCursorX(210); PutChar(Stream[4]+'0');
  SetCursorX(230); PutChar(Stream[5]+'0');

  G_DrawGradianRectangle(255,255,255,84,75,80,60,36,30,1);  
  SetCursorY(64);

  SetCursorX(120); PutChar(Stream[6]+'0');
  SetCursorX(140); PutChar(Stream[7]+'0');
  SetCursorX(160); PutChar(':');	
  SetCursorX(180); PutChar(Stream[8]+'0');
  SetCursorX(200); PutChar(Stream[9]+'0');
  G_Repaint();

  CurX=90;
  SetCursorY(155);
  Refresh=1;
  while (Loc<=9)
  {
    switch (Key=ScanKeyboard())
    {
       case BUP:
       case 'R':
         if (Stream[Loc]<9)
         {
           SetFgColor(RGB(84,75,80));
           SetCursorX(CurX); 
           PutChar(Stream[Loc]+'0');
	   Stream[Loc]++;
           Refresh=1;
         }
 	 break;
       case 'L':
       case BDOWN: 
         if (Stream[Loc]>0)
         {
           SetFgColor(RGB(84,75,80));
           SetCursorX(CurX); 
           PutChar(Stream[Loc]+'0');
	   Stream[Loc]--;
           Refresh=1;
         }
	 break;
       case BRIGHT:
         if (Loc<9)
         {
           SetFgColor(RGB(255,255,255));
           SetCursorX(CurX); 
           PutChar(Stream[Loc]+'0');
           Loc++;
           Refresh=1;
         }
         break;
       case BLEFT:
         if (Loc>0)
         {
           SetFgColor(RGB(255,255,255));
           SetCursorX(CurX); 
           PutChar(Stream[Loc]+'0');
           Loc--;
           Refresh=1;
         }
         break;
       case 120:   
       case BOK : 
       case BF3:
         SetFgColor(WHITE);
         SetCursorX(CurX); 
         PutChar(Stream[Loc]+'0');
         Loc++;
         Refresh=1;
         if ((Loc>9) || (Key==BF3)) 
         {
           Loc=10;
           Hour=(Stream[6]*10)+Stream[7];
           Min=(Stream[8]*10)+Stream[9];
           Sec=0;
           Year=(Stream[0]*10)+Stream[1];
           Year += 1300;
           Month=(Stream[2]*10)+Stream[3];
           Day=(Stream[4]*10)+Stream[5];
           Config.StartYear=Year;
           Config.StartMonth=Month;
           Config.StartDay=Day;
           Config.StartHour=Hour;
           Config.StartMin=Min;
           Config.StartSec=0;     
           // Configure RTT for a 1 second tick interrupt
           RTT_SetPrescaler(AT91C_BASE_RTTC, RTTPrescaler);
           previousTime = RTT_GetTime(AT91C_BASE_RTTC);
           while (previousTime == RTT_GetTime(AT91C_BASE_RTTC));           
           buf[0] =Config.StartYear & 0xFF;
           buf[1] =(Config.StartYear>>8) & 0xFF;
           buf[2] =Config.StartMonth;
           buf[3] =Config.StartDay;
           buf[4] =Config.StartHour;
           buf[5] =Config.StartMin;
           buf[6] =Config.StartSec;
           buf[7] =RTTPrescaler & 0xFF;
           buf[8] =(RTTPrescaler>>8) & 0xFF;
           for (time=7; time<16; time++)
             buf[time] =0;
           OkBip(1);
           break;
         }  
	 break;
       case BF2:
         return 0;
    }
    if (Refresh)
    {
      Refresh=0;
      switch (Loc)
      {
        case 0: CurX=90; break;
        case 1: CurX=110; break;
        case 2: CurX=150; break;
        case 3: CurX=170; break;
        case 4: CurX=210; break;
        case 5: CurX=230; break;
        case 6: CurX=120; break;
        case 7: CurX=140; break;
        case 8: CurX=180; break;
        case 9: CurX=200; break;
      }
      if (Loc>5) SetCursorY(64);
      else      SetCursorY(155);
      SetFgColor(RGB(255,168,0));
      SetCursorX(CurX); 
      PutChar(Stream[Loc]+'0');
      G_Repaint();
    }
  }
*/  
}

//================================================================================================
unsigned char GetChargeCode(void) {
	unsigned short CurX;
	unsigned long int i=1, n1=0;
	unsigned char Refresh=1, k, cnt=0;
	unsigned char Loc=0;
	unsigned char Stream[] ="00000";
	char FileName[] ="0:desktop/clock/0.bmp";
	unsigned char idx=0;
/*
	#ifdef Simorgh50N
  GUI_DrawGradientV(0, TFT_START_Y+0, 480, TFT_START_Y+272, GUI_MAKE_ALPHA(0x00, 0xA02020), GUI_MAKE_ALPHA(0x00, 0x000000));
  GUI_SetColor(RGB(237,237,237));
  GUI_FillRect(88, TFT_START_Y+40, 362, TFT_START_Y+200);
  GUI_SetColor(RGB(33,31,32));
  GUI_FillRect(93, TFT_START_Y+45, 358, TFT_START_Y+195);
  GUI_SetColor(GUI_LIGHTGRAY);
  GUI_FillRect(110, TFT_START_Y+105, 340, TFT_START_Y+130);
  GUI_SetColor(GUI_WHITE);
  PutText(128,60,352,90,"کد شارژ را وارد نماييد",GUI_TA_CENTER);
  DisplayKeys("", "انصراف", "تاييد");
	#endif
	#ifdef Torgheh
  //GUI_DrawGradientV(0, 0, 480, 272, GUI_MAKE_ALPHA(0x00, 0xA02020), GUI_MAKE_ALPHA(0x00, 0x000000));
  GUI_SetColor(RGB(237,237,237));
  GUI_FillRect(1, 20, 219, 170);
  GUI_SetColor(RGB(33,31,32));
  GUI_FillRect(3, 23, 216, 167);
  G_LoadBMP(10,85,"0:desktop/InputBox.bmp",0);
	DisplayKeysTorgheh(""," "," ","");
  //LoadFont(FontBKoodak40);
  GUI_SetColor(GUI_WHITE);
	BkColor=RGB888_RGB565(GUI_ORANGE ^ 0xFFFFFF);
  PutText(10,40,210,70,"کد شارژ را وارد نماييد",GUI_TA_CENTER);
	GUI_SetColor(GUI_BLACK);
	#endif
  
	ChargeCode[idx] ='0';
	
  while (1)
  {
     switch (ScanKeyboard()) 
     {
   		 #ifdef Simorgh50N
			 case BP4:       ChargeCode[idx++] ='1'; Refresh=1; break;
			 case BP3:       ChargeCode[idx++] ='2'; Refresh=1; break;
			 case BP2:       ChargeCode[idx++] ='3'; Refresh=1; break;
			 case BP1:       ChargeCode[idx++] ='4'; Refresh=1; break;
			 case BHELP:     ChargeCode[idx++] ='5'; Refresh=1; break;
			 case BF1:       ChargeCode[idx++] ='6'; Refresh=1; break;
			 case BMAP:      ChargeCode[idx++] ='7'; Refresh=1; break;
			 case BPAYMENT2: ChargeCode[idx++] ='8'; Refresh=1; break;
			 case BF2:       ChargeCode[idx++] ='9'; Refresh=1; break;
			 case BF3:       ChargeCode[idx++] ='0'; Refresh=1; break;
			 #endif	 
       case BDOWN : 
       case 'L':
				 k=ChargeCode[idx];
         if (k>'0') k--;
         else if (++cnt>3) break;
				 ChargeCode[idx] =k;
				 Refresh=1;
         break;
       case BUP:
       case 'R':
				 k=ChargeCode[idx];
         if (k<'9') k++;
				 ChargeCode[idx] =k;
				 Refresh=1;
         break;
       case 120:    
       case BRIGHT: 
	
			   if ((ChargeCode[idx]<'0') || (ChargeCode[idx]>'9'))
					 ChargeCode[idx] ='0';
				 else
				   ChargeCode[++idx] ='0';
				 Refresh=1;
         break;
       case BLEFT:
				 if (idx>0)
					 idx--;
				 Refresh=1;
				 break;
       case BOK: 
       	 ChargeCode[idx+1] =0;
         return 1;
         break;
       case BCANCEL:
         return 0;
     }
     if (Refresh)
     {
     	 ChargeCode[idx+1] =0;
			 #ifdef Torgheh
       G_LoadBMP(10,85,"0:desktop/InputBox.bmp",0);
       PutText(20,85,210,115,ChargeCode,GUI_TA_LEFT);
     	 #endif
     	 #ifdef Simorgh50N
       GUI_SetColor(GUI_LIGHTGRAY);
       GUI_FillRect(110, TFT_START_Y+105, 340, TFT_START_Y+130);
       GUI_SetColor(GUI_DARKBLUE);
       PutText(110,100,340,130,ChargeCode,GUI_TA_LEFT);
     	 #endif
       Refresh=0;
     }
  }
	*/
  return 0;
}

//==============================================================================
unsigned int GetNumber(const unsigned char *msg, unsigned int n, unsigned char Password) {
	unsigned short CurX;
	unsigned long int i = 1, n1 = 0;
	unsigned char Refresh = 1, k, cnt = 0;
	unsigned char Loc = 0;
	unsigned int DefValue = n;
	unsigned char Stream[] = "00000";
	char FileName[] = "0:desktop/clock/0.bmp";
	unsigned char str[20];
	
	
	#ifdef Simorgh50N
  GUI_DrawGradientV(0, TFT_START_Y + 0, 272 + SCREENRESIZE, TFT_START_Y + 480, GUI_MAKE_ALPHA(0x00, 0xA02020), GUI_MAKE_ALPHA(0x00, 0x000000));
  GUI_SetColor(RGB(237,237,237));
  GUI_FillRect(10, TFT_START_Y+40, 272-10+SCREENRESIZE, TFT_START_Y+400);
  GUI_SetColor(RGB(33,31,32));
  GUI_FillRect(15, TFT_START_Y+45, 272-15+SCREENRESIZE, TFT_START_Y+390);
	
	GUI_SetColor(RGB(255, 255, 255));
  GUI_FillRoundedRect(35, 105+45, 272-35+SCREENRESIZE, 140+45,3);
	GUI_SetColor(RGB(100,100,120));
  GUI_DrawRoundedFrame(35, 105+45, 272-35+SCREENRESIZE, 140+45,3,2);			
	GUI_SetColor(RGB(0,0,0));
  GUI_DrawRoundedFrame(35+1, 105+1+45, 272-35-1+SCREENRESIZE, 140-1+45,3,1);	
	
  LoadFont(FontBKoodak40);
  GUI_SetColor(GUI_WHITE);
  PutText(0,80,270+SCREENRESIZE,150,msg,GUI_TA_CENTER);
  DisplayKeys("", "انصراف", "تاييد");
	#endif

  
  while (1) {
     switch (ScanKeyboard()) {
   		 #ifdef Simorgh50N
			 case BA1:  n *= 10; n += 1; break;
			 case BA2:  n *= 10; n += 2; break;
			 case BA3:  n *= 10; n += 3; break;
			 case BA4:  n *= 10; n += 4; break;
			 case BA5:  n *= 10; n += 5; break;
			 case BA6:  n *= 10; n += 6; break;
			 case BA7:  n *= 10; n += 7; break;
			 case BA8:  n *= 10; n += 8; break;
			 case BA9:  n *= 10; n += 9; break;
			 case BA10: n *= 10; n += 0; break;
			 #endif	 
			 
       case BUP : //ssssssssssssssssssssssssssssssssss
       case 'R':
         k=n%10;
         n/=10; 
         if (k>0) k--;
         else if (++cnt>3) break;
         n*=10;
         n += k;
         break;
       case BDOWN:
       case 'L':
         k=n%10;
         n/=10; if (k<9) k++;
         n*=10;
         n += k;
         break;
       case 120:    
       case BRIGHT: 
				 /*
         for (i=0; i<500000; i++) if (RowStatus(3)) break;
         if (i>=500000)
           return n; 
			   */
         if (n<99999999)
           n=n*10;
         break;
       case BLEFT:
				 if (n>0)
				 {
					 n=n/10;
				 }
				 break;
       case BOK: 
         return n;
         break;
       case BCANCEL:
         return DefValue;
     }
     if (n != n1)
     {
       cnt=0;
     	 #ifdef Torgheh
       G_LoadBMP(10,85,"0:desktop/InputBox.bmp",0);
       sprintf(str, "%d", n);
       PutText(20,85,210,115,str,GUI_TA_LEFT);
     	 #endif
     	 #ifdef Simorgh50N
	     GUI_SetColor(RGB(255,255,255));
       GUI_FillRoundedRect(35, 105+45, 272-35+SCREENRESIZE, 140+45,3);
	     GUI_SetColor(RGB(100,100,120));
       GUI_DrawRoundedFrame(35, 105+45, 272-35+SCREENRESIZE, 140+45,3,2);			
	     GUI_SetColor(RGB(0,0,0));
       GUI_DrawRoundedFrame(35+1, 105+1+45, 272-35-1+SCREENRESIZE, 140-1+45,3,1);	
			 
       GUI_SetColor(GUI_ORANGE);
       sprintf(str, "%d", n);
       PutText(50,150,272-50+SCREENRESIZE,190,str,GUI_TA_LEFT);
     	 #endif
       n1=n;
       Refresh=0;
     }
  }
  return n;
}

//==============================================================================
unsigned int GetNumberByVolume(unsigned char *msg, unsigned int n) {}
//==============================================================================
unsigned char SetDeviceID(void) {
  Config.DeviceID=GetNumber("کد دستگاه را وارد نماييد .", Config.DeviceID, 0);
	
	#if (DeviceType == BUSDOOR)
	if ((Config.DeviceID != 1) && (Config.DeviceID != 2)) Config.DeviceID=1;
	#endif
}

//==============================================================================
unsigned char SetX10(void) {
  Config.X10 = GetNumber("نرخ ويژه", Config.X10, 0);
  OkBip(1);
}

//==============================================================================
unsigned char SetDeviceUC(void) {
	//Config.UC=DeviceUC;
  Config.UC=GetNumber("کد مرکز را وارد نماييد", Config.UC, 0);
  //DeviceUC=Config.UC;
}

//==============================================================================
unsigned char SetTransactionsSendInterval(void) { Config.TransactionsSendInterval=GetNumber("زمان ارسال رکورد", Config.TransactionsSendInterval, 0); }
//==============================================================================
unsigned char ChangeRelayTimeout(void) { Config.Relay_Timeout=GetNumber("فعال بودن رله - ميلي ثانيه", Config.Relay_Timeout, 0); }
//==============================================================================
unsigned char SetGameTime(void) { Config.G_GameTimer=GetNumber("زمان بازي - ثانيه", Config.G_GameTimer, 0); }
//================================================================================================
unsigned char ChangeWifiPassword(void) {
	unsigned int AAA = 0;
	
	AAA = atoi(Config.WiFiPassword);
	AAA = GetNumber("رمز وايفاي", AAA, 0);
	
	sprintf(Config.WiFiPassword, "%d", AAA);
  OkBip(1);
}
//==============================================================================
unsigned char ChangePassword(void) {
 unsigned int Pass1, Pass2;
  
  if ((Pass1=GetNumber("رمز جديد", 0, 1))==0)
    return 0;
  if ((Pass2=GetNumber("تکرار رمز", 0, 1))==0)
    return 0;
  if (Pass1==Pass2)
  {
    Config.Password=Pass1;
    SaveConfiguration();
    OkBip(1);
  }else{
    ShowMessageDlg(mtError, "رمز اشتباه است",1,0,1000,4);
  }
}

//==============================================================================
unsigned char SetTrakingInterval(void) {
  Config.TrackingInterval=GetNumber("زمان ثبت موقعيت", Config.TrackingInterval, 0);
  OkBip(1);
}
//==============================================================================
unsigned char SetTFTType(void) {
  Config.TFTType=GetNumber("نوع نمايشگر", Config.TFTType, 0);
  OkBip(1);
}
//==============================================================================
unsigned char SetSimCardType(void) {
  Config.SIMCardType=GetNumber("نوع سيم کارت", Config.SIMCardType, 0);
  OkBip(1);
}
//==============================================================================
unsigned char SetPrinter(void) { ExecRadioMenu(LanguageRadioMenu, 1); }
//==============================================================================
unsigned char SetGatewayIP(void) {
	unsigned short CurX;
	unsigned char Loc=0;
	unsigned char Stream[] ="192168110127";
	char FileName[] ="0:desktop/clock/0.bmp";
	unsigned long int i=1, n1=0;
	unsigned char Refresh=1, k, cnt=0;
	unsigned char str[20];

  //G_LoadBMPPart(0, 0, 0, 0, 320, 240-25, "0:desktop/0000.bmp");
	if ((Config.RoutIpAddress[0]<'0') || (Config.RoutIpAddress[0]>'9'))
	{
		sprintf(Config.RoutIpAddress,"192.168.110.001");
		Config.UC=251;
		Config.TransactionsSendInterval=60;
		Config.TrackingInterval=60;
	}
  for (CurX=0; CurX<16; CurX++)
    Stream[CurX] =Config.RoutIpAddress[CurX];

  for (CurX=0; CurX<16; CurX++)
    if ((Stream[CurX]<'0') || (Stream[CurX]>'9'))
      Stream[CurX] ='.';
    
	#ifdef Simorgh50N
  GUI_DrawGradientV(0, TFT_START_Y+0, 272, TFT_START_Y+480, GUI_MAKE_ALPHA(0x00, 0xA02020), GUI_MAKE_ALPHA(0x00, 0x000000));
  GUI_SetColor(RGB(237,237,237));
  GUI_FillRect(20, TFT_START_Y+40, 272-20, TFT_START_Y+350);
  GUI_SetColor(RGB(33,31,32));
  GUI_FillRect(25, TFT_START_Y+45, 272-25, TFT_START_Y+345);
  G_LoadBMP(128,105,"0:desktop/InputBox.bmp",0);
  GUI_SetColor(GUI_WHITE);
  PutText(0,60,270,90,"آدرس دروازه",GUI_TA_CENTER);
  DisplayKeys("", "انصراف", "تاييد");
  LoadFont(FontTahoma20);
  #endif
	#ifdef Torgheh
  GUI_DrawGradientV(0, 30, 219, 172, GUI_MAKE_ALPHA(0x00, 0xA02020), GUI_MAKE_ALPHA(0x00, 0x000000));
  G_LoadBMP(10,85,"0:desktop/InputBox.bmp",0);
  GUI_SetColor(GUI_WHITE);
  PutText(10,40,210,70,"آدرس سرور",GUI_TA_CENTER);
	DisplayKeysTorgheh(""," "," ","");
  #endif

  while (Loc <= 14) {
     switch (ScanKeyboard()) {
       case BUP ://ssssssssssssssssssssssssssssssssssssssssss 
       case 'R':
         if (Stream[Loc]>'0') {
      	   Stream[Loc]--;
					 Refresh=1;
         }
         break;
       case BDOWN:
       case 'L':
         if (Stream[Loc]<'9') {
      	   Stream[Loc]++;
					 Refresh=1;
         }
         break;
    	 #ifdef Torgheh
       case BOK: 
         Loc=15; 
         break;
       case BRIGHT: 
         Loc++;
         if ((Loc==3) || (Loc==7) || (Loc==11)) Loc++;
				 Refresh=1;
         break;
       case BLEFT:
				 if (Loc>0)
				 {	
					 Loc--;
    			 Refresh=1;
				 }
				 break;
			 #endif
     	 #ifdef Simorgh50N
       case BOK: 
       case BF3:
         Loc++;
         if ((Loc==3) || (Loc==7) || (Loc==11)) Loc++;
         if (Loc>14) 
					 break;
				 Refresh=1;
         break;
			 #endif
			 case BCANCEL:	 
    	 #ifdef Simorgh50N
       case BLEFT:
       case BF2:
			 #endif
         LoadFont(FontBKoodak40);
         return 1;
     }
     if (Refresh)
     {
       cnt=0;
			 #ifdef Simorgh50N
       //G_LoadBMP(128,105,"0:desktop/InputBox.bmp",0);
			 
			 GUI_SetColor(RGB(255,255,255));
       GUI_FillRoundedRect(35, 105, 272-35, 140,3);
			 GUI_SetColor(RGB(100,100,120));
       GUI_DrawRoundedFrame(35, 105, 272-35, 140,3,2);			
			 GUI_SetColor(RGB(0,0,0));
       GUI_DrawRoundedFrame(35+1, 105+1, 272-35-1, 140-1,3,1);	
			 
			 for (i=0; i<15; i++)
			 {
         if (i==Loc) GUI_SetColor(GUI_ORANGE);
				 else       GUI_SetColor(GUI_BLACK);
			   GUI_DispCharAt(Stream[i], 50+(i*12) ,TFT_START_Y+110);
			 }
			 #endif

       Refresh=0;
     }
  }

  for (CurX=0; CurX<16; CurX++)
    Config.RoutIpAddress[CurX] =Stream[CurX];
  Config.RoutIpAddress[3] ='.';
  Config.RoutIpAddress[7] ='.';
  Config.RoutIpAddress[11] ='.';
  OkBip(1);  
  LoadFont(FontBKoodak40);

}
//==============================================================================
unsigned char SetSubnetIP(void) {
	unsigned short CurX;
	unsigned char Loc=0;
	unsigned char Stream[] ="192168110127";
	char FileName[] ="0:desktop/clock/0.bmp";
	unsigned long int i=1, n1=0;
	unsigned char Refresh=1, k, cnt=0;
	unsigned char str[20];

  //G_LoadBMPPart(0, 0, 0, 0, 320, 240-25, "0:desktop/0000.bmp");
	if ((Config.SubnetIP[0]<'0') || (Config.SubnetIP[0]>'9'))
	{
		sprintf(Config.SubnetIP,"255.255.255.000");
		Config.UC=251;
		Config.TransactionsSendInterval=60;
		Config.TrackingInterval=60;
	}
  for (CurX=0; CurX<16; CurX++)
    Stream[CurX] =Config.SubnetIP[CurX];

  for (CurX=0; CurX<16; CurX++)
    if ((Stream[CurX]<'0') || (Stream[CurX]>'9'))
      Stream[CurX] ='.';
    
	#ifdef Simorgh50N
  GUI_DrawGradientV(0, TFT_START_Y+0, 272, TFT_START_Y+480, GUI_MAKE_ALPHA(0x00, 0xA02020), GUI_MAKE_ALPHA(0x00, 0x000000));
  GUI_SetColor(RGB(237,237,237));
  GUI_FillRect(20, TFT_START_Y+40, 272-20, TFT_START_Y+350);
  GUI_SetColor(RGB(33,31,32));
  GUI_FillRect(25, TFT_START_Y+45, 272-25, TFT_START_Y+345);
  G_LoadBMP(128,105,"0:desktop/InputBox.bmp",0);
  GUI_SetColor(GUI_WHITE);
  PutText(0,60,270,90,"آدرس ساب نت",GUI_TA_CENTER);
  DisplayKeys("", "انصراف", "تاييد");
  LoadFont(FontTahoma20);
  #endif
	#ifdef Torgheh
  GUI_DrawGradientV(0, 30, 219, 172, GUI_MAKE_ALPHA(0x00, 0xA02020), GUI_MAKE_ALPHA(0x00, 0x000000));
  G_LoadBMP(10,85,"0:desktop/InputBox.bmp",0);
  GUI_SetColor(GUI_WHITE);
  PutText(10,40,210,70,"آدرس سرور",GUI_TA_CENTER);
	DisplayKeysTorgheh(""," "," ","");
  #endif

  while (Loc<=14)
  {
     switch (ScanKeyboard()) 
     {
       case BUP ://ssssssssssssssssssssssssssssssssssssssssss 
       case 'R':
         if (Stream[Loc]>'0')
         {
      	   Stream[Loc]--;
					 Refresh=1;
         }
         break;
       case BDOWN:
       case 'L':
         if (Stream[Loc]<'9')
         {
      	   Stream[Loc]++;
					 Refresh=1;
         }
         break;
    	 #ifdef Torgheh
       case BOK: 
         Loc=15; 
         break;
       case BRIGHT: 
         Loc++;
         if ((Loc==3) || (Loc==7) || (Loc==11)) Loc++;
				 Refresh=1;
         break;
       case BLEFT:
				 if (Loc>0)
				 {	
					 Loc--;
    			 Refresh=1;
				 }
				 break;
			 #endif
     	 #ifdef Simorgh50N
       case BOK: 
       case BF3:
         Loc++;
         if ((Loc==3) || (Loc==7) || (Loc==11)) Loc++;
         if (Loc>14) 
					 break;
				 Refresh=1;
         break;
			 #endif
			 case BCANCEL:	 
    	 #ifdef Simorgh50N
       case BLEFT:
       case BF2:
			 #endif
         LoadFont(FontBKoodak40);
         return 1;
     }
     if (Refresh)
     {
       cnt=0;
			 #ifdef Simorgh50N
			 GUI_SetColor(RGB(255,255,255));
       GUI_FillRoundedRect(35, 105, 272-35, 140,3);
			 GUI_SetColor(RGB(100,100,120));
       GUI_DrawRoundedFrame(35, 105, 272-35, 140,3,2);			
			 GUI_SetColor(RGB(0,0,0));
       GUI_DrawRoundedFrame(35+1, 105+1, 272-35-1, 140-1,3,1);	
			 
			 for (i=0; i<15; i++)
			 {
         if (i==Loc) GUI_SetColor(GUI_ORANGE);
				 else       GUI_SetColor(GUI_BLACK);
			   GUI_DispCharAt(Stream[i], 50+(i*12) ,TFT_START_Y+110);
			 }
			 #endif
			 #ifdef Torgheh
       G_LoadBMP(10,85,"0:desktop/InputBox.bmp",0);
			 for (i=0; i<15; i++)
			 {
         if (i==Loc) 	GUI_SetColor(GUI_ORANGE);
				 else         GUI_SetColor(GUI_BLACK);
			   GUI_DispCharAt(Stream[i], 20+(i*12) ,85);
			 }
			 #endif
       Refresh=0;
     }
  }

  for (CurX=0; CurX<16; CurX++)
    Config.SubnetIP[CurX] =Stream[CurX];
  Config.SubnetIP[3] ='.';
  Config.SubnetIP[7] ='.';
  Config.SubnetIP[11] ='.';
  OkBip(1);  
  LoadFont(FontBKoodak40);

}
//==============================================================================
unsigned char SetServerIP(void) {
	unsigned short CurX;
	unsigned char Loc=0;
	unsigned char Stream[] ="192168110127";
	char FileName[] ="0:desktop/clock/0.bmp";
	unsigned long int i=1, n1=0;
	unsigned char Refresh=1, k, cnt=0;
	unsigned char str[20];

  //G_LoadBMPPart(0, 0, 0, 0, 320, 240-25, "0:desktop/0000.bmp");
	if ((Config.ServerIP[0]<'0') || (Config.ServerIP[0]>'9'))
	{
		sprintf(Config.ServerIP,"192.168.110.002");
		Config.UC=251;
		Config.TransactionsSendInterval=60;
		Config.TrackingInterval=60;
	}
	
  for (CurX=0; CurX<16; CurX++)
    Stream[CurX] =Config.ServerIP[CurX];

  for (CurX=0; CurX<16; CurX++)
    if ((Stream[CurX]<'0') || (Stream[CurX]>'9'))
      Stream[CurX] ='.';
    
	#ifdef Simorgh50N
  GUI_DrawGradientV(0, TFT_START_Y+0, 272, TFT_START_Y+480, GUI_MAKE_ALPHA(0x00, 0xA02020), GUI_MAKE_ALPHA(0x00, 0x000000));
  GUI_SetColor(RGB(237,237,237));
  GUI_FillRect(20, TFT_START_Y+40, 272-20, TFT_START_Y+350);
  GUI_SetColor(RGB(33,31,32));
  GUI_FillRect(25, TFT_START_Y+45, 272-25, TFT_START_Y+345);
  G_LoadBMP(128,105,"0:desktop/InputBox.bmp",0);
  GUI_SetColor(GUI_WHITE);
  PutText(0,60,270,90,"آدرس سرور",GUI_TA_CENTER);
  DisplayKeys("", "انصراف", "تاييد");
  LoadFont(FontTahoma20);
  #endif
	#ifdef Torgheh
  GUI_DrawGradientV(0, 30, 219, 172, GUI_MAKE_ALPHA(0x00, 0xA02020), GUI_MAKE_ALPHA(0x00, 0x000000));
  G_LoadBMP(10,85,"0:desktop/InputBox.bmp",0);
  GUI_SetColor(GUI_WHITE);
  PutText(10,40,210,70,"آدرس سرور",GUI_TA_CENTER);
	DisplayKeysTorgheh(""," "," ","");
  #endif

  while (Loc<=14)
  {
     switch (ScanKeyboard()) 
     {
       case BUP : 
       case 'R'://sssssssssssssssssssssssssssssssssssss
         if (Stream[Loc]>'0')
         {
      	   Stream[Loc]--;
					 Refresh=1;
         }
         break;
       case BDOWN:
       case 'L':
         if (Stream[Loc]<'9')
         {
      	   Stream[Loc]++;
					 Refresh=1;
         }
         break;
    	 #ifdef Torgheh
       case BOK: 
         Loc=15; 
         break;
       case BRIGHT: 
         Loc++;
         if ((Loc==3) || (Loc==7) || (Loc==11)) Loc++;
				 Refresh=1;
         break;
       case BLEFT:
				 if (Loc>0)
				 {	
					 Loc--;
    			 Refresh=1;
				 }
				 break;
			 #endif
     	 #ifdef Simorgh50N
       case BOK: 
       case BF3:
         Loc++;
         if ((Loc==3) || (Loc==7) || (Loc==11)) Loc++;
         if (Loc>14) 
					 break;
				 Refresh=1;
         break;
			 #endif
			 case BCANCEL:	 
    	 #ifdef Simorgh50N
       case BLEFT:
       case BF2:
			 #endif
         LoadFont(FontBKoodak40);
         return 1;
     }
     if (Refresh)
     {
       cnt=0;
			 #ifdef Simorgh50N
			 GUI_SetColor(RGB(255,255,255));
       GUI_FillRoundedRect(35, 105, 272-35, 140,3);
			 GUI_SetColor(RGB(100,100,120));
       GUI_DrawRoundedFrame(35, 105, 272-35, 140,3,2);			
			 GUI_SetColor(RGB(0,0,0));
       GUI_DrawRoundedFrame(35+1, 105+1, 272-35-1, 140-1,3,1);	
			 
			 for (i=0; i<15; i++)
			 {
         if (i==Loc) GUI_SetColor(GUI_ORANGE);
				 else       GUI_SetColor(GUI_BLACK);
			   GUI_DispCharAt(Stream[i], 50+(i*12) ,TFT_START_Y+110);
			 }
			 #endif
			 #ifdef Torgheh
       G_LoadBMP(10,85,"0:desktop/InputBox.bmp",0);
			 for (i=0; i<15; i++)
			 {
         if (i==Loc) 	GUI_SetColor(GUI_ORANGE);
				 else         GUI_SetColor(GUI_BLACK);
			   GUI_DispCharAt(Stream[i], 20+(i*12) ,85);
			 }
			 #endif
       Refresh=0;
     }
  }

  for (CurX=0; CurX<16; CurX++)
    Config.ServerIP[CurX] =Stream[CurX];
  Config.ServerIP[3] ='.';
  Config.ServerIP[7] ='.';
  Config.ServerIP[11] ='.';
  OkBip(1);  
  LoadFont(FontBKoodak40);

}
//==============================================================================
unsigned char SetLocalIP(void) {
	unsigned short CurX;
	unsigned char Loc=0;
	unsigned char Stream[] ="192168110127";
	char FileName[] ="0:desktop/clock/0.bmp";
	unsigned long int i=1, n1=0;
	unsigned char Refresh=1, k, cnt=0;
	unsigned char str[20];

  //G_LoadBMPPart(0, 0, 0, 0, 320, 240-25, "0:desktop/0000.bmp");
	
	if ((Config.LocalIP[0]<'0') || (Config.LocalIP[0]>'9'))
	{
		sprintf(Config.LocalIP,"192.168.110.004");
		Config.UC=251;
		Config.TransactionsSendInterval=60;
		Config.TrackingInterval=60;
	}
  for (CurX=0; CurX<16; CurX++)
    Stream[CurX] =Config.LocalIP[CurX];

  for (CurX=0; CurX<16; CurX++)
    if ((Stream[CurX]<'0') || (Stream[CurX]>'9'))
      Stream[CurX] ='.';
    
	#ifdef Simorgh50N
  GUI_DrawGradientV(0, TFT_START_Y+0, 272, TFT_START_Y+480, GUI_MAKE_ALPHA(0x00, 0xA02020), GUI_MAKE_ALPHA(0x00, 0x000000));
  GUI_SetColor(RGB(237,237,237));
  GUI_FillRect(20, TFT_START_Y+40, 272-20, TFT_START_Y+350);
  GUI_SetColor(RGB(33,31,32));
  GUI_FillRect(25, TFT_START_Y+45, 272-25, TFT_START_Y+345);
  G_LoadBMP(128,105,"0:desktop/InputBox.bmp",0);
  GUI_SetColor(GUI_WHITE);
  PutText(0,60,270,90,"آدرس دستگاه",GUI_TA_CENTER);
  DisplayKeys("", "انصراف", "تاييد");
  LoadFont(FontTahoma20);
  #endif


  while (Loc<=14)
  {
     switch (ScanKeyboard()) 
     {
       case BUP : //sssssssssssssssssssssssssssssssssssss
       case 'R':
         if (Stream[Loc]>'0')
         {
      	   Stream[Loc]--;
					 Refresh=1;
         }
         break;
       case BDOWN:
       case 'L':
         if (Stream[Loc]<'9')
         {
      	   Stream[Loc]++;
					 Refresh=1;
         }
         break;
    	 #ifdef Torgheh
       case BOK: 
         Loc=15; 
         break;
       case BRIGHT: 
         Loc++;
         if ((Loc==3) || (Loc==7) || (Loc==11)) Loc++;
				 Refresh=1;
         break;
       case BLEFT:
				 if (Loc>0)
				 {	
					 Loc--;
    			 Refresh=1;
				 }
				 break;
			 #endif
     	 #ifdef Simorgh50N
       case BOK: 
       case BF3:
         Loc++;
         if ((Loc==3) || (Loc==7) || (Loc==11)) Loc++;
         if (Loc>14) 
					 break;
				 Refresh=1;
         break;
			 #endif
			 case BCANCEL:	 
    	 #ifdef Simorgh50N
       case BLEFT:
       case BF2:
			 #endif
         LoadFont(FontBKoodak40);
         return 1;
     }
     if (Refresh)
     {
       cnt=0;
			 #ifdef Simorgh50N
			 GUI_SetColor(RGB(255,255,255));
       GUI_FillRoundedRect(35, 105, 272-35, 140,3);
			 GUI_SetColor(RGB(100,100,120));
       GUI_DrawRoundedFrame(35, 105, 272-35, 140,3,2);			
			 GUI_SetColor(RGB(0,0,0));
       GUI_DrawRoundedFrame(35+1, 105+1, 272-35-1, 140-1,3,1);	
			 for (i=0; i<15; i++)
			 {
         if (i==Loc) GUI_SetColor(GUI_ORANGE);
				 else       GUI_SetColor(GUI_BLACK);
			   GUI_DispCharAt(Stream[i], 50+(i*12) ,TFT_START_Y+110);
			 }
			 #endif
			 #ifdef Torgheh
       G_LoadBMP(10,85,"0:desktop/InputBox.bmp",0);
			 for (i=0; i<15; i++)
			 {
         if (i==Loc) 	GUI_SetColor(GUI_ORANGE);
				 else         GUI_SetColor(GUI_BLACK);
			   GUI_DispCharAt(Stream[i], 20+(i*12) ,85);
			 }
			 #endif
       Refresh=0;
     }
  }

  for (CurX=0; CurX<16; CurX++)
    Config.LocalIP[CurX] =Stream[CurX];
  Config.LocalIP[3] ='.';
  Config.LocalIP[7] ='.';
  Config.LocalIP[11] ='.';
  OkBip(1);  
  LoadFont(FontBKoodak40);

}
 
//==============================================================================
unsigned char DoClockCalibration(void) {
	FLASH_UNLOCK
  if (GetNumber("!!!حافظه پاک مي شود!!!", 0, 1) != 57)
		return 0;
  ShowMessageDlg(mtError, "آماده سازي حافظه",0,0,0,0);  

	SPI_Flash_Erase_Chip();
	LoadResourceFromSD2DF();	
	FLASH_LOCK
}

//==============================================================================
unsigned char LEDTest(void) {
	unsigned char Key;
	unsigned short Line;
	char s[50];
	
	WDTR;
		RELAY1=1;
    RELAY2=0;
	
			LEDOK=1;
			LEDERR=0;
	GUI_Delay(3000);
			LEDOK=0;
			LEDERR=1;
		RELAY1=0;
    RELAY2=1;	
	WDTR;
	GUI_Delay(3000);
				LEDOK=0;
			LEDERR=0;
		RELAY1=0;
    RELAY2=0;		
	WDTR;
	GUI_Delay(1000);
			LEDOK=1;
			LEDERR=1;
		RELAY1=1;
    RELAY2=1;				
		WDTR;
	GUI_Delay(3000);	
			LEDOK=1;
			LEDERR=1;
		RELAY1=0;
    RELAY2=0;				
		WDTR;
	GUI_Delay(500);	
			LEDOK=0;
			LEDERR=0;
	GUI_Delay(500);
			LEDOK=1;
			LEDERR=1;
			WDTR;
	GUI_Delay(500);	
			LEDOK=0;
			LEDERR=0;
	GUI_Delay(500);	
	WDTR;
}
//==============================================================================
unsigned char KeypadTest(void) {
	unsigned char Key;
	unsigned short Line;
	char s[50];
	
  LoadFont(FontTahoma20);
  GUI_Clear();
	GUI_SetColor(GUI_WHITE);
	while (1) {
		WDTR;
		Key = ScanKeyboard();
		if (Key) {
    	Line += 15;
	    if (Line > 260) {
		    Line = 0;
		    GUI_Clear();
	    }
			sprintf(s, "Key = %d", Key);
	    GUI_DispStringAt(s, 0, Line);
		}
	}
}



