#ifndef GUIDRV_ILI9327_H
#define GUIDRV_ILI9327_H

extern const GUI_DEVICE_API GUIDRV_ILI9327_API;
#define GUIDRV_ILI9327 &GUIDRV_ILI9327_API

/* Physical display size */
#if 1
#define XSIZE_PHYS  240
#define YSIZE_PHYS  400
#else
#define XSIZE_PHYS  400
#define YSIZE_PHYS  240
#endif

typedef struct _ILI9327_HW_API
{
    //! Reset LCD controller
    void (*pfWriteReg)(U16 data);
    //! Write 16-bit data to LCD (RS = 1)
    void (*pfWriteData)(U16 data);
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
} ILI9327_HW_API;


void GUIDRV_ILI9327_SetFunc(GUI_DEVICE *device, ILI9327_HW_API *pAPI);


#endif  //GUIDRV_ILI9327_H
