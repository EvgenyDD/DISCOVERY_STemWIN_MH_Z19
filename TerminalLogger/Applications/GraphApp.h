/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef GRAPH_H_H
#define GRAPH_H_H

/* Includes ------------------------------------------------------------------*/
#include "GUI.h"
#include "DIALOG.h"
#include "../src/STemWIN/inc/GRAPH.h"
#include "ili9327.h"


/* Exported types ------------------------------------------------------------*/
/* Exported constants --------------------------------------------------------*/
/* Exported macro ------------------------------------------------------------*/
/* Exported define -----------------------------------------------------------*/
/* Exported functions ------------------------------------------------------- */
void GraphApp_Task(void * pvParameters);
void GraphCallback(WM_MESSAGE * pMsg);
void _ToggleFullScreenMode(WM_HWIN hDlg);
void _ForEach(WM_HWIN hWin, void * pData);
void _UserDraw(WM_HWIN hWin, int Stage);
void Graph_AddValues();




#endif //GRAPH_H_H
