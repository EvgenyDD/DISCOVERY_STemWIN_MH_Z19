/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef UART_H
#define UART_H

/* Includes ------------------------------------------------------------------*/
#include "GUI.h"
#include "DIALOG.h"


/* Exported types ------------------------------------------------------------*/
/* Exported constants --------------------------------------------------------*/
/* Exported macro ------------------------------------------------------------*/
/* Exported define -----------------------------------------------------------*/
/* Exported functions ------------------------------------------------------- */
void USART_InitTerminal();
void USART_ChangeBaud(uint8_t newBaudPtr);
void TerminalApp_Task(void * pvParameters);
void App_TerminalInit();

void _cbKeypad(WM_MESSAGE * pMsg);
void _cbTerminalWnd(WM_MESSAGE * pMsg);

void _DrawCentered(WM_HWIN hWin, const GUI_BITMAP * pBM);
int _DrawSkinFlex_BUTTON(const WIDGET_ITEM_DRAW_INFO * pDrawItemInfo);
int _BUTTON_DrawSkinFlex(const WIDGET_ITEM_DRAW_INFO * pDrawItemInfo);


#endif //UART_H
