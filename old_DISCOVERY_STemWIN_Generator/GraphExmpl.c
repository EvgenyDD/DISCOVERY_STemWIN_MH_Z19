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

/* Private typedef -----------------------------------------------------------*/
/* Private define ------------------------------------------------------------*/
// Recommended memory to run the sample with adequate performance
#define RECOMMENDED_MEMORY (1024L * 30)

/* Private macro -------------------------------------------------------------*/
/* Private variables ---------------------------------------------------------*/
uint32_t freqSet;
uint32_t freqReal;
bool genEnable = true;

GRAPH_DATA_Handle _ahData[3]; // Array of handles for the GRAPH_DATA objects
GRAPH_SCALE_Handle _hScaleV; // Handle of vertical scale
GRAPH_SCALE_Handle _hScaleH; // Handle of horizontal scale

GUI_COLOR _aColor[] = { GUI_RED, GUI_GREEN, GUI_LIGHTBLUE }; // Array of colors for the GRAPH_DATA objects

int _aValue[3];
int _Stop = 0;

// Dialog resource
const GUI_WIDGET_CREATE_INFO _aDialogCreate[] = { { FRAMEWIN_CreateIndirect,
		"Frequency Generator", 0, 0, 0, 400, 240, FRAMEWIN_CF_MOVEABLE },

{ BUTTON_CreateIndirect, "10M", GUI_BTN0, 0, 1, 35, 35 }, {
		BUTTON_CreateIndirect, "1M", GUI_BTN1, 40, 1, 35, 35 }, {
		BUTTON_CreateIndirect, "100k", GUI_BTN2, 80, 1, 35, 35 }, {
		BUTTON_CreateIndirect, "10k", GUI_BTN3, 120, 1, 35, 35 }, {
		BUTTON_CreateIndirect, "1k", GUI_BTN4, 160, 1, 35, 35 }, {
		BUTTON_CreateIndirect, "100", GUI_BTN5, 200, 1, 35, 35 }, {
		BUTTON_CreateIndirect, "10", GUI_BTN6, 240, 1, 35, 35 }, {
		BUTTON_CreateIndirect, "1", GUI_BTN7, 280, 1, 35, 35 }, {
		BUTTON_CreateIndirect, "=0", GUI_BTN8, 400 - 40, 1, 35, 35 },

{ EDIT_CreateIndirect, 0, GUI_ID_EDIT0, 0, 37, 200, 25 }, { EDIT_CreateIndirect,
		0, GUI_ID_EDIT1, 200, 37, 200, 25 },

{ BUTTON_CreateIndirect, "10M", GUI_BTN9, 0, 65, 35, 35 }, {
		BUTTON_CreateIndirect, "1M", GUI_BTN10, 40, 65, 35, 35 }, {
		BUTTON_CreateIndirect, "100k", GUI_BTN11, 80, 65, 35, 35 }, {
		BUTTON_CreateIndirect, "10k", GUI_BTN12, 120, 65, 35, 35 }, {
		BUTTON_CreateIndirect, "1k", GUI_BTN13, 160, 65, 35, 35 }, {
		BUTTON_CreateIndirect, "100", GUI_BTN14, 200, 65, 35, 35 }, {
		BUTTON_CreateIndirect, "10", GUI_BTN15, 240, 65, 35, 35 }, {
		BUTTON_CreateIndirect, "1", GUI_BTN16, 280, 65, 35, 35 },

{ BUTTON_CreateIndirect, "On", GUI_BTN17, 400 - 40, 65, 35, 35 }, };

/* Extern variables ----------------------------------------------------------*/
/* Private function prototypes -----------------------------------------------*/
/* Private functions ---------------------------------------------------------*/
/*******************************************************************************
 * Function Name  : _UserDraw
 * Description    : Is called by the GRAPH object before anything is drawn
 *   			   and after the last drawing operation
 *******************************************************************************/
void _UserDraw(WM_HWIN hWin, int Stage) {

}

/*******************************************************************************
 * Function Name  : _ForEach
 * Description    : Hides/shows all windows except the button, graph and scroll bar widgets
 *******************************************************************************/
void _ForEach(WM_HWIN hWin, void * pData) {
	int FullScreenMode = *(int*) pData;
	int WidgetID = WM_GetId(hWin);

	if (WidgetID == GUI_ID_GRAPH0 || WidgetID == GUI_ID_BUTTON0
			|| WidgetID == GUI_ID_VSCROLL || WidgetID == GUI_ID_HSCROLL)
		return;

	if (FullScreenMode)
		WM_HideWindow(hWin);
	else
		WM_ShowWindow(hWin);
}

/*******************************************************************************
 * Function Name  : _ToggleFullScreenMode
 * Description    : Switches between full screen mode and normal mode by hiding or showing the
 *   			   widgets of the dialog, enlarging/shrinking the graph widget and modifying some other
 *   			   attributes of the dialog widgets
 *******************************************************************************/
void _ToggleFullScreenMode(WM_HWIN hDlg) {
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
	if (FullScreenMode) {
		// Enter the full screen mode
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
		// Return to normal mode
		BUTTON_SetText(hButton, "Full Screen");

		WM_MoveWindow(hButton, 0, -11);
		WM_ForEachDesc(WM_GetClientWindow(hDlg), _ForEach, &FullScreenMode); // Show all descendants

		WM_SetWindowPos(hGraph, Rect.x0, Rect.y0, Rect.x1 - Rect.x0 + 1,
				Rect.y1 - Rect.y0 + 1);

		FRAMEWIN_SetTitleVis(hDlg, 1);
		GRAPH_SCALE_SetPos(_hScaleH, ScalePos);
	}
}

void ChangeVal(uint16_t widget) {
	switch (widget) {
	case GUI_BTN0:
		freqSet += 10000000;
		break;
	case GUI_BTN1:
		freqSet += 1000000;
		break;
	case GUI_BTN2:
		freqSet += 100000;
		break;
	case GUI_BTN3:
		freqSet += 10000;
		break;
	case GUI_BTN4:
		freqSet += 1000;
		break;
	case GUI_BTN5:
		freqSet += 100;
		break;
	case GUI_BTN6:
		freqSet += 10;
		break;
	case GUI_BTN7:
		freqSet += 1;
		break;
	case GUI_BTN8:
		freqSet = 0;
		break;
	case GUI_BTN9:
		freqSet -= 10000000;
		break;
	case GUI_BTN10:
		freqSet -= 1000000;
		break;
	case GUI_BTN11:
		freqSet -= 100000;
		break;
	case GUI_BTN12:
		freqSet -= 10000;
		break;
	case GUI_BTN13:
		freqSet -= 1000;
		break;
	case GUI_BTN14:
		freqSet -= 100;
		break;
	case GUI_BTN15:
		freqSet -= 10;
		break;
	case GUI_BTN16:
		freqSet -= 1;
		break;
	case GUI_BTN17:
		genEnable = !genEnable;
		break;
	}
}

#define MakeDIsplayValue(int)

#include "stm32f4xx.h"
#include "stm32f4xx_gpio.h"
#include "stm32f4xx_rcc.h"
#include "stm32f4xx_tim.h"

static void setFreq(uint32_t freq) {
	RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOB, ENABLE);
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_TIM1, ENABLE);

	GPIO_InitTypeDef init;
	init.GPIO_Mode = GPIO_Mode_AF;
	init.GPIO_OType = GPIO_OType_PP;
	init.GPIO_Pin = GPIO_Pin_1;
	init.GPIO_Speed = GPIO_Speed_100MHz;
	GPIO_Init(GPIOB, &init);

	GPIO_PinAFConfig(GPIOB, GPIO_PinSource1, GPIO_AF_TIM1);

	TIM_TimeBaseInitTypeDef base_timer;
	TIM_TimeBaseStructInit(&base_timer);

	const uint32_t baseFreq = 160000000;

	uint32_t psc, arr;
	uint32_t mlt = baseFreq / freqSet;
	arr = sqrtf(mlt);
	arr += arr % 2;
	psc = mlt / arr;

	if (freq == 0)
		return;

	base_timer.TIM_Prescaler = psc - 1; // äåëèòåëü ÷àñòîòû
	base_timer.TIM_Period = arr - 1; // ïåðèîä
	base_timer.TIM_CounterMode = TIM_CounterMode_Up; // ñ÷¸ò ââåðõ
	TIM_TimeBaseInit(TIM1, &base_timer);

	TIM_BDTRInitTypeDef TIM_BDTRInitStructure;
	TIM_BDTRInitStructure.TIM_OSSRState = TIM_OSSRState_Disable;
	TIM_BDTRInitStructure.TIM_OSSIState = TIM_OSSIState_Disable;
	TIM_BDTRInitStructure.TIM_LOCKLevel = TIM_LOCKLevel_OFF;
	TIM_BDTRInitStructure.TIM_DeadTime = 0;
	TIM_BDTRInitStructure.TIM_Break = TIM_Break_Disable;
	TIM_BDTRInitStructure.TIM_AutomaticOutput = TIM_AutomaticOutput_Disable;
	TIM_BDTRConfig(TIM1, &TIM_BDTRInitStructure);

	TIM_OCInitTypeDef oc_init;
	TIM_OCStructInit(&oc_init);
	oc_init.TIM_OCMode = TIM_OCMode_PWM1; // ðàáîòàåì â ðåæèìå ØÈÌ ( PWM )
	oc_init.TIM_OutputState = TIM_OutputState_Enable;
	oc_init.TIM_OutputNState = TIM_OutputNState_Enable;
	oc_init.TIM_Pulse = TIM1->ARR / 2;
	TIM_OC3Init(TIM1, &oc_init);

	TIM_OC3PreloadConfig(TIM1, TIM_OCPreload_Enable);
	TIM_ARRPreloadConfig(TIM1, ENABLE);
	TIM_CtrlPWMOutputs(TIM1, ENABLE);

	freqReal = baseFreq / ((TIM1->ARR + 1) * (TIM1->PSC + 1));
}

/*******************************************************************************
 * Function Name  : GraphCallback
 *******************************************************************************/
void GraphCallback(WM_MESSAGE * pMsg) {
	int NCode;
	uint16_t WidgetID;
	int Value;
	WM_HWIN hDlg;
	WM_HWIN hItem;

	hDlg = pMsg->hWin;

	//WM_HWIN hEdit = ;

	char c[50] = "";
	char d[50];

	EDIT_SetMaxLen(WM_GetDialogItem(pMsg->hWin, GUI_ID_EDIT0), 40);
	EDIT_SetMaxLen(WM_GetDialogItem(pMsg->hWin, GUI_ID_EDIT1), 40);

	hItem = WM_GetDialogItem(hDlg, GUI_BTN17);
	BUTTON_SetText(hItem, genEnable ? "On" : "Off");

	BUTTON_SetBkColor(hItem, BUTTON_CI_UNPRESSED, genEnable ? GUI_GREEN : GUI_RED);

	TIM_Cmd(TIM1, genEnable ? ENABLE : DISABLE); // çàïóñêàåì ñ÷¸ò

	setFreq(freqSet);

	if (freqSet < 1000) {
		ftoa_((float) freqSet, d, 1);
		c[0] = '\0';
		strcat_(c, d);
		strcat_(c, " Hz");
		EDIT_SetText(WM_GetDialogItem(pMsg->hWin, GUI_ID_EDIT0), c);

		ftoa_((float) freqReal, d, 1);
		c[0] = '\0';
		strcat_(c, d);
		strcat_(c, " Hz");
		EDIT_SetText(WM_GetDialogItem(pMsg->hWin, GUI_ID_EDIT1), c);
	} else if (freqSet < 1000000) {
		ftoa_((float) freqSet * 0.001, d, 3);
		c[0] = '\0';
		strcat_(c, d);
		strcat_(c, " kHz");
		EDIT_SetText(WM_GetDialogItem(pMsg->hWin, GUI_ID_EDIT0), c);

		ftoa_((float) freqReal * 0.001, d, 3);
		c[0] = '\0';
		strcat_(c, d);
		strcat_(c, " kHz");
		EDIT_SetText(WM_GetDialogItem(pMsg->hWin, GUI_ID_EDIT1), c);
	} else {
		ftoa_((float) freqSet * 0.000001, d, 6);
		c[0] = '\0';
		strcat_(c, d);
		strcat_(c, " MHz");
		EDIT_SetText(WM_GetDialogItem(pMsg->hWin, GUI_ID_EDIT0), c);

		ftoa_((float) freqReal * 0.000001, d, 6);
		c[0] = '\0';
		strcat_(c, d);
		strcat_(c, " MHz");
		EDIT_SetText(WM_GetDialogItem(pMsg->hWin, GUI_ID_EDIT1), c);
	}

	switch (pMsg->MsgId) {
	case WM_INIT_DIALOG:
		hItem = WM_GetDialogItem(hDlg, GUI_ID_GRAPH0);

		// Add graphs
		for (uint16_t i = 0; i < GUI_COUNTOF(_aColor); i++) {
			_aValue[i] = rand() % 180;
			_ahData[i] = GRAPH_DATA_YT_Create(_aColor[i], 500, 0, 0);
			GRAPH_AttachData(hItem, _ahData[i]);
		}

		// Set graph attributes
		GRAPH_SetGridDistY(hItem, 10);
		GRAPH_SetGridVis(hItem, 1);
		GRAPH_SetGridFixedX(hItem, 1);
		GRAPH_SetUserDraw(hItem, _UserDraw);

		// Create and add vertical scale
		_hScaleV = GRAPH_SCALE_Create(35, GUI_TA_RIGHT, GRAPH_SCALE_CF_VERTICAL,
				10);
		GRAPH_SCALE_SetTextColor(_hScaleV, GUI_YELLOW);
		GRAPH_AttachScale(hItem, _hScaleV);

		GRAPH_SCALE_SetFactor(_hScaleV, 40.0);

		// Create and add horizontal scale
		_hScaleH = GRAPH_SCALE_Create(155, GUI_TA_HCENTER,
				GRAPH_SCALE_CF_HORIZONTAL, 10);
		GRAPH_SCALE_SetTextColor(_hScaleH, GUI_DARKGREEN);
		GRAPH_AttachScale(hItem, _hScaleH);

		// Init check boxes
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

		// Init slider widgets
		hItem = WM_GetDialogItem(hDlg, GUI_ID_SLIDER0);
		SLIDER_SetRange(hItem, 0, 10);
		SLIDER_SetValue(hItem, 5);
		SLIDER_SetNumTicks(hItem, 6);
		hItem = WM_GetDialogItem(hDlg, GUI_ID_SLIDER1);
		SLIDER_SetRange(hItem, 0, 20);
		SLIDER_SetValue(hItem, 5);
		SLIDER_SetNumTicks(hItem, 6);

		// Init radio widget
		hItem = WM_GetDialogItem(hDlg, GUI_ID_RADIO0);
		RADIO_SetText(hItem, "3D", 0);
		RADIO_SetText(hItem, "flat", 1);
		RADIO_SetText(hItem, "-", 2);

		// Init button widget
		hItem = WM_GetDialogItem(hDlg, GUI_ID_BUTTON0);
		WM_SetStayOnTop(hItem, 1);
		break; //end of pMsg->MsgId == WM_INIT_DIALOG

	case WM_NOTIFY_PARENT:
		WidgetID = WM_GetId(pMsg->hWinSrc); // WidgetID of widget
		NCode = pMsg->Data.v; // Notification code

		switch (NCode) {
		case WM_NOTIFICATION_CLICKED: //BUTTONS
			switch (WidgetID) {
			case GUI_ID_BUTTON0:
				_ToggleFullScreenMode(hDlg);
				break;

			default:
				ChangeVal(WidgetID);
				break;
			}
			break; //end of NCode == WM_NOTIFICATION_CLICKED

		case WM_NOTIFICATION_VALUE_CHANGED:
			switch (WidgetID) {
			case GUI_ID_CHECK0:
				// Toggle stop mode
				_Stop ^= 1;
				break; //end of WidgetID == GUI_ID_CHECK0

			case GUI_ID_CHECK1:
				// Toggle grid
				hItem = WM_GetDialogItem(hDlg, GUI_ID_GRAPH0);
				GRAPH_SetGridVis(hItem,
						CHECKBOX_IsChecked(
								WM_GetDialogItem(hDlg, GUI_ID_CHECK1)));
				break; //end of WidgetID == GUI_ID_CHECK1

			case GUI_ID_CHECK2:
			case GUI_ID_CHECK3:
			case GUI_ID_CHECK4:
			case GUI_ID_CHECK5:
				// Toggle border
				hItem = WM_GetDialogItem(hDlg, GUI_ID_GRAPH0);
				GRAPH_SetBorder(hItem,
						CHECKBOX_IsChecked(
								WM_GetDialogItem(hDlg, GUI_ID_CHECK2)) * 40,
						CHECKBOX_IsChecked(
								WM_GetDialogItem(hDlg, GUI_ID_CHECK3)) * 5,
						CHECKBOX_IsChecked(
								WM_GetDialogItem(hDlg, GUI_ID_CHECK4)) * 5,
						CHECKBOX_IsChecked(
								WM_GetDialogItem(hDlg, GUI_ID_CHECK5)) * 5);
				break; //end of WidgetID == GUI_ID_CHECK[2:5]

			case GUI_ID_SLIDER0:
				// Set horizontal grid spacing
				hItem = WM_GetDialogItem(hDlg, GUI_ID_GRAPH0);
				Value = SLIDER_GetValue(pMsg->hWinSrc) * 10;
				GRAPH_SetGridDistX(hItem, Value);
				GRAPH_SCALE_SetTickDist(_hScaleH, Value);
				break; //end of WidgetID == GUI_ID_SLIDER0

			case GUI_ID_SLIDER1:
				// Set vertical grid spacing
				hItem = WM_GetDialogItem(hDlg, GUI_ID_GRAPH0);
				Value = SLIDER_GetValue(pMsg->hWinSrc) * 5;
				GRAPH_SetGridDistY(hItem, Value);
				GRAPH_SCALE_SetTickDist(_hScaleV, Value);
				break; //end of WidgetID == GUI_ID_SLIDER1

			case GUI_ID_RADIO0:
				// Set the widget effect
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
				break; //end of WidgetID == GUI_ID_RADIO0

			case GUI_ID_CHECK6:
				// Toggle horizontal scroll bar
				hItem = WM_GetDialogItem(hDlg, GUI_ID_GRAPH0);
#define Y_SCALE_MIN 15.0
#define Y_SCALE_MAX 300.0
#define GRAPH_PIXELS_Y 200
#define Y_SCALE_FACTOR ((float)(Y_SCALE_MAX-(Y_SCALE_MIN))/GRAPH_PIXELS_Y)

				//GRAPH_SCALE_SetFactor(hItem, (float)1.0);

				if (CHECKBOX_IsChecked(WM_GetDialogItem(hDlg, GUI_ID_CHECK6)))
					GRAPH_SetVSizeX(hItem, 500);
				else
					GRAPH_SetVSizeX(hItem, 0);
				break; //end of WidgetID == GUI_ID_CHECK6

			case GUI_ID_CHECK7:
				// Toggle vertical scroll bar
				hItem = WM_GetDialogItem(hDlg, GUI_ID_GRAPH0);

				if (CHECKBOX_IsChecked(WM_GetDialogItem(hDlg, GUI_ID_CHECK7)))
					GRAPH_SetVSizeY(hItem, 10);
				else
					GRAPH_SetVSizeY(hItem, 0);
				break; //end of WidgetID == GUI_ID_CHECK7

			case GUI_ID_CHECK8:
				// Toggle alignment
				WM_GetDialogItem(hDlg, GUI_ID_GRAPH0);

				for (uint16_t i = 0; i < GUI_COUNTOF(_aColor); i++) {
					if (CHECKBOX_IsChecked(
							WM_GetDialogItem(hDlg, GUI_ID_CHECK8))) {
						GRAPH_DATA_YT_SetAlign(_ahData[i], GRAPH_ALIGN_LEFT);
						GRAPH_DATA_YT_MirrorX(_ahData[i], 1);
					} else {
						GRAPH_DATA_YT_SetAlign(_ahData[i], GRAPH_ALIGN_RIGHT);
						GRAPH_DATA_YT_MirrorX(_ahData[i], 0);

					}
				}
				break; //end of WidgetID == GUI_ID_CHECK8
			}
			break; //end of NCode == WM_NOTIFICATION_VALUE_CHANGED
		}
		break; //end of pMsg->MsgId == WM_NOTIFY_PARENT

	default:
		WM_DefaultProc(pMsg);
	}
}

uint16_t __x, __y;

void Main_Task(void * pvParameters) {
	uint32_t ticks = 0;

	ILI9327_Fill(VGA_BLACK + 1);
	GUI_DispString(
			"Initializing...");

//	vTaskDelay(1000);

	/*ILI9327_Fill(VGA_GREEN);
	 vTaskDelay(100);
	 ILI9327_Fill(VGA_RED);
	 vTaskDelay(100);
	 ILI9327_Fill(VGA_BLUE);
	 vTaskDelay(100);*/

	WM_HWIN hDlg;
	WM_HWIN hGraph = 0;
	//GUI_CURSOR_Show();

	if (GUI_ALLOC_GetNumFreeBytes() < RECOMMENDED_MEMORY) {
		GUI_ErrorOut("Not enough memory available.");
		return;
	}

	hDlg = GUI_CreateDialogBox(_aDialogCreate, GUI_COUNTOF(_aDialogCreate),
			GraphCallback, 0, 0, 0);

#if 0
	// Ustawienie domyÅ›lnego stylu widÅ¼etÃ³w
	BUTTON_SetDefaultSkin(BUTTON_SKIN_FLEX);
	FRAMEWIN_SetDefaultSkin(FRAMEWIN_SKIN_FLEX);

	// Ustawienie koloru tÅ‚a pulpitu
	WM_SetDesktopColor(GUI_BLACK);

#if GUI_SUPPORT_MEMDEV
	WM_SetCreateFlags(WM_CF_MEMDEV);
#endif

	GUI_SetFont(&GUI_Font8x16);

// Utworzenie okienka - widÅ¼et FRAMEWIN
	hFrame1 = FRAMEWIN_CreateEx(35, 50, 170, 150, 0, WM_CF_SHOW, 0, 10, "MyFirstApp", &cbFRAMEWIN_1);
	FRAMEWIN_SetFont (hFrame1, &GUI_Font13HB_1);

	FRAMEWIN_AddCloseButton (hFrame1, FRAMEWIN_BUTTON_RIGHT, 0);
	FRAMEWIN_AddMaxButton (hFrame1, FRAMEWIN_BUTTON_RIGHT, 0);
	FRAMEWIN_AddMinButton (hFrame1, FRAMEWIN_BUTTON_RIGHT, 0);
	FRAMEWIN_SetTextAlign (hFrame1, GUI_TA_HCENTER);

	// UtworzenietekstÃ³w - widÅ¼etTEXT
	hText_1 = TEXT_CreateEx(5, 25, 160, 20, hFrame1, WM_CF_SHOW, TEXT_CF_HCENTER, 0, "Hello, World!");
	hText_2 = TEXT_CreateEx(5, 50, 160, 20, hFrame1, WM_CF_SHOW, TEXT_CF_LEFT, 1, "To be...");
	hText_3 = TEXT_CreateEx(5, 75, 160, 20, hFrame1, WM_CF_SHOW, TEXT_CF_RIGHT , 2, "... or not to be");

	// Ustawienie czcionki tekstÃ³w
	TEXT_SetFont(hText_1, &GUI_Font13_1);
	TEXT_SetFont(hText_2, &GUI_Font13_1);
	TEXT_SetFont(hText_3, &GUI_Font13_1);

	// UtworzenieprzyciskÃ³w - widÅ¼et BUTTON
	hButton_1 = BUTTON_CreateEx( 5, 100, 50, 20, hFrame1, WM_CF_SHOW, 0, 0);
	hButton_2 = BUTTON_CreateEx( 60, 100, 50, 20, hFrame1, WM_CF_SHOW, 0, 1);
	hButton_3 = BUTTON_CreateEx(115, 100, 50, 20, hFrame1, WM_CF_SHOW, 0, 2);
	hButton_4 = BUTTON_CreateEx( 5, 125, 77, 20, hFrame1, WM_CF_SHOW, 0, 3);
	hButton_5 = BUTTON_CreateEx( 88, 125, 77, 20, hFrame1, WM_CF_SHOW, 0, 4);

	// Dodanie etykiet do przyciskÃ³w
	BUTTON_SetText(hButton_1, "red");
	BUTTON_SetText(hButton_2, "green");
	BUTTON_SetText(hButton_3, "blue");
	BUTTON_SetText(hButton_4, "BIG FONT");
	BUTTON_SetText(hButton_5, "small font");

	// Przypisanie funkcji zwrotnych do przyciskÃ³w
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

	while (1) {
#if 0
		if(ticks++ > 10)
		{
			ticks = 0;

			/* Inwersja stanu LED3 kaÅ¼de 100ms */
			//STM_EVAL_LEDToggle(LED3);
			static int r=0;
			if(r)
			{
				//BUTTON_SetState(hButton_1, BUTTON_STATE_PRESSED);
				//FRAMEWIN_Minimize(hFrame1);
				//FRAMEWIN_SetResizeable(hFrame1, FRAMEWIN_SF_MAXIMIZED);
				r=0;
			} else {
				//BUTTON_SetState(hButton_1, BUTTON_STATE_FOCUS);
				//FRAMEWIN_Maximize(hFrame1);
				//FRAMEWIN_SetResizeable(hFrame1, FRAMEWIN_SF_MINIMIZED);
				r=1;
			}

		}
#endif

		extern const unsigned char SmallFont[];

		ILI9327_setFont(SmallFont);

		ILI9327_setColor(VGA_GREEN);
//		ILI9327_NumWDesc(100, 0, "P=", __x);
//		ILI9327_NumWDesc(200, 0, "P=", __y);

		GUI_Exec();
		GUI_Delay(5);

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

#if 0

void cbFRAMEWIN_1(WM_MESSAGE * pMsg)
{
	switch(pMsg->MsgId)
	{
		default:
		// The original callback
		FRAMEWIN_Callback(pMsg);
		break;
	}
}

/**
 * @brief  Callback function
 * @param  None
 * @retval None
 */
void cbBUTTON_1(WM_MESSAGE * pMsg) {
	int NCode;

	switch(pMsg->MsgId) {
		case WM_NOTIFY_PARENT_REFLECTION: {
			NCode = pMsg->Data.v;
			switch(NCode) {
				case WM_NOTIFICATION_RELEASED: {
					TEXT_SetTextColor(hText_1, GUI_RED);
					break;
				}
			}
			break;
		}
		default: {
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
void cbBUTTON_2(WM_MESSAGE * pMsg) {
	int NCode;

	switch(pMsg->MsgId) {
		case WM_NOTIFY_PARENT_REFLECTION: {
			NCode = pMsg->Data.v;
			switch(NCode) {
				case WM_NOTIFICATION_RELEASED: {
					TEXT_SetTextColor(hText_1, GUI_GREEN);
					break;
				}
			}
			break;
		}
		default: {
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
void cbBUTTON_3(WM_MESSAGE * pMsg) {
	int NCode;

	switch(pMsg->MsgId) {
		case WM_NOTIFY_PARENT_REFLECTION: {
			NCode = pMsg->Data.v;
			switch(NCode) {
				case WM_NOTIFICATION_RELEASED: {
					TEXT_SetTextColor(hText_1, GUI_BLUE);
					break;
				}
			}
			break;
		}
		default: {
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
void cbBUTTON_4(WM_MESSAGE * pMsg) {
	int NCode;

	switch(pMsg->MsgId) {
		case WM_NOTIFY_PARENT_REFLECTION: {
			NCode = pMsg->Data.v;
			switch(NCode) {
				case WM_NOTIFICATION_RELEASED: {
					TEXT_SetFont(hText_1, &GUI_Font20_1);
					TEXT_SetFont(hText_2, &GUI_Font20_1);
					TEXT_SetFont(hText_3, &GUI_Font20_1);
					break;
				}
			}
			break;
		}
		default: {
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
void cbBUTTON_5(WM_MESSAGE * pMsg) {
	int NCode;

	switch(pMsg->MsgId) {
		case WM_NOTIFY_PARENT_REFLECTION: {
			NCode = pMsg->Data.v;
			switch(NCode) {
				case WM_NOTIFICATION_RELEASED: {
					TEXT_SetFont(hText_1, &GUI_Font13_1);
					TEXT_SetFont(hText_2, &GUI_Font13_1);
					TEXT_SetFont(hText_3, &GUI_Font13_1);
					break;
				}
			}
			break;
		}
		default: {
			BUTTON_Callback(pMsg);
			break;
		}
	}
}
#endif
