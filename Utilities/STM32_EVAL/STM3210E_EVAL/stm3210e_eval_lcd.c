/**
  ******************************************************************************
  * @file    stm3210e_eval_lcd.c
  * @author  MCD Application Team
  * @version V5.1.0
  * @date    18-January-2013
  * @brief   This file includes the LCD driver for AM-240320L8TNQW00H 
  *          (LCD_ILI9320), AM-240320LDTNQW00H (LCD_SPFD5408B)
  *          and AM240320LGTNQW00H (HX8347-D) Liquid Crystal
  *          Display Module of STM3210E-EVAL board.
  ******************************************************************************
  * @attention
  *
  * <h2><center>&copy; COPYRIGHT 2013 STMicroelectronics</center></h2>
  *
  * Licensed under MCD-ST Liberty SW License Agreement V2, (the "License");
  * You may not use this file except in compliance with the License.
  * You may obtain a copy of the License at:
  *
  *        http://www.st.com/software_license_agreement_liberty_v2
  *
  * Unless required by applicable law or agreed to in writing, software 
  * distributed under the License is distributed on an "AS IS" BASIS, 
  * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
  * See the License for the specific language governing permissions and
  * limitations under the License.
  *
  ******************************************************************************
  */ 

/* Includes ------------------------------------------------------------------*/
#include "stm3210e_eval_lcd.h"
#include "../Common/fonts.c"
#include "config.h"

/** @addtogroup Utilities
  * @{
  */ 

/** @addtogroup STM32_EVAL
  * @{
  */ 

/** @addtogroup STM3210E_EVAL
  * @{
  */
    
/** @defgroup STM3210E_EVAL_LCD 
  * @brief This file includes the LCD driver for AM-240320L8TNQW00H 
  *        (LCD_ILI9320) and AM-240320LDTNQW00H (LCD_SPFD5408B) Liquid Crystal
  *        Display Module of STM3210E-EVAL board.
  * @{
  */ 

/** @defgroup STM3210E_EVAL_LCD_Private_TypesDefinitions
  * @{
  */ 
typedef struct
{
  __IO uint16_t LCD_REG;
  __IO uint16_t LCD_RAM;
} LCD_TypeDef;
/**
  * @}
  */ 


/** @defgroup STM3210E_EVAL_LCD_Private_Defines
  * @{
  */
/* Note: LCD /CS is CE4 - Bank 4 of NOR/SRAM Bank 1~4 */
#define LCD_BASE           ((uint32_t)(0x6C000000 | 0x000007FE))
#define LCD                ((LCD_TypeDef *) LCD_BASE)
#define MAX_POLY_CORNERS   200
#define POLY_Y(Z)          ((int32_t)((Points + Z)->X))
#define POLY_X(Z)          ((int32_t)((Points + Z)->Y)) 
#define LCD_HX8347D        0x0047

  
unsigned int XSIZE_PHYS=480;
unsigned int YSIZE_PHYS=272;



/* Global variables to set the written text color */
static __IO uint16_t LCD_ID = 0;

/**
  * @}
  */ 


/** @defgroup STM3210E_EVAL_LCD_Private_FunctionPrototypes
  * @{
  */ 
#ifndef USE_Delay
static void delay(__IO uint32_t nCount);
#endif /* USE_Delay*/


/*
unsigned int  HDP=480; //Horizontal Display Period
unsigned int  HT=1000; //Horizontal Total
unsigned int  HPS=51;  //LLINE Pulse Start Position
unsigned int  LPS=3;   //	Horizontal Display Period Start Position
unsigned char HPW=8;   //	LLINE Pulse Width


unsigned int  VDP=272;	//Vertical Display Period
unsigned int  VT=530;	//Vertical Total
unsigned int  VPS=24;	//	LFRAME Pulse Start Position
unsigned int  FPS=23;	//Vertical Display Period Start Positio
unsigned char   VPW=3;	// LFRAME Pulse Width
*/
/*
unsigned int  HDP=799; //Horizontal Display Period
unsigned int  HT=1000; //Horizontal Total
unsigned int  HPS=51;  //LLINE Pulse Start Position
unsigned int  LPS=3;   //	Horizontal Display Period Start Position
unsigned char HPW=8;   //	LLINE Pulse Width


unsigned int  VDP=479;	//Vertical Display Period
unsigned int  VT=530;	//Vertical Total
unsigned int  VPS=24;	//	LFRAME Pulse Start Position
unsigned int  FPS=23;	//Vertical Display Period Start Positio
unsigned char   VPW=3;	// LFRAME Pulse Width
*/

//================================================================================
void LCD_WriteCom(u16 LCD_Reg)
{
  	LCD->LCD_REG = LCD_Reg;
}

//================================================================================
void STM3210E_LCD_Init(void)
{ 

#ifndef WVGA

unsigned int HDP=479;
unsigned int HT=525;
unsigned int HPS=45;
unsigned int LPS=8;
unsigned int HPW=10;

unsigned int VDP=271;
unsigned int VT=288;
unsigned int VPS=16;
unsigned int FPS=1;
unsigned int VPW=10;

#else

unsigned int HDP=799;
unsigned int HT=1000;
unsigned int HPS=51;
unsigned int LPS=3;
unsigned int HPW=8;

unsigned int VDP=479;
unsigned int VT=530;
unsigned int VPS=24;
unsigned int FPS=23;
unsigned int VPW=3;

  if (LCDType==LCD480_272)
  {
    HDP=479;
    HT=525;
    HPS=45;
    LPS=8;
    HPW=10;

    VDP=271;
    VT=288;
    VPS=16;
    FPS=1;
    VPW=10;
		/*
    HDP=479;
    HT=531;
    HPS=43;
    LPS=8;
    HPW=10;

    VDP=271;
    VT=288;
    VPS=12;
    FPS=4;
    VPW=10;
		*/
    XSIZE_PHYS=480;
    YSIZE_PHYS=272;
  }else{
    XSIZE_PHYS=800;
    YSIZE_PHYS=480;
	}
	
#endif



  /* Configure the LCD Control pins */
  LCD_CtrlLinesConfig();

  /* Configure the FSMC Parallel interface */
  LCD_FSMCConfig();
  
  /* delay 50 ms */
  _delay_(5); 

	LCD_WriteCom(0x00E2);	
	LCD_WriteRAM(0x0023);
	// Set PLL with OSC = 10MHz (hardware)
  // Multiplier N = 35, VCO (>250MHz)= OSC*(N+1), VCO = 360MHz	   
	LCD_WriteRAM(0x0002);
	// Divider M = 2, PLL = 360/(M+1) = 120MHz
	LCD_WriteRAM(0x0004);
	// Validate M and N values

	LCD_WriteCom(0x00E0);  // PLL enable
	LCD_WriteRAM(0x0001);
  _delay_(10);
	LCD_WriteCom(0x00E0);
	LCD_WriteRAM(0x0003);
	_delay_(50);
	LCD_WriteCom(0x0001);  // software reset
	_delay_(50);
	
  #ifndef WVGA
  LCD_WriteCom(0x00E6);					//PLL setting for PCLK, depends on resolution
  LCD_WriteRAM(0x0000);
  LCD_WriteRAM(0x00D9);
  LCD_WriteRAM(0x0016);
	#else
	if (LCDType==LCD480_272)
	{
	  LCD_WriteCom(0x00E6);					//PLL setting for PCLK, depends on resolution
	  LCD_WriteRAM(0x0000);
	  LCD_WriteRAM(0x00D9);
	  LCD_WriteRAM(0x0016);
	}else{
	  LCD_WriteCom(0x00E6);
	  LCD_WriteRAM(0x0004);
	  LCD_WriteRAM(0x0093);
	  LCD_WriteRAM(0x00e0);	
  }
	#endif
	LCD_WriteCom(0x00B0);	//LCD SPECIFICATION
	LCD_WriteRAM(0x0020);
	LCD_WriteRAM(0x0000);
	LCD_WriteRAM((HDP >> 8)&0X00FF);  //Set HDP
	LCD_WriteRAM(HDP&0X00FF);
    LCD_WriteRAM((VDP >> 8)&0X00FF);  //Set VDP
	LCD_WriteRAM(VDP&0X00FF);
    LCD_WriteRAM(0x0000);

	LCD_WriteCom(0x00B4);	//HSYNC
	LCD_WriteRAM((HT >> 8)&0X00FF);  //Set HT
	LCD_WriteRAM(HT&0X00FF);
	LCD_WriteRAM((HPS >> 8)&0X00FF);  //Set HPS
	LCD_WriteRAM(HPS&0X00FF);
	LCD_WriteRAM(HPW);			   //Set HPW
	LCD_WriteRAM((LPS >> 8)&0X00FF);  //Set HPS
	LCD_WriteRAM(LPS&0X00FF);
	LCD_WriteRAM(0x0000);

	LCD_WriteCom(0x00B6);	//VSYNC
	LCD_WriteRAM((VT >> 8)&0X00FF);   //Set VT
	LCD_WriteRAM(VT&0X00FF);
	LCD_WriteRAM((VPS >> 8)&0X00FF);  //Set VPS
	LCD_WriteRAM(VPS&0X00FF);
	LCD_WriteRAM(VPW);			   //Set VPW
	LCD_WriteRAM((FPS >> 8)&0X00FF);  //Set FPS
	LCD_WriteRAM(FPS&0X00FF);

	LCD_WriteCom(0x00BA);
	LCD_WriteRAM(0x0005);//0x000F);    //GPIO[3:0] out 1

	LCD_WriteCom(0x00B8);
	LCD_WriteRAM(0x0007);    //GPIO3=input, GPIO[2:0] =output
	LCD_WriteRAM(0x0001);    //GPIO0 normal

	LCD_WriteCom(0x0036); //rotation
	LCD_WriteRAM(0x0000);

	LCD_WriteCom(0x00F0); //pixel data interface
	LCD_WriteRAM(0x0003);
/*
  //Set the image post processor
  LCD_WriteCom(0xBC); 
  LCD_WriteRAM(0x0040);//contrast value
  LCD_WriteRAM(0x0080);//brightness value
  LCD_WriteRAM(0x0040);//saturation value
  LCD_WriteRAM(0x0001);//Post Processor Enable
		
		*/
	_delay_(50);
	LCD_WriteCom(0x0029); //display on

	LCD_WriteCom(0x00BE); //set PWM for B/L
	LCD_WriteRAM(0x0006);
	LCD_WriteRAM(0x0080);
	
	LCD_WriteRAM(0x0001);
	LCD_WriteRAM(0x00f0);
	LCD_WriteRAM(0x0000);
	LCD_WriteRAM(0x0000);

	LCD_WriteCom(0x00d0);
	LCD_WriteRAM(0x000d);
	
}

/**
  * @brief  Writes to the selected LCD register.
  * @param  LCD_Reg: address of the selected register.
  * @param  LCD_RegValue: value to write to the selected register.
  * @retval None
  */
void LCD_WriteReg(uint8_t LCD_Reg, uint16_t LCD_RegValue)
{
  /* Write 16-bit Index, then Write Reg */
  LCD->LCD_REG = LCD_Reg;
  /* Write 16-bit Reg */
  LCD->LCD_RAM = LCD_RegValue;
}

/**
  * @brief  Reads the selected LCD Register.
  * @param  LCD_Reg: address of the selected register.
  * @retval LCD Register Value.
  */
uint16_t LCD_ReadReg(uint8_t LCD_Reg)
{
  /* Write 16-bit Index (then Read Reg) */
  LCD->LCD_REG = LCD_Reg;
  /* Read 16-bit Reg */
  return (LCD->LCD_RAM);
}

/**
  * @brief  Prepare to write to the LCD RAM.
  * @param  None
  * @retval None
  */
void LCD_WriteRAM_Prepare(void)
{
  LCD->LCD_REG = LCD_REG_34;
}

/**
  * @brief  Writes to the LCD RAM.
  * @param  RGB_Code: the pixel color in RGB mode (5-6-5).
  * @retval None
  */
void LCD_WriteRAM(uint16_t RGB_Code)
{
  /* Write 16-bit GRAM Reg */
  LCD->LCD_RAM = RGB_Code;
}

/**
  * @brief  Reads the LCD RAM.
  * @param  None
  * @retval LCD RAM Value.
  */
uint16_t LCD_ReadRAM(void)
{
  /* Write 16-bit Index (then Read Reg) */
  LCD->LCD_REG = LCD_REG_34; /* Select GRAM Reg */
  /* Read 16-bit Reg */
  return LCD->LCD_RAM;
}


/**
  * @brief  Configures LCD Control lines (FSMC Pins) in alternate function mode.
  * @param  None
  * @retval None
  */
void LCD_CtrlLinesConfig(void)
{
GPIO_InitTypeDef  GPIO_InitStructure;

	RCC_AHBPeriphClockCmd(RCC_AHBPeriph_FSMC,ENABLE);
 	
	//--------------------------PORT D
  GPIO_InitStructure.GPIO_Pin = GPIO_Pin_0|GPIO_Pin_1|GPIO_Pin_4|GPIO_Pin_5|GPIO_Pin_8|GPIO_Pin_9|GPIO_Pin_10|GPIO_Pin_14|GPIO_Pin_15;	 
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;
  GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;	
  GPIO_Init(GPIOD, &GPIO_InitStructure);	  		
	
	//--------------------------PORT E
  GPIO_InitStructure.GPIO_Pin = GPIO_Pin_7|GPIO_Pin_8|GPIO_Pin_9|GPIO_Pin_10|GPIO_Pin_11|GPIO_Pin_12|GPIO_Pin_13|GPIO_Pin_14|GPIO_Pin_15;	 
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;
  GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;	
  GPIO_Init(GPIOE, &GPIO_InitStructure);	  		
	
	//--------------------------PORT G
  GPIO_InitStructure.GPIO_Pin = GPIO_Pin_0|GPIO_Pin_12;	 
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;
  GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;	
  GPIO_Init(GPIOG, &GPIO_InitStructure);	  		
}

/**
  * @brief  Configures the Parallel interface (FSMC) for LCD(Parallel mode)
  * @param  None
  * @retval None
  */
void LCD_FSMCConfig(void)
{
FSMC_NORSRAMInitTypeDef  FSMC_NORSRAMInitStructure;
FSMC_NORSRAMTimingInitTypeDef  p;

  p.FSMC_AddressSetupTime = 1;
  p.FSMC_AddressHoldTime = 1;
  p.FSMC_DataSetupTime = 3;
  p.FSMC_BusTurnAroundDuration = 0;
  p.FSMC_CLKDivision = 1;
  p.FSMC_DataLatency = 0;
  p.FSMC_AccessMode = FSMC_AccessMode_A;


    FSMC_NORSRAMInitStructure.FSMC_Bank = FSMC_Bank1_NORSRAM4;
    FSMC_NORSRAMInitStructure.FSMC_DataAddressMux = FSMC_DataAddressMux_Disable;
    FSMC_NORSRAMInitStructure.FSMC_MemoryType =FSMC_MemoryType_SRAM;				// FSMC_MemoryType_SRAM;  //SRAM   
    FSMC_NORSRAMInitStructure.FSMC_MemoryDataWidth = FSMC_MemoryDataWidth_16b;		//16bit   
    FSMC_NORSRAMInitStructure.FSMC_BurstAccessMode =FSMC_BurstAccessMode_Disable;	// FSMC_BurstAccessMode_Disable; 
    FSMC_NORSRAMInitStructure.FSMC_WaitSignalPolarity = FSMC_WaitSignalPolarity_Low;
	FSMC_NORSRAMInitStructure.FSMC_AsynchronousWait=FSMC_AsynchronousWait_Disable; 
    FSMC_NORSRAMInitStructure.FSMC_WrapMode = FSMC_WrapMode_Disable;   
    FSMC_NORSRAMInitStructure.FSMC_WaitSignalActive = FSMC_WaitSignalActive_BeforeWaitState;  
    FSMC_NORSRAMInitStructure.FSMC_WriteOperation = FSMC_WriteOperation_Enable;
    FSMC_NORSRAMInitStructure.FSMC_WaitSignal = FSMC_WaitSignal_Disable;   
    FSMC_NORSRAMInitStructure.FSMC_ExtendedMode = FSMC_ExtendedMode_Enable;
    FSMC_NORSRAMInitStructure.FSMC_WriteBurst = FSMC_WriteBurst_Disable; 
    FSMC_NORSRAMInitStructure.FSMC_ReadWriteTimingStruct = &p;
    FSMC_NORSRAMInitStructure.FSMC_WriteTimingStruct = &p;

    FSMC_NORSRAMInit(&FSMC_NORSRAMInitStructure);

   	FSMC_NORSRAMCmd(FSMC_Bank1_NORSRAM4, ENABLE);
}

#ifndef USE_Delay
/**
  * @brief  Inserts a delay time.
  * @param  nCount: specifies the delay time length.
  * @retval None
  */
static void delay(vu32 nCount)
{
  vu32 index = 0; 
  for (index = (100000 * nCount); index != 0; index--)
  {
  }
}
#endif /* USE_Delay */


/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
