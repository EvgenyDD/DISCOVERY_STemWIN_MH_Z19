#include "GUI.h"
#include "stm32f4xx.h"

#include "ILI9327.h"
#include "stdio.h"

#if 0

static void LcdReadDataMultiple(U16 * pData, int NumItems)
{
  while (NumItems--)
  {
    *pData++ = STM_FSMC_BANK3_ReadData();
  }
}

#endif

#include "GUIDRV_ILI9327.h"
void LCD_X_Config(void)
{
    GUI_DEVICE *pDevice;
    ILI9327_HW_API hwAPI = {0};

    // Set display driver and color conversion
    pDevice = GUI_DEVICE_CreateAndLink(GUIDRV_ILI9327, GUICC_565, 0, 0);

    // Display driver is compile-time configured. Setting (virtual) screen size is not necessary
    //LCD_SetSizeEx (0, 400, 240);
    //LCD_SetVSizeEx(0, 400, 240);

    // Port access functions
    hwAPI.pfWriteReg = ILI9327_WriteCom;
    hwAPI.pfWriteData = ILI9327_WriteData;
    hwAPI.pfWriteRepeatingData = _fast_fill_16;
    hwAPI.pfWriteMultipleData = _fill_multiple;
    hwAPI.pfReadData = ILI9327_ReadData;
    hwAPI.pfReadMultipleData = ILI9327_ReadDataMultiple;

    // Give driver access to the functions
    GUIDRV_ILI9327_SetFunc(pDevice, &hwAPI);
}

/*********************************************************************
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
int LCD_X_DisplayDriver(unsigned LayerIndex, unsigned cmd, void *pData)
{
    int r = -1;

    printf("#LCD_X_DisplayDriver->%d\n\r", cmd);
    //(void) LayerIndex;
    //(void) pData;

    switch(cmd)
    {
    case LCD_X_INITCONTROLLER:
        ILI9327_Init();
        r = 0;
        break;

    default:
        break;
    }

    return r;
}
