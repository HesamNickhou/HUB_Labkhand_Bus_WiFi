#define  BSP_GLOBALS
#include <includes.h>
#include "sd/mmc_sd.h"			   
#include "usart/usart.h"			   
#include "sram/sram.h"			   
#include "fatfs/ff.h"
#include "rtc/rtc.h"
#include "touch/touch.h"
#include "flash/flash.h"
#include <rdlib/types/RdLib.h>
#include <WM.h>
#include "DIALOG.h"
#include "config.h"
#include "wdg/wdg.h"

static volatile ErrorStatus HSEStartUpStatus = SUCCESS;
static void SysTick_Configuration(void);
void GPIO_Config(void);
void SPI_Config(void);
void InterruptConfig(void);

FATFS fs;

//=========================================================================== Independent Watchdog Configuration
// <e0> Independent Watchdog Configuration
//   <o1> IWDG period [us] <125-32000000:125>
//   <i> Set the timer period for Independent Watchdog.
//   <i> Default: 1000000  (1s)
// </e>
#define __IWDG_SETUP              1
#define __IWDG_PERIOD             0x989680   
                                  //0x001E8480
/*----------------------------------------------------------------------------
 Define  IWDG PR and RLR settings
 *----------------------------------------------------------------------------*/
#if   (__IWDG_PERIOD >  16384000UL)
  #define __IWDG_PR             (6)
  #define __IWDGCLOCK (32000UL/256)
#elif (__IWDG_PERIOD >   8192000UL)
  #define __IWDG_PR             (5)
  #define __IWDGCLOCK (32000UL/128)
#elif (__IWDG_PERIOD >   4096000UL)
  #define __IWDG_PR             (4)
  #define __IWDGCLOCK  (32000UL/64)
#elif (__IWDG_PERIOD >   2048000UL)
  #define __IWDG_PR             (3)
  #define __IWDGCLOCK  (32000UL/32)
#elif (__IWDG_PERIOD >   1024000UL)
  #define __IWDG_PR             (2)
  #define __IWDGCLOCK  (32000UL/16)
#elif (__IWDG_PERIOD >    512000UL)
  #define __IWDG_PR             (1)
  #define __IWDGCLOCK   (32000UL/8)
#else
  #define __IWDG_PR             (0)
  #define __IWDGCLOCK   (32000UL/4)
#endif
#define __IWGDCLK  (32000UL/(0x04<<__IWDG_PR))
#define __IWDG_RLR (__IWDG_PERIOD*__IWGDCLK/1000000UL-1)

//==============================================================================
void stm32_IwdgSetup (void) {

//  RCC->CSR |= (1<<0);                                           // LSI enable, necessary for IWDG
//  while ((RCC->CSR & (1<<1)) == 0);                             // wait till LSI is ready

  IWDG->KR  = 0x5555;                                           // enable write to PR, RLR
  IWDG->PR  = __IWDG_PR;                                        // Init prescaler
  IWDG->RLR = __IWDG_RLR;                                       // Init RLR
  IWDG->KR  = 0xAAAA;                                           // Reload the watchdog
  IWDG->KR  = 0xCCCC;                                           // Start the watchdog
} // end of stm32_IwdgSetup

//==============================================================================
void DAC_Configuration(void) {
	// Sine wave
	const int sineWave[120] = {
		0x7ff, 0x86a, 0x8d5, 0x93f, 0x9a9, 0xa11, 0xa78, 0xadd, 0xb40, 0xba1,
		0xbff, 0xc5a, 0xcb2, 0xd08, 0xd59, 0xda7, 0xdf1, 0xe36, 0xe77, 0xeb4,
		0xeec, 0xf1f, 0xf4d, 0xf77, 0xf9a, 0xfb9, 0xfd2, 0xfe5, 0xff3, 0xffc,
		0xfff, 0xffc, 0xff3, 0xfe5, 0xfd2, 0xfb9, 0xf9a, 0xf77, 0xf4d, 0xf1f,
		0xeec, 0xeb4, 0xe77, 0xe36, 0xdf1, 0xda7, 0xd59, 0xd08, 0xcb2, 0xc5a,
		0xbff, 0xba1, 0xb40, 0xadd, 0xa78, 0xa11, 0x9a9, 0x93f, 0x8d5, 0x86a,
		0x7ff, 0x794, 0x729, 0x6bf, 0x655, 0x5ed, 0x586, 0x521, 0x4be, 0x45d,
		0x3ff, 0x3a4, 0x34c, 0x2f6, 0x2a5, 0x257, 0x20d, 0x1c8, 0x187, 0x14a,
		0x112, 0x0df, 0x0b1, 0x087, 0x064, 0x045, 0x02c, 0x019, 0x00b, 0x002,
		0x000, 0x002, 0x00b, 0x019, 0x02c, 0x045, 0x064, 0x087, 0x0b1, 0x0df,
		0x112, 0x14a, 0x187, 0x1c8, 0x20d, 0x257, 0x2a5, 0x2f6, 0x34c, 0x3a4,
		0x3ff, 0x45d, 0x4be, 0x521, 0x586, 0x5ed, 0x655, 0x6bf, 0x729, 0x794
	};	
	unsigned int i, j;	
	GPIO_InitTypeDef GPIO_InitStructure;
	DAC_InitTypeDef            DAC_InitStructure;
	DMA_InitTypeDef            DMA_InitStructure;

	RCC_APB1PeriphClockCmd(RCC_APB1Periph_DAC ,ENABLE);
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA | RCC_APB2Periph_AFIO,ENABLE);


	GPIO_InitStructure.GPIO_Pin   = GPIO_Pin_5;
	GPIO_InitStructure.GPIO_Mode  = GPIO_Mode_AF_PP;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init(GPIOA ,&GPIO_InitStructure);

	RCC_AHBPeriphClockCmd(RCC_AHBPeriph_DMA2, ENABLE);


  /* DAC deinitialize */
  DAC_DeInit();
  DAC_StructInit(&DAC_InitStructure);


      DAC_InitStructure.DAC_Trigger = DAC_Trigger_None;
      DAC_InitStructure.DAC_WaveGeneration = DAC_WaveGeneration_None;
      DAC_InitStructure.DAC_OutputBuffer = DAC_OutputBuffer_Enable;
      DAC_Init(DAC_Channel_2, &DAC_InitStructure);

  /* Enable DAC Channel2 */
  DAC_Cmd(DAC_Channel_2, ENABLE);
	
  VOICEEN=1;
	PlayVoice("12.wav");
	

/* Enable DMA for DAC Channel1 */
//  DAC_DMACmd(DAC_Channel_2, ENABLE);
}

//==============================================================================
#ifdef Simorgh50N
void GPIO_Config(void) {
	GPIO_InitTypeDef  GPIO_InitStructure;
 	
  RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA | RCC_APB2Periph_AFIO, ENABLE);
  RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB | RCC_APB2Periph_AFIO, ENABLE);
  RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOC | RCC_APB2Periph_AFIO, ENABLE);
  RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOD | RCC_APB2Periph_AFIO, ENABLE);
  RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOE | RCC_APB2Periph_AFIO, ENABLE);
  RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOF | RCC_APB2Periph_AFIO, ENABLE);
  RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOG | RCC_APB2Periph_AFIO, ENABLE);

  GPIO_InitStructure.GPIO_Pin   = GPIO_Pin_3 | GPIO_Pin_4;
  GPIO_InitStructure.GPIO_Mode  = GPIO_Mode_IPU;
  GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;	
  GPIO_Init(GPIOG, &GPIO_InitStructure);	  		
	
	//if ((PGin(3) == 0) && (PGin(4) == 0))
	//	BOARD_VER=VII;
	//else
	//	BOARD_VER=VI;
	BOARD_VER = VII;
	if (BOARD_VER==VII) {
	  //--------------------------PORT A
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_0;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IPD;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;	
    GPIO_Init(GPIOA, &GPIO_InitStructure);	  		

    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_3|GPIO_Pin_10;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;	
    GPIO_Init(GPIOA, &GPIO_InitStructure);	  		

    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_1|GPIO_Pin_8;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;	
    GPIO_Init(GPIOA, &GPIO_InitStructure);	  		

    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_2|GPIO_Pin_9;	 
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;	
    GPIO_Init(GPIOA, &GPIO_InitStructure);	  

	
	  //--------------------------PORT B
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_0;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IPD;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;	
    GPIO_Init(GPIOB, &GPIO_InitStructure);	  		

    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_6;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IPU;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;	
    GPIO_Init(GPIOB, &GPIO_InitStructure);	  		

    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_11;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;	
    GPIO_Init(GPIOB, &GPIO_InitStructure);	  		

    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_1|GPIO_Pin_5|GPIO_Pin_7|GPIO_Pin_12;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;	
    GPIO_Init(GPIOB, &GPIO_InitStructure);	  		

    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_10;	 
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;	
    GPIO_Init(GPIOB, &GPIO_InitStructure);	  		
	
	  //--------------------------PORT C
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_2|GPIO_Pin_6|GPIO_Pin_4|GPIO_Pin_5;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IPU;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;	
    GPIO_Init(GPIOC, &GPIO_InitStructure);	  		

    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_1|GPIO_Pin_3|GPIO_Pin_4|GPIO_Pin_5|GPIO_Pin_13;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;	
    GPIO_Init(GPIOC, &GPIO_InitStructure);	  		

    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_12;	 
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;	
    GPIO_Init(GPIOC, &GPIO_InitStructure);	  		
	
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_8|GPIO_Pin_9|GPIO_Pin_10|GPIO_Pin_11;	 
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;	
    GPIO_Init(GPIOC, &GPIO_InitStructure);	  		

    
		
		
		//***************************


    //--------------------------PORT D
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_12|GPIO_Pin_6;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IPU;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;	
    GPIO_Init(GPIOD, &GPIO_InitStructure);	  		

    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_3;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;	
    GPIO_Init(GPIOD, &GPIO_InitStructure);	  		

    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_0|GPIO_Pin_1|GPIO_Pin_2|GPIO_Pin_4|GPIO_Pin_5|GPIO_Pin_8|GPIO_Pin_9|GPIO_Pin_10|GPIO_Pin_11|GPIO_Pin_13|GPIO_Pin_14|GPIO_Pin_15;	 
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;	
    GPIO_Init(GPIOD, &GPIO_InitStructure);	  		
	

    //***************************
	

    //--------------------------PORT E
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_2|GPIO_Pin_3|GPIO_Pin_4|GPIO_Pin_5|GPIO_Pin_6;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;	
    GPIO_Init(GPIOE, &GPIO_InitStructure);	  		

    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_0|GPIO_Pin_1|GPIO_Pin_7|GPIO_Pin_8|GPIO_Pin_9|GPIO_Pin_10|GPIO_Pin_11|GPIO_Pin_12|GPIO_Pin_13|GPIO_Pin_14|GPIO_Pin_15;	 
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;	
    GPIO_Init(GPIOE, &GPIO_InitStructure);	  		
	
  	//--------------------------PORT F
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_6|GPIO_Pin_7|GPIO_Pin_9|GPIO_Pin_15;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;	
    GPIO_Init(GPIOF, &GPIO_InitStructure);	  		

    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_10|GPIO_Pin_11;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IPU;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;	
    GPIO_Init(GPIOF, &GPIO_InitStructure);	  		

    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_8;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IPD;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;	
    GPIO_Init(GPIOF, &GPIO_InitStructure);	  		

    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_0|GPIO_Pin_1|GPIO_Pin_2|GPIO_Pin_3|GPIO_Pin_4|GPIO_Pin_5|GPIO_Pin_12|GPIO_Pin_13|GPIO_Pin_14;	 
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;	
    GPIO_Init(GPIOF, &GPIO_InitStructure);	  		
	
	  //--------------------------PORT G
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_5|GPIO_Pin_9|GPIO_Pin_13|GPIO_Pin_15;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;	
    GPIO_Init(GPIOG, &GPIO_InitStructure);	  		

    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_7|GPIO_Pin_8|GPIO_Pin_14;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IPU;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;	
    GPIO_Init(GPIOG, &GPIO_InitStructure);	  		

    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_0|GPIO_Pin_1|GPIO_Pin_2|GPIO_Pin_3|GPIO_Pin_4|GPIO_Pin_12|GPIO_Pin_14;	 
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;	
    GPIO_Init(GPIOG, &GPIO_InitStructure);	  

    //SAM CK
		GPIO_InitStructure.GPIO_Pin = GPIO_Pin_7;	 
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;	
    GPIO_Init(GPIOD, &GPIO_InitStructure);	  
    //SAM TX
		GPIO_InitStructure.GPIO_Pin = GPIO_Pin_10;	 
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_OD;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;	
    GPIO_Init(GPIOB, &GPIO_InitStructure);	  
    //SAM RESET
		GPIO_InitStructure.GPIO_Pin = GPIO_Pin_0;	 
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;	
    GPIO_Init(GPIOC, &GPIO_InitStructure);	  
		
		
    //***************************
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_10 | GPIO_Pin_11;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IPU;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;	
    GPIO_Init(GPIOG, &GPIO_InitStructure);	  
		
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_5 | GPIO_Pin_6 | GPIO_Pin_9;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;	
    GPIO_Init(GPIOG, &GPIO_InitStructure);	  		
		
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_3;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;	
    GPIO_Init(GPIOE, &GPIO_InitStructure);	
		
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_11 | GPIO_Pin_12 | GPIO_Pin_13;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;	
    GPIO_Init(GPIOD, &GPIO_InitStructure);	  		
		
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_2;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;	
    GPIO_Init(GPIOG, &GPIO_InitStructure);	  		
	} 
	else{
	  //--------------------------PORT A
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_0;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IPD;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;	
    GPIO_Init(GPIOA, &GPIO_InitStructure);	  		

    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_3|GPIO_Pin_10;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;	
    GPIO_Init(GPIOA, &GPIO_InitStructure);	  		

    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_1|GPIO_Pin_8;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;	
    GPIO_Init(GPIOA, &GPIO_InitStructure);	  		

    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_2|GPIO_Pin_9;	 
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;	
    GPIO_Init(GPIOA, &GPIO_InitStructure);	  

	
	  //--------------------------PORT B
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_0;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IPD;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;	
    GPIO_Init(GPIOB, &GPIO_InitStructure);	  		

    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_6;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IPU;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;	
    GPIO_Init(GPIOB, &GPIO_InitStructure);	  		

    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_11;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;	
    GPIO_Init(GPIOB, &GPIO_InitStructure);	  		

    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_1|GPIO_Pin_4|GPIO_Pin_5|GPIO_Pin_7|GPIO_Pin_12;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;	
    GPIO_Init(GPIOB, &GPIO_InitStructure);	  		

    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_10;	 
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;	
    GPIO_Init(GPIOB, &GPIO_InitStructure);	  		
	
	  //--------------------------PORT C
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_4|GPIO_Pin_5|GPIO_Pin_6;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IPU;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;	
    GPIO_Init(GPIOC, &GPIO_InitStructure);	  		

    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_2;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IPU;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;	
    GPIO_Init(GPIOC, &GPIO_InitStructure);	  		

    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_1|GPIO_Pin_3|GPIO_Pin_4|GPIO_Pin_5|GPIO_Pin_7|GPIO_Pin_13;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;	
    GPIO_Init(GPIOC, &GPIO_InitStructure);	  		

    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_12;	 
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;	
    GPIO_Init(GPIOC, &GPIO_InitStructure);	  		
	
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_8|GPIO_Pin_9|GPIO_Pin_10|GPIO_Pin_11;	 
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;	
    GPIO_Init(GPIOC, &GPIO_InitStructure);	  		

    //--------------------------PORT D
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_3|GPIO_Pin_6;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;	
    GPIO_Init(GPIOD, &GPIO_InitStructure);	  		

    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_0|GPIO_Pin_1|GPIO_Pin_2|GPIO_Pin_4|GPIO_Pin_5|GPIO_Pin_8|GPIO_Pin_9|GPIO_Pin_10|GPIO_Pin_11|GPIO_Pin_12|GPIO_Pin_13|GPIO_Pin_14|GPIO_Pin_15;	 
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;	
    GPIO_Init(GPIOD, &GPIO_InitStructure);	  		
	
	  //--------------------------PORT E
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_2|GPIO_Pin_3|GPIO_Pin_4|GPIO_Pin_5|GPIO_Pin_6;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;	
    GPIO_Init(GPIOE, &GPIO_InitStructure);	  		

    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_0|GPIO_Pin_1|GPIO_Pin_7|GPIO_Pin_8|GPIO_Pin_9|GPIO_Pin_10|GPIO_Pin_11|GPIO_Pin_12|GPIO_Pin_13|GPIO_Pin_14|GPIO_Pin_15;	 
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;	
    GPIO_Init(GPIOE, &GPIO_InitStructure);	  		
	
  	//--------------------------PORT F
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_6|GPIO_Pin_7|GPIO_Pin_9;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;	
    GPIO_Init(GPIOF, &GPIO_InitStructure);	  		

    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_8|GPIO_Pin_10|GPIO_Pin_11;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IPU;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;	
    GPIO_Init(GPIOF, &GPIO_InitStructure);	  		

    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_0|GPIO_Pin_1|GPIO_Pin_2|GPIO_Pin_3|GPIO_Pin_4|GPIO_Pin_5|GPIO_Pin_12|GPIO_Pin_13|GPIO_Pin_14|GPIO_Pin_15;	 
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;	
    GPIO_Init(GPIOF, &GPIO_InitStructure);	  		
	
	  //--------------------------PORT G
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_9|GPIO_Pin_11|GPIO_Pin_13|GPIO_Pin_15;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;	
    GPIO_Init(GPIOG, &GPIO_InitStructure);	  		

    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_6|GPIO_Pin_7|GPIO_Pin_8|GPIO_Pin_14;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IPU;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;	
    GPIO_Init(GPIOG, &GPIO_InitStructure);	  		

    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_0|GPIO_Pin_1|GPIO_Pin_2|GPIO_Pin_3|GPIO_Pin_4|GPIO_Pin_5|GPIO_Pin_10|GPIO_Pin_12|GPIO_Pin_14;	 
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;	
    GPIO_Init(GPIOG, &GPIO_InitStructure);	  		
	}
}
#endif

//==============================================================================
#ifdef Torgheh
void GPIO_Config(void)
{
GPIO_InitTypeDef GPIO_InitStructure;

		BOARD_VER=VII;
	
		RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOD | RCC_APB2Periph_GPIOB | RCC_APB2Periph_GPIOA | RCC_APB2Periph_GPIOC | RCC_APB2Periph_AFIO
								,ENABLE);	
	
  	//--------------------------TFT
  	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_0 | GPIO_Pin_1 | GPIO_Pin_2 | GPIO_Pin_3 | 
                                GPIO_Pin_4 | GPIO_Pin_5 | GPIO_Pin_6 | GPIO_Pin_7;
  	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;					  
  	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;						   
  	GPIO_Init(GPIOC, &GPIO_InitStructure);

  	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_1 | GPIO_Pin_2; 
		GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
		GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
		GPIO_Init(GPIOB, &GPIO_InitStructure);

  	//--------------------------SpeedSensor
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_0;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IPU;
	  GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_Init(GPIOA, &GPIO_InitStructure);

  	//--------------------------USART1
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_9;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;
	  GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_Init(GPIOA, &GPIO_InitStructure);

    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_10;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;
    GPIO_Init(GPIOA, &GPIO_InitStructure);

  	//--------------------------USART3
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_10;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;
	  GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_Init(GPIOB, &GPIO_InitStructure);

    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_11;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;
    GPIO_Init(GPIOB, &GPIO_InitStructure);

  	//--------------------------USART2
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_2;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;
	  GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_Init(GPIOA, &GPIO_InitStructure);

    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_3;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;
    GPIO_Init(GPIOA, &GPIO_InitStructure);

    //--------------------------Shift Register
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_8 | GPIO_Pin_9;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;	
    GPIO_Init(GPIOB, &GPIO_InitStructure);	  		
    
		GPIO_InitStructure.GPIO_Pin = GPIO_Pin_13;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;	
    GPIO_Init(GPIOC, &GPIO_InitStructure);	  		

    //--------------------------FLASH CS
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_12;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;	
    GPIO_Init(GPIOB, &GPIO_InitStructure);	  		

    //--------------------------FLASH2 CS
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_0;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;	
    GPIO_Init(GPIOB, &GPIO_InitStructure);	  		

    //--------------------------GSM
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_6;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;	
    GPIO_Init(GPIOA, &GPIO_InitStructure);	  		
		
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_7;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IPU;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;	
    GPIO_Init(GPIOA, &GPIO_InitStructure);	  		

    //--------------------------Reader
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_3;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;	
    GPIO_Init(GPIOB, &GPIO_InitStructure);	  		

    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_8;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;	
    GPIO_Init(GPIOA, &GPIO_InitStructure);	  		

    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_8;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IPU;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;	
    GPIO_Init(GPIOC, &GPIO_InitStructure);	  		

    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_9;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;	
    GPIO_Init(GPIOC, &GPIO_InitStructure);	  		

    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_13;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;	
    GPIO_Init(GPIOA, &GPIO_InitStructure);	  		

    //--------------------------Keypad
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_7;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;	
    GPIO_Init(GPIOB, &GPIO_InitStructure);	  		

    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_12;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;	
    GPIO_Init(GPIOA, &GPIO_InitStructure);	  		

    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_5|GPIO_Pin_6;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IPU;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;	
    GPIO_Init(GPIOB, &GPIO_InitStructure);	
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_2;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IPU;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;	
    GPIO_Init(GPIOD, &GPIO_InitStructure);	
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_12;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IPU;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;	
    GPIO_Init(GPIOC, &GPIO_InitStructure);	

    //--------------------------DAC
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_4;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;	
    GPIO_Init(GPIOA, &GPIO_InitStructure);	 

    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_4;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;	
    GPIO_Init(GPIOB, &GPIO_InitStructure);	
    
    VOICEEN=0;		
		GPIO_PinRemapConfig(GPIO_Remap_SWJ_Disable, ENABLE);
		//-------------------------TWI
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_14 | GPIO_Pin_15;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;	
    GPIO_Init(GPIOA, &GPIO_InitStructure);	  		

  	//--------------------------USART4
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_10;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;
	  GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_Init(GPIOC, &GPIO_InitStructure);

    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_11;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;
    GPIO_Init(GPIOC, &GPIO_InitStructure);

}
#endif

//==============================================================================
void  BSP_Init(void) {
	PROGBAR_Handle ahProgBar;
	short i;
	SD_CardInfo SDCardInfo;
	char str[100];
	
	NVIC_SetVectorTable(NVIC_VectTab_FLASH, 0x0);
  NVIC_PriorityGroupConfig(NVIC_PriorityGroup_4);
  SysTick_Config(SystemCoreClock / 1000);
  RCC_AHBPeriphClockCmd(RCC_AHBPeriph_CRC, ENABLE);

	GPIO_Config();

	if (LCDCHECK == 0)
		LCDType = LCD800_480;
	else
		LCDType = LCD480_272;
	
	//Written by Hesam Nickhou
	if ((DeviceType == BUSDOOR_WIFI) || (DeviceType == APARK))
		LCDType = LCD480_272;
	else if (DeviceType == BUSDOOR)
		LCDType = LCD800_480;
	//=============================HNA
	
	
	GUI_Init();
  WM_SetCreateFlags(WM_CF_MEMDEV);
	PROGBAR_SetDefaultSkin(PROGBAR_SKIN_FLEX);

	TEXT_SetDefaultWrapMode(1);
  GUI_UC_SetEncodeUTF8();
	GUI_UC_EnableBIDI(1);
	GUI_SetTextMode(GUI_TM_TRANS);

  SPI_Flash_Init();		

  #ifdef Simorgh50N
	if (LCDType == LCD800_480) {
	  ahProgBar = PROGBAR_Create(72, 300, 128 + SCREENRESIZE, 20, WM_CF_SHOW);
    G_LoadBMP(180, 80, "0:ani/A0000/0028.bmp", 0);
	}
	else {
    ahProgBar = PROGBAR_Create(20, 95, 232, 20, WM_CF_SHOW);
    G_LoadBMP(80, -15, "0:ani/A0000/0028.bmp", 0);
	}
	#endif
  #ifdef Torgheh
  ahProgBar = PROGBAR_Create(20, 95, 180, 20, WM_CF_SHOW);
  G_LoadBMP(80, -15, "0:ani/A0000/0028.bmp", 0);
	#endif

  PROGBAR_SetValue(ahProgBar, 1);
  GUI_Delay(1);
 	uart_init(57600); 	


	GUI_SetColor(GUI_LIGHTGREEN);
 	switch (BOARD_VER)	{
		case  VI : GUI_DispStringAt("HW: V1", 10, 460); break;
		case VII : GUI_DispStringAt("HW: V2", 10, 460); break;
		default  : GUI_SetColor(GUI_RED); GUI_DispStringAt("HW: Unknown", 80, 220); break;
	}
			
  sprintf(str,"V%d.%03d",Ver,Release);
	PutText(10, 440, 220, 480, str, GUI_TA_LEFT);	

	if (SPI_Flash_ReadID() == W25Q64)
    LoadConfiguration();	
  PROGBAR_SetValue(ahProgBar, 30);

	LoadFont(FontBKoodak40);
	
	#ifdef Simorgh50N
  GUI_SetColor(GUI_MAGENTA);
	PutText(0, 15, 270+201, 50, "سيمرغ جهانگستر", GUI_TA_RIGHT);
  GUI_SetColor(GUI_WHITE);
	#if (DeviceType == BUSDOOR)
	PutText(0, -10, 270, 25, "درب اتوبوس", GUI_TA_RIGHT);
	#endif
	#if (DeviceType==APARK)
		//PutText(0, -10, 270, 25, "شهر شهربازي", GUI_TA_RIGHT);
		PutText(0, 10, 270, 45, "شهربازي پالاديوم", GUI_TA_CENTER);
	#endif
	#endif

  GUI_SetColor(GUI_CYAN);

 	uart2_init(9600); 	
 	uart3_init(115200); //wifi
	#ifdef Simorgh50N
	//GSMEN=1;
	#endif
	//GSMPWR=0;
	if (BOARD_VER == VI)	{
    LEDBK  = 1;
	  LEDPWR = 1;
	}
	else {
		LEDPS1 = 1;
		LEDPS2 = 1;
		LEDPS3 = 1;
		LEDPS4 = 1;
	}
	
  memset(&fs, 0, sizeof(FATFS));      // Clear file system object
  if (SD_Init() == SD_OK) {
		SDCardPresent = 1;
  	GUI_SetColor(GUI_LIGHTGREEN);
  	f_mount(&fs, "", 0);
		printf("\n\rMicroSD OK");
	}
	else{
		SDCardPresent = 0;
		GUI_SetColor(GUI_RED);
		printf("\n\rMicroSD ERROR");
	}
 	GUI_DispStringAt("MicroSD", 400, 220);
	
  PROGBAR_SetValue(ahProgBar, 20);
  GUI_Delay(1);

	RTC_Init();
	GSMPWR = 1;
  
	DAC_Configuration();
	
	
/*
	while (1)
	{
 		WDTR;
		VOICEEN=1;
		PlayVoice("04.wav");
		VOICEEN=0; 
		GUI_Delay(3000);
		VOICEEN=1;
		PlayVoice("96.wav");
		VOICEEN=0; 
		GUI_Delay(1000);		
	}
*/
	
	for (i=40; i<80; i++) {
    PROGBAR_SetValue(ahProgBar, i);
	  GUI_Delay(20);
	}	
	
	#ifdef Torgheh
  GPIO_PinRemapConfig(GPIO_Remap_SWJ_Disable, ENABLE);
	R_RST=0;
	#endif

	i = 159;

  #ifdef CLRC_CHIP
	i=InitializeRdLib();
	#else
		MFRC531_Reset();
	#endif

	sprintf(str, " %d * ",i );
	PutText(0, 160, 480, 220, str, GUI_TA_CENTER);

	#ifdef WithSAMCARD
	SAM_Init();
  #endif
	
  PROGBAR_Delete(ahProgBar);
	GUI_Exec();

	
	#if (DeviceType == Simorgh)
	stm32_IwdgSetup();
	#endif
	
}


//==============================================================================
void InterruptConfig(void)
{

}

//==============================================================================
unsigned char SPI_WriteByte(unsigned char data) 
{ 
 unsigned char Data = 0; 

   //Wait until the transmit buffer is empty 
  while (SPI_I2S_GetFlagStatus(SPI2,SPI_I2S_FLAG_TXE)==RESET); 
  // Send the byte  
  SPI_I2S_SendData(SPI2,data); 

   //Wait until a data is received 
  while (SPI_I2S_GetFlagStatus(SPI2,SPI_I2S_FLAG_RXNE)==RESET); 
  // Get the received data 
  Data = SPI_I2S_ReceiveData(SPI2); 

  // Return the shifted data 
  return Data; 
}  

//==============================================================================
void SpiDelay(unsigned int DelayCnt) {
	unsigned int i;
  for (i = 0; i < DelayCnt; i++);
}

//==============================================================================
u16 TPReadX(void) { 
	u16 x = 0;
	TP_CS();
	SpiDelay(10);
	SPI_WriteByte(0x90);
	SpiDelay(10);
	x = SPI_WriteByte(0x00);
	x <<= 8;
	x += SPI_WriteByte(0x00);
	SpiDelay(10);
	TP_DCS(); 
	x = x >> 3;
	return (x);
}

//==============================================================================
u16 TPReadY(void) {
	u16 y=0;
	TP_CS();
	SpiDelay(10);
	SPI_WriteByte(0xD0);
	SpiDelay(10);
	y=SPI_WriteByte(0x00);
	y <<= 8;
	y += SPI_WriteByte(0x00);
	SpiDelay(10);
	TP_DCS();
	y = y >> 3; 
	return (y);
}

