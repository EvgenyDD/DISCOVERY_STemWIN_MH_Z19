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


#include "ILI9327.h"
#include "string.h"
#include "NRF24L01.h"

//#include "class.h"

uint8_t MyAddress[] = {
    0xE7,
    0xE7,
    0xE7,
    0xE7,
    0xE7
};
/* Receiver address */
uint8_t TxAddress[] = {
    0x7E,
    0x7E,
    0x7E,
    0x7E,
    0x7E
};

uint16_t RxFailCounter = 0;

/* Private typedef -----------------------------------------------------------*/
/* Private define ------------------------------------------------------------*/
/* Private macro -------------------------------------------------------------*/
/* Private variables ---------------------------------------------------------*/
/* Extern variables ----------------------------------------------------------*/
/* Private function prototypes -----------------------------------------------*/
/* Private functions ---------------------------------------------------------*/
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
	GPIOD->ODR ^= GPIO_Pin_15;
	if(RxFailCounter) RxFailCounter--;
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

    ILI9327_Init();
    ILI9327_Clear();

   /* ILI9327 obj;
    RxFailCounter = obj.vogue();*/

}


/*******************************************************************************
* Function Name  :
* Description    :
*******************************************************************************/
void vLedTask(void *pvParameters)
{
	uint8_t state=0;

    while(1)
    {
    	state ? GPIO_SetBits(GPIOD, GPIO_Pin_14) : GPIO_ResetBits(GPIOD, GPIO_Pin_14);
		state = state?0:1;

		vTaskDelay(100);
    }

    vTaskDelete(NULL);
}


void vTimeTask(void *pvParameters)
{
	//static uint16_t tCnt=0;

	while(1)
	{
		vTaskDelay(1);
	}
}
/*******************************************************************************
* Function Name  :
* Description    :
*******************************************************************************/
void vRfTxRxTask( void *pvParameters)
{
	uint8_t dataOut[32], dataIn[32];

	for(uint8_t i=0; i<32; i++)
	{
				dataOut[i]=i;
	}


	while(1)
	{
		for(uint8_t i=0; i<32; i++)
			dataOut[i]++;


		/* Transmit data, goes automatically to TX mode */
		GPIO_SetBits(GPIOD, GPIO_Pin_13);
		NRF24L01_Transmit(dataOut);
		GPIO_ResetBits(GPIOD, GPIO_Pin_13);

		/* Turn on led to indicate sending */
		GPIO_SetBits(GPIOD, GPIO_Pin_13);
		/* Wait for data to be sent */
		while(NRF24L01_GetTransmissionStatus() == NRF24L01_Transmit_Status_Sending);
		/* Turn off led */
		GPIO_ResetBits(GPIOD, GPIO_Pin_13);

		vTaskDelay(1);



		/* Go back to RX mode */
		NRF24L01_PowerUpRx();

		/* Wait received data, wait max 100ms, if time is larger, then data were probably lost */
		RxFailCounter = 9/*ms*/ * 10;
		while(!NRF24L01_DataReady() && RxFailCounter);
		uint8_t RxStatus = (RxFailCounter != 0)?1:0;


		/* Get data from NRF2L01+ */
		NRF24L01_GetData(dataIn);


		static uint8_t i=0;
extern unsigned char SmallFont[];
		ILI9327_setColor(VGA_GREEN);
		ILI9327_setFont(SmallFont);

		ILI9327_setColor((RxStatus)?VGA_RED:VGA_GREEN);
		ILI9327_NumWDesc(0, 80, "Cnt: ", i++);

		for(uint8_t i=0; i<6; i++)
		{
			char I[20] = "Rx";
			char temp[5]; temp[0] = i+'0';temp[1] = '\0';
			strcat_(I, temp);
			ILI9327_NumWDesc(0, 12+i*12, I, dataIn[i]);

			char O[20] = "Tx";
			strcat_(O, temp);
			ILI9327_NumWDesc(80, 12+i*12, I, dataOut[i]);
		}

		ILI9327_NumWDesc(0, 12+6*12, "Status:", NRF24L01_ReadRegister(NRF24L01_REG_STATUS));
		ILI9327_NumWDesc(80, 12+6*12, "FIFO:", NRF24L01_ReadRegister(NRF24L01_REG_FIFO_STATUS));

		ILI9327_NumWDesc(0, 12+7*12, "Ch:", NRF24L01_ReadRegister(NRF24L01_REG_RF_CH));
		ILI9327_NumWDesc(80, 12+7*12, "RFSt:", NRF24L01_ReadRegister(NRF24L01_REG_RF_SETUP));


		vTaskDelay(40);
	}
}



/*******************************************************************************
* Function Name  : main
*******************************************************************************/
int main()
{
	/* Initialize NRF24L01+ on channel 15 and 32bytes of payload */
	/* By default 2Mbps data rate and 0dBm output power */
	/* NRF24L01 goes to RX mode by default */
	NRF24L01_Init(15, 32);

	/* Set 2MBps data rate and -18dBm output power */
	NRF24L01_SetRF(NRF24L01_DataRate_2M, NRF24L01_OutputPower_0dBm);

	/* Set my address, 5 bytes */
	//NRF24L01_SetMyAddress(MyAddress);
	/* Set TX address, 5 bytes */
	//NRF24L01_SetTxAddress(TxAddress);



#if 0

	while (1) {
	        /* If data is ready on NRF24L01+ */
	        if (NRF24L01_DataReady()) {
	            /* Get data from NRF24L01+ */
	            NRF24L01_GetData(dataIn);

	            /* Send it back, automatically goes to TX mode */
	            NRF24L01_Transmit(dataIn);

	            /* Start send */
	            //DISCO_LedOn(LED_GREEN);
	            /* Wait for data to be sent */
	            do {
	                transmissionStatus = NRF24L01_GetTransmissionStatus();
	            } while (transmissionStatus == NRF24L01_Transmit_Status_Sending);
	            /* Send done */
	           // DISCO_LedOff(LED_GREEN);

	            /* Go back to RX Mode */
	            NRF24L01_PowerUpRx();
	        }
	    }
#endif

/* todo:
system clock + flash
systick
...
drivers for all chips: microphone, dac, accel, leds, button, usb otg


*/
    xTaskCreate(vLedTask, "LedTask", configMINIMAL_STACK_SIZE, NULL, tskIDLE_PRIORITY + 1, NULL);
    xTaskCreate(vRfTxRxTask, "RfTxRxTask", configMINIMAL_STACK_SIZE, NULL, tskIDLE_PRIORITY + 1, NULL);
  //  xTaskCreate(vTimeTask, "vTimeTask", configMINIMAL_STACK_SIZE, NULL, tskIDLE_PRIORITY + 1+4, NULL);
    vTaskStartScheduler();

    return 0;
}
