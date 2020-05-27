/* Includes ------------------------------------------------------------------*/
#include "stm32f4xx.h"
#include "stm32f4xx_conf.h"
#include "system_stm32f4xx.h"

#include "misc.h"
#include "stm32f4xx_adc.h"
#include "stm32f4xx_exti.h"
#include "stm32f4xx_flash.h"
#include "stm32f4xx_fsmc.h"
#include "stm32f4xx_gpio.h"
#include "stm32f4xx_i2c.h"
#include "stm32f4xx_iwdg.h"
#include "stm32f4xx_pwr.h"
#include "stm32f4xx_rcc.h"
#include "stm32f4xx_rng.h"
#include "stm32f4xx_sdio.h"
#include "stm32f4xx_tim.h"
#include "stm32f4xx_usart.h"

#include "FreeRTOS.h"
#include "FreeRTOSConfig.h"
#include "croutine.h"
#include "portmacro.h"
#include "queue.h"
#include "task.h"
#include "timers.h"

#include "ILI9327.h"
#include "touch.h"

#include "semphr.h"
#include "touch.h"

#include "string_alt.h"

#include "DIALOG.h"
#include "GUI.h"

#include "semihosting.h"
#include <stdio.h>

#include "GUIDEMO.h"

enum
{
    OFF,
    RX,
    ACQ
};
static uint8_t rx_cnt = 0;
static uint16_t rx_value = 0;
static uint8_t rx_phase = OFF;

/* Private typedef -----------------------------------------------------------*/
/* Private define ------------------------------------------------------------*/
#define Main_Task_PRIO (tskIDLE_PRIORITY + 9)
#define Main_Task_STACK (3048)

/* Private macro -------------------------------------------------------------*/
#define LOBYTE(x) ((uint8_t)(x & 0x00FF))
#define HIBYTE(x) ((uint8_t)((x & 0xFF00) >> 8))

/* Private variables ---------------------------------------------------------*/
xTaskHandle Task_Handle;
xTimerHandle TouchScreenTimer;

extern WM_HWIN ALARM_hWin;

BUTTON_Handle hButton_1, hButton_2, hButton_3, hButton_4, hButton_5;
FRAMEWIN_Handle hFrame1, hFrame2;
TEXT_Handle hText_1, hText_2, hText_3;

/* Private function prototypes -----------------------------------------------*/
void cbFRAMEWIN_1(WM_MESSAGE *pMsg);

static void Main_Task(void *pvParameters);
static void vTimerCallback(xTimerHandle pxTimer);
extern void ALARM_BackgroundProcess(void);

void vApplicationIdleHook(void) {}
void vApplicationMallocFailedHook(void)
{
    for(;;)
        ;
}
void vApplicationTickHook(void) {}
void vApplicationStackOverflowHook(xTaskHandle pxTask, signed char *pcTaskName)
{
    (void)pcTaskName;
    (void)pxTask;

    for(;;)
        ;
}

/*******************************************************************************
* Function Name  : InitPeriph
* Description    :
*******************************************************************************/
void InitPeriph()
{
    /** DISCOVERY LEDS */
    //ORANGE - LD3 - PD13
    //GREEN  - LD4 - PD12
    //RED 	 - LD5 - PD14
    //BLUE	 - LD6 - PD15
    RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOC, ENABLE);
    RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOD, ENABLE);

    GPIO_InitTypeDef GPIO_InitStructure;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_OUT;
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_12 | GPIO_Pin_13 | GPIO_Pin_14 | GPIO_Pin_15;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_Init(GPIOD, &GPIO_InitStructure);

    RCC_AHB2PeriphClockCmd(RCC_AHB2Periph_RNG, ENABLE);
    RNG_Cmd(ENABLE);
}

#define MAX_VALUE 180

// Recommended memory to run the sample with adequate performance
#define RECOMMENDED_MEMORY (1024L * 30)

static GRAPH_DATA_Handle _ahData;   // handlesfor the GRAPH_DATA objects
static GRAPH_SCALE_Handle _hScaleV; // Handle of vertical scale
static GRAPH_SCALE_Handle _hScaleH; // Handle of horizontal scale

static int _Stop = 0;

//
// Dialog ressource
//
static const GUI_WIDGET_CREATE_INFO _aDialogCreate[] = {
    {FRAMEWIN_CreateIndirect, "Graph widget demo", 0, 0, 0, 400, 240, FRAMEWIN_CF_MAXIMIZED},
    {GRAPH_CreateIndirect, 0, GUI_ID_GRAPH0, 5, 5, 400 - 5 - 5, 240 - 70 - 10},
    {TEXT_CreateIndirect, "1234", GUI_ID_TEXT0, 80, 240 - 70, 400, 48},
    {TEXT_CreateIndirect, "CO2:", GUI_ID_TEXT1, 5, 240 - 70 - 5, 400, 48},
    {TEXT_CreateIndirect, "ppm", GUI_ID_TEXT2, 5, 240 - 70 - 5 + 25, 400, 48},
};

static void _UserDraw(WM_HWIN hWin, int Stage)
{
    if(Stage == GRAPH_DRAW_LAST)
    {
        char acText[] = "CO2 ppm";
        GUI_RECT Rect;
        GUI_RECT RectInvalid;
        int FontSizeY;

        GUI_SetFont(&GUI_Font13_ASCII);
        FontSizeY = GUI_GetFontSizeY();
        WM_GetInsideRect(&Rect);
        WM_GetInvalidRect(hWin, &RectInvalid);
        Rect.x1 = Rect.x0 + FontSizeY;
        GUI_SetColor(GUI_YELLOW);
        GUI_DispStringInRectEx(acText, &Rect, GUI_TA_HCENTER, strlen(acText), GUI_ROTATE_CCW);

        {
            GUI_DispStringAt("minutes", 180, 145);
        }
    }
}

/*********************************************************************
*
*       _ForEach
*
* Function description
*   This routine hides/shows all windows except the button, graph and scroll bar widgets
*/
static void _ForEach(WM_HWIN hWin, void *pData)
{
    int Id;
    int FullScreenMode;

    FullScreenMode = *(int *)pData;
    Id = WM_GetId(hWin);
    if((Id == GUI_ID_GRAPH0) || (Id == GUI_ID_BUTTON0) || (Id == GUI_ID_VSCROLL) || (Id == GUI_ID_HSCROLL))
    {
        return;
    }
    if(FullScreenMode)
    {
        WM_HideWindow(hWin);
    }
    else
    {
        WM_ShowWindow(hWin);
    }
}

/*********************************************************************
*
*       _ToggleFullScreenMode
*
* Function description
*   This routine switches between full screen mode and normal mode by hiding or showing the
*   widgets of the dialog, enlarging/shrinking the graph widget and modifying some other
*   attributes of the dialog widgets.
*/
static void _ToggleFullScreenMode(WM_HWIN hDlg)
{
    static int FullScreenMode;
    static GUI_RECT Rect;
    static unsigned ScalePos;
    WM_HWIN hGraph;
    WM_HWIN hButton;
    WM_HWIN hClient;
    GUI_RECT RectInside;
    int xPos, yPos;

    hGraph = WM_GetDialogItem(hDlg, GUI_ID_GRAPH0);
    hButton = WM_GetDialogItem(hDlg, GUI_ID_BUTTON0);
    FullScreenMode ^= 1;
    if(FullScreenMode)
    {
        //
        // Enter the full screen mode
        //
        hClient = WM_GetClientWindow(hDlg);
        BUTTON_SetText(hButton, "Back");
        WM_MoveWindow(hButton, 0, 11);
        FRAMEWIN_SetTitleVis(hDlg, 0);
        WM_GetInsideRectEx(hClient, &RectInside);
        WM_GetWindowRectEx(hGraph, &Rect);
        WM_ForEachDesc(hClient, _ForEach, &FullScreenMode); // Hide all descendants
        xPos = WM_GetWindowOrgX(hClient);
        yPos = WM_GetWindowOrgY(hClient);
        WM_SetWindowPos(hGraph, xPos, yPos, RectInside.x1, RectInside.y1);
        ScalePos = GRAPH_SCALE_SetPos(_hScaleH, RectInside.y1 - 20);
    }
    else
    {
        //
        // Return to normal mode
        //
        BUTTON_SetText(hButton, "Full Screen");
        WM_MoveWindow(hButton, 0, -11);
        WM_ForEachDesc(WM_GetClientWindow(hDlg), _ForEach, &FullScreenMode); // Show all descendants
        WM_SetWindowPos(hGraph, Rect.x0, Rect.y0, Rect.x1 - Rect.x0 + 1, Rect.y1 - Rect.y0 + 1);
        FRAMEWIN_SetTitleVis(hDlg, 1);
        GRAPH_SCALE_SetPos(_hScaleH, ScalePos);
    }
}

static void _cbCallback(WM_MESSAGE *pMsg)
{
    int NCode;
    int Id;
    int Value;
    WM_HWIN hDlg;
    WM_HWIN hItem;

    hDlg = pMsg->hWin;
    switch(pMsg->MsgId)
    {
    case WM_INIT_DIALOG:
        hItem = WM_GetDialogItem(hDlg, GUI_ID_GRAPH0);

        _ahData = GRAPH_DATA_YT_Create(GUI_GREEN, 500, 0, 0);
        GRAPH_AttachData(hItem, _ahData);

        //
        // Set graph attributes
        //
        //        GRAPH_SetGridDistY(hItem, 25);
        GRAPH_SetGridVis(hItem, 1);
        //        GRAPH_SetGridFixedX(hItem, 1);
        GRAPH_SetUserDraw(hItem, _UserDraw);
        //
        // Create and add vertical scale
        //
        _hScaleV = GRAPH_SCALE_Create(35, GUI_TA_RIGHT, GRAPH_SCALE_CF_VERTICAL, 25);
        GRAPH_SCALE_SetTextColor(_hScaleV, GUI_YELLOW);
        GRAPH_SCALE_SetFactor(_hScaleV, 10);
        GRAPH_AttachScale(hItem, _hScaleV);
        //
        // Create and add horizontal scale
        //
        _hScaleH = GRAPH_SCALE_Create(140, GUI_TA_CENTER, GRAPH_SCALE_CF_HORIZONTAL, 50);
        GRAPH_SCALE_SetTextColor(_hScaleH, GUI_YELLOW);
        GRAPH_AttachScale(hItem, _hScaleH);
        GRAPH_SCALE_SetFactor(_hScaleH, 0.166666);
        //
        // Init check boxes
        //
        //        CHECKBOX_SetState(hItem, 1);
        //        hItem = WM_GetDialogItem(hDlg, GUI_ID_CHECK6);
        //        CHECKBOX_SetText(hItem, "HScroll");
        //        CHECKBOX_SetState(hItem, 1);
        //        hItem = WM_GetDialogItem(hDlg, GUI_ID_CHECK7);
        //        CHECKBOX_SetText(hItem, "VScroll");
        //        hItem = WM_GetDialogItem(hDlg, GUI_ID_CHECK8);
        //        CHECKBOX_SetText(hItem, "MirrorX");
        //
        // Init slider widgets
        //
        //        hItem = WM_GetDialogItem(hDlg, GUI_ID_SLIDER0);
        //        SLIDER_SetRange(hItem, 0, 10);
        //        SLIDER_SetValue(hItem, 5);
        //        SLIDER_SetNumTicks(hItem, 6);
        //        hItem = WM_GetDialogItem(hDlg, GUI_ID_SLIDER1);
        //        SLIDER_SetRange(hItem, 0, 20);
        //        SLIDER_SetValue(hItem, 5);
        //        SLIDER_SetNumTicks(hItem, 6);
        //
        // Init radio widget
        //
        //        hItem = WM_GetDialogItem(hDlg, GUI_ID_RADIO0);
        //        RADIO_SetText(hItem, "3D", 0);
        //        RADIO_SetText(hItem, "flat", 1);
        //        RADIO_SetText(hItem, "-", 2);
        //
        // Init button widget
        //
        //        hItem = WM_GetDialogItem(hDlg, GUI_ID_BUTTON0);
        //        WM_SetStayOnTop(hItem, 1);
        break;
    case WM_NOTIFY_PARENT:
        Id = WM_GetId(pMsg->hWinSrc); // Id of widget
        NCode = pMsg->Data.v;         // Notification code
        switch(NCode)
        {
        case WM_NOTIFICATION_CLICKED:
            switch(Id)
            {
            case GUI_ID_BUTTON0:
                _ToggleFullScreenMode(hDlg);
                break;
            }
            break;
        case WM_NOTIFICATION_VALUE_CHANGED:
            switch(Id)
            {
            case GUI_ID_CHECK0:
                //
                // Toggle stop mode
                //
                _Stop ^= 1;
                break;

            case GUI_ID_SLIDER0:
                //
                // Set horizontal grid spacing
                //
                hItem = WM_GetDialogItem(hDlg, GUI_ID_GRAPH0);
                Value = SLIDER_GetValue(pMsg->hWinSrc) * 10;
                GRAPH_SetGridDistX(hItem, Value);
                GRAPH_SCALE_SetTickDist(_hScaleH, Value);
                break;
            case GUI_ID_SLIDER1:
                //
                // Set vertical grid spacing
                //
                hItem = WM_GetDialogItem(hDlg, GUI_ID_GRAPH0);
                Value = SLIDER_GetValue(pMsg->hWinSrc) * 5;
                GRAPH_SetGridDistY(hItem, Value);
                GRAPH_SCALE_SetTickDist(_hScaleV, Value);
                break;
            case GUI_ID_RADIO0:
                //
                // Set the widget effect
                //
                hItem = WM_GetDialogItem(hDlg, GUI_ID_GRAPH0);
                switch(RADIO_GetValue(pMsg->hWinSrc))
                {
                case 0:
                    WIDGET_SetEffect(hItem, &WIDGET_Effect_3D);
                    break;
                case 1:
                    WIDGET_SetEffect(hItem, &WIDGET_Effect_Simple);
                    break;
                case 2:
                    WIDGET_SetEffect(hItem, &WIDGET_Effect_None);
                    break;
                }
                break;
            case GUI_ID_CHECK6:
                //
                // Toggle horizontal scroll bar
                //
                hItem = WM_GetDialogItem(hDlg, GUI_ID_GRAPH0);
                if(CHECKBOX_IsChecked(WM_GetDialogItem(hDlg, GUI_ID_CHECK6)))
                {
                    GRAPH_SetVSizeX(hItem, 500);
                }
                else
                {
                    GRAPH_SetVSizeX(hItem, 0);
                }
                break;
            case GUI_ID_CHECK7:
                //
                // Toggle vertical scroll bar
                //
                hItem = WM_GetDialogItem(hDlg, GUI_ID_GRAPH0);
                if(CHECKBOX_IsChecked(WM_GetDialogItem(hDlg, GUI_ID_CHECK7)))
                {
                    GRAPH_SetVSizeY(hItem, 300);
                }
                else
                {
                    GRAPH_SetVSizeY(hItem, 0);
                }
                break;
            case GUI_ID_CHECK8:
                //
                // Toggle alignment
                //
                WM_GetDialogItem(hDlg, GUI_ID_GRAPH0);
                //                for(i = 0; i < GUI_COUNTOF(_aColor); i++)
                {
                    if(CHECKBOX_IsChecked(WM_GetDialogItem(hDlg, GUI_ID_CHECK8)))
                    {
                        GRAPH_DATA_YT_SetAlign(_ahData, GRAPH_ALIGN_LEFT);
                        GRAPH_DATA_YT_MirrorX(_ahData, 1);
                    }
                    else
                    {
                        GRAPH_DATA_YT_SetAlign(_ahData, GRAPH_ALIGN_RIGHT);
                        GRAPH_DATA_YT_MirrorX(_ahData, 0);
                    }
                }
                break;
            }
            break;
        }
        break;
    default:
        WM_DefaultProc(pMsg);
    }
}

static void Main_Task(void *pvParameters)
{
    WM_HWIN hDlg = GUI_CreateDialogBox(_aDialogCreate, GUI_COUNTOF(_aDialogCreate), _cbCallback, 0, 0, 0);
    // WM_HWIN hGraph = WM_GetDialogItem(hDlg, GUI_ID_GRAPH0);

    WM_HWIN hText = WM_GetDialogItem(hDlg, GUI_ID_TEXT1);
    TEXT_SetFont(hText, &GUI_Font32B_ASCII);
    TEXT_SetTextColor(hText, GUI_GREEN);

    hText = WM_GetDialogItem(hDlg, GUI_ID_TEXT2);
    TEXT_SetFont(hText, &GUI_Font32B_ASCII);
    TEXT_SetTextColor(hText, GUI_GREEN);

    hText = WM_GetDialogItem(hDlg, GUI_ID_TEXT0);
    TEXT_SetFont(hText, &GUI_FontD36x48);
    TEXT_SetTextColor(hText, GUI_GREEN);

    while(1)
    {
        static int16_t _aValue = 0;

        if(rx_phase == RX)
        {
            rx_phase = ACQ;
            _aValue = rx_value;
            GRAPH_DATA_YT_AddValue(_ahData, _aValue / 10);
        }
        //#define GUI_BLUE          0x00FF0000
        //#define GUI_GREEN         0x0000FF00
        //#define GUI_RED           0x000000FF
        //#define GUI_CYAN          0x00FFFF00
        //#define GUI_MAGENTA       0x00FF00FF
        //#define GUI_YELLOW        0x0000FFFF
        char s[10];
        itoa_(_aValue, s);
        TEXT_SetText(hText, s);
        if(_aValue < 900)
            TEXT_SetTextColor(hText, /*GUI_GREEN*/ 0x0000FF00);
        else if((_aValue < 1200))
            TEXT_SetTextColor(hText, /*GUI_YELLOW*/ 0x00FFFF00);
        else
            TEXT_SetTextColor(hText, /*GUI_RED*/ 0x00FF0000);

        GUI_Exec();

        GUI_Delay(100);
    }
}

static void send_char(uint8_t c)
{
    while(!USART_GetFlagStatus(USART2, USART_FLAG_TC))
        ;
    USART_SendData(USART2, c);
}

static void task_mh_z19(void *pvParameters)
{
    while(1)
    {
    }
}

/**
  * @brief  Callback function
  * @param  None
  * @retval None
  */
void cbFRAMEWIN_1(WM_MESSAGE *pMsg)
{
    switch(pMsg->MsgId)
    {
    default:
        // The original callback
        FRAMEWIN_Callback(pMsg);
        break;
    }
}

static void vTimerCallback(xTimerHandle pxTimer)
{
    //BSP_Pointer_Update();

    //>>Touch Callback

    static GUI_PID_STATE TS_State;
    uint16_t x, y;

    if(!GPIO_ReadInputDataBit(XPT2046_IRQ_PORT, XPT2046_IRQ_PAD))
    {
        XPT2046_GetCursor(&x, &y);

        TS_State.x = x;
        TS_State.y = y;

        TS_State.Pressed = 1;
        TS_State.Layer = 0;

        if((x < LCD_GetXSize()) && (y < LCD_GetYSize()))
            GUI_TOUCH_StoreStateEx(&TS_State);
    }
    else
    {
        TS_State.Pressed = 0;
        GUI_TOUCH_StoreStateEx(&TS_State);
    }
    //<< Touch Callback
}

#define Main_Task_PRIO (tskIDLE_PRIORITY + 9)
#define Main_Task_STACK (3048)

extern const unsigned char SmallFont[1144];

int i2c_write(uint8_t address, uint16_t reg)
{
#define I2C1_TIMEOUT 1000
    volatile uint32_t timeout = I2C1_TIMEOUT;
    I2C_GenerateSTART(I2C1, ENABLE);

    timeout = I2C1_TIMEOUT;
    while(!I2C_CheckEvent(I2C1, I2C_EVENT_MASTER_MODE_SELECT))
    {
        if(timeout != 0)
            timeout--;
        else
            return (-1);
    }

    I2C_Send7bitAddress(I2C1, address, I2C_Direction_Transmitter);

    timeout = I2C1_TIMEOUT;
    while(!I2C_CheckEvent(I2C1, I2C_EVENT_MASTER_TRANSMITTER_MODE_SELECTED))
    {
        if(timeout != 0)
            timeout--;
        else
            return (-2);
    }

    // ADDR-Flag löschen
    I2C1->SR2;

    timeout = I2C1_TIMEOUT;
    while(!I2C_GetFlagStatus(I2C1, I2C_FLAG_TXE))
    {
        if(timeout != 0)
            timeout--;
        else
            return (-3);
    }
    I2C_GenerateSTOP(I2C1, ENABLE);

    return 0;
}

/*******************************************************************************
* Function Name  : main
*******************************************************************************/
int main()
{
    InitPeriph();
    SystemInit();

    // Necessary for STemWin to authenticate that it is runing on STM32 processor
    RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_CRC, ENABLE);

    PWM_BL_Init();
    XPT2046_Init();
    //ILI9327_Init();
    //ILI9327_Clear();
    //printf("#System started!\n\r");
    //printf("#Gui initialization...\n\r");

    //printf("#...Gui finished initialization!\n\r");

    GUI_Init();

    ILI9327_setFont(SmallFont);

    ILI9327_setColor(VGA_GREEN);

    GPIO_InitTypeDef GPIO_InitStruct; // this is for the GPIO pins used as I2C1SDA and I2C1SCL
    I2C_InitTypeDef I2C_InitStruct;   // this is for the I2C1 initialization
    RCC_APB1PeriphClockCmd(RCC_APB1Periph_I2C1, ENABLE);
    RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOB, ENABLE);
    GPIO_InitStruct.GPIO_Pin = GPIO_Pin_6 | GPIO_Pin_7;
    GPIO_InitStruct.GPIO_Mode = GPIO_Mode_AF;               // the pins are configured as alternate function so the USART peripheral has access to them
    GPIO_InitStruct.GPIO_Speed = GPIO_Speed_2MHz;           // this defines the IO speed and has nothing to do with the baudrate!
    GPIO_InitStruct.GPIO_OType = GPIO_OType_OD;             // this defines the output type as open drain
    GPIO_InitStruct.GPIO_PuPd = GPIO_PuPd_UP;               // this activates the pullup resistors on the IO pins
    GPIO_Init(GPIOB, &GPIO_InitStruct);                     // now all the values are passed to the GPIO_Init()
    GPIO_PinAFConfig(GPIOB, GPIO_PinSource6, GPIO_AF_I2C1); //
    GPIO_PinAFConfig(GPIOB, GPIO_PinSource7, GPIO_AF_I2C1);

    /* Set the I2C structure parameters */
    I2C_InitStruct.I2C_Mode = I2C_Mode_I2C;
    I2C_InitStruct.I2C_DutyCycle = I2C_DutyCycle_2;
    I2C_InitStruct.I2C_OwnAddress1 = 0xEE;
    I2C_InitStruct.I2C_Ack = I2C_Ack_Enable;
    I2C_InitStruct.I2C_AcknowledgedAddress = I2C_AcknowledgedAddress_7bit;
    I2C_InitStruct.I2C_ClockSpeed = 100000; //moderat, keine Eile geboten...

    /* Enable the I2C peripheral */
    I2C_Cmd(I2C1, ENABLE);
    /* Initialize the I2C peripheral w/ selected parameters */
    I2C_Init(I2C1, &I2C_InitStruct);

    while(1)
    {
        int sts = i2c_write(0x52, 0);

        static int cnt = 0;
        cnt++;
        // ILI9327_String(10, 10, "fuck!");

        ILI9327_NumWDesc(0, 0, "P=", cnt);
        ILI9327_NumWDesc(0, 15, "P=", sts);
        _delay_ms(500);
    }

    GPIO_InitTypeDef gpio;
    USART_InitTypeDef usart;

    RCC_APB1PeriphClockCmd(RCC_APB1Periph_USART2, ENABLE);
    RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOA, ENABLE);
    GPIO_StructInit(&gpio);

    gpio.GPIO_Mode = GPIO_Mode_AF;
    gpio.GPIO_Pin = GPIO_Pin_2 | GPIO_Pin_3;
    gpio.GPIO_Speed = GPIO_Speed_50MHz;
    gpio.GPIO_OType = GPIO_OType_PP;
    gpio.GPIO_PuPd = GPIO_PuPd_UP;
    GPIO_Init(GPIOA, &gpio);

    GPIO_PinAFConfig(GPIOA, GPIO_PinSource2, GPIO_AF_USART2);
    GPIO_PinAFConfig(GPIOA, GPIO_PinSource3, GPIO_AF_USART2);

    USART_StructInit(&usart);
    usart.USART_Mode = USART_Mode_Rx | USART_Mode_Tx;
    usart.USART_BaudRate = 9600;
    USART_Init(USART2, &usart);

    // �������� ���������� � ��������� USART
    NVIC_EnableIRQ(USART2_IRQn);
    USART_Cmd(USART2, ENABLE);

    USART_ITConfig(USART2, USART_IT_RXNE, ENABLE);

    //Touch_Init();

    //tFlashLine = xTimerCreate("FlashLine", (100/portTICK_RATE_MS), pdTRUE, 0, Flashing);
    //xTimerReset(tFlashLine, 0);

    //xLEDMutex = xSemaphoreCreateMutex();

    GPIO_InitTypeDef GPIO_InitStructure;

    NVIC_PriorityGroupConfig(NVIC_PriorityGroup_4);

    /* Enable the BUTTON Clock */
    RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOA, ENABLE);

    /* Configure Button pin as input */
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_OUT;
    GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
    GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_NOPULL;
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_0;
    GPIO_Init(GPIOA, &GPIO_InitStructure);

    /* Force capacity to be charged quickly */
    GPIO_WriteBit(GPIOA, GPIO_Pin_0, Bit_RESET);
    // for (i = 0x00; i<0xFF; i++);

    //STM_EVAL_PBInit(BUTTON_USER, BUTTON_MODE_GPIO);

    /* Create main task */
    xTaskCreate(Main_Task, "MainTask", Main_Task_STACK, NULL, Main_Task_PRIO, &Task_Handle);

    /* Launch Touchscreen Timer */
    TouchScreenTimer = xTimerCreate("Timer", 50, pdTRUE, (void *)1, vTimerCallback);

    if(TouchScreenTimer != NULL)
    {
        if(xTimerStart(TouchScreenTimer, 0) != pdPASS)
        {
            /* The timer could not be set into the Active state. */
        }
    }

    xTaskCreate(task_mh_z19, "MHZ19", configMINIMAL_STACK_SIZE, NULL, tskIDLE_PRIORITY + 1, NULL);
    //xTaskCreate(vDisplayTask, "DisplayTask", configMINIMAL_STACK_SIZE, NULL, tskIDLE_PRIORITY + 1, NULL);
    //xTaskCreate(vTouchDispatcher, "vTouch", configMINIMAL_STACK_SIZE, NULL, tskIDLE_PRIORITY + 1, NULL);
    /* Start the FreeRTOS scheduler */
    vTaskStartScheduler();

    /*static portBASE_TYPE xHigherPriorityTaskWoken;
		xHigherPriorityTaskWoken = pdFALSE;

		xSemaphoreGiveFromISR(xSemaphore_Wait, &xHigherPriorityTaskWoken );
		if(xHigherPriorityTaskWoken == pdTRUE)
		{
			taskYIELD();
		}*/

    return 0;
}

void USART2_IRQHandler()
{
    // ����������, ��� ���������� ������� ������ ������� � �������� ������
    if(USART_GetITStatus(USART2, USART_IT_RXNE) != RESET)
    {
        // ������ ���� ����������
        USART_ClearITPendingBit(USART2, USART_IT_RXNE);

        // �� ���� ���� ��� ������� � ��������� ������ )
        char x = USART_ReceiveData(USART2);
        if(rx_cnt == 2)
        {
            rx_value = x << 8;
        }
        else if(rx_cnt == 3)
        {
            rx_value |= x;
            rx_phase = RX;
        }
        rx_cnt++;
    }
}
