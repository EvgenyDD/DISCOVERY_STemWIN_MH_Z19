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
#include "semphr.h"
#include "task.h"
#include "timers.h"
#include "queue.h"

#include "ILI9327.h"
#include "touch.h"
#include "terminalApp.h"

#include <stdio.h>
#include "string.h"

#include "GUI.h"
#include "DIALOG.h"

#include "semihosting.h"


/* Private typedef -----------------------------------------------------------*/
/* Private define ------------------------------------------------------------*/
/* Private macro -------------------------------------------------------------*/
/* Private variables ---------------------------------------------------------*/
xTaskHandle    	MainTask_Handle = NULL;
xTimerHandle 	TouchScreenTimer;
xTimerHandle 	tFlashLine;

SemaphoreHandle_t 	xSPI1_Mutex;


/* Private function prototypes -----------------------------------------------*/
void Main_Task(void * pvParameters);
void UART_Task(void * pvParameters);


/* Extern variables ---------------------------------------------------------*/
extern xTaskHandle 	TerminalApp_Handle;


/* Extern function prototypes -----------------------------------------------*/
/* Private function prototypes -----------------------------------------------*/
/* Private functions ---------------------------------------------------------*/
/*******************************************************************************
* Function Name  : vApplicationIdleHook
*******************************************************************************/
void vApplicationIdleHook()
{
}


/*******************************************************************************
* Function Name  : vApplicationMallocFailedHook
*******************************************************************************/
void vApplicationMallocFailedHook()
{
    for( ;; );
}


/*******************************************************************************
* Function Name  : vApplicationStackOverflowHook
*******************************************************************************/
void vApplicationStackOverflowHook(xTaskHandle pxTask, signed char *pcTaskName)
{
    (void) pcTaskName;
    (void) pxTask;

    for( ;; );
}


/*******************************************************************************
* Function Name  : vApplicationTickHook
*******************************************************************************/
void vApplicationTickHook()
{

}


/*******************************************************************************
* Function Name  : InitPeriph
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

   /* GPIO_InitTypeDef GPIO_InitStructure;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_OUT;
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_12 | GPIO_Pin_13 | GPIO_Pin_14 | GPIO_Pin_15;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_Init(GPIOD, &GPIO_InitStructure);*/

    RCC_AHB2PeriphClockCmd(RCC_AHB2Periph_RNG, ENABLE);
    RNG_Cmd(ENABLE);

    // Necessary for STemWin to authenticate that it is runing on STM32 processor
    RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_CRC, ENABLE);
}



void Flashing(xTimerHandle xTimer)
{
	//AddKey();
	//static bool flag = FALSE;

/*	xSemaphoreTake(xLEDMutex, portMAX_DELAY);

	if(flag)
	{
		ILI9327_setColor(VGA_BLACK);
		flag = FALSE;
	}
	else
	{
		ILI9327_setColor(VGA_WHITE);
		flag = TRUE;
	}
	ILI9327_Char('_', x*8, y*13);

	 xSemaphoreGive(xLEDMutex);*/
}


/*******************************************************************************
* Function Name  : main
*******************************************************************************/
int main()
{
	InitPeriph();
	SystemInit(); //Set PLL frequency

	xSPI1_Mutex = xSemaphoreCreateMutex();

	PWM_BL_Init(); //Backlight controller
	XPT2046_Init(); //Touch controller. Also need for nRF24L01 driver functionality

	GUI_Init();
	//GUI_UC_EnableBIDI(1);

	USART_InitTerminal();

	/* Bluetooth */
	/*RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOC | RCC_AHB1Periph_GPIO, ENABLE);
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_OUT;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_100MHz;
	GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
	GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_UP;
	GPIO_Init(GPIO, &GPIO_InitStructure);

	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_6 ! GPIO_Pin_7
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_100MHz;
	GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
	GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_UP;
	GPIO_Init(GPIOC, &GPIO_InitStructure);

	RCC_APB2PeriphClockCmd(RCC_APB2Periph_USART1, ENABLE);

	GPIO_InitTypeDef GPIO_InitStructure;
	USART_InitTypeDef USART_InitStructure;
	GPIO_PinAFConfig(GPIOB, GPIO_PinSource6, GPIO_AF_0);

	USART_InitStructure.USART_BaudRate = 115200;
	USART_InitStructure.USART_WordLength = USART_WordLength_8b;
	USART_InitStructure.USART_StopBits = USART_StopBits_1;
	USART_InitStructure.USART_Parity = USART_Parity_No;
	USART_InitStructure.USART_HardwareFlowControl = USART_HardwareFlowControl_None;
	USART_InitStructure.USART_Mode = USART_Mode_Tx;
	USART_Init(USART1, &USART_InitStructure);

	USART_Cmd(USART1, ENABLE);*/


	tFlashLine = xTimerCreate("FlashLine", (300/portTICK_RATE_MS), pdTRUE, 0, Flashing);
	xTimerStart( tFlashLine, 0 );

	//xLEDMutex = xSemaphoreCreateMutex();

	TouchScreenTimer = xTimerCreate ("TouchscreenTimer", 50, pdTRUE, ( void * ) 1, vTouchscreenTimerCallback );

	xTimerStart(TouchScreenTimer, 0);
	/*if( TouchScreenTimer != NULL )
	if(  != pdPASS )
	{
	  // The timer could not be set into the Active state.
	}*/

	xTaskCreate(Main_Task, "MainTask", configMINIMAL_STACK_SIZE, NULL, tskIDLE_PRIORITY + 1, &MainTask_Handle);

    vTaskStartScheduler();

		/*static portBASE_TYPE xHigherPriorityTaskWoken;
		xHigherPriorityTaskWoken = pdFALSE;

		xSemaphoreGiveFromISR(xSemaphore_Wait, &xHigherPriorityTaskWoken );
		if(xHigherPriorityTaskWoken == pdTRUE)
		{
			taskYIELD();
		}*/
}
