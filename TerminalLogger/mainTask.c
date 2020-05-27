/* Includes ------------------------------------------------------------------*/
#include "stm32f4xx.h"
#include "stm32f4xx_conf.h"
#include <string.h>
#include "FreeRTOS.h"
#include "task.h"

#include "GUI.h"
#include "DIALOG.h"

#include "ILI9327.h"
#include <stdbool.h>

#include "terminalApp.h"
#include "GraphApp.h"


/* Private typedef -----------------------------------------------------------*/
/* Private define ------------------------------------------------------------*/
// Recommended memory to run the sample with adequate performance
#define RECOMMENDED_MEMORY (1024L * 1700)

#define COLOR_BACK0            0xFF3333
#define COLOR_BACK1            0x550000


/* Private macro -------------------------------------------------------------*/
/* Private variables ---------------------------------------------------------*/
uint16_t PValue = 10000;
WM_HWIN hFrame;


/* Extern variables ----------------------------------------------------------*/
extern xTaskHandle 	MainTask_Handle;
extern xTaskHandle 	TerminalApp_Handle;
extern xTaskHandle 	GraphApp_Handle;


/* Private function prototypes -----------------------------------------------*/
/* Private functions ---------------------------------------------------------*/
/*******************************************************************************
* Function Name  : _cbBk
* Description    : Drawing of background window
*******************************************************************************/
static void _cbBk(WM_MESSAGE * pMsg)
{
	switch (pMsg->MsgId)
	{
		case WM_PAINT:
			GUI_DrawGradientV(0, 0, 239, 399, COLOR_BACK0, COLOR_BACK1);

			//GUI_SetFont(&GUI_Font20_AA4);
			GUI_SetFont(&GUI_Font8_1/*GUI_Font20_AA4*/);
			GUI_SetTextMode(GUI_TM_TRANS);
			GUI_SetColor(GUI_WHITE);
			GUI_DispStringHCenterAt("Window effects are\n the easy to use\n"
								  "solution for\n animating windows...", 160, 140);
		break;
	}
}


/*******************************************************************************
* Function Name  : _cbComputerWnd
* Description    : Callback function for handling window events
*******************************************************************************/
void _cbComputerWnd(WM_MESSAGE * pMsg)
{
	//WM_HWIN    hWin;
	//hWin = pMsg->hWin;

	switch (pMsg->MsgId)
	{
		//events from children of the terminal window
		case WM_NOTIFY_PARENT:
			switch(pMsg->Data.v)
			{
				case WM_NOTIFICATION_CLICKED:
					switch(WM_GetId(pMsg->hWinSrc))
					{
						case GUI_ID_BUTTON9:
							xTaskCreate(TerminalApp_Task, "TerApp", configMINIMAL_STACK_SIZE, NULL, tskIDLE_PRIORITY + 1, &TerminalApp_Handle);
							vTaskSuspend(MainTask_Handle);
						break;

						case GUI_ID_BUTTON8:
							xTaskCreate(GraphApp_Task, "GrphApp", configMINIMAL_STACK_SIZE, NULL, tskIDLE_PRIORITY + 1, &GraphApp_Handle);
							vTaskSuspend(MainTask_Handle);
						break;
					}
				break;
			}
		break;
	}
}


/*******************************************************************************
* Function Name  : Main_Task
* Description    : Start window
*******************************************************************************/
void Main_Task(void * pvParameters)
{
	ILI9327_Fill(VGA_BLUE+1);
	GUI_DispString("Hello world!");
	GUI_DispString("Hello world!FUCK YOU");

	//WM_HWIN hAnim;

	WM_SetCallback(WM_HBKWIN, _cbBk);

	WM_HWIN hBtnApp1, hBtnApp2, hBtnApp3;

	// Create frame window
	hFrame = FRAMEWIN_CreateEx(0, 0, 240, 320, WM_HBKWIN, WM_CF_SHOW, 0, 0, "MicroMbed", _cbComputerWnd);
	FRAMEWIN_SetTextColor(hFrame, GUI_BLACK);
	FRAMEWIN_SetFont(hFrame, &GUI_Font13B_ASCII/*GUI_Font20_AA4*/);
	FRAMEWIN_SetClientColor(hFrame, GUI_WHITE);
	FRAMEWIN_AddCloseButton(hFrame, FRAMEWIN_BUTTON_RIGHT, 0);

	hBtnApp1 = BUTTON_CreateEx(10,10,60,20, WM_GetClientWindow(hFrame),WM_CF_SHOW, 0, GUI_ID_BUTTON9 );
	BUTTON_SetText(hBtnApp1, "Terminal");
	hBtnApp2 = BUTTON_CreateEx(10,40,60,20, WM_GetClientWindow(hFrame),WM_CF_SHOW, 0, GUI_ID_BUTTON8 );
	BUTTON_SetText(hBtnApp2, "Graph");
	hBtnApp3 = BUTTON_CreateEx(10,70,60,20, WM_GetClientWindow(hFrame),WM_CF_SHOW, 0, GUI_ID_BUTTON7 );
	BUTTON_SetText(hBtnApp3, "Bluetooth");

	//FRAMEWIN_SetActive(hFrame, 1);
	//FRAMEWIN_Maximize(hFrame);

	while(1)
	{

		GUI_Delay(10);
	}

	vTaskSuspend(NULL/*MainTask_Handle*/); //NULL - the task itself
}

#if 0
	BUTTON_SetDefaultSkin(BUTTON_SKIN_FLEX);
	FRAMEWIN_SetDefaultSkin(FRAMEWIN_SKIN_FLEX);

	WM_SetDesktopColor(GUI_BLACK);

#if GUI_SUPPORT_MEMDEV
	WM_SetCreateFlags(WM_CF_MEMDEV);
#endif

	FRAMEWIN_AddCloseButton (hFrame1, FRAMEWIN_BUTTON_RIGHT, 0);
	FRAMEWIN_AddMaxButton   (hFrame1, FRAMEWIN_BUTTON_RIGHT, 0);
	FRAMEWIN_AddMinButton   (hFrame1, FRAMEWIN_BUTTON_RIGHT, 0);
	FRAMEWIN_SetTextAlign   (hFrame1, GUI_TA_HCENTER);

	hText_1 = TEXT_CreateEx(5, 25, 160, 20, hFrame1, WM_CF_SHOW, TEXT_CF_HCENTER, 0, "Hello, World!");

	hButton_1 = BUTTON_CreateEx(  5, 100, 50, 20, hFrame1, WM_CF_SHOW, 0, 0);

	BUTTON_SetText(hButton_1, "red");

	WM_SetCallback(hButton_1, cbBUTTON_1);

#endif
