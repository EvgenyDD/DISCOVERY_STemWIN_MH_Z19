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

	GUIDEMO_Main();

	// Ustawienie domyślnego stylu widżetów
	BUTTON_SetDefaultSkin(BUTTON_SKIN_FLEX);
	FRAMEWIN_SetDefaultSkin(FRAMEWIN_SKIN_FLEX);

	// Ustawienie koloru tła pulpitu
	WM_SetDesktopColor(GUI_BLACK);

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
				BUTTON_SetState(hButton_1, BUTTON_STATE_PRESSED);
				FRAMEWIN_Minimize(hFrame1);
				//FRAMEWIN_SetResizeable(hFrame1, FRAMEWIN_SF_MAXIMIZED);
				r=0;
			}else {
				BUTTON_SetState(hButton_1, BUTTON_STATE_FOCUS);
				FRAMEWIN_Maximize(hFrame1);
				//FRAMEWIN_SetResizeable(hFrame1, FRAMEWIN_SF_MINIMIZED);
				r=1;
			}

		}
		//ILI9327_Fill(VGA_RED+ticks*20);
		GUI_Delay(50);


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
	//ILI9327_Init();
//ILI9327_Clear();
	printf("#System started!\n\r");


	printf("#Gui initialization...\n\r");
	GUI_Init();
	printf("#...Gui finished initialization!\n\r");

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
	  for (i = 0x00; i<0xFF; i++);

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
