/* Includes ------------------------------------------------------------------*/
#include "stm32f4xx.h"
#include "stm32f4xx_conf.h"
#include "system_stm32f4xx.h"

#include "stm32f4xx_adc.h"
#include "stm32f4xx_flash.h"
#include "stm32f4xx_fsmc.h"
#include "stm32f4xx_gpio.h"
#include "stm32f4xx_iwdg.h"
#include "stm32f4xx_exti.h"
#include "stm32f4xx_pwr.h"
#include "stm32f4xx_rcc.h"
#include "stm32f4xx_rng.h"
#include "stm32f4xx_sdio.h"
#include "stm32f4xx_tim.h"
#include "stm32f4xx_usart.h"
#include "misc.h"

#include "FreeRTOSConfig.h"
#include "FreeRTOS.h"
#include "croutine.h"
#include "portmacro.h"
#include "task.h"
#include "timers.h"
#include "queue.h"

#include "ILI9327.h"
#include "touch.h"

#include "touch.h"
#include "semphr.h"

#include "string.h"

#include "GUI.h"
#include "DIALOG.h"

#include "semihosting.h"
#include <stdio.h>


#include "GUIDEMO.h"


/* Private typedef -----------------------------------------------------------*/
/* Private define ------------------------------------------------------------*/
#define Main_Task_PRIO          ( tskIDLE_PRIORITY  + 9 )
#define Main_Task_STACK         ( 3048 )



/* Private macro -------------------------------------------------------------*/
#define LOBYTE(x)  ((uint8_t)(x & 0x00FF))
#define HIBYTE(x)  ((uint8_t)((x & 0xFF00) >>8))


/* Private variables ---------------------------------------------------------*/
xTaskHandle                   Task_Handle;
xTimerHandle                  TouchScreenTimer;

extern WM_HWIN  ALARM_hWin;

BUTTON_Handle   hButton_1, hButton_2, hButton_3, hButton_4, hButton_5;
FRAMEWIN_Handle hFrame1, hFrame2;
TEXT_Handle     hText_1, hText_2, hText_3;


/* Private function prototypes -----------------------------------------------*/
       void cbFRAMEWIN_1(WM_MESSAGE * pMsg);
       void cbBUTTON_1(WM_MESSAGE * pMsg);
       void cbBUTTON_2(WM_MESSAGE * pMsg);
       void cbBUTTON_3(WM_MESSAGE * pMsg);
       void cbBUTTON_4(WM_MESSAGE * pMsg);
       void cbBUTTON_5(WM_MESSAGE * pMsg);
static void Main_Task(void * pvParameters);
static void vTimerCallback(xTimerHandle pxTimer);
extern void ALARM_BackgroundProcess(void);

/* Private function prototypes -----------------------------------------------*/
/* Private functions ---------------------------------------------------------*/
/*******************************************************************************
* Function Name  : vApplicationIdleHook
* Description    :
*******************************************************************************/
void vApplicationIdleHook( void ){}


/*******************************************************************************
* Function Name  : vApplicationMallocFailedHook
* Description    :
*******************************************************************************/
void vApplicationMallocFailedHook( void )
{
    for( ;; );
}


/*******************************************************************************
* Function Name  : vApplicationStackOverflowHook
* Description    :
*******************************************************************************/
void vApplicationStackOverflowHook( xTaskHandle pxTask, signed char *pcTaskName )
{
    ( void ) pcTaskName;
    ( void ) pxTask;

    for( ;; );
}


/*******************************************************************************
* Function Name  : vApplicationTickHook
* Description    :
*******************************************************************************/
void vApplicationTickHook( void )
{
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




/*
void vDisplayTask(void *pvParameters)
{
	while(1)
	{
		vTaskDelay(80);
	}
	vTaskDelete(NULL);
}
*/

#define MAX_VALUE 180

// Recommended memory to run the sample with adequate performance
#define RECOMMENDED_MEMORY (1024L * 30)

static GRAPH_DATA_Handle  _ahData[3]; // Array of handles for the GRAPH_DATA objects
static GRAPH_SCALE_Handle _hScaleV;   // Handle of vertical scale
static GRAPH_SCALE_Handle _hScaleH;   // Handle of horizontal scale

static I16 _aValue[3];
static int _Stop = 0;

static GUI_COLOR _aColor[] = {GUI_RED, GUI_GREEN, GUI_LIGHTBLUE}; // Array of colors for the GRAPH_DATA objects

//
// Dialog ressource
//
static const GUI_WIDGET_CREATE_INFO _aDialogCreate[] = {
  { FRAMEWIN_CreateIndirect, "Graph widget demo",  0                ,   0,   0, 240, 240, FRAMEWIN_CF_MOVEABLE },
  { GRAPH_CreateIndirect,     0,                   GUI_ID_GRAPH0    ,   5,   5, 265, 170 },
  { TEXT_CreateIndirect,      "Spacing X:",        0                ,  10, 180,  50,  20 },
  { TEXT_CreateIndirect,      "Spacing Y:",        0                ,  10, 200,  50,  20 },
  { SLIDER_CreateIndirect,    0,                   GUI_ID_SLIDER0   ,  60, 180,  60,  16 },
  { SLIDER_CreateIndirect,    0,                   GUI_ID_SLIDER1   ,  60, 200,  60,  16 },
  { CHECKBOX_CreateIndirect,  0,                   GUI_ID_CHECK0    , 130, 180,  50,   0 },
  { CHECKBOX_CreateIndirect,  0,                   GUI_ID_CHECK1    , 130, 200,  50,   0 },
  { TEXT_CreateIndirect,      "Border",            0                , 275,   5,  35,  15 },
  { CHECKBOX_CreateIndirect,  0,                   GUI_ID_CHECK2    , 275,  20,  35,   0 },
  { CHECKBOX_CreateIndirect,  0,                   GUI_ID_CHECK3    , 275,  40,  35,   0 },
  { CHECKBOX_CreateIndirect,  0,                   GUI_ID_CHECK4    , 275,  60,  35,   0 },
  { CHECKBOX_CreateIndirect,  0,                   GUI_ID_CHECK5    , 275,  80,  35,   0 },
  { TEXT_CreateIndirect,      "Effect",            0                , 275, 100,  35,  15 },
  { RADIO_CreateIndirect,     0,                   GUI_ID_RADIO0    , 270, 115,  35,   0, 0, 3 },
  { CHECKBOX_CreateIndirect,  0,                   GUI_ID_CHECK6    , 180, 180,  50,   0 },
  { CHECKBOX_CreateIndirect,  0,                   GUI_ID_CHECK7    , 180, 200,  50,   0 },
  { BUTTON_CreateIndirect,    "Full Screen",       GUI_ID_BUTTON0   , 240, 180,  65,  18 },
  { CHECKBOX_CreateIndirect,  0,                   GUI_ID_CHECK8    , 240, 200,  70,   0 },
};

/*********************************************************************
*
*       _AddValues
*
* Function description
*   This routine calculates new random values in dependence of the previous added values
*   and adds them to the GRAPH_DATA objects
*/
static void _AddValues(void) {
  unsigned i;

  for (i = 0; i < GUI_COUNTOF(_aColor); i++) {
    int Add = ((unsigned)rand()) % (2 + i * i);
    int Vz  = (((unsigned)(rand()) % 2) << 1) - 1;
    _aValue[i] += Add * Vz;
    if (_aValue[i] > MAX_VALUE) {
      _aValue[i] = MAX_VALUE;
    } else if (_aValue[i] < 0) {
      _aValue[i] = 0;
    }
    GRAPH_DATA_YT_AddValue(_ahData[i], _aValue[i]);
  }
}

/*********************************************************************
*
*       _UserDraw
*
* Function description
*   This routine is called by the GRAPH object before anything is drawn
*   and after the last drawing operation.
*/
static void _UserDraw(WM_HWIN hWin, int Stage) {
  if (Stage == GRAPH_DRAW_LAST) {
    char acText[] = "Temperature";
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
  }
}

/*********************************************************************
*
*       _ForEach
*
* Function description
*   This routine hides/shows all windows except the button, graph and scroll bar widgets
*/
static void _ForEach(WM_HWIN hWin, void * pData) {
  int Id;
  int FullScreenMode;

  FullScreenMode = *(int *)pData;
  Id = WM_GetId(hWin);
  if ((Id == GUI_ID_GRAPH0) || (Id == GUI_ID_BUTTON0) || (Id == GUI_ID_VSCROLL) || (Id == GUI_ID_HSCROLL)) {
    return;
  }
  if (FullScreenMode) {
    WM_HideWindow(hWin);
  } else {
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
static void _ToggleFullScreenMode(WM_HWIN hDlg) {
  static int FullScreenMode;
  static GUI_RECT Rect;
  static unsigned ScalePos;
  WM_HWIN hGraph;
  WM_HWIN hButton;
  WM_HWIN hClient;
  GUI_RECT RectInside;
  int xPos, yPos;

  hGraph  = WM_GetDialogItem(hDlg, GUI_ID_GRAPH0);
  hButton = WM_GetDialogItem(hDlg, GUI_ID_BUTTON0);
  FullScreenMode ^= 1;
  if (FullScreenMode) {
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
  } else {
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

static void _cbCallback(WM_MESSAGE * pMsg) {
  unsigned i;
  int      NCode;
  int      Id;
  int      Value;
  WM_HWIN  hDlg;
  WM_HWIN  hItem;

  hDlg = pMsg->hWin;
  switch (pMsg->MsgId) {
  case WM_INIT_DIALOG:
    hItem = WM_GetDialogItem(hDlg, GUI_ID_GRAPH0);
    //
    // Add graphs
    //
    for (i = 0; i < GUI_COUNTOF(_aColor); i++) {
      _aValue[i] = rand() % 180;
      _ahData[i] = GRAPH_DATA_YT_Create(_aColor[i], 500, 0, 0);
      GRAPH_AttachData(hItem, _ahData[i]);
    }
    //
    // Set graph attributes
    //
    GRAPH_SetGridDistY(hItem, 25);
    GRAPH_SetGridVis(hItem, 1);
    GRAPH_SetGridFixedX(hItem, 1);
    GRAPH_SetUserDraw(hItem, _UserDraw);
    //
    // Create and add vertical scale
    //
    _hScaleV = GRAPH_SCALE_Create( 35, GUI_TA_RIGHT, GRAPH_SCALE_CF_VERTICAL, 25);
    GRAPH_SCALE_SetTextColor(_hScaleV, GUI_YELLOW);
    GRAPH_AttachScale(hItem, _hScaleV);
    //
    // Create and add horizontal scale
    //
    _hScaleH = GRAPH_SCALE_Create(155, GUI_TA_HCENTER, GRAPH_SCALE_CF_HORIZONTAL, 50);
    GRAPH_SCALE_SetTextColor(_hScaleH, GUI_DARKGREEN);
    GRAPH_AttachScale(hItem, _hScaleH);
    //
    // Init check boxes
    //
    hItem = WM_GetDialogItem(hDlg, GUI_ID_CHECK2);
    CHECKBOX_SetText(hItem, "L");
    hItem = WM_GetDialogItem(hDlg, GUI_ID_CHECK3);
    CHECKBOX_SetText(hItem, "T");
    hItem = WM_GetDialogItem(hDlg, GUI_ID_CHECK4);
    CHECKBOX_SetText(hItem, "R");
    hItem = WM_GetDialogItem(hDlg, GUI_ID_CHECK5);
    CHECKBOX_SetText(hItem, "B");
    hItem = WM_GetDialogItem(hDlg, GUI_ID_CHECK0);
    CHECKBOX_SetText(hItem, "Stop");
    hItem = WM_GetDialogItem(hDlg, GUI_ID_CHECK1);
    CHECKBOX_SetText(hItem, "Grid");
    CHECKBOX_SetState(hItem, 1);
    hItem = WM_GetDialogItem(hDlg, GUI_ID_CHECK6);
    CHECKBOX_SetText(hItem, "HScroll");
    CHECKBOX_SetState(hItem, 1);
    hItem = WM_GetDialogItem(hDlg, GUI_ID_CHECK7);
    CHECKBOX_SetText(hItem, "VScroll");
    hItem = WM_GetDialogItem(hDlg, GUI_ID_CHECK8);
    CHECKBOX_SetText(hItem, "MirrorX");
    //
    // Init slider widgets
    //
    hItem = WM_GetDialogItem(hDlg, GUI_ID_SLIDER0);
    SLIDER_SetRange(hItem, 0, 10);
    SLIDER_SetValue(hItem, 5);
    SLIDER_SetNumTicks(hItem, 6);
    hItem = WM_GetDialogItem(hDlg, GUI_ID_SLIDER1);
    SLIDER_SetRange(hItem, 0, 20);
    SLIDER_SetValue(hItem, 5);
    SLIDER_SetNumTicks(hItem, 6);
    //
    // Init radio widget
    //
    hItem = WM_GetDialogItem(hDlg, GUI_ID_RADIO0);
    RADIO_SetText(hItem, "3D", 0);
    RADIO_SetText(hItem, "flat", 1);
    RADIO_SetText(hItem, "-", 2);
    //
    // Init button widget
    //
    hItem = WM_GetDialogItem(hDlg, GUI_ID_BUTTON0);
    WM_SetStayOnTop(hItem, 1);
    break;
  case WM_NOTIFY_PARENT:
    Id    = WM_GetId(pMsg->hWinSrc);      // Id of widget
    NCode = pMsg->Data.v;                 // Notification code
    switch (NCode) {
    case WM_NOTIFICATION_CLICKED:
      switch (Id) {
      case GUI_ID_BUTTON0:
        _ToggleFullScreenMode(hDlg);
        break;
      }
      break;
    case WM_NOTIFICATION_VALUE_CHANGED:
      switch (Id) {
      case GUI_ID_CHECK0:
        //
        // Toggle stop mode
        //
        _Stop ^= 1;
        break;
      case GUI_ID_CHECK1:
        //
        // Toggle grid
        //
        hItem = WM_GetDialogItem(hDlg, GUI_ID_GRAPH0);
        GRAPH_SetGridVis(hItem, CHECKBOX_IsChecked(WM_GetDialogItem(hDlg, GUI_ID_CHECK1)));
        break;
      case GUI_ID_CHECK2:
      case GUI_ID_CHECK3:
      case GUI_ID_CHECK4:
      case GUI_ID_CHECK5:
        //
        // Toggle border
        //
        hItem = WM_GetDialogItem(hDlg, GUI_ID_GRAPH0);
        GRAPH_SetBorder(hItem,
                        CHECKBOX_IsChecked(WM_GetDialogItem(hDlg, GUI_ID_CHECK2)) * 40,
                        CHECKBOX_IsChecked(WM_GetDialogItem(hDlg, GUI_ID_CHECK3)) * 5,
                        CHECKBOX_IsChecked(WM_GetDialogItem(hDlg, GUI_ID_CHECK4)) * 5,
                        CHECKBOX_IsChecked(WM_GetDialogItem(hDlg, GUI_ID_CHECK5)) * 5);
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
        switch (RADIO_GetValue(pMsg->hWinSrc)) {
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
        if (CHECKBOX_IsChecked(WM_GetDialogItem(hDlg, GUI_ID_CHECK6))) {
          GRAPH_SetVSizeX(hItem, 500);
        } else {
          GRAPH_SetVSizeX(hItem, 0);
        }
        break;
      case GUI_ID_CHECK7:
        //
        // Toggle vertical scroll bar
        //
        hItem = WM_GetDialogItem(hDlg, GUI_ID_GRAPH0);
        if (CHECKBOX_IsChecked(WM_GetDialogItem(hDlg, GUI_ID_CHECK7))) {
          GRAPH_SetVSizeY(hItem, 300);
        } else {
          GRAPH_SetVSizeY(hItem, 0);
        }
        break;
      case GUI_ID_CHECK8:
        //
        // Toggle alignment
        //
        WM_GetDialogItem(hDlg, GUI_ID_GRAPH0);
        for (i = 0; i < GUI_COUNTOF(_aColor); i++) {
          if (CHECKBOX_IsChecked(WM_GetDialogItem(hDlg, GUI_ID_CHECK8))) {
            GRAPH_DATA_YT_SetAlign(_ahData[i], GRAPH_ALIGN_LEFT);
            GRAPH_DATA_YT_MirrorX (_ahData[i], 1);
          } else {
            GRAPH_DATA_YT_SetAlign(_ahData[i], GRAPH_ALIGN_RIGHT);
            GRAPH_DATA_YT_MirrorX (_ahData[i], 0);
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


/**
  * @brief  Main task
  * @parampvParameters not used
  * @retval None
  */
static void Main_Task(void * pvParameters)
{
	uint32_t ticks = 0;



//ILI9327_Init();
	ILI9327_Fill(VGA_BLACK+1);
	GUI_DispString("Hello world!");

	ILI9327_Fill(VGA_GREEN);
	vTaskDelay(100);
	ILI9327_Fill(VGA_RED);
	vTaskDelay(100);
	ILI9327_Fill(VGA_BLUE);
	vTaskDelay(100);

	//GUIDEMO_Main();

	WM_HWIN hDlg;
	WM_HWIN hGraph;

	hGraph = 0;
	hDlg = GUI_CreateDialogBox(_aDialogCreate, GUI_COUNTOF(_aDialogCreate), _cbCallback, 0, 0, 0);
#if 0
	// Ustawienie domyślnego stylu widżetów
	BUTTON_SetDefaultSkin(BUTTON_SKIN_FLEX);
	FRAMEWIN_SetDefaultSkin(FRAMEWIN_SKIN_FLEX);

	// Ustawienie koloru tła pulpitu
	WM_SetDesktopColor(GUI_BLACK);

#if GUI_SUPPORT_MEMDEV
	WM_SetCreateFlags(WM_CF_MEMDEV);
#endif

	GUI_SetFont(&GUI_Font8x16);

// Utworzenie okienka - widżet FRAMEWIN
	hFrame1 = FRAMEWIN_CreateEx(35, 50, 170, 150, 0, WM_CF_SHOW, 0, 10, "MyFirstApp", &cbFRAMEWIN_1);
	FRAMEWIN_SetFont        (hFrame1, &GUI_Font13HB_1);



	FRAMEWIN_AddCloseButton (hFrame1, FRAMEWIN_BUTTON_RIGHT, 0);
	FRAMEWIN_AddMaxButton   (hFrame1, FRAMEWIN_BUTTON_RIGHT, 0);
	FRAMEWIN_AddMinButton   (hFrame1, FRAMEWIN_BUTTON_RIGHT, 0);
	FRAMEWIN_SetTextAlign   (hFrame1, GUI_TA_HCENTER);


	// Utworzenietekstów - widżetTEXT
	hText_1 = TEXT_CreateEx(5, 25, 160, 20, hFrame1, WM_CF_SHOW, TEXT_CF_HCENTER, 0, "Hello, World!");
	hText_2 = TEXT_CreateEx(5, 50, 160, 20, hFrame1, WM_CF_SHOW, TEXT_CF_LEFT, 1, "To be...");
	hText_3 = TEXT_CreateEx(5, 75, 160, 20, hFrame1, WM_CF_SHOW, TEXT_CF_RIGHT  , 2, "... or not to be");

	// Ustawienie czcionki tekstów
	TEXT_SetFont(hText_1, &GUI_Font13_1);
	TEXT_SetFont(hText_2, &GUI_Font13_1);
	TEXT_SetFont(hText_3, &GUI_Font13_1);

	// Utworzenieprzycisków - widżet BUTTON
	hButton_1 = BUTTON_CreateEx(  5, 100, 50, 20, hFrame1, WM_CF_SHOW, 0, 0);
	hButton_2 = BUTTON_CreateEx( 60, 100, 50, 20, hFrame1, WM_CF_SHOW, 0, 1);
	hButton_3 = BUTTON_CreateEx(115, 100, 50, 20, hFrame1, WM_CF_SHOW, 0, 2);
	hButton_4 = BUTTON_CreateEx(  5, 125, 77, 20, hFrame1, WM_CF_SHOW, 0, 3);
	hButton_5 = BUTTON_CreateEx( 88, 125, 77, 20, hFrame1, WM_CF_SHOW, 0, 4);

	// Dodanie etykiet do przycisków
	BUTTON_SetText(hButton_1, "red");
	BUTTON_SetText(hButton_2, "green");
	BUTTON_SetText(hButton_3, "blue");
	BUTTON_SetText(hButton_4, "BIG FONT");
	BUTTON_SetText(hButton_5, "small font");

	// Przypisanie funkcji zwrotnych do przycisków
	WM_SetCallback(hButton_1, cbBUTTON_1);
	WM_SetCallback(hButton_2, cbBUTTON_2);
	WM_SetCallback(hButton_3, cbBUTTON_3);
	WM_SetCallback(hButton_4, cbBUTTON_4);
	WM_SetCallback(hButton_5, cbBUTTON_5);

	/*hFrame2 = FRAMEWIN_CreateEx(50, 210, 170, 100, 0, WM_CF_SHOW, 0, 10, "WTF", &cbFRAMEWIN_1);
	FRAMEWIN_SetFont        (hFrame2, &GUI_Font13HB_1);

	FRAMEWIN_AddCloseButton (hFrame2, FRAMEWIN_BUTTON_RIGHT, 0);
	FRAMEWIN_AddMaxButton   (hFrame2, FRAMEWIN_BUTTON_RIGHT, 0);
	FRAMEWIN_SetTextAlign   (hFrame2, GUI_TA_HCENTER);

	FRAMEWIN_SetActive(hFrame1, FRAMEWIN_CI_ACTIVE);
*/

#endif



	while (1)
	{
		if(ticks++ > 10)
		{
			ticks = 0;
			  /* Inwersja stanu LED3 każde 100ms */
			//STM_EVAL_LEDToggle(LED3);
			static int r=0;
			if(r)
			{
				//BUTTON_SetState(hButton_1, BUTTON_STATE_PRESSED);
				//FRAMEWIN_Minimize(hFrame1);
				//FRAMEWIN_SetResizeable(hFrame1, FRAMEWIN_SF_MAXIMIZED);
				r=0;
			}else {
				//BUTTON_SetState(hButton_1, BUTTON_STATE_FOCUS);
				//FRAMEWIN_Maximize(hFrame1);
				//FRAMEWIN_SetResizeable(hFrame1, FRAMEWIN_SF_MINIMIZED);
				r=1;
			}

		}
		if (!hGraph) {
			hGraph = WM_GetDialogItem(hDlg, GUI_ID_GRAPH0);
		  }
		  _AddValues();
		  GUI_Exec();
		//ILI9327_Fill(VGA_RED+ticks*20);
		GUI_Delay(10);


		//GUI_SetBkColor(GUI_BLUE);
		//GUI_Clear();
		//GUI_SetPenSize(10);
		//GUI_SetColor(GUI_RED);
		/*GUI_DrawLine(80, 10, 240, 90);
		GUI_DrawLine(80, 90, 240, 10);
		GUI_SetBkColor(GUI_BLACK);
		GUI_SetColor(GUI_WHITE);
		GUI_SetTextMode(GUI_TM_NORMAL);
		GUI_DispStringHCenterAt("GUI_TM_NORMAL" , 160, 10);
		*/
		/*
		vTaskDelay(200);*/
	}
}

/**
  * @brief  Callback function
  * @param  None
  * @retval None
  */
void cbFRAMEWIN_1(WM_MESSAGE * pMsg){
  switch(pMsg->MsgId){
    default:
      // The original callback
      FRAMEWIN_Callback(pMsg);
      break;
  }
}
#if 1


/**
  * @brief  Callback function
  * @param  None
  * @retval None
  */
void cbBUTTON_1(WM_MESSAGE * pMsg){
  int NCode;

  switch(pMsg->MsgId){
    case WM_NOTIFY_PARENT_REFLECTION:{
      NCode = pMsg->Data.v;
      switch(NCode){
        case WM_NOTIFICATION_RELEASED:{
          TEXT_SetTextColor(hText_1, GUI_RED);
          break;
        }
      }
      break;
    }
    default:{
      BUTTON_Callback(pMsg);
      break;
    }
  }
}

/**
  * @brief  Callback function
  * @param  None
  * @retval None
  */
void cbBUTTON_2(WM_MESSAGE * pMsg){
  int NCode;

  switch(pMsg->MsgId){
    case WM_NOTIFY_PARENT_REFLECTION:{
      NCode = pMsg->Data.v;
      switch(NCode){
        case WM_NOTIFICATION_RELEASED:{
          TEXT_SetTextColor(hText_1, GUI_GREEN);
          break;
        }
      }
      break;
    }
    default:{
      BUTTON_Callback(pMsg);
      break;
    }
  }
}

/**
  * @brief  Callback function
  * @param  None
  * @retval None
  */
void cbBUTTON_3(WM_MESSAGE * pMsg){
  int NCode;

  switch(pMsg->MsgId){
    case WM_NOTIFY_PARENT_REFLECTION:{
      NCode = pMsg->Data.v;
      switch(NCode){
        case WM_NOTIFICATION_RELEASED:{
          TEXT_SetTextColor(hText_1, GUI_BLUE);
          break;
        }
      }
      break;
    }
    default:{
      BUTTON_Callback(pMsg);
      break;
    }
  }
}

/**
  * @brief  Callback function
  * @param  None
  * @retval None
  */
void cbBUTTON_4(WM_MESSAGE * pMsg){
  int NCode;

  switch(pMsg->MsgId){
    case WM_NOTIFY_PARENT_REFLECTION:{
      NCode = pMsg->Data.v;
      switch(NCode){
        case WM_NOTIFICATION_RELEASED:{
          TEXT_SetFont(hText_1, &GUI_Font20_1);
          TEXT_SetFont(hText_2, &GUI_Font20_1);
          TEXT_SetFont(hText_3, &GUI_Font20_1);
          break;
        }
      }
      break;
    }
    default:{
      BUTTON_Callback(pMsg);
      break;
    }
  }
}

/**
  * @brief  Callback function
  * @param  None
  * @retval None
  */
void cbBUTTON_5(WM_MESSAGE * pMsg){
  int NCode;

  switch(pMsg->MsgId){
    case WM_NOTIFY_PARENT_REFLECTION:{
      NCode = pMsg->Data.v;
      switch(NCode){
        case WM_NOTIFICATION_RELEASED:{
          TEXT_SetFont(hText_1, &GUI_Font13_1);
          TEXT_SetFont(hText_2, &GUI_Font13_1);
          TEXT_SetFont(hText_3, &GUI_Font13_1);
          break;
        }
      }
      break;
    }
    default:{
      BUTTON_Callback(pMsg);
      break;
    }
  }
}
#endif





















static void vTimerCallback( xTimerHandle pxTimer )
{
   //BSP_Pointer_Update();

	//>>Touch Callback

	static GUI_PID_STATE TS_State;
	uint16_t x, y;

	if(!GPIO_ReadInputDataBit(XPT2046_IRQ_PORT, XPT2046_IRQ_PAD))
	{
		XPT2046_GetCursor ( &x, &y );

		TS_State.x = x;
		TS_State.y = y;

		TS_State.Pressed = 1;
		TS_State.Layer = 0;

		if ((x < LCD_GetXSize()) && (y < LCD_GetYSize()))
			GUI_TOUCH_StoreStateEx(&TS_State);
	}
	else
	{
		TS_State.Pressed = 0;
		GUI_TOUCH_StoreStateEx(&TS_State);
	}
	//<< Touch Callback
}


#define Main_Task_PRIO          ( tskIDLE_PRIORITY  + 9 )
#define Main_Task_STACK         ( 3048 )

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
	GUI_Init();
	//printf("#...Gui finished initialization!\n\r");

	//Touch_Init();


//tFlashLine = xTimerCreate("FlashLine", (100/portTICK_RATE_MS), pdTRUE, 0, Flashing);
//xTimerReset(tFlashLine, 0);

//xLEDMutex = xSemaphoreCreateMutex();

	 __IO uint32_t i;
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
	TouchScreenTimer = xTimerCreate ("Timer", 50, pdTRUE, ( void * ) 1, vTimerCallback );

	if( TouchScreenTimer != NULL )
	{
		if( xTimerStart( TouchScreenTimer, 0 ) != pdPASS )
		{
		  /* The timer could not be set into the Active state. */
		}
	}




//xTaskCreate(vLedTask, "LedTask", configMINIMAL_STACK_SIZE, NULL, tskIDLE_PRIORITY + 1, NULL);
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
