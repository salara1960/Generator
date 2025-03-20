#include "kbd.h"

#ifdef SET_KBD

//-----------------------------------------------------------------------------

volatile bool lkey = false;
volatile bool rkey = false;
volatile bool mkey = false;

//-----------------------------------------------------------------------------

void EXTI7_0_IRQHandler(void) __attribute__((interrupt("WCH-Interrupt-fast")));
void EXTI7_0_IRQHandler(void)
{
    if (EXTI_GetITStatus(EXTI_Line2) != RESET) {
        if (!mkey && (evt == noneEvt)) {
            if (GPIO_ReadInputDataBit(GPIOA, GPIO_Pin_2)) {
                mkey = true;
                evt = encEvt;
            }
        }
        EXTI_ClearITPendingBit(EXTI_Line2);
	} else if (EXTI_GetITStatus(EXTI_Line1) != RESET) {
        if (!rkey && (evt == noneEvt)) {
            if (GPIO_ReadInputDataBit(GPIOA, GPIO_Pin_1)) {
                rkey = true;
                evt = ienEvt;
            }
        }
        EXTI_ClearITPendingBit(EXTI_Line1);
	} else if (EXTI_GetITStatus(EXTI_Line0) != RESET) {
        if (lkey && (evt == noneEvt)) {
            if (GPIO_ReadInputDataBit(GPIOA, GPIO_Pin_0)) {
                lkey = true;
                evt = ienEvt;
            }
        }
        EXTI_ClearITPendingBit(EXTI_Line0);
	}
}
//
void EXTI_PA012_Init(void)
{
    GPIO_InitTypeDef GPIO_InitStructure = {0};
    EXTI_InitTypeDef EXTI_InitStructure = {0};
    NVIC_InitTypeDef NVIC_InitStructure = {0};

    RCC->APB2PCENR |= RCC_APB2Periph_GPIOA;//RCC_APB2Periph_AFIO;

    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_0 | GPIO_Pin_1 | GPIO_Pin_2;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IPU;
    GPIO_Init(GPIOA, &GPIO_InitStructure);

    // GPIOA ----> EXTI_Line0
    GPIO_EXTILineConfig(GPIO_PortSourceGPIOA, GPIO_PinSource0);
    EXTI_InitStructure.EXTI_Line = EXTI_Line0;
    EXTI_InitStructure.EXTI_Mode = EXTI_Mode_Interrupt;
    EXTI_InitStructure.EXTI_Trigger = EXTI_Trigger_Rising;
    EXTI_InitStructure.EXTI_LineCmd = ENABLE;
    EXTI_Init(&EXTI_InitStructure);
    // GPIOA ----> EXTI_Line1
    GPIO_EXTILineConfig(GPIO_PortSourceGPIOA, GPIO_PinSource1);
    EXTI_InitStructure.EXTI_Line = EXTI_Line1;
    EXTI_InitStructure.EXTI_Mode = EXTI_Mode_Interrupt;
    EXTI_InitStructure.EXTI_Trigger = EXTI_Trigger_Rising;
    EXTI_InitStructure.EXTI_LineCmd = ENABLE;
    EXTI_Init(&EXTI_InitStructure);
    // GPIOA ----> EXTI_Line2
    GPIO_EXTILineConfig(GPIO_PortSourceGPIOA, GPIO_PinSource2);
    EXTI_InitStructure.EXTI_Line = EXTI_Line2;
    EXTI_InitStructure.EXTI_Mode = EXTI_Mode_Interrupt;
    EXTI_InitStructure.EXTI_Trigger = EXTI_Trigger_Rising;
    EXTI_InitStructure.EXTI_LineCmd = ENABLE;
    EXTI_Init(&EXTI_InitStructure);

    NVIC_InitStructure.NVIC_IRQChannel = EXTI7_0_IRQn;
    NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 1;
    NVIC_InitStructure.NVIC_IRQChannelSubPriority = 2;
    NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
    NVIC_Init(&NVIC_InitStructure);
}
//--------------------------------------------------------------------

#endif

