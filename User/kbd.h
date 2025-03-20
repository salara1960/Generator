#ifndef __KBD_H_
#define __KBD_H_

#include "hdr.h"

#ifdef SET_KBD
    extern volatile bool lkey, rkey, mkey;
    extern volatile uint8_t evt;

    void EXTI_PA012_Init(void);
#endif


#endif