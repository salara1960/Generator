#ifndef __AD9833_H_
#define __AD9833_H_

#include "hdr.h"


#ifdef SET_GEN


/*** Control Register Bits (DataSheet AD9833 p. 14, Table 6) ***/
#define B28_CFG					(1 << 13)
#define HLB_CFG		  			(1 << 12)
#define F_SELECT_CFG			(1 << 11)
#define P_SELECT_CFG			(1 << 10)
#define RESET_CFG				(1 << 8)
#define SLEEP1_CFG				(1 << 7)
#define SLEEP12_CFG				(1 << 6)
#define OPBITEN_CFG				(1 << 5)
#define DIV2_CFG				(1 << 3)
#define MODE_CFG				(1 << 1)

/*** Bitmask to register access ***/
#define FREQ0_REG				0x4000
#define PHASE0_REG				0xC000
//#define FREQ1_ACCESS  			0x8000
//#define PHASE1_ACCESS 			0xE000

/*** Waveform Types (DataSheet p. 16, Table 15) ***/
#define WAVEFORM_SINE         	0
#define WAVEFORM_TRIANGLE     	MODE_CFG
#define WAVEFORM_SQUARE       	OPBITEN_CFG | DIV2_CFG
#define WAVEFORM_SQUARE_DIV2  	OPBITEN_CFG

/*** Sleep Modes ***/
#define NO_POWERDOWN	  		0
#define DAC_POWERDOWN			SLEEP12_CFG
#define CLOCK_POWERDOWN			SLEEP1_CFG
#define FULL_POWERDOWN			SLEEP12_CFG | SLEEP1_CFG


//#define FMCLK	 				25000000
//#define BITS_PER_DEG 			11.3777777777778	// 4096 / 360

#ifdef SET_HARD_CS
	#define AD9833_Select()   {}//GPIO_ResetBits(GPIOA, AD9833_CS_PIN); //set to 0
	#define AD9833_Unselect() {}//GPIO_SetBits  (GPIOA, AD9833_CS_PIN); //set to 1
#else
	#define AD9833_Select()   GPIO_ResetBits(GPIOA, AD9833_CS_PIN); //set to 0
	#define AD9833_Unselect() GPIO_SetBits  (GPIOA, AD9833_CS_PIN); //set to 1
#endif

typedef enum {
	wave_triangle,
	wave_square,
	wave_sine,
	wave_none
} WaveDef;

#ifdef SET_WITH_DMA
	extern bool tx_done;
#endif

const char *formName(uint8_t wf);

#ifdef SET_DBG
	extern const char *eol;
	extern bool dbg;
#endif

//void AD9833_SetCfgReg(uint16_t val);
void AD9833_SetFrequency(uint32_t freq);
void AD9833_SetWaveform(WaveDef Wave);
//void AD9833_SetPhase(uint16_t phase_deg);
void AD9833_Init(WaveDef Wave, uint32_t freq, uint16_t phase_deg);
//void AD9833_OutputEnable(uint8_t output_state);
//void AD9833_SleepMode(uint8_t mode);
//void AD9833_setFreqWavePhase(uint32_t freq, WaveDef Wave, uint16_t phase_deg);

#endif

#endif /* INC_AD9833_H_ */
