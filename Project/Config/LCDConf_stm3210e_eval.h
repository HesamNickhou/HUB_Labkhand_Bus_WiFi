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
File        : LCDConf.h
Purpose     : Display driver configuration file
---------------------------END-OF-HEADER------------------------------
*/

/**
  ******************************************************************************
  * @file    LCDConf.h
  * @author  MCD Application Team
  * @version V1.1.2
  * @date    28-January-2014
  * @brief   Display driver configuration file
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

#ifndef LCDCONF_H
#define LCDCONF_H

#include "GUI.h"
#include "GUIDRV_FlexColor.h"
#include "global_includes.h"

#include "Config.h"

/*********************************************************************
*
*       Layer configuration (to be modified)
*
**********************************************************************
*/

//
// Physical display size
//
#ifdef Simorgh50N


#endif

#ifdef Torgheh
#define XSIZE_PHYS  176
#define YSIZE_PHYS  220
#endif

/*********************************************************************
*
*       Configuration checking
*
**********************************************************************
*/

/*
#ifndef   VXSIZE_PHYS
  #define VXSIZE_PHYS XSIZE_PHYS
#endif
#ifndef   VYSIZE_PHYS
  #define VYSIZE_PHYS YSIZE_PHYS
#endif
#ifndef   XSIZE_PHYS
  #error Physical X size of display is not defined!
#endif
#ifndef   YSIZE_PHYS
  #error Physical Y size of display is not defined!
#endif
*/
#ifndef   GUICC_565
  #error Color conversion not defined!
#endif
#ifndef   GUIDRV_FLEXCOLOR
  #error No display driver defined!
#endif

/*********************************************************************
*
*       Defines, sfrs
*
**********************************************************************
*/
//
// COG interface register addr.
//
#define LCD_BASE           ((uint32_t)(0x6C000000 | 0x000007FE))
	//((uint32_t)(0x6C000000 | 0x000007FE))
#define LCD_REG_ADDRESS   (*(volatile U16*)LCD_BASE)
#define LCD_DATA_ADDRESS  (*(volatile U16*)(LCD_BASE + 2))

#endif /* LCDCONF_H */

/*************************** End of file ****************************/
