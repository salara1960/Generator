/*
 * !!!!IMPORTANT!!!!
 * Setup Hardware SPI to POLATRITY HIGH, PHASE 1 EDGE
 *
 * Offical Documents:
 * https://www.analog.com/media/en/technical-documentation/application-notes/AN-1070.pdf
 * https://www.analog.com/media/en/technical-documentation/data-sheets/AD9833.pdf
 *
 */

#include "AD9833.h"

#ifdef SET_GEN

//-----------------------------------------------------------------------------------------

uint8_t _waveform = WAVEFORM_SQUARE; //WAVEFORM_SINE;
uint8_t _sleep_mode = NO_POWERDOWN;
uint8_t _freq_source = 0;
uint8_t _phase_source = 0;
uint8_t _reset_state = 0;
const float_t pow2_28 = (float)(1 << 28);
#ifdef SET_WITH_DMA
	bool tx_done = true;
#endif


#ifdef SET_DBG
	bool dbg = false;
#endif

//-----------------------------------------------------------------------------------------
const char *formName(uint8_t wf)
{
	switch (wf) {
		case wave_triangle:
			return "triangle";
		case wave_square:
			return "square";
		case wave_sine:
			return "sine"; 
	}
	return "???";
} 
//-----------------------------------------------------------------------------------------
static void AD9833_WriteRegister(uint16_t data)
{
	AD9833_Select();
#ifdef SET_WITH_DMA
		tx_done = false;
		DMA_Tx_Init(DMA1_Channel6, (uint32_t)&SPI1->DATAR, (uint32_t)data, 1);
		DMA1_Channel6->CFGR |= DMA_CFGR1_EN;//DMA_Cmd(DMA1_Channel6, ENABLE);// Start channel6
		while (!tx_done) {Delay_Us(1)};
#else	
		SPI_I2S_SendData(SPI1, data >> 8);
		SPI_I2S_SendData(SPI1, data & 0xff);
		AD9833_Unselect();
#endif
	
}
//-----------------------------------------------------------------------------------------
/*
 * @brief Update Control Register Bits
 */
static void AD9833_WriteCfgReg(void)
{
uint16_t cfg = 0;

	cfg |= _waveform;
	cfg |= _sleep_mode;
	cfg |= (_freq_source ? F_SELECT_CFG : 0);	//it's unimportant because don't use FREQ1
	cfg |= (_phase_source ? P_SELECT_CFG : 0);	//it's unimportant because don't use PHASE1
	cfg |= (_reset_state ? RESET_CFG : 0);
	cfg |= B28_CFG;

#ifdef SET_DBG
	if (dbg) printf("[%s] cfg_reg:%u/0x%X%s", __func__, cfg, cfg, eol);
#endif

	AD9833_WriteRegister(cfg);
}
//-----------------------------------------------------------------------------------------
void AD9833_SetWaveform(WaveDef Wave)
{
	     if (Wave == wave_sine)     _waveform = WAVEFORM_SINE;
	else if (Wave == wave_square) 	_waveform = WAVEFORM_SQUARE;
	else if (Wave == wave_triangle)	_waveform = WAVEFORM_TRIANGLE;

#ifdef SET_DBG
	if (dbg) printf("[%s] Wave:%u set waveform:%u/0x%X%s", __func__, Wave, _waveform, _waveform, eol);
#endif

	AD9833_WriteCfgReg();
}
//-----------------------------------------------------------------------------------------
void AD9833_SetPhase(uint16_t phase_deg)
{
	     if (phase_deg < 0) phase_deg = 0;
	else if (phase_deg > 360) phase_deg %= 360;

	//uint16_t phase_val  = ((uint16_t)(phase_deg * BITS_PER_DEG)) &  0xFFF;
	uint16_t phase_val  = (phase_deg * 11) & 0xFFF;

	AD9833_WriteRegister(PHASE0_REG | phase_val);
}
//-----------------------------------------------------------------------------------------
void AD9833_SetFrequency(uint32_t freq)
{
	// TODO: calculate max frequency based on refFrequency.
	// Use the calculations for sanity checks on numbers.
	// Sanity check on frequency: Square - refFrequency / 2
	// Sine/Triangle - refFrequency / 4

	if (freq > (FMCLK >> 1)) freq = FMCLK >> 1;	//bitwise FMCLK / 2
	else
	if (freq < 0) freq = 0;

	//uint32_t bit32 = ((freq * pow2_28) / FMCLK) + 1;
	//uint16_t LSB = (uint16_t)(bit32 & 0x3FFF) | FREQ0_REG;
	//uint16_t MSB = (uint16_t)(bit32 >> 14) | FREQ0_REG;
	uint32_t bit32 = (uint32_t)(((freq * pow2_28) / FMCLK)) + 1;
	uint16_t LSB = FREQ0_REG | (uint16_t)(bit32 & 0x3FFF);
	uint16_t MSB = FREQ0_REG | (uint16_t)(bit32 >> 14);

	AD9833_WriteCfgReg();
	AD9833_WriteRegister(LSB);
	AD9833_WriteRegister(MSB);

#ifdef SET_DBG
	if (dbg) printf("[%s] freq:%u freq_reg:%u/0x%08X LSB:0x%04X MSB:0x%04X%s", __func__, freq, bit32, bit32, LSB, MSB, eol);
#endif	
}
//-----------------------------------------------------------------------------------------
/*
void AD9833_SleepMode(uint8_t mode)
{
	_sleep_mode = mode;

	AD9833_WriteCfgReg();
}
*/
//-----------------------------------------------------------------------------------------
void AD9833_OutputEnable(uint8_t output_state)
{
	_reset_state = !output_state;

	AD9833_WriteCfgReg();
}
//-----------------------------------------------------------------------------------------
void AD9833_Init(WaveDef Wave, uint32_t freq, uint16_t phase_deg)
{
	AD9833_OutputEnable(0);

	AD9833_SetWaveform(Wave);
	AD9833_WriteCfgReg();
	AD9833_SetFrequency(freq);
	AD9833_SetPhase(phase_deg);
	
	AD9833_OutputEnable(1);
}
//-----------------------------------------------------------------------------------------
/*
void AD9833_setFreqWavePhase(uint32_t freq, WaveDef Wave, uint16_t phase)
{
	AD9833_SetFrequency(freq);
	AD9833_SetPhase(phase);
	AD9833_SetWaveform(Wave);

}
*/
//-----------------------------------------------------------------------------------------

#endif
