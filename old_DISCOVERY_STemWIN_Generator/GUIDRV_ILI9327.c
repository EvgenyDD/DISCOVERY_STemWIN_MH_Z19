#include "stm32f4xx.h"
#include "ILI9327.h"

#include <stddef.h>

#include "LCD_Private.h"
#include "GUI_Private.h"
#include "LCD_ConfDefaults.h"
#include "GUIDRV_ILI9327.h"


#define LOBYTE(x) (((WORD_VAL)(x)).v[0])
#define HIBYTE(x) (((WORD_VAL)(x)).v[1])

#include <stdio.h>
#define USE_NEW_ILI9327

// Macros for MIRROR_, SWAP_ and LUT_
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

/*********************************************************************
*
*       Types
*
**********************************************************************
*/
typedef struct {
	U32 VRAMAddr;
	int xSize, ySize;
	int vxSize, vySize;
	int vxSizePhys;
	int BitsPerPixel;
} DRIVER_CONTEXT_TEMPLATE;

typedef struct _DRIVER_CONTEXT
{
    //! Hardware clip rectangle depends on screen rotation
    LCD_RECT hwClip;  // hardware clip rectangle
    // LCD access functions
    //! Write 16-bit data to LCD (RS = 0)
    void (*pfWriteReg)(uint16_t data);
    //! Write 16-bit data to LCD (RS = 1)
    void (*pfWriteData)(uint16_t data);
    //! Write data buffer to LCD (RS = 0)
    void (*pfWriteMultipleData)(uint16_t *pData, long count);
    //! Write same data to LCD multiple times (RS = 0)
    void (*pfWriteRepeatingData)(uint16_t data, long count);
    //! Read Register from LCD (RS = 1)
    U16 (*pfReadReg)(void);
    //! Read GRAM from LCD (RS = 1)
    U16 (*pfReadData)(void);
    //! Read multiple 16-bit data from LCD GRAM (RS = 1)
    void (*pfReadMultipleData)(uint16_t *pData, long count);
} DRIVER_CONTEXT;



typedef union _WORD_VAL
{
    I16 sval;
    U16 uval;
    struct __PACKED
    {
        U8 LSB;
        U8 MSB;
    } byte;
    U8 v[2];
} WORD_VAL;


/*********************************************************************
*
*       _SetPixelIndex
*
* Purpose:
*   Sets the index of the given pixel. The upper layers
*   calling this routine make sure that the coordinates are in range, so
*   that no check on the parameters needs to be performed.
*/
/*===========================================================================*/
/* Static Functions                                                          */
/*===========================================================================*/


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
	printf("_SetPixelIndex\n\r");

    DRIVER_CONTEXT *pContext;

    // Convert coordinates
#if (LCD_MIRROR_X == 1) || (LCD_MIRROR_Y == 1) || (LCD_SWAP_XY == 1)
    U16 xphys, yphys;
    xphys = LOG2PHYS_X(x, y);
    yphys = LOG2PHYS_Y(x, y);
#else
    //! @cond DOXYGEN_SHOW_UNDOC_MACRO
    #define xphys ((U16)x)
    #define yphys ((U16)y)
    //! @endcond
#endif
    pContext = (DRIVER_CONTEXT *)pDevice->u.pContext;

    // Move cursor
    setXY(xphys,yphys, xphys,yphys);\

    // Write data
    pContext->pfWriteData(color);

#if (LCD_MIRROR_X == 0) && (LCD_MIRROR_Y == 0) && (LCD_SWAP_XY == 0)
    #undef xphys
    #undef yphys
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
	printf("_GetPixelIndex\n\r");
    DRIVER_CONTEXT *pContext;
    U16 reads[3];
    unsigned int color;
    // Convert coordinates
#if (LCD_MIRROR_X == 1) || (LCD_MIRROR_Y == 1) || (LCD_SWAP_XY == 1)
    U16 xphys, yphys;
    xphys = LOG2PHYS_X(x, y);
    yphys = LOG2PHYS_Y(x, y);
#else
    //! @cond DOXYGEN_SHOW_UNDOC_MACRO
    #define xphys ((U16)x)
    #define yphys ((U16)y)
    //! @endcond
#endif
    pContext = (DRIVER_CONTEXT *)pDevice->u.pContext;
    // Move cursor
    setXY(xphys,yphys, xphys,yphys);

    // Start read data
    pContext->pfReadMultipleData(reads, 3);

    // Pack RGB
    color = ((reads[1] & 0xF800) | ((reads[1] & 0x00FC) << 3) | (reads[2] >> 11));

#if (LCD_MIRROR_X == 0) && (LCD_MIRROR_Y == 0) && (LCD_SWAP_XY == 0)
    #undef xphys
    #undef yphys
#endif
    return color;
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
	printf("_XorPixel\n\r");
    LCD_PIXELINDEX color;
    LCD_PIXELINDEX mask;
    color = _GetPixelIndex(pDevice, x, y);
    mask = pDevice->pColorConvAPI->pfGetIndexMask();
    _SetPixelIndex(pDevice, x, y, color ^ mask);
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
	printf("_DrawHLine\n\r");
    DRIVER_CONTEXT *pContext;
    LCD_PIXELINDEX color;

    if (GUI_pContext->DrawMode & LCD_DRAWMODE_XOR)
    {
        for (; x0 <= x1; ++x0)
            _XorPixel(pDevice, x0, y);
    }
    else
    {
        color = LCD__GetColorIndex();
        // Draw horizontal line using "block writing" method takes (25 + length) writing operations.
        // Draw the same line using _SetPixelIndex takes 10 * length writing operations.
        // So we use block writing only when (length >= 3)
        if (x1 - x0 < 3)
        {
            for (; x0 <= x1; ++x0)
                _SetPixelIndex(pDevice, x0, y, color);
        }
        else
        {
            pContext = (DRIVER_CONTEXT *)pDevice->u.pContext;
#if (LCD_MIRROR_X == 1) || (LCD_MIRROR_Y == 1) || (LCD_SWAP_XY == 1)
            U16 x0phys, x1phys, yphys;
            x0phys = LOG2PHYS_X(x0, y);
            x1phys = LOG2PHYS_X(x1, y);
            yphys = LOG2PHYS_Y(x0, y);
#else
            //! @cond DOXYGEN_SHOW_UNDOC_MACRO
            #define x0phys ((U16)x0)
            #define x1phys ((U16)x1)
            #define yphys ((U16)y)
            //! @endcond
#endif
            // Set frame (box) size
            setXY(x0phys,yphys, x1phys,yphys);

            //Write data
            pContext->pfWriteRepeatingData(color, x1 - x0 + 1);

#if (LCD_MIRROR_X == 0) && (LCD_MIRROR_Y == 0) && (LCD_SWAP_XY == 0)
            #undef x0phys
            #undef x1phys
            #undef yphys
#endif
            // Restore column/row end
            /*pContext->pfWriteReg(0x0004);
            pContext->pfWriteData(HIBYTE(pContext->hwClip.x1));
            pContext->pfWriteReg(0x0005);
            pContext->pfWriteData(LOBYTE(pContext->hwClip.x1));
            pContext->pfWriteReg(0x0008);
            pContext->pfWriteData(HIBYTE(pContext->hwClip.y1));
            pContext->pfWriteReg(0x0009);
            pContext->pfWriteData(LOBYTE(pContext->hwClip.y1));*/
        }
    }
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
	printf("_DrawVLine\n\r");
    DRIVER_CONTEXT *pContext;
    LCD_PIXELINDEX color;

    if (GUI_pContext->DrawMode & LCD_DRAWMODE_XOR)
    {
        for (; y0 <= y1; ++y0)
            _XorPixel(pDevice, x, y0);
    }
    else
    {
        color = LCD__GetColorIndex();
        // Draw vertical line using "block writing" method takes (25 + length) writing operations.
        // Draw the same line using _SetPixelIndex takes 10 * length writing operations.
        // So we use block writing only when (length >= 3)
        if (y1 - y0 < 3)
        {
            for (; y0 <= y1; ++y0)
                _SetPixelIndex(pDevice, x, y0, color);
        }
        else
        {
            pContext = (DRIVER_CONTEXT *)pDevice->u.pContext;
#if (LCD_MIRROR_X == 1) || (LCD_MIRROR_Y == 1) || (LCD_SWAP_XY == 1)
            U16 x0phys, y0phys, y1phys;
            x0phys = LOG2PHYS_X(x, y0);
            y0phys = LOG2PHYS_Y(x, y0);
            y1phys = LOG2PHYS_Y(x, y1);
#else
            #define x0phys ((U16)x)
            #define y0phys ((U16)y0)
            #define y1phys ((U16)y1)
#endif
            // Set frame (box) size
            setXY(x0phys,y0phys, x0phys,y1phys);

            // Write data
            pContext->pfWriteRepeatingData(color, y1 - y0 + 1);

    #if (LCD_MIRROR_X == 0) && (LCD_MIRROR_Y == 0) && (LCD_SWAP_XY == 0)
            #undef xphys
            #undef y0phys
            #undef y1phys
    #endif
            // Restore column/row end
            /*pContext->pfWriteReg(0x0004);
            pContext->pfWriteData(HIBYTE(pContext->hwClip.x1));
            pContext->pfWriteReg(0x0005);
            pContext->pfWriteData(LOBYTE(pContext->hwClip.x1));
            pContext->pfWriteReg(0x0008);
            pContext->pfWriteData(HIBYTE(pContext->hwClip.y1));
            pContext->pfWriteReg(0x0009);
            pContext->pfWriteData(LOBYTE(pContext->hwClip.y1));*/
        }
    }
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
	printf("_FillRect:%dx%d - %dx%d\n\r", x0, y0, x1, y1);
    DRIVER_CONTEXT *pContext;
    LCD_PIXELINDEX index;
    I32 total;

    if (GUI_pContext->DrawMode & LCD_DRAWMODE_XOR)
    {
        for (; y0 <= y1; ++y0)
            _DrawHLine(pDevice, x0, y0, x1);
    }
    else
    {
        index = LCD__GetColorIndex();
        total = (x1 - x0 + 1) * (y1 - y0 + 1);
        pContext = (DRIVER_CONTEXT *)pDevice->u.pContext;
#if (LCD_MIRROR_X == 1) || (LCD_MIRROR_Y == 1) || (LCD_SWAP_XY == 1)
        U16 x0phys, x1phys, y0phys, y1phys;
        x0phys = LOG2PHYS_X(x0, y0);
        x1phys = LOG2PHYS_X(x1, y1);
        y0phys = LOG2PHYS_Y(x0, y0);
        y1phys = LOG2PHYS_Y(x1, y1);
#else
        #define x0phys ((U16)x0)
        #define x1phys ((U16)x1)
        #define y0phys ((U16)y0)
        #define y1phys ((U16)y1)
#endif
        // Set frame (box) size
        setXY(x0phys,y0phys,  x1phys,y1phys);
        // Write data
        pContext->pfWriteRepeatingData(index, total);

#if (LCD_MIRROR_X == 0) && (LCD_MIRROR_Y == 0) && (LCD_SWAP_XY == 0)
        #undef x0phys
        #undef x1phys
        #undef y0phys
        #undef y1phys
#endif
        // Restore column/row end
        /*pContext->pfWriteReg(0x0004);
        pContext->pfWriteData(HIBYTE(pContext->hwClip.x1));
        pContext->pfWriteReg(0x0005);
        pContext->pfWriteData(LOBYTE(pContext->hwClip.x1));
        pContext->pfWriteReg(0x0008);
        pContext->pfWriteData(HIBYTE(pContext->hwClip.y1));
        pContext->pfWriteReg(0x0009);
        pContext->pfWriteData(LOBYTE(pContext->hwClip.y1));*/
    }
}


/**
 * @brief   Draw 1BPP bitmap, without considering transparent color
 * @param   pDevice Device context
 * @param   x0      Starting X coordinate
 * @param   y0      Starting Y coordinate
 * @param   width   Bitmap width
 * @param   height  Bitmap height
 * @param   stride  Bytes per scan line, can be 0 if (height == 1)
 * @param   diff    diff = [0:7], if diff = 0, image start with most significant bit
 * @param   pData   Bitmap data
 * @param   pTrans  Palette
 */

static inline void _DrawBitmap1BPPOpaque(GUI_DEVICE *pDevice, int x0, int y0, int width, int height, int stride, const U8 GUI_UNI_PTR *pData, int diff, const LCD_PIXELINDEX *pTrans)
{
#if 0
	LCD_PIXELINDEX index0, index1, pixels;
    const U8 GUI_UNI_PTR *pTempData;
    int x1, y1;
    int curPixel;
    DRIVER_CONTEXT *pContext;

    pContext = (DRIVER_CONTEXT *)pDevice->u.pContext;
    x0 = x0 + diff; // Adjust starting X coordinate
    x1 = x0 + width - 1;    // X end
    y1 = y0 + height - 1;   // Y end

#if (LCD_MIRROR_X == 1) || (LCD_MIRROR_Y == 1) || (LCD_SWAP_XY == 1)
    U16 x0phys, x1phys, y0phys, y1phys;
    x0phys = LOG2PHYS_X(x0, y0);
    x1phys = LOG2PHYS_X(x1, y1);
    y0phys = LOG2PHYS_Y(x0, y0);
    y1phys = LOG2PHYS_Y(x1, y1);
#else
    #define x0phys ((U16)x0)
    #define x1phys ((U16)x1)
    #define y0phys ((U16)y0)
    #define y1phys ((U16)y1)
#endif
    // Set frame (box) size
    setXY(x0phys,y0phys, x1phys,y1phys);
    // Write data

    index0 = *pTrans;
    index1 = *(pTrans + 1);
    for (y0 = 0; y0 < height; ++y0) // reuse x0, y0 here
    {
        curPixel = diff;
        pTempData = pData;
        pixels = *pTempData;
        for (x0 = 0; x0 < width; ++x0)
        {
            if (pixels & (0x80 >> curPixel))
            {
                //pContext->pfWriteData((U16)index1);
            	_SetPixelIndex(pDevice, x0, y0, index1);
            }
            else
            {
                //pContext->pfWriteData((U16)index0);
            	_SetPixelIndex(pDevice, x0, y0, index0);
            }
            if (++curPixel == 8)
            {
                curPixel = 0;
                pixels = *(++pTempData);
            }
        }
        pData += stride;    // Next line
    }

#if (LCD_MIRROR_X == 0) && (LCD_MIRROR_Y == 0) && (LCD_SWAP_XY == 0)
    #undef x0phys
    #undef x1phys
    #undef y0phys
    #undef y1phys
#endif
#else
	 LCD_PIXELINDEX pixels, index0, index1;
	const U8 GUI_UNI_PTR *pTempData;
	int x1, y1, x, y;
	int curPixel;

	x0 = x0 + diff; // Adjust starting X coordinate
	x1 = x0 + width - 1;    // X end
	y1 = y0 + height - 1;   // Y end
	index0 = *pTrans;
	index1 = *(pTrans + 1);
	for (y = y0; y <= y1; ++y)
	{
		curPixel = diff;
		pTempData = pData;
		pixels = *pTempData;
		for (x = x0; x <= x1; ++x)
		{
			if (pixels & (0x80 >> curPixel))
				_SetPixelIndex(pDevice, x, y, index1);
			else
				_SetPixelIndex(pDevice, x, y, index0);

			if (++curPixel == 8)
			{
			   curPixel = 0;
			   pixels = *(++pTempData);
			}
		}
		pData += stride;
	}
#endif
    // Restore column/row end
    /*pContext->pfWriteReg(0x0004);
    pContext->pfWriteData(HIBYTE(pContext->hwClip.x1));
    pContext->pfWriteReg(0x0005);
    pContext->pfWriteData(LOBYTE(pContext->hwClip.x1));
    pContext->pfWriteReg(0x0008);
    pContext->pfWriteData(HIBYTE(pContext->hwClip.y1));
    pContext->pfWriteReg(0x0009);
    pContext->pfWriteData(LOBYTE(pContext->hwClip.y1));*/
}


/**
 * @brief   Draw 1BPP bitmap, consider (0) color as transparent
 * @param   pDevice Device context
 * @param   x0      Starting X coordinate
 * @param   y0      Starting Y coordinate
 * @param   width   Bitmap width
 * @param   height  Bitmap height
 * @param   stride  Bytes per scan line, can be 0 if (height == 1)
 * @param   diff    diff = [0:7], if diff = 0, image start with most significant bit
 * @param   pData   Bitmap data
 * @param   pTrans  Palette
 */
static inline void _DrawBitmap1BPPTransparent(GUI_DEVICE *pDevice, int x0, int y0, int width, int height, int stride, const U8 GUI_UNI_PTR *pData, int diff, const LCD_PIXELINDEX *pTrans)
{
    LCD_PIXELINDEX pixels, index1;
    const U8 GUI_UNI_PTR *pTempData;
    int x1, y1, x, y;
    int curPixel;

    x0 = x0 + diff; // Adjust starting X coordinate
    x1 = x0 + width - 1;    // X end
    y1 = y0 + height - 1;   // Y end
    index1 = *(pTrans + 1);
    for (y = y0; y <= y1; ++y)
    {
        curPixel = diff;
        pTempData = pData;
        pixels = *pTempData;
        for (x = x0; x <= x1; ++x)
        {
            if (pixels & (0x80 >> curPixel))
                _SetPixelIndex(pDevice, x, y, index1);
            if (++curPixel == 8)
            {
               curPixel = 0;
               pixels = *(++pTempData);
            }
        }
        pData += stride;
    }
}


/**
 * @brief   Draw 1BPP bitmap using XOR mode
 * @param   pDevice Device context
 * @param   x0      Starting X coordinate
 * @param   y0      Starting Y coordinate
 * @param   width   Bitmap width
 * @param   height  Bitmap height
 * @param   stride  Bytes per scan line, can be 0 if (height == 1)
 * @param   diff    diff = [0:7], if diff = 0, image start with most significant bit
 * @param   pData   Bitmap data
 * @param   pTrans  Palette, not used
 */
static inline void _DrawBitmap1BPPXOR(GUI_DEVICE *pDevice, int x0, int y0, int width, int height, int stride, const U8 GUI_UNI_PTR *pData, int diff, const LCD_PIXELINDEX *pTrans)
{
    LCD_PIXELINDEX pixels, pixel, mask;
    const U8 GUI_UNI_PTR *pTempData;
    int x1, y1, x, y;
    int curPixel;

    x0 = x0 + diff; // Adjust starting X coordinate
    x1 = x0 + width - 1;    // X end
    y1 = y0 + height - 1;   // Y end
    mask = pDevice->pColorConvAPI->pfGetIndexMask();

    for (y = y0; y <= y1; ++y)
    {
        curPixel = diff;
        pTempData = pData;
        pixels = *pTempData;

        for (x = x0; x <= x1; ++x)
        {
            if (pixels & (0x80 >> curPixel))
            {
                pixel = _GetPixelIndex(pDevice, x, y);
                _SetPixelIndex(pDevice, x, y, pixel ^ mask);
            }
            if (++curPixel == 8)
            {
               curPixel = 0;
               pixels = *(++pTempData);
            }
        }
        pData += stride;
    }
}

/**
 * @brief   Draw 1BPP bitmap
 * @param   pDevice Device context
 * @param   x0      Starting X coordinate
 * @param   y0      Starting Y coordinate
 * @param   width   Bitmap width
 * @param   height  Bitmap height
 * @param   stride  Bytes per scan line, can be 0 if (height == 1)
 * @param   pData   Bitmap data
 * @param   diff    diff = [0:7], if diff = 0, image start with most significant bit
 * @param   pTrans  Palette
 */
static inline void _DrawBitmap1BPP(GUI_DEVICE *pDevice, int x0, int y0, int width, int height, int stride, U8 const GUI_UNI_PTR *pData, int diff, const LCD_PIXELINDEX *pTrans)
{
	printf("###N->%d   \n\t", GUI_pContext->DrawMode & (LCD_DRAWMODE_TRANS | LCD_DRAWMODE_XOR));
    switch (GUI_pContext->DrawMode & (LCD_DRAWMODE_TRANS | LCD_DRAWMODE_XOR))
    {
    case 0:
        _DrawBitmap1BPPOpaque(pDevice, x0, y0, width, height, stride, pData, diff, pTrans);
        break;
    case LCD_DRAWMODE_TRANS:
        _DrawBitmap1BPPTransparent(pDevice, x0, y0, width, height, stride, pData, diff, pTrans);
        break;
    case LCD_DRAWMODE_XOR:
        _DrawBitmap1BPPXOR(pDevice, x0, y0, width, height, stride, pData, diff, pTrans);
        break;
    }
}


/**
 * @brief   Draw 2BPP bitmap, without considering transparent color
 * @param   pDevice Device context
 * @param   x0      Starting X coordinate
 * @param   y0      Starting Y coordinate
 * @param   width   Bitmap width
 * @param   height  Bitmap height
 * @param   stride  Bytes per scan line, can be 0 if (height == 1)
 * @param   diff    diff = [0:3], if diff = 0, image start with most significant 2 bits
 * @param   pData   Bitmap data
 * @param   pTrans  If pTrans is not NULL, it points to a buffer palette and pData points to color index
 */

static inline void _DrawBitmap2BPPOpaque(GUI_DEVICE *pDevice, int x0, int y0, int width, int height, int stride, const U8 GUI_UNI_PTR *pData, int diff, const LCD_PIXELINDEX *pTrans)
{
    LCD_PIXELINDEX pixels;
    const U8 GUI_UNI_PTR *pTempData;
    int x1, y1;
    int curPixel, shift, index;
    DRIVER_CONTEXT *pContext;

    pContext = (DRIVER_CONTEXT *)pDevice->u.pContext;
    x0 = x0 + diff; // Adjust starting X coordinate
    x1 = x0 + width - 1;    // X end
    y1 = y0 + height - 1;   // Y end

#if (LCD_MIRROR_X == 1) || (LCD_MIRROR_Y == 1) || (LCD_SWAP_XY == 1)
    U16 x0phys, x1phys, y0phys, y1phys;
    x0phys = LOG2PHYS_X(x0, y0);
    x1phys = LOG2PHYS_X(x1, y1);
    y0phys = LOG2PHYS_Y(x0, y0);
    y1phys = LOG2PHYS_Y(x1, y1);
#else
    #define x0phys ((U16)x0)
    #define x1phys ((U16)x1)
    #define y0phys ((U16)y0)
    #define y1phys ((U16)y1)
#endif

    // Set frame (box) size
	setXY(x0phys,y0phys, x1phys,y1phys);

	// Write data
    if (pTrans)
    {
        for (y0 = 0; y0 < height; ++y0) // reuse x0, y0 here
        {
            curPixel = diff;
            pTempData = pData;
            pixels = *pTempData;
            for (x0 = 0; x0 < width; ++x0)
            {
                shift = (3 - curPixel) << 1;
                index = (pixels & (0xC0 >> (6 - shift))) >> shift;
                pContext->pfWriteData((U16)(pTrans[index]));
                if (++curPixel == 4)
                {
                    curPixel = 0;
                    pixels = *(++pTempData);
                }
            }
            pData += stride;    // Next line
        }
    }
    else
    {
        for (y0 = 0; y0 < height; ++y0) // reuse x0, y0 here
        {
            curPixel = diff;
            pTempData = pData;
            pixels = *pTempData;
            for (x0 = 0; x0 < width; ++x0)
            {
                shift = (3 - curPixel) << 1;
                index = (pixels & (0xC0 >> (6 - shift))) >> shift;
                pContext->pfWriteData((U16)index);
                if (++curPixel == 4)
                {
                    curPixel = 0;
                    pixels = *(++pTempData);
                }
            }
            pData += stride;    // Next line
        }
    }
#if (LCD_MIRROR_X == 0) && (LCD_MIRROR_Y == 0) && (LCD_SWAP_XY == 0)
    #undef x0phys
    #undef x1phys
    #undef y0phys
    #undef y1phys
#endif
    // Restore column/row end
    /*pContext->pfWriteReg(0x0004);
    pContext->pfWriteData(HIBYTE(pContext->hwClip.x1));
    pContext->pfWriteReg(0x0005);
    pContext->pfWriteData(LOBYTE(pContext->hwClip.x1));
    pContext->pfWriteReg(0x0008);
    pContext->pfWriteData(HIBYTE(pContext->hwClip.y1));
    pContext->pfWriteReg(0x0009);
    pContext->pfWriteData(LOBYTE(pContext->hwClip.y1));*/
}



/**
 * @brief   Draw 2BPP bitmap, consider black or index 0 pixel as transparent
 * @param   pDevice Device context
 * @param   x0      Starting X coordinate
 * @param   y0      Starting Y coordinate
 * @param   width   Bitmap width
 * @param   height  Bitmap height
 * @param   stride  Bytes per scan line, can be 0 if (height == 1)
 * @param   diff    diff = [0:3], if diff = 0, image start with most significant 2 bits
 * @param   pData   Bitmap data
 * @param   pTrans  If pTrans is not NULL, it points to the palette and pData points is color index
 */

static inline void _DrawBitmap2BPPTransparent(GUI_DEVICE *pDevice, int x0, int y0, int width, int height, int stride, const U8 GUI_UNI_PTR *pData, int diff, const LCD_PIXELINDEX *pTrans)
{
    LCD_PIXELINDEX pixels;
    const U8 GUI_UNI_PTR *pTempData;
    int x1, y1, x, y;
    int curPixel, shift, index;

    x0 = x0 + diff; // Adjust starting X coordinate
    x1 = x0 + width - 1;    // X end
    y1 = y0 + height - 1;   // Y end
    if (pTrans)
    {
        for (y = y0; y <= y1; ++y)
        {
            curPixel = diff;
            pTempData = pData;
            pixels = *pTempData;
            for (x = x0; x <= x1; ++x)
            {
                shift = (3 - curPixel) << 1;
                index = (pixels & (0xC0 >> (6 - shift))) >> shift;
                if (index)
                    _SetPixelIndex(pDevice, x, y, pTrans[index]);
                if (++curPixel == 4)
                {
                    curPixel = 0;
                    pixels = *(++pTempData);
                }
            }
            pData += stride;
        }
    }
    else
    {
        for (y = y0; y <= y1; ++y)
        {
            curPixel = diff;
            pTempData = pData;
            pixels = *pTempData;
            for (x = x0; x <= x1; ++x)
            {
                shift = (3 - curPixel) << 1;
                index = (pixels & (0xC0 >> (6 - shift))) >> shift;
                if (index)
                    _SetPixelIndex(pDevice, x, y, index);
                if (++curPixel == 4)
               {
                   curPixel = 0;
                   pixels = *(++pTempData);
               }
            }
            pData += stride;
        }
    }
}



/**
 * @brief   Draw 2BPP bitmap
 * @param   pDevice Device context
 * @param   x0      Starting X coordinate
 * @param   y0      Starting Y coordinate
 * @param   width   Bitmap width
 * @param   height  Bitmap height
 * @param   stride  Bytes per scan line, can be 0 if (height == 1)
 * @param   pData   Bitmap data
 * @param   diff    diff = [0:3], if diff = 0, image start with most significant 2 bits*
 * @param   pTrans  If pTrans is not NULL, it points to the palette and pData points is color index
 */

static inline void _DrawBitmap2BPP(GUI_DEVICE *pDevice, int x0, int y0, int width, int height, int stride, U8 const GUI_UNI_PTR *pData, int diff, const LCD_PIXELINDEX *pTrans)
{
    switch (GUI_pContext->DrawMode & LCD_DRAWMODE_TRANS)
    {
    case 0:
        _DrawBitmap2BPPOpaque(pDevice, x0, y0, width, height, stride, pData, diff, pTrans);
        break;
    case LCD_DRAWMODE_TRANS:
        _DrawBitmap2BPPTransparent(pDevice, x0, y0, width, height, stride, pData, diff, pTrans);
        break;
        break;
    }
}


/**
 * @brief   Draw 4BPP bitmap, without considering transparent color
 * @param   pDevice Device context
 * @param   x0      Starting X coordinate
 * @param   y0      Starting Y coordinate
 * @param   width   Bitmap width
 * @param   height  Bitmap height
 * @param   stride  Bytes per scan line, can be 0 if (height == 1)
 * @param   diff    For 4BPP image buffer, high nibble is the pixel to the left and low nibble is at the right. If diff = 0, image start with high nibble
 * @param   pData   Bitmap data
 * @param   pTrans  If pTrans is not NULL, it points to the palette and pData points is color index
 */

static inline void _DrawBitmap4BPPOpaque(GUI_DEVICE *pDevice, int x0, int y0, int width, int height, int stride, const U8 GUI_UNI_PTR *pData, int diff, const LCD_PIXELINDEX *pTrans)
{
    LCD_PIXELINDEX pixels;
    const U8 GUI_UNI_PTR *pTempData;
    int x1, y1;
    int curPixel, shift, index;
    DRIVER_CONTEXT *pContext;

    pContext = (DRIVER_CONTEXT *)pDevice->u.pContext;
    x0 = x0 + diff; // Adjust starting X coordinate
    x1 = x0 + width - 1;    // X end
    y1 = y0 + height - 1;   // Y end

#if (LCD_MIRROR_X == 1) || (LCD_MIRROR_Y == 1) || (LCD_SWAP_XY == 1)
    U16 x0phys, x1phys, y0phys, y1phys;
    x0phys = LOG2PHYS_X(x0, y0);
    x1phys = LOG2PHYS_X(x1, y1);
    y0phys = LOG2PHYS_Y(x0, y0);
    y1phys = LOG2PHYS_Y(x1, y1);
#else
    #define x0phys ((U16)x0)
    #define x1phys ((U16)x1)
    #define y0phys ((U16)y0)
    #define y1phys ((U16)y1)
#endif
    // Set frame (box) size
	setXY(x0phys,y0phys, x1phys,y1phys);

	// Write data
    if (pTrans)
    {
        for (y0 = 0; y0 < height; ++y0) // reuse x0, y0 here
        {
            curPixel = diff;
            pTempData = pData;
            pixels = *pTempData;
            for (x0 = 0; x0 < width; ++x0)
            {
                shift = (1 - curPixel) << 2;
                index = (pixels & (0xF0 >> (4 - shift))) >> shift;
                pContext->pfWriteData((U16)(pTrans[index]));
                if (++curPixel == 2)
                {
                    curPixel = 0;
                    pixels = *(++pTempData);
                }
            }
            pData += stride;    // Next line
        }
    }
    else
    {
        for (y0 = 0; y0 < height; ++y0) // reuse x0, y0 here
        {
            curPixel = diff;
            pTempData = pData;
            pixels = *pTempData;
            for (x0 = 0; x0 < width; ++x0)
            {
                shift = (1 - curPixel) << 2;
                index = (pixels & (0xF0 >> (4 - shift))) >> shift;
                pContext->pfWriteData((U16)index);
                if (++curPixel == 2)
                {
                    curPixel = 0;
                    pixels = *(++pTempData);
                }
            }
            pData += stride;    // Next line
        }
    }
#if (LCD_MIRROR_X == 0) && (LCD_MIRROR_Y == 0) && (LCD_SWAP_XY == 0)
    #undef x0phys
    #undef x1phys
    #undef y0phys
    #undef y1phys
#endif
    // Restore column/row end
    /*pContext->pfWriteReg(0x0004);
    pContext->pfWriteData(HIBYTE(pContext->hwClip.x1));
    pContext->pfWriteReg(0x0005);
    pContext->pfWriteData(LOBYTE(pContext->hwClip.x1));
    pContext->pfWriteReg(0x0008);
    pContext->pfWriteData(HIBYTE(pContext->hwClip.y1));
    pContext->pfWriteReg(0x0009);
    pContext->pfWriteData(LOBYTE(pContext->hwClip.y1));*/
}



/**
 * @brief   Draw 4BPP bitmap, consider black (0x00) pixel as transparent
 * @param   pDevice Device context
 * @param   x0      Starting X coordinate
 * @param   y0      Starting Y coordinate
 * @param   width   Bitmap width
 * @param   height  Bitmap height
 * @param   stride  Bytes per scan line, can be 0 if (height == 1)
 * @param   diff    For 4BPP image buffer, high nibble is the pixel to the left and low nibble is at the right. If diff = 0, image start with high nibble
 * @param   pData   Bitmap data
 * @param   pTrans  If pTrans is not NULL, it points to the palette and pData points is color index
 */
static inline void _DrawBitmap4BPPTransparent(GUI_DEVICE *pDevice, int x0, int y0, int width, int height, int stride, const U8 GUI_UNI_PTR *pData, int diff, const LCD_PIXELINDEX *pTrans)
{
    LCD_PIXELINDEX pixels;
    const U8 GUI_UNI_PTR *pTempData;
    int x1, y1, x, y;
    int curPixel, shift, index;

    x0 = x0 + diff; // Adjust starting X coordinate
    x1 = x0 + width - 1;    // X end
    y1 = y0 + height - 1;   // Y end
    if (pTrans)
    {
        for (y = y0; y <= y1; ++y)
        {
            curPixel = diff;
            pTempData = pData;
            pixels = *pTempData;
            for (x = x0; x <= x1; ++x)
            {
                shift = (1 - curPixel) << 2;
                index = (pixels & (0xF0 >> (4 - shift))) >> shift;
                if (index)
                    _SetPixelIndex(pDevice, x, y, pTrans[index]);
                if (++curPixel == 2)
                {
                    curPixel = 0;
                    pixels = *(++pTempData);
                }
            }
            pData += stride;
        }
    }
    else
    {
        for (y = y0; y <= y1; ++y)
        {
            curPixel = diff;
            pTempData = pData;
            pixels = *pTempData;
            for (x = x0; x <= x1; ++x)
            {
                shift = (1 - curPixel) << 2;
                index = (pixels & (0xF0 >> (4 - shift))) >> shift;
                if (index)
                    _SetPixelIndex(pDevice, x, y, index);
                if (++curPixel == 2)
               {
                   curPixel = 0;
                   pixels = *(++pTempData);
               }
            }
            pData += stride;
        }
    }
}



/**
 * @brief   Draw 4BPP bitmap
 * @param   pDevice Device context
 * @param   x0      Starting X coordinate
 * @param   y0      Starting Y coordinate
 * @param   width   Bitmap width
 * @param   height  Bitmap height
 * @param   stride  Bytes per scan line, can be 0 if (height == 1)
 * @param   pData   Bitmap data
 * @param   diff    For 4BPP image buffer, high nibble is the pixel to the left and low nibble is at the right. If diff = 0, image start with high nibble
 * @param   pTrans  If pTrans is not NULL, it points to the palette and pData points is color index
 */
static inline void _DrawBitmap4BPP(GUI_DEVICE *pDevice, int x0, int y0, int width, int height, int stride, U8 const GUI_UNI_PTR *pData, int diff, const LCD_PIXELINDEX *pTrans)
{
    switch (GUI_pContext->DrawMode & LCD_DRAWMODE_TRANS)
    {
    case 0:
        _DrawBitmap4BPPOpaque(pDevice, x0, y0, width, height, stride, pData, diff, pTrans);
        break;
    case LCD_DRAWMODE_TRANS:
        _DrawBitmap4BPPTransparent(pDevice, x0, y0, width, height, stride, pData, diff, pTrans);
        break;
    }
}


/**
 * @brief   Draw 8BPP bitmap, without considering transparent color
 * @param   pDevice Device context
 * @param   x0      Starting X coordinate
 * @param   y0      Starting Y coordinate
 * @param   width   Bitmap width
 * @param   height  Bitmap height
 * @param   stride  Bytes per scan line, can be 0 if (height == 1)
 * @param   pData   Bitmap data
 * @param   pTrans  If pTrans is not NULL, it points to the palette and pData points is color index
 */
static inline void _DrawBitmap8BPPOpaque(GUI_DEVICE *pDevice, int x0, int y0, int width, int height, int stride, U8 const GUI_UNI_PTR *pData, const LCD_PIXELINDEX *pTrans)
{
    LCD_PIXELINDEX pixel;
    DRIVER_CONTEXT *pContext = (DRIVER_CONTEXT *)pDevice->u.pContext;
    int x1 = x0 + width - 1;
    int y1 = y0 + height - 1;
#if (LCD_MIRROR_X == 1) || (LCD_MIRROR_Y == 1) || (LCD_SWAP_XY == 1)
    U16 x0phys, x1phys, y0phys, y1phys;
    x0phys = LOG2PHYS_X(x0, y0);
    x1phys = LOG2PHYS_X(x1, y1);
    y0phys = LOG2PHYS_Y(x0, y0);
    y1phys = LOG2PHYS_Y(x1, y1);
#else
    #define x0phys ((U16)x0)
    #define x1phys ((U16)x1)
    #define y0phys ((U16)y0)
    #define y1phys ((U16)y1)
#endif
    // Set frame (box) size
	setXY(x0phys,y0phys, x1phys,y1phys);
	// Write data
    if (pTrans)
    {
        for (y0 = 0; y0 < height; ++y0) // reuse x0, y0 here
        {
            for (x0 = 0; x0 < width; ++x0)
            {
                pixel = pTrans[pData[x0]];
                pContext->pfWriteData((U16)pixel);
            }
            pData += stride;
        }
    }
    else
    {
        for (y0 = 0; y0 < height; ++y0) // reuse x0, y0 here
        {
            for (x0 = 0; x0 < width; ++x0)
            {
                pixel = pData[x0];
                pContext->pfWriteData((U16)pixel);
            }
            pData += stride;
        }
    }
#if (LCD_MIRROR_X == 0) && (LCD_MIRROR_Y == 0) && (LCD_SWAP_XY == 0)
    #undef x0phys
    #undef x1phys
    #undef y0phys
    #undef y1phys
#endif
    // Restore column/row end
    /*pContext->pfWriteReg(0x0004);
    pContext->pfWriteData(HIBYTE(pContext->hwClip.x1));
    pContext->pfWriteReg(0x0005);
    pContext->pfWriteData(LOBYTE(pContext->hwClip.x1));
    pContext->pfWriteReg(0x0008);
    pContext->pfWriteData(HIBYTE(pContext->hwClip.y1));
    pContext->pfWriteReg(0x0009);
    pContext->pfWriteData(LOBYTE(pContext->hwClip.y1));*/
}


/**
 * @brief   Draw 8BPP bitmap, consider black or index 0 pixel as transparent
 * @param   pDevice Device context
 * @param   x0      Starting X coordinate
 * @param   y0      Starting Y coordinate
 * @param   width   Bitmap width
 * @param   height  Bitmap height
 * @param   stride  Bytes per scan line, can be 0 if (height == 1)
 * @param   pData   Bitmap data
 * @param   pTrans  If pTrans is not NULL, it points to a buffer palette and pData points to color index
 */
static inline void _DrawBitmap8BPPTransparent(GUI_DEVICE *pDevice, int x0, int y0, int width, int height, int stride, U8 const GUI_UNI_PTR *pData, const LCD_PIXELINDEX * pTrans)
{
    LCD_PIXELINDEX pixel;
    int x1 = x0 + width - 1;
    int y1 = y0 + height - 1;
    int x, y, i;

    if (pTrans)
    {
        for (y = y0; y <= y1; ++y)
        {
            for (x = x0, i = 0; x <= x1; ++x, ++i)
            {
                pixel = pData[i];
                if (pixel)
                    _SetPixelIndex(pDevice, x, y, pTrans[pixel]);
            }
            pData += stride;
        }
    }
    else
    {
        for (y = y0; y <= y1; ++y)
        {
            for (x = x0, i = 0; x <= x1; ++x, ++i)
            {
                pixel = pData[i];
                if (pixel)
                    _SetPixelIndex(pDevice, x, y, pixel);
            }
            pData += stride;
        }
    }
}

/**
 * @brief   Draw 8BPP bitmap
 * @param   pDevice Device context
 * @param   x0      Starting X coordinate
 * @param   y0      Starting Y coordinate
 * @param   width   Bitmap width
 * @param   height  Bitmap height
 * @param   stride  Bytes per scan line, can be 0 if (height == 1)
 * @param   pData   Bitmap data
 * @param   pTrans  If pTrans is not NULL, it points to the palette and pData points is color index
 */
static inline void _DrawBitmap8BPP(GUI_DEVICE *pDevice, int x0, int y0, int width, int height, int stride, U8 const GUI_UNI_PTR *pData, const LCD_PIXELINDEX *pTrans)
{
    switch (GUI_pContext->DrawMode & LCD_DRAWMODE_TRANS)
    {
    case 0:
        _DrawBitmap8BPPOpaque(pDevice, x0, y0, width, height, stride, pData, pTrans);
        break;
    case LCD_DRAWMODE_TRANS:
        _DrawBitmap8BPPTransparent(pDevice, x0, y0, width, height, stride, pData, pTrans);
        break;
    }
}


/**
 * @brief   Draw 16BPP bitmap
 * @param   pDevice Device context
 * @param   x0      Starting X coordinate
 * @param   y0      Starting Y coordinate
 * @param   width   Bitmap width
 * @param   height  Bitmap height
 * @param   stride  Bytes per scan line, can be 0 if (height == 1)
 * @param   pData   Bitmap data
 */
static inline void _DrawBitmap16BPP(GUI_DEVICE *pDevice, int x0, int y0, int width, int height, int stride, U16 const GUI_UNI_PTR *pData)
{
    DRIVER_CONTEXT *pContext = (DRIVER_CONTEXT *)pDevice->u.pContext;
    int x1 = x0 + width - 1;
    int y1 = y0 + height - 1;
#if (LCD_MIRROR_X == 1) || (LCD_MIRROR_Y == 1) || (LCD_SWAP_XY == 1)
    U16 x0phys, x1phys, y0phys, y1phys;
    x0phys = LOG2PHYS_X(x0, y0);
    x1phys = LOG2PHYS_X(x1, y1);
    y0phys = LOG2PHYS_Y(x0, y0);
    y1phys = LOG2PHYS_Y(x1, y1);
#else
    #define x0phys ((U16)x0)
    #define x1phys ((U16)x1)
    #define y0phys ((U16)y0)
    #define y1phys ((U16)y1)
#endif

    // Set frame (box) size
	setXY(x0phys,y0phys, x1phys,y1phys);

	// Write data
    for (y0 = 0; y0 < height; ++y0) // reuse x0, y0 here
    {
        pContext->pfWriteMultipleData((U16 *)pData, width);
        pData = (U16 const GUI_UNI_PTR*)((U8 *)pData + stride);
     }
#if (LCD_MIRROR_X == 0) && (LCD_MIRROR_Y == 0) && (LCD_SWAP_XY == 0)
     #undef x0phys
     #undef x1phys
     #undef y0phys
     #undef y1phys
#endif
    // Restore column/row end
    /*pContext->pfWriteReg(0x0004);
    pContext->pfWriteData(HIBYTE(pContext->hwClip.x1));
    pContext->pfWriteReg(0x0005);
    pContext->pfWriteData(LOBYTE(pContext->hwClip.x1));
    pContext->pfWriteReg(0x0008);
    pContext->pfWriteData(HIBYTE(pContext->hwClip.y1));
    pContext->pfWriteReg(0x0009);
    pContext->pfWriteData(LOBYTE(pContext->hwClip.y1));*/
}


/**
 * @brief   Draw a bitmap
 * @param   pDevice Device context
 * @param   x0      Starting X coordinate
 * @param   y0      Starting Y coordinate
 * @param   width   Bitmap width
 * @param   height  Bitmap height
 * @param   bpp     Bits per pixel
 * @param   stride  Bytes per scan line, can be 0 if (height == 1)
 * @param   pData   Bitmap data
 * @param   diff    In 1,2,4 bit mode, diff indicates the offset to the first pixel in a byte
 * @param   pTrans  If pTrans is not NULL, it points to the palette and pData points is color index
 */
static void _DrawBitmap(GUI_DEVICE *pDevice, int x0, int y0, int width, int height, int bpp, int stride, const U8 GUI_UNI_PTR *pData, int diff, const LCD_PIXELINDEX *pTrans)
{
	printf("_DrawBitmap->%d %dx%d - %dx%d - str%d - %c - diff%d \n\r", bpp,
			x0, y0, width, height, stride, (char)*pData, diff);

    switch (bpp)
    {
    case 1:
        _DrawBitmap1BPP(pDevice, x0, y0, width, height, stride, (const U8 *)pData, diff, pTrans);
        break;
    case 2:
        _DrawBitmap2BPP(pDevice, x0, y0, width, height, stride, (const U8 *)pData, diff, pTrans);
        break;
    case 4:
        _DrawBitmap4BPP(pDevice, x0, y0, width, height, stride, (const U8 *)pData, diff, pTrans);
        break;
    case 8:
        _DrawBitmap8BPP(pDevice, x0, y0, width, height, stride, (const U8 *)pData, pTrans);
        break;
    case 16:
        _DrawBitmap16BPP(pDevice, x0, y0, width, height, stride, (const U16 *)pData);
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
	printf("_InitOnce\n\r");
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
	printf("_GetDevProp->%d\n\r", index);
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
	printf("_GetDevData->%d\n\r", index);
    GUI_USE_PARA(pDevice);
#if GUI_SUPPORT_MEMDEV
    switch (index)
    {
        case LCD_DEVDATA_MEMDEV:
            return (void *)&GUI_MEMDEV_DEVICE_16;   // Must match LCD color depth
    }
#else
    GUI_USE_PARA(Index);
#endif
    return NULL;
}


/**
 * @brief   Get device dimension
 * @param   pDevice Device context
 * @param   pRect   Device dimension encapsulated in LCD_RECT structure
 */
static void _GetRect(GUI_DEVICE *pDevice, LCD_RECT *pRect)
{
	printf("_GetRect:%dx%d - %dx%d\n\r", pRect->x0, pRect->y0, pRect->x1, pRect->y1);
    GUI_USE_PARA(pDevice);
    pRect->x0 = 0;
    pRect->y0 = 0;
    pRect->x1 = LCD_XSIZE - 1;
    pRect->y1 = LCD_YSIZE - 1;
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
	printf("_SetOrg:%dx%d\n\r", x, y);
    // Virtual screen not supported on this controller
    GUI_USE_PARA(pDevice);
    GUI_USE_PARA(x);
    GUI_USE_PARA(y);
}



/*********************************************************************
*
*       _On
*/

static void _ReadRect(GUI_DEVICE *pDevice, int x0, int y0, int x1, int y1, LCD_PIXELINDEX *pBuffer)
{
	printf("_ReadRect\n\r");
    DRIVER_CONTEXT *pContext;
    I32 total;
    U16 read[3];
    U16 *pBuf;
    total = (x1 - x0 + 1) * (y1 - y0 + 1);
    pContext = (DRIVER_CONTEXT *)pDevice->u.pContext;
    pBuf = (U16*)pBuffer;
#if (LCD_MIRROR_X == 1) || (LCD_MIRROR_Y == 1) || (LCD_SWAP_XY == 1)
    U16 x0phys, x1phys, y0phys, y1phys;
    x0phys = LOG2PHYS_X(x0, y0);
    x1phys = LOG2PHYS_X(x1, y1);
    y0phys = LOG2PHYS_Y(x0, y0);
    y1phys = LOG2PHYS_Y(x1, y1);
#else
    //! @cond DOXYGEN_SHOW_UNDOC_MACRO
    #define x0phys ((U16)x0)
    #define x1phys ((U16)x1)
    #define y0phys ((U16)y0)
    #define y1phys ((U16)y1)
    //! @endcond
#endif

    // Set frame (box) size
    setXY(x0phys,y0phys, x1phys,y1phys);

    // Read data
    pContext->pfReadData();    // Dummy read

    while (total > 1)
    {
        pContext->pfReadMultipleData(read, 3);
        *pBuf = ((read[0] & 0xF800) | ((read[0] & 0x00FC) << 3) | (read[1] >> 11));
        ++pBuf;
        *pBuf = (((read[1] & 0x00F8) << 8) | ((read[2] & 0xFC00) >> 5) | ((read[2] & 0x00F8) >> 3));
        ++pBuf;
        total -= 2;
    }
    if (total > 0)  // total == 1
    {
        pContext->pfReadMultipleData(read, 2);
        *pBuf = ((read[0] & 0xF800) | ((read[0] & 0x00FC) << 3) | (read[1] >> 11));
    }
#if (LCD_MIRROR_X == 0) && (LCD_MIRROR_Y == 0) && (LCD_SWAP_XY == 0)
    #undef x0phys
    #undef x1phys
    #undef y0phys
    #undef y1phys
#endif
    // Restore column/row end
    /*pContext->pfWriteReg(0x0004);
    pContext->pfWriteData(HIBYTE(pContext->hwClip.x1));
    pContext->pfWriteReg(0x0005);
    pContext->pfWriteData(LOBYTE(pContext->hwClip.x1));
    pContext->pfWriteReg(0x0008);
    pContext->pfWriteData(HIBYTE(pContext->hwClip.y1));
    pContext->pfWriteReg(0x0009);
    pContext->pfWriteData(LOBYTE(pContext->hwClip.y1));*/
}


static void _On(GUI_DEVICE *pDevice)
{
	printf("_On\n\r");
    // Call user command handler
    LCD_X_DisplayDriver(pDevice->LayerIndex, LCD_X_ON, NULL);
}


static void _Off(GUI_DEVICE *pDevice)
{
	printf("_Off\n\r");
    // Call user command handler
    LCD_X_DisplayDriver(pDevice->LayerIndex, LCD_X_OFF, NULL);
}


static int _Init(GUI_DEVICE *pDevice)
{
	printf("_Init\n\r");
    int r;
    //DRIVER_CONTEXT *pContext;

    // Call user command handler
    r = LCD_X_DisplayDriver(pDevice->LayerIndex, LCD_X_INITCONTROLLER, NULL);
    if (!r)
    	ILI9327_Init();

    return r;
}


/*********************************************************************
*
*       _GetDevFunc
*/
#ifndef USE_NEW_ILI9327
static void (* _GetDevFunc(GUI_DEVICE ** ppDevice, int Index))(void) {
  GUI_USE_PARA(ppDevice);
  switch (Index) {
  case LCD_DEVFUNC_SET_VRAM_ADDR:
    return (void (*)(void))_SetVRAMAddr;
  case LCD_DEVFUNC_SET_VSIZE:
    return (void (*)(void))_SetVSize;
  case LCD_DEVFUNC_SET_SIZE:
    return (void (*)(void))_SetSize;
  case LCD_DEVFUNC_INIT:
    return (void (*)(void))_Init;
  case LCD_DEVFUNC_ON:
    return (void (*)(void))_On;
  case LCD_DEVFUNC_OFF:
    return (void (*)(void))_Off;
  case LCD_DEVFUNC_SETLUTENTRY:
    return (void (*)(void))_SetLUTEntry;
  }
  return NULL;
}
#else
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
#endif


const GUI_DEVICE_API GUIDRV_ILI9327_API =
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


void GUIDRV_ILI9327_SetFunc(GUI_DEVICE *pDevice, ILI9327_HW_API *pAPI)
{
    DRIVER_CONTEXT * pContext;
    _InitOnce(pDevice);
    printf("#Init_once\n\r");

    pContext = (DRIVER_CONTEXT *)pDevice->u.pContext;
    pContext->pfWriteReg = pAPI->pfWriteReg;
    pContext->pfWriteData = pAPI->pfWriteData;
    pContext->pfWriteRepeatingData = pAPI->pfWriteRepeatingData;
    pContext->pfWriteMultipleData = pAPI->pfWriteMultipleData;
    pContext->pfReadReg = pAPI->pfReadReg;
    pContext->pfReadData = pAPI->pfReadData;
    pContext->pfReadMultipleData = pAPI->pfReadMultipleData;
}
