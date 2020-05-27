#include "cnc_engine.h"

#include "stm32f4xx.h"
#include "stm32f4xx_gpio.h"
#include "stm32f4xx_rcc.h"

enum
{
    CNC_STEP = 0,
    CNC_DIR,
    CNC_ENA,
};

#define GPIO_OUT(port, pin, value) *((volatile uint32_t *)&port->BSRRL) = ((value ? 1 : 0x100) << pin)

const uint16_t cnc_pins[3][3] = {
    {13, 14, 15}, // step
    {4, 5, 6},    // dir
    {0, 1, 2},    // enable
};

const GPIO_TypeDef *cnc_ports[3][3] = {
    {GPIOC, GPIOC, GPIOC}, // step
    {GPIOE, GPIOE, GPIOE}, // dir
    {GPIOE, GPIOE, GPIOE}, // enable
};

void cnc_engine_init(void)
{
    RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOB, ENABLE);
    RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOC, ENABLE);
    RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOD, ENABLE);
    RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOE, ENABLE);

    GPIO_InitTypeDef GPIO_InitStructure;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_OUT;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_13 | GPIO_Pin_14 | GPIO_Pin_15;
    GPIO_Init(GPIOC, &GPIO_InitStructure);

    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_0 | GPIO_Pin_1 | GPIO_Pin_2 | GPIO_Pin_4 | GPIO_Pin_5 | GPIO_Pin_6;
    GPIO_Init(GPIOE, &GPIO_InitStructure);

    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN;
    GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_DOWN;
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_4 | GPIO_Pin_5 | GPIO_Pin_7 | GPIO_Pin_8;
    GPIO_Init(GPIOB, &GPIO_InitStructure);

    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_7;
    GPIO_Init(GPIOD, &GPIO_InitStructure);

    cnc_engine_enable(false);
}

void cnc_engine_enable(bool state)
{
    for(uint32_t i = 0; i < CNC_AXIS_COUNT; i++)
        cnc_engine_enable_single(i, state);
}

void cnc_engine_enable_single(int axis, bool state)
{
    if(axis >= CNC_AXIS_COUNT) return;
    GPIO_OUT(cnc_ports[CNC_ENA][axis], cnc_pins[CNC_ENA][axis], !state);
}