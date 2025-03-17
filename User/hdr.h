/*
 * hrd.h
 *
 *  Created on: Dec 29, 2024
 *      Author: alarm
 */

#ifndef USER_HDR_H_
#define USER_HDR_H_


#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdarg.h>
#include <ctype.h>
#include <math.h>
#include <time.h>
#include "string.h"
#include <ch32x035.h>
#include "debug.h"


//#define SET_SSD1306
#define SET_SSD1306_SPI
#define SET_GEN
#define SET_ENCODER

#if defined(SET_SSD1306) || defined(SET_SSD1306_SPI)
	#define OLED_128x32
	#ifdef SET_SSD1306
		#define I2C_BaudRate 400000 //80000	
		//#define SET_SCROLL_MODE
	#endif
    #include "ssd1306.h"
	#ifdef SET_SSD1306_SPI
		#define DC_Pin GPIO_Pin_3  //PB3
		#define CS_Pin GPIO_Pin_11 //PB11
		#define RS_Pin GPIO_Pin_6  //PA6
	#endif
#endif
	
#ifdef SET_GEN
	//#define SET_HARD_CS
	#define AD9833_CS_PIN    GPIO_Pin_4
	#define SET_DBG
	//#define SET_WITH_DMA
	
	#include "AD9833.h"
	
	typedef struct {
		uint32_t cel;
		uint32_t dro;
	} s_float_t;
	#ifdef SET_WITH_DMA
		extern void DMA_Tx_Init(DMA_Channel_TypeDef *DMA_CHx, u32 ppadr, u32 memadr, u16 bufsize);
	#endif
#endif

#ifdef SET_ENCODER
	#include "enc.h"
#endif


#define FMCLK	 25000000
#define LOOP_FOREVER() while(1) { Delay_Ms(1); }


enum {
	noneEvt = 0,
	rstEvt,
	encEvt,
	ienEvt,
	secEvt,
	getEvt,
	freqEvt,
	formEvt,
	errEvt
};

enum {
	mFreq = 0,
	mForm,
	mStep
};


#endif /* USER_HDR_H_ */
