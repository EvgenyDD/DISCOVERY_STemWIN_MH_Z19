#include "DIALOG.h"
#include "FreeRTOS.h"
#include "FreeRTOSConfig.h"
#include "GUI.h"
#include "GUIDEMO.h"
#include "ILI9327.h"
#include "NRF24L01.h"
#include "croutine.h"
#include "misc.h"
#include "portmacro.h"
#include "queue.h"
#include "semihosting.h"
#include "semphr.h"
#include "stm32f4xx.h"
#include "stm32f4xx_rng.h"
#include "string.h"
#include "system_stm32f4xx.h"
#include "task.h"
#include "timers.h"
#include "touch.h"
#include "cnc_engine.h"
#include <stdio.h>

extern void main_task(void *pvParameters);

#define Main_Task_PRIO (tskIDLE_PRIORITY + 9)
#define Main_Task_STACK (3048)

SemaphoreHandle_t xSPI1_Mutex;

void vApplicationIdleHook(void) {}
void vApplicationTickHook(void) {}

void vApplicationMallocFailedHook(void)
{
    for(;;)
        asm("nop");
}

void vApplicationStackOverflowHook(xTaskHandle pxTask, signed char *pcTaskName)
{
    (void)pcTaskName;
    (void)pxTask;

    for(;;)
        ;
}

void InitPeriph(void)
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

    RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_CRC, ENABLE);
}

void main(void)
{
    InitPeriph();
    SystemInit();

    cnc_engine_init();

    xSPI1_Mutex = xSemaphoreCreateMutex();

    PWM_BL_Init();
    XPT2046_Init();

    GUI_Init();

    xTaskHandle main_task_handle;
    xTaskCreate(main_task, "MainTask", Main_Task_STACK, NULL, Main_Task_PRIO, &main_task_handle);

    xTimerHandle ts_timer = xTimerCreate("TouchscreenTimer", 50, pdTRUE, (void *)1, ts_callback);

    xTimerStart(ts_timer, 0);

    vTaskStartScheduler();
}
