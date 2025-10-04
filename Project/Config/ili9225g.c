/**
  ******************************************************************************
  * @file    GUIDRV_HX8352C.c
  * @author  Baoshi
  * @version 0.2
  * @date    19-Dec-2013
  * @brief   Driver for HX8352C based LCD
  ******************************************************************************
  * @attention
  *
  * This code is derivative work based on the STemWin which is licensed under
  * MCD-ST Liberty SW License Agreement V2, available at:
  *
  *     http://www.st.com/software_license_agreement_liberty_v2
  *
  * However this source code itself is licensed under (CC BY-SA 3.0), available at:
  *
  *     http://creativecommons.org/licenses/by-sa/3.0
  *
  * User should pay attention to the dual license embarrassment and when in doubt,
  * MCD-ST Liberty SW License Agreement V2 shall take precedence.
  *
  * THIS SOFTWARE IS PROVIDED 'AS-IS', WITHOUT ANY EXPRESS OR IMPLIED
  * WARRANTY.  IN NO EVENT WILL THE AUTHORS BE HELD LIABLE FOR ANY DAMAGES
  * ARISING FROM THE USE OF THIS SOFTWARE.
  *
  ******************************************************************************
  */
#ifdef Torgheh

#include <includes.h>
#include <stdint.h>

#include "GUI_Private.h"
#include "ili9225g.h"



/*===========================================================================*/
/* Defines                                                                   */
/*===========================================================================*/

/**
 * @{
 * Physical LCD panel size regardless of rotation/mirroring
 */
#define XSIZE_PHYS  176
#define YSIZE_PHYS  220
/** @} */


/**
 * @{
 * Macros to handle MIRROR_, SWAP_ combinations
 */
#if (!LCD_MIRROR_X && !LCD_MIRROR_Y && !LCD_SWAP_XY)
    // Rotate 0
    #define LCD_XSIZE XSIZE_PHYS
    #define LCD_YSIZE YSIZE_PHYS
    #define LOG2PHYS_X(x, y) x
    #define LOG2PHYS_Y(x, y) y
#elif (!LCD_MIRROR_X && !LCD_MIRROR_Y && LCD_SWAP_XY)
    #define LCD_XSIZE YSIZE_PHYS
    #define LCD_YSIZE XSIZE_PHYS
    #define LOG2PHYS_X(x, y) x
    #define LOG2PHYS_Y(x, y) y
#elif (!LCD_MIRROR_X && LCD_MIRROR_Y && !LCD_SWAP_XY)
    #define LCD_XSIZE XSIZE_PHYS
    #define LCD_YSIZE YSIZE_PHYS
    #define LOG2PHYS_X(x, y) x
    #define LOG2PHYS_Y(x, y) 32 + (y)
#elif (!LCD_MIRROR_X && LCD_MIRROR_Y && LCD_SWAP_XY)
    // Rotate 270
    #define LCD_XSIZE YSIZE_PHYS
    #define LCD_YSIZE XSIZE_PHYS
    #define LOG2PHYS_X(x, y) 32 + (x)
    #define LOG2PHYS_Y(x, y) y
#elif (LCD_MIRROR_X && !LCD_MIRROR_Y && !LCD_SWAP_XY)
    #define LCD_XSIZE XSIZE_PHYS
    #define LCD_YSIZE YSIZE_PHYS
    #define LOG2PHYS_X(x, y) x
    #define LOG2PHYS_Y(x, y) y
#elif (LCD_MIRROR_X && !LCD_MIRROR_Y && LCD_SWAP_XY)
    // Rotate 90
    #define LCD_XSIZE YSIZE_PHYS
    #define LCD_YSIZE XSIZE_PHYS
    #define LOG2PHYS_X(x, y) x
    #define LOG2PHYS_Y(x, y) y
#elif (LCD_MIRROR_X && LCD_MIRROR_Y && !LCD_SWAP_XY)
    // Rotate 180
    #define LCD_XSIZE XSIZE_PHYS
    #define LCD_YSIZE YSIZE_PHYS
    #define LOG2PHYS_X(x, y) x
    #define LOG2PHYS_Y(x, y) 32 + (y)
#elif (LCD_MIRROR_X &&  LCD_MIRROR_Y &&  LCD_SWAP_XY)
    #define LCD_XSIZE YSIZE_PHYS
    #define LCD_YSIZE XSIZE_PHYS
    #define LOG2PHYS_X(x, y) 32 + (x)
    #define LOG2PHYS_Y(x, y) y
#endif
/** @} */


extern unsigned int BkColor;

/*===========================================================================*/
/* Types                                                                     */
/*===========================================================================*/


/**
 * @brief   Driver private data
 */
typedef struct _DRIVER_CONTEXT
{
    //! Hardware clip rectangle depends on screen rotation
    LCD_RECT hwClip;  // hardware clip rectangle
    // LCD access functions
    //! Write 16-bit data to LCD (RS = 0)
    void (*pfWriteReg)(U16 data);
    //! Write 16-bit data to LCD (RS = 1)
  	void (*pfWriteData)(U16 data);
    //! Write data buffer to LCD (RS = 0)
    void (*pfWriteMultipleData)(U16 *pData,  int count);
    //! Write same data to LCD multiple times (RS = 0)
    void (*pfWriteRepeatingData)(U16 data,  int count);
    //! Read Register from LCD (RS = 1)
    U16 (*pfReadReg)(void);
    //! Read GRAM from LCD (RS = 1)
    U16 (*pfReadData)(void);
    //! Read multiple 16-bit data from LCD GRAM (RS = 1)
    void (*pfReadMultipleData)(U16 *pData,  int count);
} DRIVER_CONTEXT;


/*===========================================================================*/
/* Static Functions                                                          */
/*===========================================================================*/

#ifdef Torgheh
#define TFTRSPin   GPIO_Pin_2
#define TFTRSPort  	GPIOB
#define TFT_RS(SetValue)  				 ((SetValue) ? (GPIO_SetBits(TFTRSPort,TFTRSPin)) : (GPIO_ResetBits(TFTRSPort,TFTRSPin)))

#define TFTWRPin   GPIO_Pin_1
#define TFTWRPort  	GPIOB
#define TFT_WR(SetValue)  				 ((SetValue) ? (GPIO_SetBits(TFTWRPort,TFTWRPin)) : (GPIO_ResetBits(TFTWRPort,TFTWRPin)))
#endif

/**
 * @brief   Set the index (color) of a pixel
 * @note    Caller ensures the coordinates are in range.
 * @param   pDevice Device context
 * @param   x       X coordinate
 * @param   y       Y coordinate
 * @param   color   Color (or index) of the pixel
 */
static void _SetPixelIndex(GUI_DEVICE *pDevice, int x, int y, int color)
{
	
	#ifdef Torgheh
unsigned int mask;
u8 data8;
	
  //pContext->pfWriteReg(0x0021);	
	TFTRSPort->BRR = TFTRSPin;
	mask=GPIOC->ODR;
	mask&=0xff00;
	mask|=0x21;
	GPIOC->ODR=mask;
	TFTWRPort->BRR = TFTWRPin;
	TFTWRPort->BSRR = TFTWRPin;
	TFTRSPort->BSRR = TFTRSPin;

	//pContext->pfWriteData((YSIZE_PHYS-1)-x);
	data8 = (((YSIZE_PHYS-1)-x)>>8);
	
	mask=GPIOC->ODR;
	mask&=0xff00;
	mask|=data8;
	GPIOC->ODR=mask;
	TFTWRPort->BRR = TFTWRPin;
	TFTWRPort->BSRR = TFTWRPin;
		
	data8 = ((YSIZE_PHYS-1)-x);
	mask&=0xff00;
	mask|=data8;
	GPIOC->ODR=mask;
	
	TFTWRPort->BRR = TFTWRPin;
	TFTWRPort->BSRR = TFTWRPin;

	
	//pContext->pfWriteReg(0x0020);	
	TFTRSPort->BRR = TFTRSPin;
	mask=GPIOC->ODR;
	mask&=0xff00;
	mask|=0x20;
	GPIOC->ODR=mask;
	TFTWRPort->BRR = TFTWRPin;
	TFTWRPort->BSRR = TFTWRPin;
	TFTRSPort->BSRR = TFTRSPin;

	
	//pContext->pfWriteData(y);
	data8 = (y>>8);
	
	mask=GPIOC->ODR;
	mask&=0xff00;
	mask|=data8;
	GPIOC->ODR=mask;
	TFTWRPort->BRR = TFTWRPin;
	TFTWRPort->BSRR = TFTWRPin;
		
	data8 = y;
	mask&=0xff00;
	mask|=data8;
	GPIOC->ODR=mask;
	
	TFTWRPort->BRR = TFTWRPin;
	TFTWRPort->BSRR = TFTWRPin;
	
	
	//pContext->pfWriteReg(0x0022);
	TFTRSPort->BRR = TFTRSPin;
	mask=GPIOC->ODR;
	mask&=0xff00;
	mask|=0x22;
	GPIOC->ODR=mask;
	TFTWRPort->BRR = TFTWRPin;
	TFTWRPort->BSRR = TFTWRPin;
	TFTRSPort->BSRR = TFTRSPin;

	//pContext->pfWriteData(color);
	data8 = (color>>8);
	
	mask=GPIOC->ODR;
	mask&=0xff00;
	mask|=data8;
	GPIOC->ODR=mask;
	TFTWRPort->BRR = TFTWRPin;
	TFTWRPort->BSRR = TFTWRPin;
		
	data8 = color;
	mask&=0xff00;
	mask|=data8;
	GPIOC->ODR=mask;
	
	TFTWRPort->BRR = TFTWRPin;
	TFTWRPort->BSRR = TFTWRPin;
	
	
	#else
DRIVER_CONTEXT *pContext;
pContext = (DRIVER_CONTEXT *)pDevice->u.pContext;
	
  pContext->pfWriteReg(0x0021);	
	pContext->pfWriteData((YSIZE_PHYS-1)-x);
	pContext->pfWriteReg(0x0020);	
	pContext->pfWriteData(y);
	pContext->pfWriteReg(0x0022);

	//#define RGB888_RGB565(color) ((((color) >> 19) & 0x1f) << 11) | ((((color) >> 10) & 0x3f) << 5) | (((color) >> 3) & 0x1f)
	
	pContext->pfWriteData(color);
	#endif
	
}


/**
 * @brief   Get the index (color) of a pixel
 * @note    Caller ensures the coordinates are in range.
 * @param   pDevice Device context
 * @param   x       X coordinate
 * @param   y       Y coordinate
 * @return  Color (or index) of the pixel
 */
static unsigned int _GetPixelIndex(GUI_DEVICE *pDevice, int x, int y)
{
    return BkColor;
}


/**
 * @brief   XOR a pixel with color mask
 * @note    It is never called during my debugging. I do not know how and when it worked.
 * @param   pDevice Device context
 * @param   x       X coordinate
 * @param   y       Y coordinate
 */
static void _XorPixel(GUI_DEVICE *pDevice, int x, int y)
{
  LCD_PIXELINDEX PixelIndex;
  LCD_PIXELINDEX IndexMask;

  PixelIndex = _GetPixelIndex(pDevice, x, y);
  IndexMask  = pDevice->pColorConvAPI->pfGetIndexMask();
  _SetPixelIndex(pDevice, x, y, PixelIndex ^ IndexMask);
}

/**
 * @brief   Draw a filled rectangle using selected color
 * @param   pDevice Device context
 * @param   x0      Starting X coordinate
 * @param   x1      Ending X coordinate
 * @param   y0      Starting Y coordinate
 * @param   y1      Ending Y coordinate
 */
static void _FillRect(GUI_DEVICE *pDevice, int x0, int y0, int x1, int y1)
{
	
  LCD_PIXELINDEX PixelIndex;
  int x;

  PixelIndex = LCD__GetColorIndex();
  if (GUI_pContext->DrawMode & LCD_DRAWMODE_XOR) {
    for (; y0 <= y1; y0++) {
      for (x = x0; x <= x1; x++) {
        _XorPixel(pDevice, x, y0);
      }
    }
  } else {
    for (; y0 <= y1; y0++) {
      for (x = x0; x <= x1; x++) {
        _SetPixelIndex(pDevice, x, y0, PixelIndex);
      }
    }
  }	
}

/**
 * @brief   Draw a horizontal line using selected color
 * @param   pDevice Device context
 * @param   x0      Starting X coordinate
 * @param   y       Y coordinate
 * @param   x1      Ending X coordinate
 */
static void _DrawHLine(GUI_DEVICE *pDevice, int x0, int y,  int x1)
{
  _FillRect(pDevice, x0, y, x1, y);
}


/**
 * @brief   Draw a vertical line using selected color
 * @param   pDevice Device context
 * @param   x       X coordinate
 * @param   y0      Starting Y coordinate
 * @param   y1      Ending Y coordinate
 */
static void _DrawVLine(GUI_DEVICE *pDevice, int x, int y0,  int y1)
{
  _FillRect(pDevice, x, y0, x, y1);
}


/*********************************************************************
*
*       Draw Bitmap 1 BPP
*/
static void _DrawBitLine1BPP(GUI_DEVICE * pDevice, int x, int y, U8 const GUI_UNI_PTR * p, int Diff, int xsize, const LCD_PIXELINDEX * pTrans) {
  LCD_PIXELINDEX IndexMask, Index0, Index1, Pixel;

  Index0 = *(pTrans + 0);
  Index1 = *(pTrans + 1);
  x += Diff;
  switch (GUI_pContext->DrawMode & (LCD_DRAWMODE_TRANS | LCD_DRAWMODE_XOR)) {
  case 0:
    do {
      _SetPixelIndex(pDevice, x++, y, (*p & (0x80 >> Diff)) ? Index1 : Index0);
      if (++Diff == 8) {
        Diff = 0;
        p++;
      }
    } while (--xsize);
    break;
  case LCD_DRAWMODE_TRANS:
    do {
      if (*p & (0x80 >> Diff))
        _SetPixelIndex(pDevice, x, y, Index1);
      x++;
      if (++Diff == 8) {
        Diff = 0;
        p++;
      }
    } while (--xsize);
    break;
  case LCD_DRAWMODE_XOR | LCD_DRAWMODE_TRANS:
  case LCD_DRAWMODE_XOR:
    IndexMask = pDevice->pColorConvAPI->pfGetIndexMask();
    do {
      if (*p & (0x80 >> Diff)) {
        Pixel = _GetPixelIndex(pDevice, x, y);
        _SetPixelIndex(pDevice, x, y, Pixel ^ IndexMask);
      }
      x++;
      if (++Diff == 8) {
        Diff = 0;
        p++;
      }
    } while (--xsize);
    break;
  }
}

/*********************************************************************
*
*       Draw Bitmap 2 BPP
*/
static void  _DrawBitLine2BPP(GUI_DEVICE * pDevice, int x, int y, U8 const GUI_UNI_PTR * p, int Diff, int xsize, const LCD_PIXELINDEX * pTrans) {
  LCD_PIXELINDEX Pixels, PixelIndex;
  int CurrentPixel, Shift, Index;

  Pixels = *p;
  CurrentPixel = Diff;
  x += Diff;
  switch (GUI_pContext->DrawMode & (LCD_DRAWMODE_TRANS | LCD_DRAWMODE_XOR)) {
  case 0:
    if (pTrans) {
      do {
        Shift = (3 - CurrentPixel) << 1;
        Index = (Pixels & (0xC0 >> (6 - Shift))) >> Shift;
        PixelIndex = *(pTrans + Index);
        _SetPixelIndex(pDevice, x++, y, PixelIndex);
        if (++CurrentPixel == 4) {
          CurrentPixel = 0;
          Pixels = *(++p);
        }
      } while (--xsize);
    } else {
      do {
        Shift = (3 - CurrentPixel) << 1;
        Index = (Pixels & (0xC0 >> (6 - Shift))) >> Shift;
        _SetPixelIndex(pDevice, x++, y, Index);
        if (++CurrentPixel == 4) {
          CurrentPixel = 0;
          Pixels = *(++p);
        }
      } while (--xsize);
    }
    break;
  case LCD_DRAWMODE_TRANS:
    if (pTrans) {
      do {
        Shift = (3 - CurrentPixel) << 1;
        Index = (Pixels & (0xC0 >> (6 - Shift))) >> Shift;
        if (Index) {
          PixelIndex = *(pTrans + Index);
          _SetPixelIndex(pDevice, x, y, PixelIndex);
        }
        x++;
        if (++CurrentPixel == 4) {
          CurrentPixel = 0;
          Pixels = *(++p);
        }
      } while (--xsize);
    } else {
      do {
        Shift = (3 - CurrentPixel) << 1;
        Index = (Pixels & (0xC0 >> (6 - Shift))) >> Shift;
        if (Index) {
          _SetPixelIndex(pDevice, x, y, Index);
        }
        x++;
        if (++CurrentPixel == 4) {
          CurrentPixel = 0;
          Pixels = *(++p);
        }
      } while (--xsize);
    }
    break;
  }
}

/*********************************************************************
*
*       Draw Bitmap 4 BPP
*/
static void  _DrawBitLine4BPP(GUI_DEVICE * pDevice, int x, int y, U8 const GUI_UNI_PTR * p, int Diff, int xsize, const LCD_PIXELINDEX * pTrans) {
  LCD_PIXELINDEX Pixels, PixelIndex;
  int CurrentPixel, Shift, Index;

  Pixels = *p;
  CurrentPixel = Diff;
  x += Diff;
  switch (GUI_pContext->DrawMode & (LCD_DRAWMODE_TRANS | LCD_DRAWMODE_XOR)) {
  case 0:
    if (pTrans) {
      do {
        Shift = (1 - CurrentPixel) << 2;
        Index = (Pixels & (0xF0 >> (4 - Shift))) >> Shift;
        PixelIndex = *(pTrans + Index);
        _SetPixelIndex(pDevice, x++, y, PixelIndex);
        if (++CurrentPixel == 2) {
          CurrentPixel = 0;
          Pixels = *(++p);
        }
      } while (--xsize);
    } else {
      do {
        Shift = (1 - CurrentPixel) << 2;
        Index = (Pixels & (0xF0 >> (4 - Shift))) >> Shift;
        _SetPixelIndex(pDevice, x++, y, Index);
        if (++CurrentPixel == 2) {
          CurrentPixel = 0;
          Pixels = *(++p);
        }
      } while (--xsize);
    }
    break;
  case LCD_DRAWMODE_TRANS:
    if (pTrans) {
      do {
        Shift = (1 - CurrentPixel) << 2;
        Index = (Pixels & (0xF0 >> (4 - Shift))) >> Shift;
        if (Index) {
          PixelIndex = *(pTrans + Index);
          _SetPixelIndex(pDevice, x, y, PixelIndex);
        }
        x++;
        if (++CurrentPixel == 2) {
          CurrentPixel = 0;
          Pixels = *(++p);
        }
      } while (--xsize);
    } else {
      do {
        Shift = (1 - CurrentPixel) << 2;
        Index = (Pixels & (0xF0 >> (4 - Shift))) >> Shift;
        if (Index) {
          _SetPixelIndex(pDevice, x, y, Index);
        }
        x++;
        if (++CurrentPixel == 2) {
          CurrentPixel = 0;
          Pixels = *(++p);
        }
      } while (--xsize);
    }
    break;
  }
}

/*********************************************************************
*
*       Draw Bitmap 8 BPP
*/
static void  _DrawBitLine8BPP(GUI_DEVICE * pDevice, int x, int y, U8 const GUI_UNI_PTR * p, int xsize, const LCD_PIXELINDEX * pTrans) {
  LCD_PIXELINDEX Pixel;

  switch (GUI_pContext->DrawMode & (LCD_DRAWMODE_TRANS | LCD_DRAWMODE_XOR)) {
  case 0:
    if (pTrans) {
      for (; xsize > 0; xsize--, x++, p++) {
        Pixel = *p;
        _SetPixelIndex(pDevice, x, y, *(pTrans + Pixel));
      }
    } else {
      for (; xsize > 0; xsize--, x++, p++) {
        _SetPixelIndex(pDevice, x, y, *p);
      }
    }
    break;
  case LCD_DRAWMODE_TRANS:
    if (pTrans) {
      for (; xsize > 0; xsize--, x++, p++) {
        Pixel = *p;
        if (Pixel) {
          _SetPixelIndex(pDevice, x, y, *(pTrans + Pixel));
        }
      }
    } else {
      for (; xsize > 0; xsize--, x++, p++) {
        Pixel = *p;
        if (Pixel) {
          _SetPixelIndex(pDevice, x, y, Pixel);
        }
      }
    }
    break;
  }
}

/*********************************************************************
*
*       Draw Bitmap 16 BPP, not optimized
*
* Purpose:
*   Drawing of 16bpp high color bitmaps.
*   Only required for 16bpp color depth of target. Should be removed otherwise.
*/
static void _DrawBitLine16BPP(GUI_DEVICE * pDevice, int x, int y, U16 const GUI_UNI_PTR * p, int xsize) {
  for (;xsize > 0; xsize--, x++, p++) {
    _SetPixelIndex(pDevice, x, y, *p);
  }
}

/*********************************************************************
*
*       Draw Bitmap 32 BPP, not optimized
*
* Purpose:
*   Drawing of 32bpp true color bitmaps.
*   Only required for 32bpp color depth of target. Should be removed otherwise.
*/
static void _DrawBitLine32BPP(GUI_DEVICE * pDevice, int x, int y, U32 const GUI_UNI_PTR * p, int xsize) {
  for (;xsize > 0; xsize--, x++, p++) {
    _SetPixelIndex(pDevice, x, y, *p);
  }
}

/*********************************************************************
*
*       _DrawBitmap
*/
static void _DrawBitmap(GUI_DEVICE * pDevice, int x0, int y0,
                       int xSize, int ySize,
                       int BitsPerPixel, 
                       int BytesPerLine,
                       const U8 GUI_UNI_PTR * pData, int Diff,
                       const LCD_PIXELINDEX * pTrans) {
  int i;

  switch (BitsPerPixel) {
  case 1:
    for (i = 0; i < ySize; i++) {
      _DrawBitLine1BPP(pDevice, x0, i + y0, pData, Diff, xSize, pTrans);
      pData += BytesPerLine;
    }
    break;
  case 2:
    for (i = 0; i < ySize; i++) {
      _DrawBitLine2BPP(pDevice, x0, i + y0, pData, Diff, xSize, pTrans);
      pData += BytesPerLine;
    }
    break;
  case 4:
    for (i = 0; i < ySize; i++) {
      _DrawBitLine4BPP(pDevice, x0, i + y0, pData, Diff, xSize, pTrans);
      pData += BytesPerLine;
    }
    break;
  case 8:
    for (i = 0; i < ySize; i++) {
      _DrawBitLine8BPP(pDevice, x0, i + y0, pData, xSize, pTrans);
      pData += BytesPerLine;
    }
    break;
  //
  // Only required for 16bpp color depth of target. Should be removed otherwise.
  //
  case 16:
    for (i = 0; i < ySize; i++) {
      _DrawBitLine16BPP(pDevice, x0, i + y0, (const U16 *)pData, xSize);
      pData += BytesPerLine;
    }
    break;
  //
  // Only required for 32bpp color depth of target. Should be removed otherwise.
  //
  case 32:
    for (i = 0; i < ySize; i++) {
      _DrawBitLine32BPP(pDevice, x0, i + y0, (const U32 *)pData, xSize);
      pData += BytesPerLine;
    }
    break;
  }
}

/**
 * @brief   Initialize device driver context
 * @param   pDevice Device context
 * @return  Initialization result
 * @retval  0   Success
 * @retval  1   Failed (out of memory)
 */
static int _InitOnce(GUI_DEVICE *pDevice)
{
    DRIVER_CONTEXT *pContext;
    if (pDevice->u.pContext == NULL)
    {
        pDevice->u.pContext = GUI_ALLOC_GetFixedBlock(sizeof(DRIVER_CONTEXT));
        pContext = (DRIVER_CONTEXT *)pDevice->u.pContext;
        // Clipping is dependent on rotation
#if (!LCD_MIRROR_X && !LCD_MIRROR_Y && !LCD_SWAP_XY)
        pContext->hwClip.x0 = 0;
        pContext->hwClip.x1 = LCD_XSIZE - 1;
        pContext->hwClip.y0 = 0;
        pContext->hwClip.y1 = LCD_YSIZE - 1;
#elif (!LCD_MIRROR_X && !LCD_MIRROR_Y && LCD_SWAP_XY)
        pContext->hwClip.x0 = 0;
        pContext->hwClip.x1 = LCD_XSIZE - 1;
        pContext->hwClip.y0 = 0;
        pContext->hwClip.y1 = LCD_YSIZE - 1;
#elif (!LCD_MIRROR_X && LCD_MIRROR_Y && !LCD_SWAP_XY)
        pContext->hwClip.x0 = 0;
        pContext->hwClip.x1 = LCD_XSIZE - 1;
        pContext->hwClip.y0 = 432 - LCD_YSIZE;
        pContext->hwClip.y1 = 431;
#elif (!LCD_MIRROR_X && LCD_MIRROR_Y && LCD_SWAP_XY)
        pContext->hwClip.x0 = 432 - LCD_XSIZE;
        pContext->hwClip.x1 = 431;
        pContext->hwClip.y0 = 0;
        pContext->hwClip.y1 = LCD_YSIZE - 1;
#elif (LCD_MIRROR_X && !LCD_MIRROR_Y && !LCD_SWAP_XY)
        pContext->hwClip.x0 = 0;
        pContext->hwClip.x1 = LCD_XSIZE - 1;
        pContext->hwClip.y0 = 0;
        pContext->hwClip.y1 = LCD_YSIZE - 1;
#elif (LCD_MIRROR_X && !LCD_MIRROR_Y && LCD_SWAP_XY)
        pContext->hwClip.x0 = 0;
        pContext->hwClip.x1 = LCD_XSIZE - 1;
        pContext->hwClip.y0 = 0;
        pContext->hwClip.y1 = LCD_YSIZE - 1;
#elif (LCD_MIRROR_X && LCD_MIRROR_Y && !LCD_SWAP_XY)
        pContext->hwClip.x0 = 0;
        pContext->hwClip.x1 = LCD_XSIZE - 1;
        pContext->hwClip.y0 = 432 - LCD_YSIZE;
        pContext->hwClip.y1 = 431;
#elif (LCD_MIRROR_X &&  LCD_MIRROR_Y &&  LCD_SWAP_XY)
        pContext->hwClip.x0 = 432 - LCD_XSIZE;
        pContext->hwClip.x1 = 431;
        pContext->hwClip.y0 = 0;
        pContext->hwClip.y1 = LCD_YSIZE - 1;
#endif
        pContext->pfWriteReg = 0;
        pContext->pfWriteData = 0;
        pContext->pfWriteMultipleData = 0;
        pContext->pfReadData = 0;
        pContext->pfReadMultipleData = 0;
    }
    return pDevice->u.pContext ? 0 : 1;
}



/**
 * @brief   Get device properties
 * @param   pDevice Device context
 * @param   index   Property data
 * @return  Device properties or -1 if not supported
 */
static I32 _GetDevProp(GUI_DEVICE *pDevice, int index)
{
    switch (index)
    {
    case LCD_DEVCAP_XSIZE:
        return LCD_XSIZE;
    case LCD_DEVCAP_YSIZE:
        return LCD_YSIZE;
    case LCD_DEVCAP_VXSIZE:
        return LCD_XSIZE;
    case LCD_DEVCAP_VYSIZE:
        return LCD_YSIZE;
    case LCD_DEVCAP_BITSPERPIXEL:
        return LCD__GetBPP(pDevice->pColorConvAPI->pfGetIndexMask());
    case LCD_DEVCAP_NUMCOLORS:
        return (1 << LCD__GetBPP(pDevice->pColorConvAPI->pfGetIndexMask()));
    case LCD_DEVCAP_XMAG:
        return 1;
    case LCD_DEVCAP_YMAG:
        return 1;
    case LCD_DEVCAP_MIRROR_X:
        return 0;
    case LCD_DEVCAP_MIRROR_Y:
        return 0;
    case LCD_DEVCAP_SWAP_XY:
        return 0;
    case LCD_DEVCAP_SWAP_RB:
        return 0;
    }
    return -1;
}


/**
 * @brief   Get device data
 * @param   pDevice Device context
 * @param   index   Requesting data
 * @return  Device data
 */
static void * _GetDevData(GUI_DEVICE *pDevice, int index)
{
    return NULL;
}


/**
 * @brief   Get device dimension
 * @param   pDevice Device context
 * @param   pRect   Device dimension encapsulated in LCD_RECT structure
 */
static void _GetRect(GUI_DEVICE *pDevice, LCD_RECT *pRect)
{
	
    GUI_USE_PARA(pDevice);
    pRect->x0 = 0;
    pRect->y0 = 0;
    pRect->x1 = 219;//LCD_XSIZE - 1;
    pRect->y1 = 175;//LCD_YSIZE - 1;

}


/*********************************************************************
*
*       _SetOrg
*/
/**
 * @brief   Set origin, only used with Virtual Screen
 * @param   pDevice Device context
 * @param   x   X coordinate
 * @param   y   Y Coordinate
 */
static void _SetOrg(GUI_DEVICE *pDevice, int x, int y)
{
    // Virtual screen not supported on this controller
    GUI_USE_PARA(pDevice);
    GUI_USE_PARA(x);
    GUI_USE_PARA(y);
}


/**
 * @brief   Initialize display controller
 * @param   pDevice Device context
 * @return          Operation status
 * @retval  0       Initialization was successful
 * @retval  other   return code from LCD_X_DisplayDriver()
 */
static int _Init(GUI_DEVICE *pDevice)
{
  LCD_X_DisplayDriver(pDevice->LayerIndex, LCD_X_INITCONTROLLER, NULL);
}


/**
 * @brief   Turn LCD on
 * @param   pDevice Device context
 */
static void _On(GUI_DEVICE *pDevice)
{
    // Call user command handler
    LCD_X_DisplayDriver(pDevice->LayerIndex, LCD_X_ON, NULL);
}


/**
 * @brief   Turn LCD off
 * @param   pDevice Device context
 */
static void _Off(GUI_DEVICE *pDevice)
{
    // Call user command handler
    LCD_X_DisplayDriver(pDevice->LayerIndex, LCD_X_OFF, NULL);
}


/**
 * @brief   Read a block of display memory
 * @param   pDevice Device context
 * @param   x0      Starting X coordinate
 * @param   y0      Ending X coordinate
 * @param   x1      Starting Y coordinate
 * @param   y1      Ending Y coordinate
 * @param   pBuffer Buffer to fill return value
 */
static void _ReadRect(GUI_DEVICE *pDevice, int x0, int y0, int x1, int y1, LCD_PIXELINDEX *pBuffer)
{
}


/**
 * @brief   Return function pointers to emWin
 * @param   ppDevice    Device context
 * @param   index       Index of function to return
 * @return  Requested function pointer (in void (*)(void) type
 */
static void (* _GetDevFunc(GUI_DEVICE **ppDevice, int index))(void)
{
    GUI_USE_PARA(ppDevice);

    switch (index)
    {
    case LCD_DEVFUNC_INIT:
        return (void (*)(void))_Init;
    case LCD_DEVFUNC_ON:
        return (void (*)(void))_On;
    case LCD_DEVFUNC_OFF:
        return (void (*)(void))_Off;
    case LCD_DEVFUNC_READRECT:
        return (void (*)(void))_ReadRect;
    }
    return NULL;
}


/*===========================================================================*/
/* Public Data                                                               */
/*===========================================================================*/

/**
 * @brief Device driver description table
 */
const GUI_DEVICE_API GUIDRV_HX8352C_API =
{
    // Data
    DEVICE_CLASS_DRIVER,
    // Drawing functions
    _DrawBitmap,
    _DrawHLine,
    _DrawVLine,
    _FillRect,
    _GetPixelIndex,
    _SetPixelIndex,
    _XorPixel,
    // Set origin
    _SetOrg,
    // Request information
    _GetDevFunc,
    _GetDevProp,
    _GetDevData,
    _GetRect,
};


/*===========================================================================*/
/* Public Functions                                                          */
/*===========================================================================*/

/**
 * @brief   Configure Port access functions for the display driver
 * @param   pDevice Device context
 * @param   pAPI Caller supplied hardware access functions
 * @note    This function must be called immediate after GUI_DEVICE_CreateAndLink() in LCD_X_Config()
 */
void GUIDRV_HX8352C_SetFunc(GUI_DEVICE *pDevice, HX8352C_HW_API *pAPI)
{
    DRIVER_CONTEXT * pContext;
    _InitOnce(pDevice);
    pContext = (DRIVER_CONTEXT *)pDevice->u.pContext;
    pContext->pfWriteReg = pAPI->pfWriteReg;
    pContext->pfWriteData = pAPI->pfWriteData;
    pContext->pfWriteRepeatingData = 0;//pAPI->pfWriteRepeatingData;
    pContext->pfWriteMultipleData = pAPI->pfWriteMultipleData;
    pContext->pfReadReg = pAPI->pfReadReg;
    pContext->pfReadData = pAPI->pfReadData;
    pContext->pfReadMultipleData = pAPI->pfReadMultipleData;
}

#endif

/*************************** End of file ****************************/