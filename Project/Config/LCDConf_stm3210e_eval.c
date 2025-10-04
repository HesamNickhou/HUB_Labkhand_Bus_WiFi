/*********************************************************************
*          Portions COPYRIGHT 2014 STMicroelectronics                *
*          Portions SEGGER Microcontroller GmbH & Co. KG             *
*        Solutions for real time microcontroller applications        *
**********************************************************************
*                                                                    *
*        (c) 1996 - 2013  SEGGER Microcontroller GmbH & Co. KG       *
*                                                                    *
*        Internet: www.segger.com    Support:  support@segger.com    *
*                                                                    *
**********************************************************************

** emWin V5.22 - Graphical user interface for embedded applications **
All  Intellectual Property rights  in the Software belongs to  SEGGER.
emWin is protected by  international copyright laws.  Knowledge of the
source code may not be used to write a similar product.  This file may
only be used in accordance with the following terms:

The  software has  been licensed  to STMicroelectronics International
N.V. a Dutch company with a Swiss branch and its headquarters in Plan-
les-Ouates, Geneva, 39 Chemin du Champ des Filles, Switzerland for the
purposes of creating libraries for ARM Cortex-M-based 32-bit microcon_
troller products commercialized by Licensee only, sublicensed and dis_
tributed under the terms and conditions of the End User License Agree_
ment supplied by STMicroelectronics International N.V.
Full source code is available at: www.segger.com

We appreciate your understanding and fairness.
----------------------------------------------------------------------
File        : LCDConf.c
Purpose     : Display controller configuration (single layer)
---------------------------END-OF-HEADER------------------------------
*/

/**
  ******************************************************************************
  * @file    LCDConf_stm3210e_eval.c
  * @author  MCD Application Team
  * @version V1.1.2
  * @date    28-January-2014
  * @brief   Driver for STM3210E-EVAL board LCD
  ******************************************************************************
  * @attention
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


#include <math.h>
#include <LCDConf_stm3210e_eval.h>
#include <app_cfg.h>


extern unsigned int XSIZE_PHYS;
extern unsigned int YSIZE_PHYS;


#ifdef Torgheh

#include "ili9225g.h"

#define TFTRSPin   GPIO_Pin_2
#define TFTRSPort  	GPIOB
#define TFT_RS(SetValue)  				 ((SetValue) ? (GPIO_SetBits(TFTRSPort,TFTRSPin)) : (GPIO_ResetBits(TFTRSPort,TFTRSPin)))

#define TFTWRPin   GPIO_Pin_1
#define TFTWRPort  	GPIOB
#define TFT_WR(SetValue)  				 ((SetValue) ? (GPIO_SetBits(TFTWRPort,TFTWRPin)) : (GPIO_ResetBits(TFTWRPort,TFTWRPin)))

unsigned short BkColor=0x0;
#endif

/********************************************************************
*
*       LcdWriteReg
*
* Function description:
*   Sets display register
*/
#ifdef Torgheh
void LcdWriteReg(U16 Data) {
#endif
#ifdef Simorgh50N
static void LcdWriteReg(U16 Data) {
#endif
	#ifdef Torgheh
	u32 mask;
	
	TFTRSPort->BRR = TFTRSPin;
	mask=GPIOC->ODR;
	mask&=0xff00;
	mask|=(Data&0xFF);
	GPIOC->ODR=mask;
	TFTWRPort->BRR = TFTWRPin;
	TFTWRPort->BSRR = TFTWRPin;
	TFTRSPort->BSRR = TFTRSPin;
	#endif
	
	#ifdef Simorgh50N
  LCD_REG_ADDRESS = Data;
	#endif
}

/********************************************************************
*
*       LcdWriteData
*
* Function description:
*   Writes a value to a display register
*/
#ifdef Torgheh
void LcdWriteData(U16 Data) {
#endif
#ifdef Simorgh50N
static void LcdWriteData(U16 Data) {
#endif
	#ifdef Torgheh
	u32 mask;
	u8 data8 = (Data>>8);
	
	mask=GPIOC->ODR;
	mask&=0xff00;
	mask|=data8;
	GPIOC->ODR=mask;
	TFTWRPort->BRR = TFTWRPin;
	TFTWRPort->BSRR = TFTWRPin;
		
	//data8 = Data;
	mask&=0xff00;
	mask|=(Data&0xFF);
	GPIOC->ODR=mask;
	
	TFTWRPort->BRR = TFTWRPin;
	TFTWRPort->BSRR = TFTWRPin;
	#endif

	#ifdef Simorgh50N
  LCD_DATA_ADDRESS = Data;
	#endif
}

/********************************************************************
*
*       LcdWriteDataMultiple
*
* Function description:
*   Writes multiple values to a display register.
*/
static void LcdWriteDataMultiple(U16 * pData, int NumItems) {
  while (NumItems--) {
  	#ifdef Torgheh
    LcdWriteData(*pData++);
	  #endif
		
  	#ifdef Simorgh50N
    LCD_DATA_ADDRESS = *pData++;
		#endif
  }
}

/********************************************************************
*
*       LcdReadDataMultiple
*
* Function description:
*   Reads multiple values from a display register.
*/
static void LcdReadDataMultiple(U16 * pData, int NumItems) {
  while (NumItems--) {
  	#ifdef Simorgh50N
    *pData++ = LCD_DATA_ADDRESS;
		#endif
  	#ifdef Torgheh
		*pData++ = BkColor;
		#endif
  }
}

static U16 LcdReadData(void) 
{
	#ifdef Simorgh50N
  return LCD_DATA_ADDRESS;
	#endif
	#ifdef Torgheh
  return BkColor;
	#endif
}
/*********************************************************************
*
*       Public functions
*
**********************************************************************
*/
/*********************************************************************
*
*       LCD_X_Config
*
* Function description:
*   Called during the initialization process in order to set up the
*   display driver configuration.
*
*/

void LCD_X_Config(void) {
GUI_DEVICE * pDevice;
CONFIG_FLEXCOLOR Config = {0};
GUI_PORT_API PortAPI = {0};

#ifdef WVGA
  if(LCDType == LCD480_272) {
    XSIZE_PHYS = 480;
    YSIZE_PHYS = 272;
  }
	
	else {
    XSIZE_PHYS = 800;
    YSIZE_PHYS = 480;
	}
	
#endif

	#ifdef Simorgh50N

  //
  // Set display driver and color conversion
  //
  pDevice = GUI_DEVICE_CreateAndLink(GUIDRV_FLEXCOLOR, GUICC_M565, 0, 0);	//0xFFFF -> RRRRRRGGGGGBBBBB
  //
  // Display driver configuration, required for Lin-driver
  //
  LCD_SetSizeEx (0, XSIZE_PHYS , YSIZE_PHYS);
  LCD_SetVSizeEx(0, XSIZE_PHYS, YSIZE_PHYS);

	  Config.Orientation = GUI_MIRROR_Y | GUI_SWAP_XY;
  //
  // Orientation
	//Config.Orientation = 0;
	
	Config.NumDummyReads = 0;
	
  GUIDRV_FlexColor_Config(pDevice, &Config);
  //
  // Set controller and operation mode
  //
  PortAPI.pfWrite16_A0  = LcdWriteReg;
  PortAPI.pfWrite16_A1  = LcdWriteData;
  PortAPI.pfWriteM16_A1 = LcdWriteDataMultiple;
  PortAPI.pfRead16_A1   = LcdReadData;
  PortAPI.pfReadM16_A1  = LcdReadDataMultiple;
  GUIDRV_FlexColor_SetFunc(pDevice, &PortAPI, GUIDRV_FLEXCOLOR_F66720, GUIDRV_FLEXCOLOR_M16C0B16);	//SSD1963	
  #endif	
	
	#ifdef Torgheh
    HX8352C_HW_API hwAPI = {0};

    // Set display driver and color conversion
    pDevice = GUI_DEVICE_CreateAndLink(GUIDRV_HX8352C, GUICC_M565  , 0, 0);
    // Display driver is compile-time configured. Setting (virtual) screen size is not necessary
    LCD_SetSizeEx (0, XSIZE_PHYS,  YSIZE_PHYS);
    LCD_SetVSizeEx (0, XSIZE_PHYS,  YSIZE_PHYS);
		
		
	  Config.Orientation = GUI_MIRROR_Y | GUI_SWAP_XY;
		//GUI_SetOrientation();
		
    // Port access functions
    hwAPI.pfWriteReg = LcdWriteReg;
    hwAPI.pfWriteData = LcdWriteData;
    //hwAPI.pfWriteRepeatingData = lcdWriteRepeatingData;
    hwAPI.pfWriteMultipleData = LcdWriteDataMultiple;
    hwAPI.pfReadData = LcdReadData;
    hwAPI.pfReadMultipleData = LcdReadDataMultiple;
    // Give driver access to the functions
    GUIDRV_HX8352C_SetFunc(pDevice, &hwAPI);	
	#endif
	
}

#ifdef Torgheh
//==========================
void Write_LCDReg(u16 RDataIn , u16 DDataIn)
{
	LcdWriteReg(RDataIn);
	LcdWriteData(DDataIn);
}

//==========================
void Init_ili9225(void)
{
	SDA=1;
	SCL=1;
	GUI_Delay(50);
	SDA=0;
	SCL=0;
	GUI_Delay(200);
	SDA=1;
	SCL=1;
	Write_LCDReg(0x0001, 0x011C);        // set SS and NL bit 
	Write_LCDReg(0x0002, 0x0100);        // set 1 line inversion 
	Write_LCDReg(0x0003, 0x1030);        // set GRAM write direction and BGR=1.   
	Write_LCDReg(0x0008, 0x0808);        // set BP and FP 
	Write_LCDReg(0x000C, 0x0000);        // RGB interface setting    R0Ch=0x0110 for RGB 18Bit and R0Ch=0111for RGB16Bit         
	Write_LCDReg(0x000F, 0x0B01);        // Set frame rate 
	Write_LCDReg(0x0020, 0x0000);            // Set GRAM Address 
	Write_LCDReg(0x0021, 0x0000);            // Set GRAM Address 
	//*************Power On sequence ****************// 
	GUI_Delay(50);
	Write_LCDReg(0x0010, 0x0A00);            // Set SAP,DSTB,STB 
	Write_LCDReg(0x0011, 0x1038);            // Set APON,PON,AON,VCI1EN,VC 
	GUI_Delay(50);
	Write_LCDReg(0x0012, 0x1121);            // Internal reference voltage= Vci; 
	Write_LCDReg(0x0013, 0x0066);            // Set GVDD 
	Write_LCDReg(0x0014, 0x5F60);            // Set VCOMH/VCOML voltage 
	//------------------------ Set GRAM area --------------------------------// 
	Write_LCDReg (0x30, 0x0000);   
	Write_LCDReg (0x31, 0x00DB);   
	Write_LCDReg (0x32, 0x0000);   
	Write_LCDReg (0x33, 0x0000);   
	Write_LCDReg (0x34, 0x00DB);   
	Write_LCDReg (0x35, 0x0000);   
	Write_LCDReg (0x36, 0x00AF);   
	Write_LCDReg (0x37, 0x0000);   
	Write_LCDReg (0x38, 0x00DB);   
	Write_LCDReg (0x39, 0x0000);     
	// ----------- Adjust the Gamma    Curve ----------// 
	Write_LCDReg(0x0050, 0x0400); 
	Write_LCDReg(0x0051, 0x060B); 
	Write_LCDReg(0x0052, 0x0C0A); 
	Write_LCDReg(0x0053, 0x0105); 
	Write_LCDReg(0x0054, 0x0A0C); 
	Write_LCDReg(0x0055, 0x0B06); 
	Write_LCDReg(0x0056, 0x0004); 
	Write_LCDReg(0x0057, 0x0501); 
	Write_LCDReg(0x0058, 0x0E00); 
	Write_LCDReg(0x0059, 0x000E); 
	GUI_Delay(50);
	Write_LCDReg(0x0007, 0x1017);
}

#endif

/*********************************************************************
*
*       LCD_X_DisplayDriver
*
* Function description:
*   This function is called by the display driver for several purposes.
*   To support the according task the routine needs to be adapted to
*   the display controller. Please note that the commands marked with
*   'optional' are not cogently required and should only be adapted if
*   the display controller supports these features.
*
* Parameter:
*   LayerIndex - Index of layer to be configured
*   Cmd        - Please refer to the details in the switch statement below
*   pData      - Pointer to a LCD_X_DATA structure
*
* Return Value:
*   < -1 - Error
*     -1 - Command not handled
*      0 - Ok
*/
int LCD_X_DisplayDriver(unsigned LayerIndex, unsigned Cmd, void * pData) {
  int r;
  (void) LayerIndex;
  (void) pData;
  
  switch (Cmd) {
  case LCD_X_INITCONTROLLER: {
		#ifdef Simorgh50N
    STM3210E_LCD_Init();
		#endif
		#ifdef Torgheh
		Init_ili9225();
		#endif
    return 0;
  }
  default:
    r = -1;
  }
  return r;
}

/*************************** End of file ****************************/

