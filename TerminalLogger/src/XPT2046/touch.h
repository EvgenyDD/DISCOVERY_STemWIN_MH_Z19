/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef TOUCH_H
#define TOUCH_H

/* Includes ------------------------------------------------------------------*/
#include "stm32f4xx.h"
#include "FreeRTOS.h"
#include "timers.h"
#include "GUI.h"


/* Exported types ------------------------------------------------------------*/
typedef struct{
	uint16_t x;
	uint16_t y;
}PointType;

typedef enum {FALSE=0, TRUE=!FALSE} bool;

/* Exported constants --------------------------------------------------------*/
//	IRQ	(any port will do)
#define XPT2046_IRQ_PORT	GPIOC
#define XPT2046_IRQ_PAD		GPIO_Pin_1
//	NSS	(any port will do, this is done manually)
#define XPT2046_NSS_PORT	GPIOC
#define XPT2046_NSS_PAD		GPIO_Pin_2


//	CLK	(must be a SPI clock port)
#define XPT2046_CLK_PORT	GPIOA
#define XPT2046_CLK_PAD		GPIO_Pin_5
//	DIN	(must be a SPI MOSI port)
#define XPT2046_DIN_PORT	GPIOA
#define XPT2046_DIN_PAD		GPIO_Pin_7
//	DOUT	(must be a SPI MISO port)
#define XPT2046_DOUT_PORT	GPIOA
#define XPT2046_DOUT_PAD	GPIO_Pin_6


/* Exported macro ------------------------------------------------------------*/
/* Exported define -----------------------------------------------------------*/
/* Exported functions ------------------------------------------------------- */
/*void Touch_Init();
void Touch_Calibrate(uint8_t point);
int Touch_GetPos();
PointType Touch_Read();

void ADC_Enable();
u16 readADC(u8 channel);*/


void XPT2046_Init();
int XPT2046_GetCoord( uint16_t * pX , uint16_t * pY );
int XPT2046_GetAvgCoord( uint16_t * pX , uint16_t * pY , uint16_t nSamples );
void spiExchange(u8 num, u8 *tx, u8 *rx);
void XPT2046_GetCursor(uint16_t * pX , uint16_t * pY);

void vTouchscreenTimerCallback( xTimerHandle pxTimer );

#endif //TOUCH_H
