#include "flash.h" 
#include "Config.h" 
#include "spi/spi.h"
#include "crc/crc16.h"


u16 SPI_FLASH_TYPE=W25Q64;

//=============================================================================
void SPI_Flash_Init(void)
{	
GPIO_InitTypeDef GPIO_InitStructure;

	RCC_APB2PeriphClockCmd(	RCC_APB2Periph_GPIOB|RCC_APB2Periph_GPIOD|RCC_APB2Periph_GPIOG, ENABLE );
  /*
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_12; 
 	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
 	GPIO_Init(GPIOB, &GPIO_InitStructure);
 	GPIO_SetBits(GPIOB,GPIO_Pin_12);

	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_2;  
 	GPIO_Init(GPIOD, &GPIO_InitStructure);
 	GPIO_SetBits(GPIOD,GPIO_Pin_2);

	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_7;   
 	GPIO_Init(GPIOG, &GPIO_InitStructure);
 	GPIO_SetBits(GPIOG,GPIO_Pin_7);
	*/
	
  #ifdef Torgheh
	SPI_FLASH2_CS=1;
	#endif
	
	SPI2_Init();		   	
	SPI2_SetSpeed(SPI_BaudRatePrescaler_2);
	SPI_FLASH_TYPE=SPI_Flash_ReadID(); 

}  

//=============================================================================
u8 SPI_Flash_ReadSR(void)   
{  
	u8 byte=0;   
	SPI_FLASH_CS=0;                             
	SPI2_ReadWriteByte(W25X_ReadStatusReg);       
	byte=SPI2_ReadWriteByte(0Xff);           
	SPI_FLASH_CS=1;                          
	return byte;   
} 

//=============================================================================
void SPI_FLASH_Write_SR(u8 sr)   
{   
	SPI_FLASH_CS=0;                            
	SPI2_ReadWriteByte(W25X_WriteStatusReg);    
	SPI2_ReadWriteByte(sr);             
	//SPI2_ReadWriteByte(0);             
	SPI_FLASH_CS=1;                     
}   

//=============================================================================
void SPI_FLASH_Write_Enable(void)   
{
	SPI_FLASH_CS=0;                     
    SPI2_ReadWriteByte(W25X_WriteEnable); 
	SPI_FLASH_CS=1;                         
} 

//=============================================================================
void SPI_FLASH_Write_Disable(void)   
{  
	SPI_FLASH_CS=0;                         
    SPI2_ReadWriteByte(W25X_WriteDisable); 
	SPI_FLASH_CS=1;                          
} 			    

//=============================================================================
u16 SPI_Flash_ReadID(void)
{
	u16 Temp = 0;	  
	SPI_FLASH_CS=0;				    
	SPI2_ReadWriteByte(0x90);
	SPI2_ReadWriteByte(0x00); 	    
	SPI2_ReadWriteByte(0x00); 	    
	SPI2_ReadWriteByte(0x00); 	 			   
	Temp|=SPI2_ReadWriteByte(0xFF)<<8;  
	Temp|=SPI2_ReadWriteByte(0xFF);	 
	SPI_FLASH_CS=1;				    
	return Temp;
}   		    

//=============================================================================
void SPI_Flash_Read(u8* pBuffer,u32 ReadAddr,u16 NumByteToRead)   
{ 
 	u16 i;    												    
	SPI_FLASH_CS=0;                         
    SPI2_ReadWriteByte(W25X_ReadData);    
    SPI2_ReadWriteByte((u8)((ReadAddr)>>16)); 
    SPI2_ReadWriteByte((u8)((ReadAddr)>>8));   
    SPI2_ReadWriteByte((u8)ReadAddr);   
    for(i=0;i<NumByteToRead;i++)
	{ 
        pBuffer[i]=SPI2_ReadWriteByte(0XFF);  
    }
	SPI_FLASH_CS=1;                           
}  

//=============================================================================
unsigned char SPI_Flash_Write_Page(u8* pBuffer,u32 WriteAddr,u16 NumByteToWrite)
{
 	u16 i;  
    SPI_FLASH_Write_Enable();               
	SPI_FLASH_CS=0;                           
    SPI2_ReadWriteByte(W25X_PageProgram);   
    SPI2_ReadWriteByte((u8)((WriteAddr)>>16)); 
    SPI2_ReadWriteByte((u8)((WriteAddr)>>8));   
    SPI2_ReadWriteByte((u8)WriteAddr);   
    for(i=0;i<NumByteToWrite;i++) SPI2_ReadWriteByte(pBuffer[i]);
	SPI_FLASH_CS=1;                   
	return(SPI_Flash_Wait_Busy());					  
} 

//=============================================================================
void SPI_Flash_Write_NoCheck(u8* pBuffer,u32 WriteAddr,u16 NumByteToWrite)   
{ 			 		 
	u16 pageremain;	   
	pageremain=256-WriteAddr%256; 
	if(NumByteToWrite<=pageremain)pageremain=NumByteToWrite;
	while(1)
	{	   
		SPI_Flash_Write_Page(pBuffer,WriteAddr,pageremain);
		if(NumByteToWrite==pageremain)break;
	 	else 
		{
			pBuffer+=pageremain;
			WriteAddr+=pageremain;	

			NumByteToWrite-=pageremain;			 
			if(NumByteToWrite>256)pageremain=256;
			else pageremain=NumByteToWrite; 	  
		}
	};	    
} 

//=============================================================================
u8 SPI_FLASH_BUF[4096];

//=============================================================================
unsigned char SaveMirror(u32 WriteAddr, u8* pBuffer)
{

}

//=============================================================================
unsigned char SaveMirrorSector(u32 WriteAddr)
{

	
}

//=============================================================================
unsigned char CheckSector(u32 Sector, u16 Offset, u8 EmptyCheck)
{
unsigned short crc, i, j;
	
unsigned int CardID;	
unsigned short Year;
unsigned char Month, Day, Hour, Min;	
	
	
	/*

	SPI_Flash_Read(SPI_FLASH_BUF,Sector*4096,4096);
	
	printf("\n\rFirst:\n\r");
	for(i=0; i<4096; i++)
	  printf("%X ", SPI_FLASH_BUF[i]);

	for(i=0; i<4096; i+=256)
	{
  	printf("\n\rRecords %d:", i);
		j=i;
      CardID=SPI_FLASH_BUF[j+3]; CardID<<=8;
      CardID+=SPI_FLASH_BUF[j+2]; CardID<<=8;
      CardID+=SPI_FLASH_BUF[j+1];
      Year=(SPI_FLASH_BUF[j+5]>>4)+1390;
      Month=SPI_FLASH_BUF[j+5]&0x0F;
      Day=SPI_FLASH_BUF[j+6]>>3;
      Hour=SPI_FLASH_BUF[j+7]>>3;
      Min=((SPI_FLASH_BUF[j+7]&0x07)<<3)+(SPI_FLASH_BUF[j+6]&0x07);
    printf("CardID: %d %d/%d/%d %d:%d",CardID,Year,Month,Day,Hour,Min);

	}
	*/

	SPI_Flash_Read(SPI_FLASH_BUF,Sector*4096,4096);
	
	for(i=0; i<Offset; i+=256)
	{
 		//printf("\n\rCheck %d",i);
    crc=crc16(0, SPI_FLASH_BUF+i, 256-6);
		j=SPI_FLASH_BUF[i+251]; j<<=8;
		j+=SPI_FLASH_BUF[i+250];
		if(j!=crc)
		{
  		//printf("\n\rCRC Error: \n\r");
			//for(j=0; j<256; j++)
			//  printf("%X ", SPI_FLASH_BUF[i+j]);
			return(1);
		}
	}
	
	if(EmptyCheck)
    for(i=0; i<256; i++)
	    if(SPI_FLASH_BUF[Offset+i]!=0xFF)
	    {
  	    //printf("\n\rEmpty Error");
		    return(2);
	    }
		
	return(0);
}

//=============================================================================
unsigned char RecoverSector(u32 WriteAddr)
{

}

//=============================================================================
unsigned char ExclusiveWrite(u32 WriteAddr, u8* pBuffer)
{
unsigned short i;
	
	//printf("\n\rWrite Address: %X",WriteAddr);
	
	//Write to new sector then need erase sector
	if((WriteAddr%4096)==0)
	{
		//printf("\n\r[Erase] Start Sector");
		SPI_Flash_Erase_Sector(WriteAddr/4096);
	}
	else
	{
		if(((WriteAddr+256)%4096)==0) 
		{
			//Now, We must erase next sector 
  		//printf("\n\r[Erase] Next Sector");
			if((WriteAddr+256) >= addTransactions+0x100000)
  		  SPI_Flash_Erase_Sector(addTransactions/4096);  //Next record is first position of ring
			else
  		  SPI_Flash_Erase_Sector((WriteAddr+256)/4096);
		}
		if(CheckSector(WriteAddr/4096, WriteAddr%4096, 1))
		{
			RecoverSector(WriteAddr);
			if(CheckSector(WriteAddr/4096, WriteAddr%4096, 1))
			  return(1);
		}
	}
	
	if(SPI_Flash_Write_Page(pBuffer,WriteAddr,256)==1) return(1);  //Error in write
	
	//Check wroted data
	SPI_Flash_Read(SPI_FLASH_BUF,WriteAddr,256);
	for(i=0; i<256; i++)
	  if(SPI_FLASH_BUF[i]!=pBuffer[i])
			break;
	if(i<256)
	{
		//printf("\n\rCheck Error");
    return(1);		//Read Error
	}
	//printf("\n\rWrite OK");
	if(SaveMirror(WriteAddr, pBuffer))
		SaveMirrorSector(WriteAddr);
	return(0);
}

//=============================================================================
unsigned char ExclusiveRead(u32 ReadAddr, u8* pBuffer)
{
unsigned short i, crc;
	
	//printf("\n\rRead Address: %X",ReadAddr);
	
  if(CheckSector(ReadAddr/4096, ReadAddr%4096, 0))
	{
		RecoverSector(ReadAddr);
		if(CheckSector(ReadAddr/4096, ReadAddr%4096, 0))
		  return(1);
	}
	
	//Check wroted data
	SPI_Flash_Read(pBuffer,ReadAddr,256);
	
  crc=crc16(0, pBuffer, 256-6);
  i=pBuffer[251]; i<<=8;
	i+=pBuffer[250];
	if(i!=crc)
	{
		//printf("\n\rCheck Error");
    return(2);		//Read Error
	}
	//printf("\n\rRead OK");
	return(0);
}

//=============================================================================
u8 SPI_Flash_Write(u8* pBuffer,u32 WriteAddr,u16 NumByteToWrite)   
{ 
u32 secpos;
u16 secoff;
u16 secremain;	   
u16 i;    
	
	secpos=WriteAddr/4096;
	secoff=WriteAddr%4096;
	secremain=4096-secoff;

	if((secoff==0)&&(secpos!=0))
		SPI_Flash_Erase_Sector(secpos);
	
	if(NumByteToWrite<=secremain)secremain=NumByteToWrite;

	while(1) 
	{	
		SPI_Flash_Read(SPI_FLASH_BUF,secpos*4096,4096);
		for(i=0;i<secremain;i++)
		{
			if(SPI_FLASH_BUF[secoff+i]!=0XFF)break;
		}
		if(i<secremain)
		{
			SPI_Flash_Erase_Sector(secpos);
			for(i=0;i<secremain;i++)	  
			{
				SPI_FLASH_BUF[i+secoff]=pBuffer[i];	  
			}
			SPI_Flash_Write_NoCheck(SPI_FLASH_BUF,secpos*4096,4096);

		}else SPI_Flash_Write_NoCheck(pBuffer,WriteAddr,secremain);


		for(i=0; i<secremain; i++)	  
			SPI_FLASH_BUF[i]=0;	  
		SPI_Flash_Read(SPI_FLASH_BUF,WriteAddr,secremain);
		for(i=0; i<secremain; i++)	  
		  if(SPI_FLASH_BUF[i]!=pBuffer[i])
				break;
    if(i<secremain)
      return(1);			

		if(NumByteToWrite==secremain)break;

		secpos++;
		secoff=0;

   	pBuffer+=secremain;
		WriteAddr+=secremain;
   	NumByteToWrite-=secremain;
		if(NumByteToWrite>4096)secremain=4096;
		else secremain=NumByteToWrite;			

	};	 	 

	return(0);
}

//=============================================================================
void SPI_Flash_Erase_Chip(void)   
{                                             
    SPI_FLASH_Write_Enable(); 
    SPI_Flash_Wait_Busy();   
  	SPI_FLASH_CS=0;           
    SPI2_ReadWriteByte(W25X_ChipErase);
	SPI_FLASH_CS=1;                      
	SPI_Flash_Wait_Busy();   				  
}   

//=============================================================================
void SPI_Flash_Erase_Sector(u32 Dst_Addr)   
{   
	Dst_Addr*=4096;
    SPI_FLASH_Write_Enable();       
    SPI_Flash_Wait_Busy();   
  	SPI_FLASH_CS=0;                 
    SPI2_ReadWriteByte(W25X_SectorErase);
    SPI2_ReadWriteByte((u8)((Dst_Addr)>>16)); 
    SPI2_ReadWriteByte((u8)((Dst_Addr)>>8));   
    SPI2_ReadWriteByte((u8)Dst_Addr);  
	SPI_FLASH_CS=1;                          
    SPI_Flash_Wait_Busy();   				
}  

//=============================================================================
unsigned char SPI_Flash_Wait_Busy(void)   
{   
unsigned long int i=0;
	
	while ((SPI_Flash_ReadSR()&0x01)==0x01)
	{
		WDTR;
		if(++i>10000000)
			return(1);
	}
	return(0);
}  

//=============================================================================
void SPI_Flash_PowerDown(void)   
{ 
  	SPI_FLASH_CS=0;                        
    SPI2_ReadWriteByte(W25X_PowerDown);    
	SPI_FLASH_CS=1;                          
    GUI_Delay(3);                           
}   

//=============================================================================
void SPI_Flash_WAKEUP(void)   
{  
  	SPI_FLASH_CS=0;                        
    SPI2_ReadWriteByte(W25X_ReleasePowerDown); 
	SPI_FLASH_CS=1;                           
    GUI_Delay(3);                            
}   


























