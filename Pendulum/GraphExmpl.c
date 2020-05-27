/* Includes ------------------------------------------------------------------*/
#include "stm32f4xx.h"
#include "stm32f4xx_conf.h"
#include <string.h>
#include "FreeRTOS.h"
#include "task.h"

#include "GUI.h"
#include "DIALOG.h"

#include "ILI9327.h"


/* Private typedef -----------------------------------------------------------*/
/* Private define ------------------------------------------------------------*/
// Recommended memory to run the sample with adequate performance
#define RECOMMENDED_MEMORY (1024L * 30)


/* Private macro -------------------------------------------------------------*/
/* Private variables ---------------------------------------------------------*/
int16_t PValue = 10000;
int16_t DValue = 10;
int16_t IValue = -2;
int16_t angleVal=0;
int16_t PWMVal=0;
int16_t speedVal=0;


GRAPH_DATA_Handle  _ahData[3]; // Array of handles for the GRAPH_DATA objects
GRAPH_SCALE_Handle _hScaleV;   // Handle of vertical scale
GRAPH_SCALE_Handle _hScaleH;   // Handle of horizontal scale

GUI_COLOR _aColor[] = {GUI_RED, GUI_GREEN, GUI_LIGHTBLUE}; // Array of colors for the GRAPH_DATA objects

int _aValue[3];
int _Stop = 0;

// Dialog resource
const GUI_WIDGET_CREATE_INFO _aDialogCreate[] = {
  { FRAMEWIN_CreateIndirect, "Graph widget demo",  0                ,   0,   0, 400, 240, FRAMEWIN_CF_MOVEABLE },
  { GRAPH_CreateIndirect,     0,                   GUI_ID_GRAPH0    ,   2,   2, 290, 170 },
  { TEXT_CreateIndirect,      "Spacing X:",        0                ,  10, 180,  50,  20 },
  { TEXT_CreateIndirect,      "Spacing Y:",        0                ,  10, 200,  50,  20 },
  { SLIDER_CreateIndirect,    0,                   GUI_ID_SLIDER0   ,  60, 180,  60,  16 },
  { SLIDER_CreateIndirect,    0,                   GUI_ID_SLIDER1   ,  60, 200,  60,  16 },
  { CHECKBOX_CreateIndirect,  0,                   GUI_ID_CHECK0    , 130, 180,  50,   0 },
  { CHECKBOX_CreateIndirect,  0,                   GUI_ID_CHECK1    , 130, 200,  50,   0 },
  /*{ TEXT_CreateIndirect,      "Border",            0                , 275,   5,  35,  15 },
  { CHECKBOX_CreateIndirect,  0,                   GUI_ID_CHECK2    , 275,  20,  35,   0 },
  { CHECKBOX_CreateIndirect,  0,                   GUI_ID_CHECK3    , 275,  40,  35,   0 },
  { CHECKBOX_CreateIndirect,  0,                   GUI_ID_CHECK4    , 275,  60,  35,   0 },
  { CHECKBOX_CreateIndirect,  0,                   GUI_ID_CHECK5    , 275,  80,  35,   0 },
  { TEXT_CreateIndirect,      "Effect",            0                , 275, 100,  35,  15 },
  { RADIO_CreateIndirect,     0,                   GUI_ID_RADIO0    , 270, 115,  35,   0, 0, 3 },*/
  { CHECKBOX_CreateIndirect,  0,                   GUI_ID_CHECK6    , 180, 180,  50,   0 },
  { CHECKBOX_CreateIndirect,  0,                   GUI_ID_CHECK7    , 180, 200,  50,   0 },
  { BUTTON_CreateIndirect,    "Full",       	   GUI_ID_BUTTON0   , 240, 180,  50,  18 },
  { CHECKBOX_CreateIndirect,  0,                   GUI_ID_CHECK8    , 240, 200,  70,   0 },

	{ BUTTON_CreateIndirect,    "P+",       		   	GUI_BTN0   , 300, 2,  23,  20 },
	{ BUTTON_CreateIndirect,    "p+",       			GUI_BTN1   , 300+24, 2,  23,  20 },
	{ BUTTON_CreateIndirect,    "p+",       			GUI_BTN2   , 300+24+24, 2,  23,  20 },
	{ BUTTON_CreateIndirect,    "P-",       			GUI_BTN3   , 300, 50,  23,  20 },
	{ BUTTON_CreateIndirect,    "p-",       			GUI_BTN4   , 300+24, 50,  23,  20 },
	{ BUTTON_CreateIndirect,    "p-",       			GUI_BTN5   , 300+24+24, 50,  23,  20 },
	{ EDIT_CreateIndirect, 	0,				  			GUI_ID_EDIT0, 300,28, 70,17},

	{ BUTTON_CreateIndirect,    "D+",       		   	GUI_BTN6   , 300, 76,  23,  20 },
	{ BUTTON_CreateIndirect,    "d+",       			GUI_BTN7   , 300+24, 76,  23,  20 },
	{ BUTTON_CreateIndirect,    "d+",       			GUI_BTN8   , 300+24+24, 76,  23,  20 },
	{ BUTTON_CreateIndirect,    "D-",       			GUI_BTN9   , 300, 126,  23,  20 },
	{ BUTTON_CreateIndirect,    "d-",       			GUI_BTN10   , 300+24, 126,  23,  20 },
	{ BUTTON_CreateIndirect,    "d-",       			GUI_BTN11   , 300+24+24, 126,  23,  20 },
	{ EDIT_CreateIndirect, 	0,				  			GUI_ID_EDIT1, 300,102, 70,17},

	{ BUTTON_CreateIndirect,    "I+",       		   	GUI_BTN12   , 300, 150,  23,  20 },
	{ BUTTON_CreateIndirect,    "i+",       			GUI_BTN13   , 300+24, 150,  23,  20 },
	{ BUTTON_CreateIndirect,    "i+",       			GUI_BTN14   , 300+24+24, 150,  23,  20 },
	{ BUTTON_CreateIndirect,    "I-",       			GUI_BTN15   , 300, 198,  23,  20 },
	{ BUTTON_CreateIndirect,    "i-",       			GUI_BTN16   , 300+24, 198,  23,  20 },
	{ BUTTON_CreateIndirect,    "i-",       			GUI_BTN17   , 300+24+24, 198,  23,  20 },
	{ EDIT_CreateIndirect, 	0,				  			GUI_ID_EDIT2, 300,176, 70,17},
};


/* Extern variables ----------------------------------------------------------*/
/* Private function prototypes -----------------------------------------------*/
/* Private functions ---------------------------------------------------------*/
/*******************************************************************************
* Function Name  :
* Description    :
* Input		 :
*******************************************************************************/

/*******************************************************************************
* Function Name  : Graph_AddValues
* Description    : Calculate new random values in dependence of the previous added values
*   				and adds them to the GRAPH_DATA objects
*******************************************************************************/
void Graph_AddValues()
{
	for(uint16_t i=0; i<GUI_COUNTOF(_aColor); i++)
	{
		int Add = ((unsigned)rand()) % (2 + i * i);
		int Vz  = (((unsigned)(rand()) % 2) << 1) - 1;

		_aValue[i] += Add * Vz;

		if(_aValue[i] > 150)
			_aValue[i] = 150;
		else if(_aValue[i] < 0)
			_aValue[i] = 0;

		//150 - max
		_aValue[1] = angleVal/2+75;;
		_aValue[2] = PWMVal/14+75;
		_aValue[0] = speedVal/10+75;
		//if(i == 0) GRAPH_DATA_YT_AddValue(_ahData[i], angleVal);
		/*else*/ GRAPH_DATA_YT_AddValue(_ahData[i], _aValue[i]);
		/*GRAPH_DATA_YT_AddValue(_ahData[i], _aValue[i]+5);
		GRAPH_DATA_YT_AddValue(_ahData[i], _aValue[i]+15);*/
	}
}


/*******************************************************************************
* Function Name  : _UserDraw
* Description    : Is called by the GRAPH object before anything is drawn
*   			   and after the last drawing operation
*******************************************************************************/
void _UserDraw(WM_HWIN hWin, int Stage)
{
	if(Stage == GRAPH_DRAW_LAST)
	{
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


/*******************************************************************************
* Function Name  : _ForEach
* Description    : Hides/shows all windows except the button, graph and scroll bar widgets
*******************************************************************************/
void _ForEach(WM_HWIN hWin, void * pData)
{
	int FullScreenMode = *(int*)pData;
	int WidgetID = WM_GetId(hWin);

	if(WidgetID==GUI_ID_GRAPH0 || WidgetID==GUI_ID_BUTTON0 || WidgetID==GUI_ID_VSCROLL || WidgetID==GUI_ID_HSCROLL)
		return;

	if(FullScreenMode)
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
void _ToggleFullScreenMode(WM_HWIN hDlg)
{
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
	if(FullScreenMode)
	{
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
	}
	else
	{
		// Return to normal mode
		BUTTON_SetText(hButton, "Full Screen");

		WM_MoveWindow(hButton, 0, -11);
		WM_ForEachDesc(WM_GetClientWindow(hDlg), _ForEach, &FullScreenMode); // Show all descendants

		WM_SetWindowPos(hGraph, Rect.x0, Rect.y0, Rect.x1 - Rect.x0 + 1, Rect.y1 - Rect.y0 + 1);

		FRAMEWIN_SetTitleVis(hDlg, 1);
		GRAPH_SCALE_SetPos(_hScaleH, ScalePos);
	}
}


void ChangeVal(uint16_t widget)
{
	switch(widget)
	{
	case GUI_BTN0:
		PValue += 500;
	break;
	case GUI_BTN1:
		PValue += 50;
	break;
	case GUI_BTN2:
		PValue += 2;
	break;
	case GUI_BTN3:
		PValue -= 500;
	break;
	case GUI_BTN4:
		PValue -= 50;
	break;
	case GUI_BTN5:
		PValue -= 2;
	break;
	case GUI_BTN6:
		DValue += 500;
	break;
	case GUI_BTN7:
		DValue += 50;
	break;
	case GUI_BTN8:
		DValue += 2;
	break;
	case GUI_BTN9:
		DValue -= 500;
	break;
	case GUI_BTN10:
		DValue -= 50;
	break;
	case GUI_BTN11:
		DValue -= 2;
	break;
	case GUI_BTN12:
		IValue += 500;
	break;
	case GUI_BTN13:
		IValue += 50;
	break;
	case GUI_BTN14:
		IValue += 2;
	break;
	case GUI_BTN15:
		IValue -= 500;
	break;
	case GUI_BTN16:
		IValue -= 50;
	break;
	case GUI_BTN17:
		IValue -= 2;
	break;
	}
	//SendValues();
}

#define MakeDIsplayValue(int)

/*******************************************************************************
* Function Name  : GraphCallback
*******************************************************************************/
void GraphCallback(WM_MESSAGE * pMsg)
{
	int      NCode;
	uint16_t WidgetID;
	int      Value;
	WM_HWIN  hDlg;
	WM_HWIN  hItem;

	hDlg = pMsg->hWin;

	//WM_HWIN hEdit = ;

	char c[10]="";
	char d[10];

	ftoa_(((float)PValue)/1000, d, 3);
	strcat_(c,"P=");
	strcat_(c,d);
	EDIT_SetText(WM_GetDialogItem(pMsg->hWin, GUI_ID_EDIT0), c);

	ftoa_((float)DValue/1000, d, 3);
	c[0] = '\0';
	strcat_(c,"D=");
	strcat_(c,d);
	EDIT_SetText(WM_GetDialogItem(pMsg->hWin, GUI_ID_EDIT1), c);

	ftoa_((float)IValue/1000, d, 3);
	c[0] = '\0';
	strcat_(c,"I=");
	strcat_(c,d);
	EDIT_SetText(WM_GetDialogItem(pMsg->hWin, GUI_ID_EDIT2), c);

	switch(pMsg->MsgId)
	{
	case WM_INIT_DIALOG:
		hItem = WM_GetDialogItem(hDlg, GUI_ID_GRAPH0);

		// Add graphs
		for(uint16_t i=0; i<GUI_COUNTOF(_aColor); i++)
		{
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
		_hScaleV = GRAPH_SCALE_Create( 35, GUI_TA_RIGHT, GRAPH_SCALE_CF_VERTICAL, 10);
		GRAPH_SCALE_SetTextColor(_hScaleV, GUI_YELLOW);
		GRAPH_AttachScale(hItem, _hScaleV);

		GRAPH_SCALE_SetFactor(_hScaleV, 40.0);

		// Create and add horizontal scale
		_hScaleH = GRAPH_SCALE_Create(155, GUI_TA_HCENTER, GRAPH_SCALE_CF_HORIZONTAL, 10);
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
		WidgetID    = WM_GetId(pMsg->hWinSrc);      // WidgetID of widget
		NCode = pMsg->Data.v;                 // Notification code

		switch(NCode)
		{
			case WM_NOTIFICATION_CLICKED: //BUTTONS
				switch(WidgetID)
				{
					case GUI_ID_BUTTON0:
						_ToggleFullScreenMode(hDlg);
					break;

					default:
						ChangeVal(WidgetID);
					break;
				}
			break; //end of NCode == WM_NOTIFICATION_CLICKED

			case WM_NOTIFICATION_VALUE_CHANGED:
				switch(WidgetID)
				{
					case GUI_ID_CHECK0:
						// Toggle stop mode
						_Stop ^= 1;
					break; //end of WidgetID == GUI_ID_CHECK0

					case GUI_ID_CHECK1:
						// Toggle grid
						hItem = WM_GetDialogItem(hDlg, GUI_ID_GRAPH0);
						GRAPH_SetGridVis(hItem, CHECKBOX_IsChecked(WM_GetDialogItem(hDlg, GUI_ID_CHECK1)));
					break; //end of WidgetID == GUI_ID_CHECK1

					case GUI_ID_CHECK2:
					case GUI_ID_CHECK3:
					case GUI_ID_CHECK4:
					case GUI_ID_CHECK5:
						// Toggle border
						hItem = WM_GetDialogItem(hDlg, GUI_ID_GRAPH0);
						GRAPH_SetBorder(hItem,
							CHECKBOX_IsChecked(WM_GetDialogItem(hDlg, GUI_ID_CHECK2)) * 40,
							CHECKBOX_IsChecked(WM_GetDialogItem(hDlg, GUI_ID_CHECK3)) * 5,
							CHECKBOX_IsChecked(WM_GetDialogItem(hDlg, GUI_ID_CHECK4)) * 5,
							CHECKBOX_IsChecked(WM_GetDialogItem(hDlg, GUI_ID_CHECK5)) * 5);
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
						switch (RADIO_GetValue(pMsg->hWinSrc))
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
					break; //end of WidgetID == GUI_ID_RADIO0

					case GUI_ID_CHECK6:
						// Toggle horizontal scroll bar
						hItem = WM_GetDialogItem(hDlg, GUI_ID_GRAPH0);
#define Y_SCALE_MIN 15.0
#define Y_SCALE_MAX 300.0
#define GRAPH_PIXELS_Y 200
#define Y_SCALE_FACTOR ((float)(Y_SCALE_MAX-(Y_SCALE_MIN))/GRAPH_PIXELS_Y)

						//GRAPH_SCALE_SetFactor(hItem, (float)1.0);

						if(CHECKBOX_IsChecked(WM_GetDialogItem(hDlg, GUI_ID_CHECK6)))
							GRAPH_SetVSizeX(hItem, 500);
						else
							GRAPH_SetVSizeX(hItem, 0);
					break; //end of WidgetID == GUI_ID_CHECK6

					case GUI_ID_CHECK7:
						// Toggle vertical scroll bar
						hItem = WM_GetDialogItem(hDlg, GUI_ID_GRAPH0);

						if(CHECKBOX_IsChecked(WM_GetDialogItem(hDlg, GUI_ID_CHECK7)))
							GRAPH_SetVSizeY(hItem, 10);
						else
							GRAPH_SetVSizeY(hItem, 0);
					break; //end of WidgetID == GUI_ID_CHECK7

					case GUI_ID_CHECK8:
						// Toggle alignment
						WM_GetDialogItem(hDlg, GUI_ID_GRAPH0);

						for(uint16_t i=0; i<GUI_COUNTOF(_aColor); i++)
						{
							if(CHECKBOX_IsChecked(WM_GetDialogItem(hDlg, GUI_ID_CHECK8)))
							{
								GRAPH_DATA_YT_SetAlign(_ahData[i], GRAPH_ALIGN_LEFT);
								GRAPH_DATA_YT_MirrorX (_ahData[i], 1);
							}
							else
							{
								GRAPH_DATA_YT_SetAlign(_ahData[i], GRAPH_ALIGN_RIGHT);
								GRAPH_DATA_YT_MirrorX (_ahData[i], 0);

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

void Main_Task(void * pvParameters)
{
	uint32_t ticks = 0;

	ILI9327_Fill(VGA_BLACK+1);
	GUI_DispString("Hello world! THIS IS VERY FUCKING SHIT IF YOU KNOW WHAT I MEAN");

	vTaskDelay(1000);

	/*ILI9327_Fill(VGA_GREEN);
	vTaskDelay(100);
	ILI9327_Fill(VGA_RED);
	vTaskDelay(100);
	ILI9327_Fill(VGA_BLUE);
	vTaskDelay(100);*/

	WM_HWIN hDlg;
	WM_HWIN hGraph =0;
	//GUI_CURSOR_Show();

	if (GUI_ALLOC_GetNumFreeBytes() < RECOMMENDED_MEMORY) {
	    GUI_ErrorOut("Not enough memory available.");
	    return;
	  }

	hDlg = GUI_CreateDialogBox(_aDialogCreate, GUI_COUNTOF(_aDialogCreate), GraphCallback, 0, 0, 0);

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


	while(1)
	{
#if 0
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
#endif

		extern const unsigned char SmallFont[];

		if (!_Stop && angleVal<500 && angleVal > -500) {
		if(!hGraph )
			hGraph = WM_GetDialogItem(hDlg, GUI_ID_GRAPH0);


			Graph_AddValues();
		}
		ILI9327_setFont(SmallFont);

			ILI9327_setColor(VGA_GREEN);
	ILI9327_NumWDesc(0,0,"P=", angleVal);
	ILI9327_NumWDesc(100,0,"P=", __x);
	ILI9327_NumWDesc(200,0,"P=", __y);


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
