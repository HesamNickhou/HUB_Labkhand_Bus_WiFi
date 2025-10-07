#include "sram.h"	  

#define Bank1_SRAM3_ADDR    ((u32)(0x68000000))		
  						   
void FSMC_SRAM_Init(void)
{	
FSMC_NORSRAMInitTypeDef  FSMC_NORSRAMInitStructure;
FSMC_NORSRAMTimingInitTypeDef  readWriteTiming;
	 			  	  
 	readWriteTiming.FSMC_AddressSetupTime = 0x00;	
    readWriteTiming.FSMC_AddressHoldTime = 0x00;
    readWriteTiming.FSMC_DataSetupTime = 0x03;	
    readWriteTiming.FSMC_BusTurnAroundDuration = 0x00;
    readWriteTiming.FSMC_CLKDivision = 0x00;
    readWriteTiming.FSMC_DataLatency = 0x00;
    readWriteTiming.FSMC_AccessMode = FSMC_AccessMode_A;	
    

 
    FSMC_NORSRAMInitStructure.FSMC_Bank = FSMC_Bank1_NORSRAM3;
    FSMC_NORSRAMInitStructure.FSMC_DataAddressMux = FSMC_DataAddressMux_Disable; 
    FSMC_NORSRAMInitStructure.FSMC_MemoryType =FSMC_MemoryType_SRAM;
    FSMC_NORSRAMInitStructure.FSMC_MemoryDataWidth = FSMC_MemoryDataWidth_16b;
    FSMC_NORSRAMInitStructure.FSMC_BurstAccessMode =FSMC_BurstAccessMode_Disable;
    FSMC_NORSRAMInitStructure.FSMC_WaitSignalPolarity = FSMC_WaitSignalPolarity_Low;
	FSMC_NORSRAMInitStructure.FSMC_AsynchronousWait=FSMC_AsynchronousWait_Disable;
    FSMC_NORSRAMInitStructure.FSMC_WrapMode = FSMC_WrapMode_Disable;   
    FSMC_NORSRAMInitStructure.FSMC_WaitSignalActive = FSMC_WaitSignalActive_BeforeWaitState;  
    FSMC_NORSRAMInitStructure.FSMC_WriteOperation = FSMC_WriteOperation_Enable;	
    FSMC_NORSRAMInitStructure.FSMC_WaitSignal = FSMC_WaitSignal_Disable;  
    FSMC_NORSRAMInitStructure.FSMC_ExtendedMode = FSMC_ExtendedMode_Disable;
    FSMC_NORSRAMInitStructure.FSMC_WriteBurst = FSMC_WriteBurst_Disable;  
    FSMC_NORSRAMInitStructure.FSMC_ReadWriteTimingStruct = &readWriteTiming;
    FSMC_NORSRAMInitStructure.FSMC_WriteTimingStruct = &readWriteTiming;

    FSMC_NORSRAMInit(&FSMC_NORSRAMInitStructure); 

   	FSMC_NORSRAMCmd(FSMC_Bank1_NORSRAM3, ENABLE); 
											
}
	  														  
void FSMC_SRAM_WriteBuffer(u8* pBuffer,u32 WriteAddr,u32 n)
{
	for (;n != 0;n--)  
	{										    
		*(vu8*)(Bank1_SRAM3_ADDR+WriteAddr)=*pBuffer;	  
		WriteAddr += 2;
		pBuffer++;
	}   
}																			    

void FSMC_SRAM_ReadBuffer(u8* pBuffer,u32 ReadAddr,u32 n)
{
	for (;n != 0;n--)  
	{											    
		*pBuffer++ = *(vu8*)(Bank1_SRAM3_ADDR+ReadAddr);    
		ReadAddr += 2;
	}  
} 

void fsmc_sram_test_write(u8 data,u32 addr)
{			   
	FSMC_SRAM_WriteBuffer(&data,addr,1);
}

u8 fsmc_sram_test_read(u32 addr)
{
	u8 data;
	FSMC_SRAM_ReadBuffer(&data,addr,1);
	return data;
}	











