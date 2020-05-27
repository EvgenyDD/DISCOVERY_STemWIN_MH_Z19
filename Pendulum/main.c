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
#include "NRF24L01.h"
#include "GUIDEMO.h"
/* Private typedef -----------------------------------------------------------*/
/* Private define ------------------------------------------------------------*/
#define Main_Task_PRIO          ( tskIDLE_PRIORITY  + 9 )
#define Main_Task_STACK         ( 3048 )
#define Radio_Task_PRIO          ( tskIDLE_PRIORITY  + 8 )
#define Radio_Task_STACK         ( 1024 )
/* Private macro -------------------------------------------------------------*/
#define LOBYTE(x)  ((uint8_t)(x & 0x00FF))
#define HIBYTE(x)  ((uint8_t)((x & 0xFF00) >>8))
/* Private variables ---------------------------------------------------------*/xTaskHandle Task_Handle;
xTimerHandle TouchScreenTimer;

SemaphoreHandle_t xSPI1_Mutex;

BUTTON_Handle hButton_1, hButton_2, hButton_3, hButton_4, hButton_5;
FRAMEWIN_Handle hFrame1, hFrame2;
TEXT_Handle hText_1, hText_2, hText_3;

/* Private function prototypes -----------------------------------------------*/
void cbFRAMEWIN_1(WM_MESSAGE * pMsg);
void cbBUTTON_1(WM_MESSAGE * pMsg);
void cbBUTTON_2(WM_MESSAGE * pMsg);
void cbBUTTON_3(WM_MESSAGE * pMsg);
void cbBUTTON_4(WM_MESSAGE * pMsg);
void cbBUTTON_5(WM_MESSAGE * pMsg);
void Main_Task(void * pvParameters);
void Radio_Task(void * pvParameters);

/* Extern variables ---------------------------------------------------------*/
extern WM_HWIN ALARM_hWin;

extern uint16_t RxFailCounter;

/* Extern function prototypes -----------------------------------------------*/
extern void ALARM_BackgroundProcess(void);

/* Private function prototypes -----------------------------------------------*/
/* Private functions ---------------------------------------------------------*/
/*******************************************************************************
 * Function Name  : vApplicationIdleHook
 *******************************************************************************/
void vApplicationIdleHook() {
}

/*******************************************************************************
 * Function Name  : vApplicationMallocFailedHook
 *******************************************************************************/
void vApplicationMallocFailedHook() {
	for (;;)
		;
}

/*******************************************************************************
 * Function Name  : vApplicationStackOverflowHook
 *******************************************************************************/
void vApplicationStackOverflowHook(xTaskHandle pxTask, signed char *pcTaskName) {
	(void) pcTaskName;
	(void) pxTask;

	for (;;)
		;
}

/*******************************************************************************
 * Function Name  : vApplicationTickHook
 *******************************************************************************/
void vApplicationTickHook() {
	if (RxFailCounter)
		RxFailCounter--;
}

/*******************************************************************************
 * Function Name  : InitPeriph
 *******************************************************************************/
void InitPeriph() {
	/** DISCOVERY LEDS */
//ORANGE - LD3 - PD13
//GREEN  - LD4 - PD12
//RED 	 - LD5 - PD14
//BLUE	 - LD6 - PD15
	RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOC, ENABLE);
	RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOD, ENABLE);

	GPIO_InitTypeDef GPIO_InitStructure;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_OUT;
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_12 | GPIO_Pin_13 | GPIO_Pin_14
			| GPIO_Pin_15;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init(GPIOD, &GPIO_InitStructure);

	RCC_AHB2PeriphClockCmd(RCC_AHB2Periph_RNG, ENABLE);
	RNG_Cmd(ENABLE);

	// Necessary for STemWin to authenticate that it is runing on STM32 processor
	RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_CRC, ENABLE);
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

/*******************************************************************************
 * Function Name  : main
 *******************************************************************************/
int main() {
	InitPeriph();
	SystemInit(); //Set PLL frequency

	xSPI1_Mutex = xSemaphoreCreateMutex();

	PWM_BL_Init(); //Backlight controller
	XPT2046_Init(); //Touch controller. Also need for nRF24L01 driver functionality

	/* Initialize NRF24L01+ on channel 15 and 32bytes of payload */
	/* NRF24L01 goes to RX mode by default */
	NRF24L01_Init(15, 32);
	NRF24L01_SetRF(NRF24L01_DataRate_2M, NRF24L01_OutputPower_0dBm);

	GUI_Init();
	//GUI_UC_EnableBIDI(1);

//tFlashLine = xTimerCreate("FlashLine", (100/portTICK_RATE_MS), pdTRUE, 0, Flashing);
//xTimerReset(tFlashLine, 0);

//xLEDMutex = xSemaphoreCreateMutex();

	xTaskCreate(Main_Task, "MainTask", Main_Task_STACK, NULL, Main_Task_PRIO,
			&Task_Handle);
	xTaskCreate(Radio_Task, "RadioTask", Radio_Task_STACK, NULL,
			Radio_Task_PRIO, NULL);

	TouchScreenTimer = xTimerCreate("TouchscreenTimer", 50, pdTRUE, (void *) 1,
			vTouchscreenTimerCallback);

	if (TouchScreenTimer != NULL)
		if (xTimerStart( TouchScreenTimer, 0 ) != pdPASS) {
			// The timer could not be set into the Active state.
		}

//xTaskCreate(vLedTask, "LedTask", configMINIMAL_STACK_SIZE, NULL, tskIDLE_PRIORITY + 1, NULL);
//xTaskCreate(vDisplayTask, "DisplayTask", configMINIMAL_STACK_SIZE, NULL, tskIDLE_PRIORITY + 1, NULL);
//xTaskCreate(vTouchDispatcher, "vTouch", configMINIMAL_STACK_SIZE, NULL, tskIDLE_PRIORITY + 1, NULL);

	vTaskStartScheduler();

	/*static portBASE_TYPE xHigherPriorityTaskWoken;
	 xHigherPriorityTaskWoken = pdFALSE;

	 xSemaphoreGiveFromISR(xSemaphore_Wait, &xHigherPriorityTaskWoken );
	 if(xHigherPriorityTaskWoken == pdTRUE)
	 {
	 taskYIELD();
	 }*/
}
