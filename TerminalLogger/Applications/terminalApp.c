/* Includes ------------------------------------------------------------------*/
#include "stm32f4xx.h"
#include "stm32f4xx_gpio.h"
#include "stm32f4xx_rcc.h"
#include "stm32f4xx_usart.h"
#include "misc.h"

#include <string.h>
#include <stdbool.h>
#include "terminalApp.h"
#include "GraphApp.h"

#include "GUI.h"
#include "DIALOG.h"

#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"


/* Private typedef -----------------------------------------------------------*/
typedef struct {
  int          xPos;
  int          yPos;
  int          xSize;
  int          ySize;
  const char * acLabel;
  void (* pfDraw)(WM_HWIN hWin);
} BUTTON_DATA;

typedef struct {
  int x;
  int y;
  int Delay;
} EVENT;

typedef struct {
  BUTTON_SKINFLEX_PROPS * pProp;
  int                     Index;
  BUTTON_SKINFLEX_PROPS   PropOld;
} BUTTON_PROP;



/* Private define ------------------------------------------------------------*/
#define BUTTON_SKINFLEX_RADIUS 4

#define APP_INIT_LOWERCASE     (WM_USER + 0)
#define ID_BUTTON              (GUI_ID_USER + 0)

#define COLOR_BORDER           0x444444
#define COLOR_KEYPAD0          0xAAAAAA
#define COLOR_KEYPAD1          0x555555

#define BUTTON_COLOR0          0xEEEEEE
#define BUTTON_COLOR1          0xCCCCCC
#define BUTTON_COLOR2          0xCCCCCC
#define BUTTON_COLOR3          0xAAAAAA


/* Private macro -------------------------------------------------------------*/
/* Private variables ---------------------------------------------------------*/
xTaskHandle 	TerminalApp_Handle = NULL;
WM_HWIN hRxEdit = 0, hTxEdit;
WM_HWIN hKeypad;
WM_HWIN hSymbCheck;

WM_HWIN hTermFrame;
xQueueHandle xUSART2RxQueue;
xQueueHandle xUSART2TxQueue;


// _aButtonProp: Colors for close button of notepad
static BUTTON_SKINFLEX_PROPS _PropsPressed = {
  { 0x008B622C, 0x00BAB09E, 0x00EFD198 },
  { 0x00FCF4E5, 0x00F6E5C4 },
  { 0x00EFD198, 0x00DBB368 },
  BUTTON_SKINFLEX_RADIUS
};

static BUTTON_SKINFLEX_PROPS _PropsFocussed = {
  { 0x00B17F3C, 0x00FBD846, 0x00DFDFDF },
  { 0x00F3F3F3, 0x00ECECEC },
  { 0x00DFDFDF, 0x00D0D0D0 },
  BUTTON_SKINFLEX_RADIUS
};

static BUTTON_SKINFLEX_PROPS _PropsEnabled = {
  { 0x00221443, 0x00CCD3F4, 0x008795DF },
  { 0x009CA9E9, 0x008795DF },
  { 0x006F7ED3, 0x008795DF },
  BUTTON_SKINFLEX_RADIUS
};

static BUTTON_SKINFLEX_PROPS _PropsDisabled = {
  { 0x00B5B2AD, 0x00FCFCFC, 0x00F4F4F4 },
  { 0x00F4F4F4, 0x00F4F4F4 },
  { 0x00F4F4F4, 0x00F4F4F4 },
  BUTTON_SKINFLEX_RADIUS
};


static GUI_CONST_STORAGE unsigned long acBackSpace[] = {
  0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xA0A0A0A0, 0x20202020, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF,
  0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xA0A0A0A0, 0x20202020, 0x00000000, 0x00000000, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF,
  0xFFFFFFFF, 0xFFFFFFFF, 0xA0A0A0A0, 0x20202020, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF,
  0xA0A0A0A0, 0x20202020, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000,
  0xA0A0A0A0, 0x20202020, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000,
  0xFFFFFFFF, 0xFFFFFFFF, 0xA0A0A0A0, 0x20202020, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF,
  0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xA0A0A0A0, 0x20202020, 0x00000000, 0x00000000, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF,
  0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xA0A0A0A0, 0x20202020, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF
};



static GUI_CONST_STORAGE unsigned long acUp[] = {
  0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0x60606060, 0x60606060, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF,
  0xFFFFFFFF, 0xFFFFFFFF, 0x60606060, 0x30303030, 0x30303030, 0x60606060, 0xFFFFFFFF, 0xFFFFFFFF,
  0xFFFFFFFF, 0x60606060, 0x30303030, 0xEFEFEFEF, 0xEFEFEFEF, 0x30303030, 0x60606060, 0xFFFFFFFF,
  0x60606060, 0x10101010, 0xAFAFAFAF, 0xFFFFFFFF, 0xFFFFFFFF, 0xAFAFAFAF, 0x10101010, 0x60606060,
  0x40404040, 0x40404040, 0x00000000, 0xFFFFFFFF, 0xFFFFFFFF, 0x00000000, 0x40404040, 0x40404040,
  0xFFFFFFFF, 0xFFFFFFFF, 0x00000000, 0xFFFFFFFF, 0xFFFFFFFF, 0x00000000, 0xFFFFFFFF, 0xFFFFFFFF,
  0xFFFFFFFF, 0xFFFFFFFF, 0x00000000, 0xFFFFFFFF, 0xFFFFFFFF, 0x00000000, 0xFFFFFFFF, 0xFFFFFFFF,
  0xFFFFFFFF, 0xFFFFFFFF, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0xFFFFFFFF, 0xFFFFFFFF
};

static BUTTON_PROP _aButtonProp[] = {
  { &_PropsPressed,  BUTTON_SKINFLEX_PI_PRESSED  },
  { &_PropsFocussed, BUTTON_SKINFLEX_PI_FOCUSSED },
  { &_PropsEnabled,  BUTTON_SKINFLEX_PI_ENABLED  },
  { &_PropsDisabled, BUTTON_SKINFLEX_PI_DISABLED },
};

static GUI_CONST_STORAGE GUI_BITMAP bmBackSpace = {
  16, // XSize
  8, // YSize
  64, // BytesPerLine
  32, // BitsPerPixel
  (unsigned char *)acBackSpace,  // Pointer to picture data
  NULL  // Pointer to palette
 ,GUI_DRAW_BMP8888
};


//bmReturn, bmBackSpace, bmUp: Bitmaps used for keyboard
static GUI_CONST_STORAGE unsigned long acReturn[] = {
  0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xA0A0A0A0, 0x20202020, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0x50505050, 0x50505050,
  0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xA0A0A0A0, 0x20202020, 0x00000000, 0x00000000, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0x00000000, 0x00000000,
  0xFFFFFFFF, 0xFFFFFFFF, 0xA0A0A0A0, 0x20202020, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0x00000000, 0x00000000,
  0xA0A0A0A0, 0x20202020, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000,
  0xA0A0A0A0, 0x20202020, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x50505050,
  0xFFFFFFFF, 0xFFFFFFFF, 0xA0A0A0A0, 0x20202020, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF,
  0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xA0A0A0A0, 0x20202020, 0x00000000, 0x00000000, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF,
  0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xA0A0A0A0, 0x20202020, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF
};

static GUI_CONST_STORAGE GUI_BITMAP bmReturn = {
  16, // XSize
  8, // YSize
  64, // BytesPerLine
  32, // BitsPerPixel
  (unsigned char *)acReturn,  // Pointer to picture data
  NULL  // Pointer to palette
 ,GUI_DRAW_BMP8888
};


static GUI_CONST_STORAGE GUI_BITMAP bmUp = {
  8, // XSize
  8, // YSize
  32, // BytesPerLine
  32, // BitsPerPixel
  (unsigned char *)acUp,  // Pointer to picture data
  NULL  // Pointer to palette
 ,GUI_DRAW_BMP8888
};



static void _DrawBkSpc (WM_HWIN hWin) {  _DrawCentered(hWin, &bmBackSpace); }
static void _DrawReturn(WM_HWIN hWin) {  _DrawCentered(hWin, &bmReturn); }
static void _DrawShift (WM_HWIN hWin) {  _DrawCentered(hWin, &bmUp); }



//Static data using function pointers
static const BUTTON_DATA _aButtonData[] = {
  {   7,   5,  17, 24, "Q" },
  {  26,   5,  17, 24, "W" },
  {  45,   5,  17, 24, "E" },
  {  64,   5,  17, 24, "R" },
  {  83,   5,  17, 24, "T" },
  { 102,   5,  17, 24, "Y" },
  { 121,   5,  17, 24, "U" },
  { 140,   5,  17, 24, "I" },
  { 159,   5,  17, 24, "O" },
  { 178,   5,  17, 24, "P" },
  { 197,   5,  17, 24, "{"},
  { 216,   5,  17, 24, "\x8", _DrawBkSpc },

  {  16,  34,  17, 24, "A" },
  {  35,  34,  17, 24, "S" },
  {  54,  34,  17, 24, "D" },
  {  73,  34,  17, 24, "F" },
  {  92,  34,  17, 24, "G" },
  { 111,  34,  17, 24, "H" },
  { 130,  34,  17, 24, "J" },
  { 149,  34,  17, 24, "K" },
  { 168,  34,  17, 24, "L" },
  { 187,  34,  17, 24, ":"},
  { 206,  34,  30, 24, "\x0d", _DrawReturn },

  {   7,  62,  17, 24, "\x19", _DrawShift },
  {  26,  62,  17, 24, "Z" },
  {  45,  62,  17, 24, "X" },
  {  64,  62,  17, 24, "C" },
  {  83,  62,  17, 24, "V" },
  { 102,  62,  17, 24, "B" },
  { 121,  62,  17, 24, "N" },
  { 140,  62,  17, 24, "M" },
  { 159,  62,  17, 24, "," },
  { 178,  62,  17, 24, "." },
  { 197,  62,  17, 24, "/" },
  { 216,  62,  17, 24, "\x19", _DrawShift },

  {   5,  90,  30, 24, "Ctrl" },
  {  37,  90,  30, 24, "Alt" },
  {  69,  90, 102, 24, " " },
  { 173,  90,  30, 24, "Alt G" },
  { 205,  90,  30, 24, "Ctrl" },
};


/* Extern variables ----------------------------------------------------------*/

extern xTaskHandle MainTask_Handle;


/* Private function prototypes -----------------------------------------------*/
/* Private functions ---------------------------------------------------------*/
/*******************************************************************************
* Function Name  : USART_InitTerminal
* Description    : Init USART IO
*******************************************************************************/
void USART_InitTerminal()
{
	/* Rx BT    : PC7
	   Tx BT    : PC6 */
	GPIO_InitTypeDef GPIO_InitStruct;
	USART_InitTypeDef USART_InitStruct;
	NVIC_InitTypeDef NVIC_InitStructure;

	RCC_APB1PeriphClockCmd(RCC_APB1Periph_USART2, ENABLE);
	RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOA, ENABLE);

	GPIO_InitStruct.GPIO_Pin = GPIO_Pin_2 | GPIO_Pin_3;
	GPIO_InitStruct.GPIO_Mode = GPIO_Mode_AF;
	GPIO_InitStruct.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_InitStruct.GPIO_OType = GPIO_OType_PP;
	GPIO_InitStruct.GPIO_PuPd = GPIO_PuPd_UP;
	GPIO_Init(GPIOA, &GPIO_InitStruct);

	GPIO_PinAFConfig(GPIOA, GPIO_PinSource2, GPIO_AF_USART2);
	GPIO_PinAFConfig(GPIOA, GPIO_PinSource3, GPIO_AF_USART2);

	USART_InitStruct.USART_BaudRate = 9600;
	USART_InitStruct.USART_WordLength = USART_WordLength_8b;
	USART_InitStruct.USART_StopBits = USART_StopBits_1;
	USART_InitStruct.USART_Parity = USART_Parity_No;
	USART_InitStruct.USART_HardwareFlowControl = USART_HardwareFlowControl_None;
	USART_InitStruct.USART_Mode = USART_Mode_Tx | USART_Mode_Rx;
	USART_Init(USART2, &USART_InitStruct);
	USART_ITConfig(USART2, USART_IT_RXNE, ENABLE);

	NVIC_InitStructure.NVIC_IRQChannel = USART2_IRQn;
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0;
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
	NVIC_Init(&NVIC_InitStructure);

	/* Queue for storing USART2 input data */
	xUSART2RxQueue = xQueueCreate(100, sizeof(char));
	xUSART2TxQueue = xQueueCreate(100, sizeof(char));

	USART_Cmd(USART2, ENABLE);
}



/*******************************************************************************
* Function Name  : USART_ChangeBaud
* Description    : Change USART2 Baud Rate
*******************************************************************************/
void USART_ChangeBaud(uint8_t newBaudPtr)
{
	USART_Cmd(USART2, DISABLE);

	const uint32_t Bauds[] = {4800, 9600, 19200, 31250, 38400, 57600, 115200, 230400, 460800, 921600, 1500000};
	if(newBaudPtr>10) newBaudPtr = 10;

	USART_InitTypeDef USART_InitStruct;
	USART_InitStruct.USART_BaudRate = Bauds[newBaudPtr];
	USART_InitStruct.USART_WordLength = USART_WordLength_8b;
	USART_InitStruct.USART_StopBits = USART_StopBits_1;
	USART_InitStruct.USART_Parity = USART_Parity_No;
	USART_InitStruct.USART_HardwareFlowControl = USART_HardwareFlowControl_None;
	USART_InitStruct.USART_Mode = USART_Mode_Tx | USART_Mode_Rx;
	USART_Init(USART2, &USART_InitStruct);
	USART_ITConfig(USART2, USART_IT_RXNE, ENABLE);

	USART_Cmd(USART2, ENABLE);
}



/*******************************************************************************
* Function Name  : USART2_IRQHandler
* Description    : Handle receive of a one char
*******************************************************************************/
void USART2_IRQHandler()
{
	BaseType_t xHigherPriorityTaskWoken;
	/* We have not woken a task at the start of the ISR. */
	xHigherPriorityTaskWoken = pdFALSE;

	if(USART_GetITStatus(USART2, USART_IT_RXNE))
	{
		char t = USART2->DR; // the character from the USART1 data register is saved in t
		xQueueSendToBackFromISR(xUSART2RxQueue, &t, &xHigherPriorityTaskWoken);
	}

	/* Now the buffer is empty we can switch context if necessary. */
	if(xHigherPriorityTaskWoken)
		portEND_SWITCHING_ISR(xHigherPriorityTaskWoken);
}



/*******************************************************************************
* Function Name  : TerminalApp_Task
* Description    : Terminal application
*******************************************************************************/
void TerminalApp_Task(void * pvParameters)
{
	//vTaskSuspend(TerminalApp_Handle);

	WM_HWIN hDropDown;
	//WM_HWIN hAnim;

	// Initialize emWin
	WM_SetCreateFlags(WM_CF_MEMDEV);

	//GUI_CURSOR_Show();

	// Set widget default settings for keypad
	BUTTON_SetDefaultSkin(_DrawSkinFlex_BUTTON);

	// Create keypad
	hKeypad = WM_CreateWindowAsChild(0, 400-120, 240, 120, WM_HBKWIN, WM_CF_HIDE | WM_CF_STAYONTOP, _cbKeypad, 0);

	// Set widget default settings for notepad
	//BUTTON_SetDefaultSkin(BUTTON_SKIN_FLEX);
	BUTTON_SetDefaultSkin(_BUTTON_DrawSkinFlex);
	FRAMEWIN_SetDefaultSkin(FRAMEWIN_SKIN_FLEX);
	FRAMEWIN_SetDefaultTextAlign(GUI_TA_HCENTER);

	// Create frame window
	hTermFrame = FRAMEWIN_CreateEx(0, 0, 240, 320, WM_HBKWIN, WM_CF_SHOW, 0, 0, "Notepad", _cbTerminalWnd);
	FRAMEWIN_SetTextColor(hTermFrame, GUI_BLACK);
	FRAMEWIN_SetFont(hTermFrame, &GUI_Font13B_ASCII/*GUI_Font20_AA4*/);
	FRAMEWIN_SetClientColor(hTermFrame, GUI_WHITE);
	FRAMEWIN_AddCloseButton(hTermFrame, FRAMEWIN_BUTTON_RIGHT, 0);

	// Create multi edit widget (Rx buffer)
	hRxEdit = MULTIEDIT_CreateEx(0, 20, 240, 180, WM_GetClientWindow(hTermFrame), WM_CF_SHOW, 0, GUI_ID_MULTIEDIT0, 100, NULL);
	MULTIEDIT_SetWrapWord(hRxEdit);
	MULTIEDIT_EnableBlink(hRxEdit, 300, 1);
	MULTIEDIT_SetFont(hRxEdit, GUI_FONT_16B_ASCII);
	WM_SetFocus(hRxEdit);

	// Create multi edit widget (Tx buffer)
	hTxEdit = MULTIEDIT_CreateEx(0, 200, 240, 40, WM_GetClientWindow(hTermFrame), WM_CF_SHOW, 0, GUI_ID_MULTIEDIT1, 100, NULL);
	MULTIEDIT_SetWrapWord(hTxEdit);
	MULTIEDIT_SetFont(hTxEdit, GUI_FONT_16B_ASCII);

	// Create multi edit widget
	hDropDown = DROPDOWN_CreateEx(0, 0, 190, 150, WM_GetClientWindow(hTermFrame), WM_CF_SHOW, 0, GUI_ID_DROPDOWN0);
	DROPDOWN_AddString(hDropDown, "4800");
	DROPDOWN_AddString(hDropDown, "9600");
	DROPDOWN_AddString(hDropDown, "19200");
	DROPDOWN_AddString(hDropDown, "31250 MIDI");
	DROPDOWN_AddString(hDropDown, "38400");
	DROPDOWN_AddString(hDropDown, "57600");
	DROPDOWN_AddString(hDropDown, "115200");
	DROPDOWN_AddString(hDropDown, "230400");
	DROPDOWN_AddString(hDropDown, "460800");
	DROPDOWN_AddString(hDropDown, "921600");
	DROPDOWN_AddString(hDropDown, "1500000");
	DROPDOWN_IncSel(hDropDown); //set pointer to 9600 bod

	hSymbCheck = CHECKBOX_CreateEx(190,0, 40, 20, WM_GetClientWindow(hTermFrame), WM_CF_SHOW, 0, GUI_ID_CHECK9);
	CHECKBOX_SetText(hSymbCheck, "\\n");

	// Use animation functions to make editor and keypad visible
	//GUI_MEMDEV_ShiftInWindow(hInput, 500, GUI_MEMDEV_EDGE_BOTTOM);
	GUI_MEMDEV_ShiftInWindow(hKeypad, 1000, GUI_MEMDEV_EDGE_BOTTOM);

	GUI_MEMDEV_ShiftInWindow(hTermFrame, 1000, GUI_MEMDEV_EDGE_LEFT);

	//GUI_MEMDEV_ShiftOutWindow(hKeypad, 500, GUI_MEMDEV_EDGE_BOTTOM);
	//if (WM_IsWindow(hTermFrame))

	while (1)
	{
		// Do keyboard animation by a hidden animation window which emits PID events
		//WM_SendMessageNoPara(hKeypad, APP_INIT_LOWERCASE); //send to _cbKeyPad()

		/*hAnim = WM_CreateWindowAsChild(0, 0, 1, 1, WM_HBKWIN, WM_CF_HIDE, _cbAnimation, 0);
		do {
			GUI_Delay(100);
		} while (WM_IsWindow(hAnim));
		*/


		GUI_Delay(10);

		char rx[2] = "\0\0";

		if(xQueueReceive(xUSART2RxQueue, &rx, 0))
			MULTIEDIT_AddText(hRxEdit, rx);

		char t = 0;
		if(xQueueReceive(xUSART2TxQueue, &t, (TickType_t) 10))
		{
			while(USART_GetFlagStatus(USART2, USART_FLAG_TXE) == RESET);
			USART_SendData(USART2, t);
		}

		//GUI_Exec(); /* Do the background work ... Update windows etc.) */
		//GUI_X_ExecIdle(); /* Nothing left to do for the moment ... Idle processing */
	}
}



/*******************************************************************************
* Function Name  : _cbTerminalWnd
* Description    : Callback routine of terminal window
*******************************************************************************/
void _cbTerminalWnd(WM_MESSAGE * pMsg)
{
	WM_HWIN hDropDown;
	WM_HWIN    hWin;

	hWin = pMsg->hWin;

	switch(pMsg->MsgId)
	{
		//events from children of the terminal window
		case WM_NOTIFY_PARENT:
			switch(WM_GetId(pMsg->hWinSrc))
			{
				case GUI_ID_DROPDOWN0:
					hDropDown = WM_GetDialogItem(hWin, GUI_ID_DROPDOWN0);
					char t[20] = {'\0'};

					switch(pMsg->Data.v)
					{
						case WM_NOTIFICATION_SEL_CHANGED:
							// selected item in dropdown was changed
							//DROPDOWN_GetItemText(hDropDown, DROPDOWN_GetSel(hDropDown), t, 20);
							USART_ChangeBaud(DROPDOWN_GetSel(hDropDown));
							strcat_(t, "\n");
							MULTIEDIT_AddText(hRxEdit, t);
						break;
					}
				break;
			}
		break;

		case WM_DELETE:
			// terminal window was closed by user
			WM_HideWindow(hKeypad); // hide QWERTY Keyboard
			vTaskResume(MainTask_Handle);
			vTaskDelete(TerminalApp_Handle);
		break;
	}
}



/*******************************************************************************
* Function Name  : _cbKeyPad
* Description    : Callback routine of keypad window
*******************************************************************************/
void _cbKeypad(WM_MESSAGE * pMsg)
{
	static int LowerCase;
	WM_HWIN    hWin;
	WM_HWIN    hButton;
	char       c;
	int        Id;
	int        xSize, ySize;


	hWin = pMsg->hWin;
	switch (pMsg->MsgId)
	{
		case APP_INIT_LOWERCASE:
			LowerCase = 0;
		break;

		case WM_CREATE:
			// Create the keyboard buttons
			for(uint16_t i=0; i<GUI_COUNTOF(_aButtonData); i++)
			{
				hButton = BUTTON_CreateEx(_aButtonData[i].xPos, _aButtonData[i].yPos, _aButtonData[i].xSize, _aButtonData[i].ySize,
										  hWin, WM_CF_SHOW | WM_CF_HASTRANS, 0, ID_BUTTON + i);
				BUTTON_SetText(hButton, _aButtonData[i].acLabel);
				//BUTTON_SetFont(hButton, &GUI_Font16_1); //default font
				BUTTON_SetFocussable(hButton, 0);
			}
		break; //end of pMsg->MsgId == WM_CREATE

		case WM_PAINT:
		  // Draw background
		  xSize = WM_GetWindowSizeX(hWin);
		  ySize = WM_GetWindowSizeY(hWin);
		  GUI_SetColor(COLOR_BORDER);
		  GUI_DrawRect(0, 0, xSize - 1, ySize - 1);
		  GUI_DrawGradientV(1, 1, xSize - 2, ySize - 2, COLOR_KEYPAD0, COLOR_KEYPAD1);
		break; //end of pMsg->MsgId == WM_PAINT

		case WM_NOTIFY_PARENT:
			// Send key message to currently focused window
			Id = WM_GetId(pMsg->hWinSrc);

			switch (pMsg->Data.v)
			{
				case WM_NOTIFICATION_RELEASED:
					if (_aButtonData[Id - ID_BUTTON].acLabel)
					{
						c = _aButtonData[Id - ID_BUTTON].acLabel[0];
						if(c == GUI_KEY_ENTER)
						{
							char s[55];
							MULTIEDIT_GetText(hTxEdit, s, 55);
							for(uint8_t i=0; i<strlen(s); i++)
								xQueueSendToBack(xUSART2TxQueue, s+i, 10);
						}
						else
						{
							if(c == GUI_KEY_SHIFT)
								LowerCase ^= 1;
							else
							{
								if (LowerCase)
									c |= 0x20;

								GUI_StoreKeyMsg(c, 1);
								GUI_StoreKeyMsg(c, 0);
							}
						}

					}
				break;
			}
		break; //end of pMsg->MsgId == WM_NOTIFY_PARENT
	}
}



/*********************************************************************
*
*       _cbAnimation
*
* Function description
*   Keyboard animation
*/
/*static void _cbAnimation(WM_MESSAGE * pMsg) {
  GUI_PID_STATE   State = {0};
  static int      Index;
  static int      Pressed;
  WM_HWIN         hWin;
  EVENT         * pEvent;
  int             Delay;

  hWin = pMsg->hWin;
  pEvent = &_aEvent[Index];
  switch (pMsg->MsgId) {
  case WM_TIMER:
    State.x = pEvent->x;
    State.y = pEvent->y;
    Pressed ^= 1;
    State.Pressed = Pressed;
    if (Pressed == 0) {
      Index = (Index == GUI_COUNTOF(_aEvent)) ? 0 : Index + 1;
      Delay = 10;
    } else {
      Delay = pEvent->Delay;
    }
    GUI_PID_StoreState(&State);
    if (Index == GUI_COUNTOF(_aEvent)) {
      //
      // End of animation
      //
      WM_DeleteWindow(hWin);
    } else {
      //
      // Continue...
      //
      WM_RestartTimer(pMsg->Data.v, Delay);
    }
    break;
  case WM_CREATE:
    //
    // Create timer to keep animation alive...
    //
    Index = Pressed = 0;
    WM_CreateTimer(hWin, 0, 1500, 0);
    break;
  default:
    WM_DefaultProc(pMsg);
  }
}
*/




/*******************************************************************************
* Function Name  : _BUTTON_DrawSkinFlex
* Description    : Skinning routine of close button. Uses the default skin but with its own colors
*******************************************************************************/
int _BUTTON_DrawSkinFlex(const WIDGET_ITEM_DRAW_INFO * pDrawItemInfo)
{
	// Set user colors
	for(uint16_t i=0; i<GUI_COUNTOF(_aButtonProp); i++)
	{
		BUTTON_GetSkinFlexProps(&_aButtonProp[i].PropOld, _aButtonProp[i].Index);
		BUTTON_SetSkinFlexProps(_aButtonProp[i].pProp,    _aButtonProp[i].Index);
	}

	int r = BUTTON_DrawSkinFlex(pDrawItemInfo);

	// Restore colors
	for(uint16_t i=0; i<GUI_COUNTOF(_aButtonProp); i++)
		BUTTON_SetSkinFlexProps(&_aButtonProp[i].PropOld, _aButtonProp[i].Index);

	return r;
}



/*******************************************************************************
* Function Name  : _DrawSkinFlex_BUTTON
* Description    : Skinning routine for keypad buttons
*******************************************************************************/
int _DrawSkinFlex_BUTTON(const WIDGET_ITEM_DRAW_INFO * pDrawItemInfo)
{
	static GUI_MEMDEV_Handle ahMemEdges[4];
	GUI_MEMDEV_Handle        hMem;
	WM_HWIN                  hWin;
	unsigned                 i;
	int                      xPos, yPos;
	int                      xSize, ySize;
	int                      x, y;
	int                      Id;
	int                      IsPressed;
	void (* pfDraw)(WM_HWIN hWin);

	x = 0;
	y = 0;
	switch(pDrawItemInfo->Cmd)
	{
		case WIDGET_ITEM_CREATE:
			if(ahMemEdges[0] == 0)
			{
				// Create small helper window which never becomes visible
				hWin = WM_CreateWindowAsChild(0, 0, 32, 32, WM_HBKWIN, WM_CF_SHOW | WM_CF_STAYONTOP, NULL, 0);
				WM_SelectWindow(hWin);

				// Create memory devices for the edges of the buttons
				for(uint16_t i=0; i<GUI_COUNTOF(ahMemEdges); i++)
				{
					x = ((i==0)||(i==3))?0:-16;
					y = (i>1)?-16:0;

					// Create magnified device
					hMem = GUI_MEMDEV_CreateFixed(0, 0, 16, 16, GUI_MEMDEV_NOTRANS, GUI_MEMDEV_APILIST_32, GUI_COLOR_CONV_8888);
					GUI_MEMDEV_Select(hMem);
					GUI_SetBkColor(GUI_TRANSPARENT);
					GUI_Clear();
					GUI_SetColor(COLOR_BORDER);
					GUI_DrawRoundedFrame(x, y, x + 31, y + 31, 16, 4);

					// Shrink to required size
					ahMemEdges[i] = GUI_MEMDEV_CreateFixed(0, 0, 4, 4, GUI_MEMDEV_NOTRANS, GUI_MEMDEV_APILIST_32, GUI_COLOR_CONV_8888);
					GUI_MEMDEV_Select(ahMemEdges[i]);
					GUI_SetBkColor(GUI_TRANSPARENT);
					GUI_Clear();
					GUI_MEMDEV_RotateHQ(hMem, ahMemEdges[i], -6, -6, 0, 250);
					GUI_MEMDEV_Delete(hMem);
					GUI_MEMDEV_Select(0);
				}
				WM_DeleteWindow(hWin);
			}
		break;

		case WIDGET_ITEM_DRAW_TEXT:
			// Use private or default function for drawing bitmap or text
			hWin = pDrawItemInfo->hWin;
			Id   = WM_GetId(hWin);
			i    = Id - ID_BUTTON;
			pfDraw = _aButtonData[i].pfDraw;
			if(pfDraw)
				pfDraw(hWin);
			else
				BUTTON_DrawSkinFlex(pDrawItemInfo);
		break;

		case WIDGET_ITEM_DRAW_BACKGROUND:
			// Draw background of button
			IsPressed = BUTTON_IsPressed(pDrawItemInfo->hWin);
			xPos      = WM_GetWindowOrgX(pDrawItemInfo->hWin);
			yPos      = WM_GetWindowOrgY(pDrawItemInfo->hWin);
			xSize     = WM_GetWindowSizeX(pDrawItemInfo->hWin);
			ySize     = WM_GetWindowSizeY(pDrawItemInfo->hWin);
			if (IsPressed)
				GUI_DrawGradientRoundedV(0, 0, xSize - 1, ySize - 1, 4, BUTTON_COLOR2, BUTTON_COLOR3);
			else
				GUI_DrawGradientRoundedV(0, 0, xSize - 1, ySize - 1, 4, BUTTON_COLOR0, BUTTON_COLOR1);

			GUI_SetColor(COLOR_BORDER);
			GUI_DrawHLine(        0, 4, xSize - 5);
			GUI_DrawHLine(ySize - 1, 4, xSize - 5);
			GUI_DrawVLine(        0, 4, ySize - 5);
			GUI_DrawVLine(xSize - 1, 4, ySize - 5);

			GUI_MEMDEV_WriteAt(ahMemEdges[0], xPos +  0, yPos +  0);
			GUI_MEMDEV_WriteAt(ahMemEdges[1], xPos + xSize - 4, yPos +  0);
			GUI_MEMDEV_WriteAt(ahMemEdges[2], xPos + xSize - 4, yPos + 20);
			GUI_MEMDEV_WriteAt(ahMemEdges[3], xPos +  0, yPos + 20);
		break;

		default:
			// Use the default skinning routine for processing all other commands
			return BUTTON_DrawSkinFlex(pDrawItemInfo);
	}
	return 0;
}



/*******************************************************************************
* Function Name  : _DrawCentered
* Description    : Draw the button bitmap
*******************************************************************************/
//
void _DrawCentered(WM_HWIN hWin, const GUI_BITMAP * pBM)
{
	int xSizeWin = WM_GetWindowSizeX(hWin);
	int ySizeWin = WM_GetWindowSizeY(hWin);
	int xSizeBMP = pBM->XSize;
	int ySizeBMP = pBM->YSize;
	int xPos = (xSizeWin - xSizeBMP) >> 1;
	int yPos = (ySizeWin - ySizeBMP) >> 1;
	GUI_DrawBitmap(pBM, xPos, yPos);
}
