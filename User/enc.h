#ifndef __ENC_H_
#define __ENC_H_

#include "hdr.h"

#ifdef SET_ENCODER
    extern volatile uint8_t evt;

    void TIM2_Encoder_Init();
    void EXTI_PA2_Init(void);

#endif


#endif
