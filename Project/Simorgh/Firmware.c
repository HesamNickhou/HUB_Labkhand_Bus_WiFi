#include "Config.h"
#include "Firmware.h"
#include "flash/flash.h"
#include "rtc/rtc.h"
#include "voice/voice.h"
#include "GUI.h"
#include "menu3d.h"

union {
  unsigned char Buf[512];
  FirmwareStruct Frm;
} FirmwareTransfer;

FirmwareStruct Firmware;

unsigned char 
	CheckUpdateTry 						= 0,
	InFirmwareDownloading 		= 0,
	IsFileForGprsDownload 		= 0,
	Check_ServerForUpdateFile = 0;

unsigned int 
	FirmwarePercent,
	Percent,
	Downloaded_FirmwareLength;

unsigned short FirmwarePacketIndex;

extern unsigned char 
	GlobalBuffer[],
	SendLedTimer,
	RequestType;

extern unsigned short RequestNo;

//==============================================================================
void IntToBytes(unsigned int i, unsigned char *buf) {
	buf[0] =  i 			 & 0xFF;
	buf[1] = (i >>  8) & 0xFF;
	buf[2] = (i >> 16) & 0xFF;
	buf[3] = (i >> 24) & 0xFF;
}

//==============================================================================
void BytesToInt(unsigned int *i, unsigned char *buf) {
	*i  = buf[3]; *i <<= 8;
	*i += buf[2]; *i <<= 8;
	*i += buf[1]; *i <<= 8;
	*i += buf[0];
}
//==============================================================================
unsigned char CheckNewFirmware(void) {
	unsigned long int Idx, i;
	unsigned int Counter,crc=0, page, Loc;
	unsigned short crc2=0;
	unsigned int pageSize;
	unsigned char str[20];
	unsigned int Calculated_CheckSum=0;

 // if (CheckFirmware==0) return 0;
  //............................................................................
  if (Firmware.HaveData != 0xAA)                      return 1;
  if ((Firmware.Length==0) || (Firmware.Length==-1))  return 1;
  //............................................................................  

  RequestNo     = 0; 
  RequestType   = 0;	
  CheckFirmware = 0;

  for (i=0; i<32; i++)
    GlobalBuffer[i] =0;
	
	FLASH_UNLOCK
  SaveFromRamToDF(addFirmwareInfo, 32, GlobalBuffer);
	FLASH_LOCK

  ShowMessageDlg(mtInformation, "بررسي نسخه جديد",1,0,0,0);  

  if (Downloaded_FirmwareLength != Firmware.Length)
  {
    ShowMessageDlg(mtError, "خطا در حجم اطلاعات",1,0,0,0);  
    Firmware.CheckSum=0;
    Downloaded_FirmwareLength=0;		
		GUI_Delay(2000);
    InfoCounter=3;
    return 1;
  }

 //.....................................................................................................  

 crc2=0;
 Calculated_CheckSum=0;

	Loc = 1024;
  for (i = 0; i < Downloaded_FirmwareLength; i++) {
		if (Loc == 1024) {
			WDTR;
			SPI_Flash_Read(GlobalBuffer, i+addFirmware, 1024);
			//for (Idx=0; Idx<1024; Idx++) SendByte(GlobalBuffer[Idx]);
			Loc = 0;
		}
		Calculated_CheckSum += GlobalBuffer[Loc];
		crc2 += GlobalBuffer[Loc];
		Loc++;
	}

 //.....................................................................................................    

 if (Calculated_CheckSum != Firmware.CheckSum) {
    ShowMessageDlg(mtError, "خطا صحت اطلاعات",1,0,0,0);  
    Firmware.CheckSum = 0;
    Downloaded_FirmwareLength = 0;
		GUI_Delay(2000);
    InfoCounter=3;
    return 2;
  }
	
	WDTR;
	
  GlobalBuffer[0] =0xC2;
  GlobalBuffer[1] =(crc2)&0xFF;
  GlobalBuffer[2] =(crc2>>8)&0xFF;
	IntToBytes(Downloaded_FirmwareLength, &GlobalBuffer[3]);
  GlobalBuffer[7] =0x55;
	FLASH_UNLOCK
  SaveFromRamToDF(addFirmwareInfo, 32, GlobalBuffer);
	FLASH_LOCK


	
 //.....................................................................................................  


  ShowMessageDlg(mtInformation, "برنامه جديد دريافت شد",0,0,0,0);  
	GUI_Delay(3000);
	
	
	WWDG_Init(0X7F,80,WWDG_Prescaler_8);
  while (1);
  //ShowPageBEAuto(0);
  return 0;
  
}
//==================================================================================================================================
unsigned char Delete_DownloadedFirmwareFromSD(unsigned char Device)
{
 unsigned int Counter;
 
 for (Counter=0;Counter<512;Counter++)   FirmwareTransfer.Buf[Counter] =0;


 FLASH_UNLOCK
	

 SaveFromRamToDF(addFirmwareInfo,   512,  FirmwareTransfer.Buf); 	

	
 return 0;  
}
//==================================================================================================================================
unsigned char Save_FirmwareProperties(unsigned char _Which_DeviceFirmware)
{

 FirmwareTransfer.Frm =Firmware;
  
 FLASH_UNLOCK 

 SaveFromRamToDF(addFirmwareInfo, 512, FirmwareTransfer.Buf);  
 
 
 return 0;
}
//==================================================================================================================================
unsigned char Load_FirmwareProperties(unsigned char _Which_DeviceFirmware)
{
 unsigned int i;

  LoadFromDFToRam(addFirmwareInfo, 512, FirmwareTransfer.Buf);
	Firmware=FirmwareTransfer.Frm;  


  return 0;
}
//==================================================================================================================================
unsigned char Request_FirmwareNextPacketfromServer(void) 
{
 unsigned char i=0;
 unsigned short crc=0;
 unsigned char SendBuf[15];
 

 
 if (!FirmwarePacketIndex) return 0;
 
 GlobalBuffer[0] =2;
 GlobalBuffer[1] =14;  
 GlobalBuffer[2] =Config.DeviceID%256;
 GlobalBuffer[3] =Config.DeviceID/256;
 GlobalBuffer[4] =3;//Len
 GlobalBuffer[5] =0;//Len

 GlobalBuffer[6] =DeviceType; 	


 GlobalBuffer[7] =FirmwarePacketIndex%256;
 GlobalBuffer[8] =FirmwarePacketIndex/256;
 crc=crc16(0,GlobalBuffer+1, 8);
 GlobalBuffer[9] =crc%256;      
 GlobalBuffer[10] =crc/256;            
 GlobalBuffer[11] =3; 


 SetTX485();
 GUI_Delay(2);
 for (i=0; i<12; i++)
         SendByte(GlobalBuffer[i]);
 GUI_Delay(5);
 SetRX485();
 
 //GUI_SetColor(GUI_GREEN);       		 GUI_FillCircle(15,15,5);
 //WIFISend(13,GlobalBuffer);
  
}
//==================================================================================================================================
unsigned char Request_FirmwareNextPacketfromServer512Byte(void) 
{
 unsigned char i=0;
 unsigned short crc=0;
 unsigned char SendBuf[15];
 

 
 if (!FirmwarePacketIndex) return 0;
 
 GlobalBuffer[0] =2;
 GlobalBuffer[1] =14;  
 GlobalBuffer[2] =Config.DeviceID%256;
 GlobalBuffer[3] =Config.DeviceID/256;
 GlobalBuffer[4] =4;//Len
 GlobalBuffer[5] =0;//Len
 GlobalBuffer[6] =99; 

 GlobalBuffer[7] =DeviceType; 	


 GlobalBuffer[8] =FirmwarePacketIndex%256;
 GlobalBuffer[9] =FirmwarePacketIndex/256;
 crc=crc16(0,GlobalBuffer+1, 9);
 GlobalBuffer[10] =crc%256;      
 GlobalBuffer[11] =crc/256;            
 GlobalBuffer[12] =3; 
 
 
 #ifdef WithWIFI
 WIFISend(13,GlobalBuffer); 
 #endif
 
 //SendData(13); 
}
//==================================================================================================================================
void Request_UploadedFirmwaresPropertiesFromServer(void)
{
unsigned char i=0;
unsigned short crc=0;
unsigned char SendBuf[15];


	
 GlobalBuffer[0] =2;
 GlobalBuffer[1] =14;  
 GlobalBuffer[2] =Config.DeviceID%256;
 GlobalBuffer[3] =Config.DeviceID/256;
 GlobalBuffer[4] =3;//Len
 GlobalBuffer[5] =0;//Len

 GlobalBuffer[6] =DeviceType; 	

 
 GlobalBuffer[7] =0;
 GlobalBuffer[8] =0;
 crc=crc16(0, GlobalBuffer+1, 8);
 GlobalBuffer[9] =crc%256;      
 GlobalBuffer[10] =crc/256;            
 GlobalBuffer[11] =3; 

// DataSent=0; 

SetTX485();
GUI_Delay(2);
    for (i=0; i<12; i++)
         SendByte(GlobalBuffer[i]);
GUI_Delay(5);
  SetRX485();
	
	//GUI_SetColor(GUI_BLUE);       		 GUI_FillCircle(15,15,5);
 FirmwarePacketIndex=1;
}

//==================================================================================================================================
void Request_UploadedFirmwaresPropertiesFromServer512Byte(void) {
	unsigned char i=0;
	unsigned short crc=0;
	unsigned char SendBuf[15];

	GlobalBuffer[0] =2;
	GlobalBuffer[1] =14;  
	GlobalBuffer[2] =Config.DeviceID%256;
	GlobalBuffer[3] =Config.DeviceID/256;
	GlobalBuffer[4] =4;//Len
	GlobalBuffer[5] =0;//Len
	GlobalBuffer[6] =99;

	GlobalBuffer[7] =DeviceType;

	GlobalBuffer[8] =0;
	GlobalBuffer[9] =0;
	crc=crc16(0, GlobalBuffer+1, 9);
	GlobalBuffer[10] =crc%256;
	GlobalBuffer[11] =crc/256;
	GlobalBuffer[12] =3; 

// DataSent=0; 

	#ifdef WithWIFI
		WIFISend(13,GlobalBuffer); 
	#endif
	//SendData(13);
	FirmwarePacketIndex=1;
}

//===================================================================================================
unsigned char Download_NewFirmwares(void) {
//	printf("Download_NewFirmwares(%d)",Check_ServerForUpdateFile);
	GUI_SetColor(GUI_WHITE);       		 GUI_FillCircle(15,15,5);
	if ((Check_ServerForUpdateFile) && (!IsFileForGprsDownload)) 
		return 0;
	
	if (!Check_ServerForUpdateFile) {
		#if (DeviceType == BUSDOOR)
			Request_UploadedFirmwaresPropertiesFromServer();
		#else
			Request_UploadedFirmwaresPropertiesFromServer512Byte();
		#endif
	}	

	if ((Check_ServerForUpdateFile) && (IsFileForGprsDownload)) {
		InFirmwareDownloading = 0x28;
    if (Firmware.HaveData == 0xAA) {
			#if (DeviceType == BUSDOOR)
				Request_FirmwareNextPacketfromServer();
			#else
				Request_FirmwareNextPacketfromServer512Byte();
			#endif
		}
    else {
			InFirmwareDownloading = 0;
			IsFileForGprsDownload = 0;
		}
	}
	return 1;
}
//==================================================================================================================================
unsigned char Check_AndSaveDownloadedFirmware(void) {
	unsigned char Byte=0,SD_Buffer[512];
	unsigned long int i;
	unsigned long int LastAddress, Address, Backup_Sector, Firmware_Sector,SerialChip;
	unsigned long int Counter;
	unsigned char Error=0;
	unsigned int  Calculated_CheckSum=0,Loc=0,Idx=0;
	unsigned short crc2=0;

	
 //Downloaded_FirmwareLength=221952;

	#if (DeviceType==BCU)	
  FirmwarePacketIndex=1;

	

	

	
  if (Which_DeviceFirmware==BCU)     UpdateFile[0].ReadyForDownload=3; //downloaded with error
  if (Which_DeviceFirmware==BD90)    UpdateFile[1].ReadyForDownload=3; //downloaded with error  
  if (Which_DeviceFirmware==ALEF)    UpdateFile[2].ReadyForDownload=3; //downloaded with error  
	

  //...................................................................................................

	ShowMessageDlg(mtInformation, "بررسي نسخه جديد",0);  
	
  if (Which_DeviceFirmware==BCU)
    {
     for (i=2; i<17; i++)
        {
         if (UpdateFile[0].FirmwareName[i-2] ==0) break;
        }      
     if (Downloaded_FirmwareLength != UpdateFile[0].SpecifiedLength) Error=1;
    }
  
  
  
  if (Which_DeviceFirmware==BD90)
    {
     for (i=2; i<17; i++)
        {
         if (UpdateFile[1].FirmwareName[i-2] ==0) break;
        }       
     if (Downloaded_FirmwareLength != UpdateFile[1].SpecifiedLength) Error=1;
    }
  
  if (Which_DeviceFirmware==ALEF)
    {
     for (i=2; i<17; i++)
        {
         if (UpdateFile[2].FirmwareName[i-2] ==0) break;
        }       
     if (Downloaded_FirmwareLength != UpdateFile[2].SpecifiedLength) Error=1;
    }  
  

  
  if (Error==1)
    {
     ShowMessageDlg(mtError, "خطا در نسخه دانلودي - سايز",0);  			
			
     GUI_SetColor(GUI_BLACK);
     GUI_FillRect(18, 6, 300, 28);
     GUI_SetColor(GUI_CYAN);
     sprintf(SD_Buffer,"%d - %d - %d",Downloaded_FirmwareLength,UpdateFile[0].SpecifiedLength,Which_DeviceFirmware);
     PutText(20, 0, 300, 28, SD_Buffer, GUI_TA_CENTER);
			
     if (Which_DeviceFirmware==BCU) 
		 {
			Firmware.CheckSum=0;

 		  for (i=0; i<512; i++)          GlobalBuffer[i] =0;
			
	    FLASH_UNLOCK
      SaveFromRamToDF(addFirmwareInfo, 512, GlobalBuffer);
	    //FLASH_LOCK
		  
		
		 }
		 else                          Firmware2.CheckSum=0;
			
     FirmwarePacketIndex=1;  
     InfoCounter=2;
     Downloaded_FirmwareLength=0;
     ErrorBip(1);
			
		 while (1)
		      {			 
		       WDTR;	
	         GUI_Delay(1000);
		      }
					
     InfoCounter=3;
     return 1;
    }
  

  //.....................................................................................................  
  crc2=0;
  Calculated_CheckSum=0;

	Loc=1024;
  for (i=0; i<Downloaded_FirmwareLength; i++)
	{
   if (Loc==1024)
		{
			WDTR;
			if (Which_DeviceFirmware==BCU)       SPI_Flash_Read(GlobalBuffer, i+addFirmware, 1024);
			else if (Which_DeviceFirmware==ALEF) SPI_Flash_Read(GlobalBuffer, i+addDoorsFirmware+512, 1024);
			else if (Which_DeviceFirmware==BD90) SPI_Flash_Read(GlobalBuffer, i+addDoorsFirmware2+512, 1024);
			
			//for (Idx=0; Idx<1024; Idx++)    SendByte(GlobalBuffer[Idx]);
			Loc=0;
		}
	  //SendByte(GlobalBuffer[Loc]);
		Calculated_CheckSum += GlobalBuffer[Loc];
		crc2 += GlobalBuffer[Loc];
		Loc++;
	}

// printf("\n\rCalculated_CheckSum=%d,BCUcheck=%d,BD90check=%d,Downloaded_FirmwareLength=%d",Calculated_CheckSum,UpdateFile[0].SpecifiedCheckSum,UpdateFile[1].SpecifiedCheckSum,Downloaded_FirmwareLength);
 //.....................................................................................................    
  Error=0;
	if (Which_DeviceFirmware==BCU)
    {
     if (Calculated_CheckSum != UpdateFile[0].SpecifiedCheckSum) Error=1;
    }
  if (Which_DeviceFirmware==BD90)
    {
     if (Calculated_CheckSum != UpdateFile[1].SpecifiedCheckSum) Error=1;
    }
  if (Which_DeviceFirmware==ALEF)
    {
     if (Calculated_CheckSum != UpdateFile[2].SpecifiedCheckSum) Error=1;
    }
 
 if (Error==1)
   {            
		ShowMessageDlg(mtError, "خطا در نسخه دانلودي - صحت",0);  
		
    GUI_SetColor(GUI_BLACK);
    GUI_FillRect(18, 6, 300, 28);
    GUI_SetColor(GUI_CYAN);
    sprintf(SD_Buffer,"%d - %d - %d",Calculated_CheckSum,UpdateFile[0].SpecifiedCheckSum,Which_DeviceFirmware);
    PutText(20, 0, 300, 28, SD_Buffer, GUI_TA_CENTER);
		 
    if (Which_DeviceFirmware==BCU) 
		 {
			Firmware.CheckSum=0;

 		  for (i=0; i<512; i++)          GlobalBuffer[i] =0;
			
	    FLASH_UNLOCK
      SaveFromRamToDF(addFirmwareInfo, 512, GlobalBuffer);
	    //FLASH_LOCK
		  
		
		 }
		else                          Firmware2.CheckSum=0;
		
    InfoCounter=2;		 
    FirmwarePacketIndex=1;
    Downloaded_FirmwareLength=0;
    ErrorBip(1);
		 while (1)
		 {			 
		 WDTR;	
	   GUI_Delay(1000);
		 }
    InfoCounter=3;
    return 1;
   }
   

 //.....................................................................................................  
  if (Which_DeviceFirmware==BCU)
    {
     for (i=0; i<16; i++)      Firmware.FirmwareFileName[i] =UpdateFile[0].FirmwareName[i];
     Firmware.FirmwareFileName[i] =0;
    }
  if (Which_DeviceFirmware==BD90)
    {
     for (i=0; i<16; i++)      Firmware2.FirmwareFileName[i] =UpdateFile[1].FirmwareName[i];
     Firmware2.FirmwareFileName[i] =0;
    }
  if (Which_DeviceFirmware==ALEF)
    {
     for (i=0; i<16; i++)      Firmware2.FirmwareFileName[i] =UpdateFile[2].FirmwareName[i];
     Firmware2.FirmwareFileName[i] =0;
    }
  
  Firmware.FirmwareFileName[i] =0;  
  Firmware.Length=Downloaded_FirmwareLength;
  Firmware.CheckSum=Calculated_CheckSum;

  //....................................................................................................
  if (Which_DeviceFirmware==BCU)     UpdateFile[0].ReadyForDownload=2; //downloaded without error
  if (Which_DeviceFirmware==BD90)    UpdateFile[1].ReadyForDownload=2; //downloaded without error  
  if (Which_DeviceFirmware==ALEF)    UpdateFile[2].ReadyForDownload=2; //downloaded without error   
  
  //Firmware.HaveData=0xaa;
		

  if (Which_DeviceFirmware==BCU) 
	{		
  GlobalBuffer[0] =0xC2;
  GlobalBuffer[1] =(crc2)&0xFF;
  GlobalBuffer[2] =(crc2>>8)&0xFF;
	IntToBytes(Downloaded_FirmwareLength, &GlobalBuffer[3]);
  GlobalBuffer[7] =0x55;
		
	FLASH_UNLOCK
  SaveFromRamToDF(addFirmwareInfo, 512, GlobalBuffer);
	//FLASH_LOCK
	}
	else Save_FirmwareProperties(Which_DeviceFirmware);
 //.....................................................................................................  


  OkBip(1);
  GUI_Delay(100);
		
   
  #endif
  return 0;    
}

//==========================================================================
void ProcessFirmwarePacket(void)
{
char Str_Temp[50];
unsigned short i,KK=0,Counter,k,j;

static unsigned int Last_Index=0;
unsigned int Data_Length=0,Percent,Sound_WritableSector;
unsigned long int Sector=0,Address=0;
unsigned short Idx, DataIndex, DataLen;

unsigned char str[512];

unsigned short New_Ver;
unsigned short New_Release;

 #if (DeviceType==BCU)
 //............................................................................................................
 //if (GlobalBuffer[6] != BCU) return 0;
 
 WDTR;

  if ((FirmwarePacketIndex==(GlobalBuffer[7]+GlobalBuffer[8]*256)) || ((GlobalBuffer[7]+GlobalBuffer[8]*256)==0))
    {
     if ((GlobalBuffer[7]+GlobalBuffer[8]*256)==0)
       {
        FirmwarePacketIndex=0;

        //Load_FirmwareProperties(BCU);
        //Firmware.SavedVer=Ver;
        //Firmware.SavedRelease=Release;
        //Save_FirmwareProperties(BCU);


        i=9;
        IsFileForGprsDownload=0;
        UpdateFile[0].ReadyForDownload=0;
        UpdateFile[1].ReadyForDownload=0;
        UpdateFile[2].ReadyForDownload=0;

        if (BufferLen>40)
          {
           for (Counter=0;Counter<(BufferLen-3);Counter += 43)
              {
               if (GlobalBuffer[Counter+i] ==BCU)
                 {
                  k=0;
                  Load_FirmwareProperties(BCU);
                 }
               else if (GlobalBuffer[Counter+i] ==BD90)
                 {
                  k=1;
                  Load_FirmwareProperties(BD90);
                 }
               else if (GlobalBuffer[Counter+i] ==ALEF)
                 {
                  k=2;
                  Load_FirmwareProperties(ALEF);
                 }               
               else
                 {
                  continue;
                 }

               
     
               New_Ver=GlobalBuffer[i+Counter+1]+GlobalBuffer[i+Counter+2]*256;
               New_Release=GlobalBuffer[i+Counter+3]+GlobalBuffer[i+Counter+4]*256;
               
               
              
               if (((GlobalBuffer[Counter+i] ==BD90) && (New_Ver != 0)) || ((GlobalBuffer[Counter+i] ==ALEF) && (New_Ver != 0)) || (New_Ver>Ver) || ((New_Release>Release) && (New_Ver==Ver)))
                 {
                  IsFileForGprsDownload++;
                  UpdateFile[k].ReadyForDownload=1;
                  UpdateFile[k].NewVer=New_Ver;                                  
                  UpdateFile[k].NewRelease=New_Release;
                  UpdateFile[k].SpecifiedLength=GlobalBuffer[i+Counter+5];
                  UpdateFile[k].SpecifiedLength += GlobalBuffer[i+Counter+6]*256;
                  UpdateFile[k].SpecifiedLength += GlobalBuffer[i+Counter+7]*256*256;
                  UpdateFile[k].SpecifiedLength += GlobalBuffer[i+Counter+8]*256*256*256;
                  UpdateFile[k].SpecifiedCheckSum=GlobalBuffer[i+Counter+9];
                  UpdateFile[k].SpecifiedCheckSum += GlobalBuffer[i+Counter+10]*256;
                  UpdateFile[k].SpecifiedCheckSum += GlobalBuffer[i+Counter+11]*256*256;
                  UpdateFile[k].SpecifiedCheckSum += GlobalBuffer[i+Counter+12]*256*256*256;
                
                  for (j=0;j<30;j++)  UpdateFile[k].FirmwareName[j] =GlobalBuffer[i+Counter+13+j];
                  UpdateFile[k].FirmwareName[j] =0;
                  FirmwarePacketIndex=1;
									 
									 
									 
									if (GlobalBuffer[Counter+i] ==BCU)
									  {
									
											
										 FLASH_UNLOCK	
										 SPI_Flash_Erase_Sector(0);
  									 //FLASH_LOCK
											
            				 BytesToInt(&Firmware.Length, &GlobalBuffer[5+Counter+i]);
					           BytesToInt(&Firmware.CheckSum, &GlobalBuffer[9+Counter+i]);

                     for (j=0; j<15; j++)
                         Firmware.FirmwareFileName[j] =GlobalBuffer[j+13+Counter+i];
					
                     //RequestNo=1;  
                     GlobalBuffer[1+Counter+i] =0xE2;
                     GlobalBuffer[2+Counter+i] =1%256;
                     GlobalBuffer[3+Counter+i] =1/256;
                     GlobalBuffer[4+Counter+i] =0;
                     GlobalBuffer[5+Counter+i] =0;
                     GlobalBuffer[6+Counter+i] =0;
                     GlobalBuffer[7+Counter+i] =0;
                     GlobalBuffer[8+Counter+i] =0xE9;
  				           
										 FLASH_UNLOCK
                     SaveFromRamToDF(addFirmwareInfo, 512, &GlobalBuffer[1+Counter+i]);
					           //FLASH_LOCK
					
										}
										
                  //printf("\n\r*****Dev=%d,Length=%d,k=%d",GlobalBuffer[Counter+i],UpdateFile[k].SpecifiedLength,k);  
                 }
               else
                 {
                   UpdateFile[k].ReadyForDownload=0;
                 }
             }

            
          }
          // printf("\n\rConfig.Firmware_CheckHour=%d",Config.Firmware_CheckHour);                                                         
           //printf("\n\rFinish update download!!!");                                       
           //printf("\n\rIsFileForGprsDownload=%d",IsFileForGprsDownload);                  
           
           Check_ServerForUpdateFile=1;
           //SaveConfiguration();
           if (IsFileForGprsDownload) 
             {
              KK=255;
              if (UpdateFile[2].ReadyForDownload==1)        KK=2;
              else if (UpdateFile[1].ReadyForDownload==1)   KK=1;
              else if (UpdateFile[0].ReadyForDownload==1)   KK=0;  
              /*
              if ((Indicators.LastFirmwareWhichDeviceFirmware != 255) && (KK<3))
                {
                 if (((Indicators.LastFirmwareWhichDeviceFirmware==BD90) && (KK==1)) || ((Indicators.LastFirmwareWhichDeviceFirmware==ALEF) && (KK==2)) || ((Indicators.LastFirmwareWhichDeviceFirmware != BD90) && (Indicators.LastFirmwareWhichDeviceFirmware != ALEF)))
                 {
                  if ((Indicators.LastFirmwareSpecifiedLength==UpdateFile[KK].SpecifiedLength) && (Indicators.LastFirmwareSpecifiedCheckSum==UpdateFile[KK].SpecifiedCheckSum)&&
                    (Indicators.LastFirmwareSpecifiedVer==UpdateFile[KK].NewVer) && (Indicators.LastFirmwareSpecifiedRelease==UpdateFile[KK].NewRelease))
                     {
                      Downloaded_FirmwareLength=Indicators.LastFirmwareDownloadedLength;  
                      FirmwarePacketIndex=Indicators.LastFirmwareFirmwarePacketIndex;                    
                     }
                 }
               }*/
              Download_NewFirmwares();  
             }
          }

         else if (FirmwarePacketIndex)
				 {
									
                 if ((Which_DeviceFirmware != BCU) && (Which_DeviceFirmware != BD90) && (Which_DeviceFirmware != ALEF))  return;
                 if ((GlobalBuffer[6] != BCU) && (GlobalBuffer[6] != BD90) && (GlobalBuffer[6] != ALEF))  return;

                
                 if (FirmwarePacketIndex==1)
                   { 
                    if (Which_DeviceFirmware==BCU)        KK=0;
                    else if (Which_DeviceFirmware==BD90)  KK=1;
                    else if (Which_DeviceFirmware==ALEF)  KK=2;
                 
                    Indicators.LastFirmwareWhichDeviceFirmware=Which_DeviceFirmware;
                    Indicators.LastFirmwareSpecifiedLength=UpdateFile[KK].SpecifiedLength;
                    Indicators.LastFirmwareSpecifiedCheckSum=UpdateFile[KK].SpecifiedCheckSum;
                    Indicators.LastFirmwareSpecifiedVer=UpdateFile[KK].NewVer; 
                    Indicators.LastFirmwareSpecifiedRelease=UpdateFile[KK].NewRelease;
                    Indicators.LastFirmwareDownloadedLength=0;  
                    Indicators.LastFirmwareFirmwarePacketIndex=1;
                    SaveIndicators();
                   }
                  
 
                 if (BufferLen<8) return;
                 if (BufferLen>=8)
                 Data_Length=(GlobalBuffer[4]+GlobalBuffer[5]*256)-3;
                 if (Data_Length>=2048)  return;
               //  printf("\n\rGlobalBuffer[6] =%d,FirmwarePacketIndex=%d,Data_Length=%d",GlobalBuffer[6],FirmwarePacketIndex,Data_Length);
           //.................................................................... 
					
                 if ((Data_Length==0) && (FirmwarePacketIndex==1)) 
                   {
                     if (Which_DeviceFirmware==ALEF) UpdateFile[2].ReadyForDownload=0;
                     if (Which_DeviceFirmware==BD90) UpdateFile[1].ReadyForDownload=0;
                     if (Which_DeviceFirmware==BCU)
                       {
                        BD90_UpdateFirmwareFlag=0;
                        InFirmwareDownloading=0;
                        IsFileForGprsDownload=0;
                        UpdateFile[0].ReadyForDownload=0; 
                       }
                     if (IsFileForGprsDownload) IsFileForGprsDownload--;
                     return;
                   }                   
                 if (Data_Length==0)
                   {
                    Indicators.LastFirmwareWhichDeviceFirmware=255;
                    Indicators.LastFirmwareSpecifiedLength=0;
                    Indicators.LastFirmwareSpecifiedCheckSum=0;
                    Indicators.LastFirmwareSpecifiedVer=0; 
                    Indicators.LastFirmwareSpecifiedRelease=0;
                    Indicators.LastFirmwareDownloadedLength=0;  
                    Indicators.LastFirmwareFirmwarePacketIndex=0;
                    SaveIndicators();
                       
                    if (!Check_AndSaveDownloadedFirmware())
                      {

                       	ShowMessageDlg(mtInformation, "نسخه جديد ذخيره گرديد",0); 
                       //Noskhe jadidi zakhire gardid                      

					
                   
                       Downloaded_FirmwareLength=0;
                       BD90_UpdateFirmwareFlag=0;
                       //printf("\n\rWhich_DeviceFirmware=%d",Which_DeviceFirmware);

                       if (Which_DeviceFirmware==BCU)
                         {
                         	WWDG_Init(0X7F,80,WWDG_Prescaler_8);
                          while (1);                
                         }
                          
                       OkBip(1);
                       GUI_Delay(2000);
                       ShowPageBusController();
                      }
                     ShowPageBusController();
                     return;
                    }    


  	             FLASH_UNLOCK               
              	 if (Which_DeviceFirmware==BCU)      
								   {
  									if (SaveFromRamToDF(addFirmware+((FirmwarePacketIndex)-1)*512, 512, GlobalBuffer+9))  
									     SaveFromRamToDF(addFirmware+((FirmwarePacketIndex)-1)*512, 512, GlobalBuffer+9); 
									 }
			           else if (Which_DeviceFirmware==ALEF) SaveFromRamToDF(addDoorsFirmware+512+((FirmwarePacketIndex)-1)*512, 512, GlobalBuffer+9);
			           else if (Which_DeviceFirmware==BD90) SaveFromRamToDF(addDoorsFirmware2+512+((FirmwarePacketIndex)-1)*512, 512, GlobalBuffer+9);
                 //FLASH_LOCK
										
                 Downloaded_FirmwareLength += Data_Length;




									// SetTX485();printf("\n\rDownloaded_FirmwareLength=%d",Downloaded_FirmwareLength);SetRX485();
                  // if (!ExtraStatus) 
                    {   

                     if (Which_DeviceFirmware==BCU)
                       {
                        if (UpdateFile[0].SpecifiedLength>=Downloaded_FirmwareLength) 
                          {
                           k=((Downloaded_FirmwareLength*100)/UpdateFile[0].SpecifiedLength);
		                       if (InfoCounter==0)
		                         {
                              GUI_SetColor(GUI_BLACK);
		                          GUI_FillRect(18, 6, 160, 28);
															
                              GUI_SetColor(GUI_CYAN);
                              sprintf(str,"برنامه: %d%%",k);
															//sprintf(str,"%d - %d - %d",Downloaded_FirmwareLength,UpdateFile[0].SpecifiedLength,Which_DeviceFirmware);
                              PutText(20, 0, 160, 28, str, GUI_TA_CENTER);
		                         }
                          }
                       }
                     
                     if (Which_DeviceFirmware==BD90)
                       {
                        if (UpdateFile[1].SpecifiedLength>=Downloaded_FirmwareLength) 
                          {
                           k=((Downloaded_FirmwareLength*100)/UpdateFile[1].SpecifiedLength);
		                       if (InfoCounter==0)
		                         {
                              GUI_SetColor(GUI_BLACK);
		                          GUI_FillRect(18, 6, 160, 28);
                              GUI_SetColor(GUI_CYAN);
                              sprintf(str,"برنامه: %d%%",k);
                              PutText(20, 0, 160, 28, str, GUI_TA_CENTER);
		                         }
                          }                        

                       }
                       
                     if (Which_DeviceFirmware==ALEF)
                       {
                        if (UpdateFile[2].SpecifiedLength>=Downloaded_FirmwareLength) 
                          {
                           k=((Downloaded_FirmwareLength*100)/UpdateFile[2].SpecifiedLength);
		                       if (InfoCounter==0)
		                         {
                              GUI_SetColor(GUI_BLACK);
		                          GUI_FillRect(18, 6, 180, 28);
                              GUI_SetColor(GUI_CYAN);
                              sprintf(str,"برنامه درب: %d%%",k);
                              PutText(20, 0, 180, 28, str, GUI_TA_CENTER);
		                         }
                          }                        

                       }                    

                    } 
                  FirmwarePacketIndex++;  

                  Request_FirmwareNextPacketfromServer512Byte();
                 
									if (Which_DeviceFirmware==BCU)
									  {
                     LoadFromDFToRam(addFirmwareInfo, 512, GlobalBuffer);
                     GlobalBuffer[0] =0xE2;
                     GlobalBuffer[1] =FirmwarePacketIndex%256;
                     GlobalBuffer[2] =FirmwarePacketIndex/256;
			               IntToBytes(Downloaded_FirmwareLength, &GlobalBuffer[3]);
                     GlobalBuffer[7] =0xE9;
                     SaveFromRamToDF(addFirmwareInfo, 512, GlobalBuffer);
									  }
										
                  Indicators.LastFirmwareDownloadedLength += Data_Length;  
                  Indicators.LastFirmwareFirmwarePacketIndex=FirmwarePacketIndex;                  
                  SaveIndicators();
                }
         }
#endif
 }
