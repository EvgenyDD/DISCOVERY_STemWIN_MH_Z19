/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef TOUCH_H
#define TOUCH_H

/* Includes ------------------------------------------------------------------*/
#include "stm32f4xx.h"


/* Exported types ------------------------------------------------------------*/
typedef struct{
	uint16_t x;
	uint16_t y;
}PointType;

typedef enum {FALSE=0, TRUE=!FALSE} bool;

/* Exported constants --------------------------------------------------------*/
/* Exported macro ------------------------------------------------------------*/
/* Exported define -----------------------------------------------------------*/
/* Exported functions ------------------------------------------------------- */
void Touch_Init();
void Touch_Calibrate(uint8_t point);
int Touch_GetPos();
PointType Touch_Read();

void ADC_Enable();
u16 readADC(u8 channel);


#endif //TOUCH_H
