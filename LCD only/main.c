/* Includes ------------------------------------------------------------------*/
#include "stm32f4xx.h"
#include "stm32f4xx_conf.h"
#include "system_stm32f4xx.h"

#include "stm32f4xx_flash.h"
#include "stm32f4xx_gpio.h"
#include "stm32f4xx_iwdg.h"
#include "stm32f4xx_pwr.h"
#include "stm32f4xx_rcc.h"
#include "stm32f4xx_rng.h"
#include "stm32f4xx_sdio.h"
#include "stm32f4xx_tim.h"
#include "stm32f4xx_usart.h"

#include "FreeRTOSConfig.h"
#include "FreeRTOS.h"
#include "croutine.h"
#include "portmacro.h"
#include "task.h"
#include "queue.h"


#include "pcd8544.h"

#include "UTFT.h"
#include "misc.h"

volatile uint32_t delay = 0;

/* Private typedef -----------------------------------------------------------*/
/* Private define ------------------------------------------------------------*/
/* Private macro -------------------------------------------------------------*/
/* Private variables ---------------------------------------------------------*/
/* Extern variables ----------------------------------------------------------*/
/* Private function prototypes -----------------------------------------------*/
/* Private functions ---------------------------------------------------------*/
/*******************************************************************************
* Function Name  : __delay_ms
* Description    :
*******************************************************************************/
void __delay_ms( volatile uint32_t nTime )
{
	//delay = nTime;
	for(long i=0; i<9000*nTime; i++);
	//while(delay);
}

/*******************************************************************************
* Function Name  : vApplicationIdleHook
* Description    :
*******************************************************************************/
void vApplicationIdleHook( void )
{
}


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
	if(delay) delay--;
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
    RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOD, ENABLE);

    GPIO_InitTypeDef GPIO_InitStructure;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_OUT;
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_12 | GPIO_Pin_13 | GPIO_Pin_14 | GPIO_Pin_15;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_Init(GPIOD, &GPIO_InitStructure);
    /* DISCOVERY LEDS **/

}


/*******************************************************************************
* Function Name  :
* Description    :
*******************************************************************************/
void vLedTask (void *pvParameters)
{
	uint8_t state=0;

    while(1)
    {
    	state?GPIO_SetBits(GPIOD, GPIO_Pin_12):GPIO_ResetBits(GPIOD, GPIO_Pin_12);
		state = state?0:1;

		vTaskDelay(100);
    }
    vTaskDelete(NULL);
}


/*******************************************************************************
* Function Name  :
* Description    :
*******************************************************************************/
void vDisplayTask(void *pvParameters)
{
	extern uint8_t SmallFont[];
	extern uint8_t BigFont[];
	extern uint8_t SevenSegNumFont[];

	uint16_t COLORS[6] = {VGA_BLACK, VGA_WHITE, VGA_RED, VGA_BLUE,VGA_GREEN, VGA_YELLOW};

	uint8_t i=0;

	while(1)
	{
		GPIOD->ODR ^= GPIO_Pin_14;
		vTaskDelay(9*10);
		ILI9327_Fill(COLORS[i]);

					//R	G B
			ILI9327_setColorRGB(255, 255, 0);
			ILI9327_setBackColor(VGA_BLACK);
		ILI9327_Rect(20,20,100,100);
			ILI9327_setFont(BigFont);
		ILI9327_Char('A', 50,50);
			ILI9327_setFont(SevenSegNumFont);
		ILI9327_Char(i+'0', 120,40);

		vTaskDelay(9*10);

		ILI9327_Circle(20,140,20);

			ILI9327_setColor(VGA_PURPLE);
			ILI9327_setBackColor(VGA_BLUE);
		ILI9327_CircleFill(20,170,10);

			ILI9327_setColor(VGA_FUCHSIA);
		ILI9327_RectFill(100,100,120,130);


		if(++i>=6) i=0;
		//if(i==0){extern const unsigned int BITMAP1[0x11760]; ILI9327_Bitmap(0,0,240,298,(unsigned int*)BITMAP1,1);vTaskDelay(9*20);}
	}
}



/*******************************************************************************
* Function Name  : main
*******************************************************************************/
int main()
{
	InitPeriph();

	//SysTick_Config(8000000/800);
	PCD8544_Init();
	ILI9327_Init();

	PCD8544_Clear();
	PCD8544_String(5,0,"YEEEEAH!", 0);
	PCD8544_String(8,10,"I MADE IT!!!", 1);
	PCD8544_Line(1,1,50,50,1);
	PCD8544_Line(20,20,40,20,1);
	PCD8544_Line(22,22,22,40,1);
	PCD8544_Line(30,30,4,5,1);
	PCD8544_Rectangle(40,40,100,60,1);
	PCD8544_Refresh();


	// Declare which fonts we will be using
	/*extern uint8_t SmallFont[];
	extern uint8_t BigFont[];
	extern uint8_t SevenSegNumFont[];*/

	GPIOD->ODR ^= GPIO_Pin_12;
	GPIOD->ODR |= GPIO_Pin_15;

	ILI9327_Clear();




	//setColorColor(255, 0, 0);
	//fillScr(VGA_YELLOW);


/* todo:
system clock + flash
systick
...
drivers for all chips: microphone, dac, accel, leds, button, usb otg


*/
    xTaskCreate(vLedTask, "LedTask", configMINIMAL_STACK_SIZE, NULL, tskIDLE_PRIORITY + 1, NULL);
    xTaskCreate(vDisplayTask, "DispTask", configMINIMAL_STACK_SIZE, NULL, tskIDLE_PRIORITY + 1, NULL);
    vTaskStartScheduler();

    return 0;
}
