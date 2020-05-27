/* Includes ------------------------------------------------------------------*/
#include "touch.h"

#include "stm32f4xx.h"
#include "stm32f4xx_conf.h"
#include "system_stm32f4xx.h"

#include "stm32f4xx_adc.h"
#include "stm32f4xx_flash.h"
#include "stm32f4xx_fsmc.h"
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

extern uint8_t SmallFont[];
extern uint8_t BigFont[];
extern uint8_t SevenSegNumFont[];
extern uint8_t font5x8[];
#define abs(x) ((x) >= 0 ? (x) : -(x))

/* Private typedef -----------------------------------------------------------*/
/* Private define ------------------------------------------------------------*/
#define X1	GPIO_Pin_2
#define X2 	GPIO_Pin_4
#define Y1	GPIO_Pin_1
#define Y2	GPIO_Pin_5


/* Private macro -------------------------------------------------------------*/
/* Private variables ---------------------------------------------------------*/
int32_t xd[3], yd[3], xt[3], yt[3], xc[3], yc[3];

PointType TouchPoint;
struct{
	float A;
	float B;
	float C;
	float D;
	float E;
	float F;
}Calibr;




/* Extern variables ----------------------------------------------------------*/
/* Private function prototypes -----------------------------------------------*/
/* Private functions ---------------------------------------------------------*/
/*******************************************************************************
* Function Name  : vApplicationIdleHook
* Description    :
*******************************************************************************/

void Touch_Init()
{
	ADC_Enable(ADC2);

	/*xd[0]=120; yd[0]=40;
	xd[1]=220; yd[1]=200;
	xd[2]=20;  yd[2]=360;

	xc[0]=115; yc[0]=70;
	xc[1]=177; yc[1]=200;
	xc[2]=50;  yc[2]=333;

	ILI9327_setColor(VGA_GREEN);
	for(uint8_t i=0; i<3; i++)
		ILI9327_Circle(xd[i], yd[i], 3);
*/
#if 0
	Calibr.A = Calibr.E = 1;
	Calibr.B = Calibr.C = Calibr.D = Calibr.F = 0;
#else
	Calibr.A = 1.55728;
	Calibr.B = -0.016047;
	Calibr.C = -58.3726;
	Calibr.D = 0.005397;
	Calibr.E = 1.20892;
	Calibr.F = -42.1718;
#endif
}


void Touch_Calibrate(uint8_t point)
{
	float a, b;

	switch(point)
	{
	case 0:
	case 1:
	case 2:
		while(abs(xt[point]-xc[point])>15 || abs(yt[point]-yc[point])>15)
		{
			if(Touch_GetPos())
			{
				xt[point] = TouchPoint.x;
				yt[point] = TouchPoint.y;
			}
		}
		break;

	case 3:
		a = (xd[0]*(yt[1]-yt[2])+xd[1]*(yt[2]-yt[0])+xd[2]*(yt[0]-yt[1]));
		b = (xt[0]*(yt[1]-yt[2])+xt[1]*(yt[2]-yt[0])+xt[2]*(yt[0]-yt[1]));
		Calibr.A = a/b;

		Calibr.B = (Calibr.A*(xt[2]-xt[1])+xd[1]-xd[2])/(yt[1]-yt[2]);
		Calibr.C = xd[2]-Calibr.A*xt[2]-Calibr.B*yt[2];

		a = (yd[0]*(yt[1]-yt[2])+yd[1]*(yt[2]-yt[0])+yd[2]*(yt[0]-yt[1]));
		b = (xt[0]*(yt[1]-yt[2])+xt[1]*(yt[2]-yt[0])+xt[2]*(yt[0]-yt[1]));
		Calibr.D =  a/b;

		Calibr.E = (Calibr.D*(xt[2]-xt[1])+yd[1]-yd[2])/(yt[1]-yt[2]);
		Calibr.F = yd[2]-Calibr.D*xt[2]-Calibr.E*yt[2];
		break;
	}

}

static bool firstPress = FALSE;
	static uint8_t pressCnt = 0;
	static PointType last;
int Touch_GetPos()
{
	GPIO_InitTypeDef GPIO_InitStructure;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_OUT;
	GPIO_InitStructure.GPIO_Pin = Y2 | Y1;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_100MHz;
	GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_NOPULL;
	GPIO_Init(GPIOC, &GPIO_InitStructure);

	GPIOC->BSRRH = Y1 | Y2;

	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN;
	GPIO_InitStructure.GPIO_Pin = X1 | X2;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_100MHz;
	GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_UP;
	GPIO_Init(GPIOC, &GPIO_InitStructure);

	vTaskDelay(1);



	if((GPIOC->IDR & X1) == 0)
	{
		vTaskDelay(1);
		PointType curTch;
		TouchPoint=curTch = Touch_Read();
		//return 1;//

		//if(!firstPress/* || abs(TouchPoint.x-lastX)>=2 || abs(TouchPoint.y-lastY)>=2*/)
		//{
			last.x += curTch.x;
			last.y += curTch.y;

			if(++pressCnt >= 1)
			{
				TouchPoint.x = last.x/1;
				TouchPoint.y = last.y/1;
				last.x = last.y = 0;
				pressCnt = 0;
				//firstPress = TRUE;
				return 1;
			}
			else
				return 0;



		/*}
		else
			return 0;*/
	}
	else
	{
		last.x = last.y = 0;
		pressCnt = 0;

		firstPress = FALSE;
		return 0;
	}
}

PointType Touch_Read()
{
	//extern uint8_t SmallFont[];
	PointType point;

	GPIO_InitTypeDef GPIO_InitStructure;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_OUT;
	GPIO_InitStructure.GPIO_Pin = X1 | X2;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_100MHz;
	GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_NOPULL;
	GPIO_Init(GPIOC, &GPIO_InitStructure);

	GPIOC->BSRRH = X1;
	GPIOC->BSRRL = X2;

	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AN;
	GPIO_InitStructure.GPIO_Pin = Y1 | Y2;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_100MHz;
	GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_NOPULL;
	GPIO_Init(GPIOC, &GPIO_InitStructure);

	vTaskDelay(3);

	uint16_t y = (4096-readADC(15))*400/4096;

	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_OUT;
	GPIO_InitStructure.GPIO_Pin = Y1 | Y2;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_100MHz;
	GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_NOPULL;
	GPIO_Init(GPIOC, &GPIO_InitStructure);

	GPIOC->BSRRH = Y1;
	GPIOC->BSRRL = Y2;

	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AN;
	GPIO_InitStructure.GPIO_Pin = X1 | X2;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_100MHz;
	GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_NOPULL;
	GPIO_Init(GPIOC, &GPIO_InitStructure);

	vTaskDelay(3);

	uint16_t x = readADC(14)*240/4096;

	point.x = Calibr.A*x + Calibr.B*y + Calibr.C;
	point.y = Calibr.D*x + Calibr.E*y + Calibr.F;

	return point;
}


void ADC_Enable()
{
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_ADC2, ENABLE);

	ADC_InitTypeDef ADC_InitStructure;
	ADC_CommonInitTypeDef ADC_CommonInitStructure;

	ADC_CommonInitStructure.ADC_Mode = ADC_Mode_Independent;
	ADC_CommonInitStructure.ADC_Prescaler = ADC_Prescaler_Div8;
	ADC_CommonInitStructure.ADC_DMAAccessMode = ADC_DMAAccessMode_Disabled;
	ADC_CommonInitStructure.ADC_TwoSamplingDelay = ADC_TwoSamplingDelay_5Cycles;
	ADC_CommonInit(&ADC_CommonInitStructure);

	ADC_InitStructure.ADC_Resolution = ADC_Resolution_12b;
	ADC_InitStructure.ADC_ScanConvMode = DISABLE;
	ADC_InitStructure.ADC_ContinuousConvMode = DISABLE;
	ADC_InitStructure.ADC_ExternalTrigConvEdge = ADC_ExternalTrigConvEdge_None;
	ADC_InitStructure.ADC_DataAlign = ADC_DataAlign_Right;
	ADC_InitStructure.ADC_NbrOfConversion = 1;

	ADC_Init(ADC2, &ADC_InitStructure);

	/* Enable the specified ADC*/
	ADC_Cmd(ADC2, ENABLE);
}


u16 readADC(u8 channel)
{
	ADC_RegularChannelConfig(ADC2, channel, 1, ADC_SampleTime_56Cycles);
	// Start the conversion
	ADC_SoftwareStartConv(ADC2);
	// Wait until conversion completion
	while (ADC_GetFlagStatus(ADC2, ADC_FLAG_EOC) == RESET);
	// Get the conversion value
	return 	(ADC_GetConversionValue(ADC2));
}



