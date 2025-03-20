/********************************** (C) COPYRIGHT *******************************
 * File Name          : main.c
 * Author             : Alarm
 * Version            : V1.0.0
 * Date               : 2025/3/13
 * Description        : Main program body.
*********************************************************************************
* Copyright (c) 2021 Nanjing Qinheng Microelectronics Co., Ltd.
* Attention: This software (modified or not) and binary are used for
* microcontroller manufactured by Nanjing Qinheng Microelectronics.
*******************************************************************************/

// ls -la | grep "${BuildArtifactFileBaseName}.*"

/*

PA0, PA1 - encoder left, right
PA2 -- encoder key

*/

//------------------------------------------------------------------------------

#include "hdr.h"

//------------------------------------------------------------------------------

const char *eol = "\n";
const char *uname = "RISC-V CH32X035";
uint8_t RxBuff[64] = {0};
volatile uint8_t evt = noneEvt;
volatile int8_t ind = 0;
volatile uint32_t epoch = 1742473555;//1742299655;//1742214977;
//1742062635;//1741965599;//1741857599;//1741692980;//1741608544;//1741292299;//1741268699;
volatile uint32_t seconda = 0;
bool set_time = true;
uint16_t arr = 20 - 1;//40 - 1;
uint16_t psc =  48000 - 1;
static uint8_t led = 0;
GPIO_TypeDef *LedPort = GPIOB;
uint16_t LedPin = GPIO_Pin_12;//led pin
char tmp[64] = {0};
bool inv = false;

#ifdef SET_GEN
    uint32_t freq = 32768;//4625000;//4625 KHz - UVB-76
    uint8_t waveform_select = wave_square;
    uint16_t phase = 0;
	s_float_t flo = { 0, 0 };
	//-----------------------------------------------------------------------------
	void floatPart(float val, s_float_t *part)
	{
		part->cel = (uint32_t)val;
		part->dro = (uint32_t)(part->cel % 1000); 
	}
#endif


#if defined(SET_ENCODER) || defined(SET_KBD)
    const uint8_t max_menu_item = 3;
    int menu_idx = -1;
    const char *menu[] = {"Freq", "Form", "Step"};
    const uint8_t menu_bin[] = {mFreq, mForm, mStep};

    const uint8_t max_form_item = 3;
    int form_idx = 1;
    const char *allForm[] = {"triangle", "square", "sine"};
    const WaveDef allForm_bin[] = {wave_triangle, wave_square, wave_sine};

    const uint8_t max_step_item = 7;
    int step_idx = 0;
    const char *allStep[] = {"1Hz", "10Hz", "100Hz", "1KHz", "10KHz", "100KHz", "1MHz"};
    const uint32_t allStep_bin[] = {1, 10, 100, 1000, 10000, 100000, 1000000};
    uint32_t step = 1;
#endif


//********************************************************************

void Leds();

//********************************************************************
void InitUSART4(uint32_t speed)
{
    GPIO_InitTypeDef  GPIO_InitStructure = {0};
    USART_InitTypeDef USART_InitStructure = {0};

    RCC->APB2PCENR |= RCC_APB2Periph_GPIOB;
    RCC->APB1PCENR |= RCC_APB1Periph_USART4;
    AFIO->PCFR1 |= AFIO_PCFR1_USART4_REMAP;
    
    // USART4 TX-->B.0   RX-->B.1
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_0;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;
    GPIO_Init(GPIOB, &GPIO_InitStructure);
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_1;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;//GPIO_Mode_IN_FLOATING;
    GPIO_Init(GPIOB, &GPIO_InitStructure);

    USART_InitStructure.USART_BaudRate = speed;//115200;
    USART_InitStructure.USART_WordLength = USART_WordLength_8b;
    USART_InitStructure.USART_StopBits = USART_StopBits_1;
    USART_InitStructure.USART_Parity = USART_Parity_No;
    USART_InitStructure.USART_HardwareFlowControl = USART_HardwareFlowControl_None;
    USART_InitStructure.USART_Mode = USART_Mode_Tx | USART_Mode_Rx;
    USART_Init(USART4, &USART_InitStructure);

    USART_ITConfig(USART4, USART_IT_RXNE, ENABLE);
    NVIC_EnableIRQ(USART4_IRQn);
    
    USART_Cmd(USART4, ENABLE);
}
//------------------------------------------------------------------------------
void USART4_IRQHandler(void) __attribute__((interrupt("WCH-Interrupt-fast")));
void USART4_IRQHandler(void)
{
    if (USART_GetFlagStatus(USART4, USART_FLAG_RXNE) != RESET) {
        uint8_t ch = USART_ReceiveData(USART4);
        RxBuff[ind] = ch;
        USART_SendData(USART4, RxBuff[ind]);
        ind++;
        if ((ch == 0x0a) || (ch == 0x0d)) {    
            RxBuff[++ind] = 0;           
            char *uk = NULL;
            if (strstr((char *)RxBuff, "rst")) {
                evt = rstEvt;
            } else if (strstr((char *)RxBuff, "get")) {
        		    evt = getEvt;
        	} else if ((uk = strstr((char *)RxBuff, "epoch="))) {
                epoch = atol(uk + 6);
                set_time = true;                
            }
#ifdef SET_GEN
            else if ((uk = strstr((char *)RxBuff, "freq="))) {
        		freq = atol(uk + 5);
				evt = freqEvt;
            } else if ((uk = strstr((char *)RxBuff, "form="))) {
        		waveform_select = atol(uk + 5);
				evt = formEvt;
        	} else if ((uk = strstr((char *)RxBuff, "step="))) {
        		step = atol(uk + 5);
                evt = errEvt;
                int i =-1;
                while (++i < max_step_item) {
                    if (step == allStep_bin[i]) {
                        step_idx = i;
                        evt = formEvt;
                        break;
                    }
                }
        	} else if ((uk = strstr((char *)RxBuff, "dbg="))) {
                uk += 4;
                if (strstr(uk, "on")) dbg = true;
                else
                if (strstr(uk, "off")) dbg = false;
                else
                evt = errEvt;
            }
#endif
            else evt = errEvt;
            ind = 0;
            memset(RxBuff, 0, sizeof(RxBuff));
        }
    }
}
//------------------------------------------------------------------------------
void TIM1_UP_IRQHandler(void) __attribute__((interrupt("WCH-Interrupt-fast")));
void TIM1_UP_IRQHandler(void)
{
    if (TIM_GetITStatus(TIM1, TIM_IT_Update) == SET) {
        Leds();
        if (set_time) {
            seconda = epoch + 1;
            set_time = false;
        }
        seconda++;      
        evt = secEvt;
    }
    TIM_ClearITPendingBit(TIM1, TIM_IT_Update);
}
//
void TIM1_Init(void)//u16 arr, u16 psc)
{
    NVIC_InitTypeDef NVIC_InitStructure={0};
    TIM_TimeBaseInitTypeDef TIM_TimeBaseInitStructure={0};

    RCC->APB2PCENR |= RCC_APB2Periph_TIM1;

    TIM_TimeBaseInitStructure.TIM_Period = arr;
    TIM_TimeBaseInitStructure.TIM_Prescaler = psc;
    TIM_TimeBaseInitStructure.TIM_ClockDivision = TIM_CKD_DIV1;
    TIM_TimeBaseInitStructure.TIM_CounterMode = TIM_CounterMode_Up;
    TIM_TimeBaseInitStructure.TIM_RepetitionCounter = 50;
    TIM_TimeBaseInit(TIM1, &TIM_TimeBaseInitStructure);

    TIM1->INTFR = (uint16_t)~TIM_IT_Update; //TIM_ClearITPendingBit(TIM1, TIM_IT_Update);
    
    NVIC_InitStructure.NVIC_IRQChannel = TIM1_UP_IRQn;
    NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0;
    NVIC_InitStructure.NVIC_IRQChannelSubPriority = 1;
    NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
    NVIC_Init(&NVIC_InitStructure);

    TIM1->DMAINTENR |= TIM_IT_Update; //IM_ITConfig(TIM1, TIM_IT_Update, ENABLE);
    TIM1->CTLR1 |= TIM_CEN; //TIM_Cmd(TIM1, ENABLE);
}
//------------------------------------------------------------------------------
void GPIO_ConfigOut(GPIO_TypeDef *port, uint16_t pin)
{
GPIO_InitTypeDef GPIO_InitStructure = {0}; //structure variable used for the GPIO configuration
uint32_t bus = RCC_APB2Periph_GPIOA;

    if (port == GPIOB) bus = RCC_APB2Periph_GPIOB;
    else
    if (port == GPIOC) bus = RCC_APB2Periph_GPIOC;
    RCC->APB2PCENR |= bus;

    GPIO_InitStructure.GPIO_Pin = pin; // Defines which Pin to configure
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP; // Defines Output Type
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz; // Defines speed
    GPIO_Init(port, &GPIO_InitStructure);
}
//------------------------------------------------------------------------------
void Leds()
{
    if (led) GPIO_ResetBits(LedPort, LedPin);
        else GPIO_SetBits  (LedPort, LedPin);
    led = ~led;
}
//------------------------------------------------------------------------------
uint32_t get_sec(uint32_t t)
{
	return (seconda + t);
}
//
int check_sec(uint32_t t)
{
	return (get_sec(0) >= t) ? 1 : 0;
}
//------------------------------------------------------------------------------
int calcTime(uint32_t sec, char *st)
{	
    sec %= (60 * 60 * 24);
	uint32_t hour = sec / (60 * 60);
	sec %= (60 * 60);
	uint32_t min = sec / (60);
	sec %= 60;

    return sprintf(st, "UTC %02u:%02u:%02u", hour, min, sec);
}
//------------------------------------------------------------------------------
#if defined(SET_GEN) || defined(SET_SSD1306_SPI)
    void spi_init(void)
    {
	    RCC->APB2PCENR |= RCC_APB2Periph_GPIOA | RCC_APB2Periph_SPI1;
    
        #ifdef SET_SSD1306_SPI
            RCC->APB2PCENR |= RCC_APB2Periph_GPIOB;
        #endif
	
        GPIO_InitTypeDef GPIO_InitStructure = {0};
        #ifdef SET_GEN
            GPIO_InitStructure.GPIO_Pin = AD9833_CS_PIN; // Defines which Pin to configure - CS pin
        
            #ifdef SET_HARD_CS    
                GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP; // Defines Output Type
            #else
                GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP; // Defines Output Type
            #endif
            GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz; // Defines speed
            GPIO_Init(GPIOA, &GPIO_InitStructure);
        #endif
    #ifdef SET_SSD1306_SPI
        GPIO_InitStructure.GPIO_Pin = CS_Pin | DC_Pin | RS_Pin;
        GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
        GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz; // Defines speed
        GPIO_Init(GPIOB, &GPIO_InitStructure);
        GPIO_InitStructure.GPIO_Pin = RS_Pin;
        GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
        GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz; // Defines speed
        GPIO_Init(GPIOA, &GPIO_InitStructure);
    #endif
    
        //GPIO_InitStructure.GPIO_Pin = GPIO_Pin_6;// Defines which Pin to configure - MISO
        //GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;
        //GPIO_Init(GPIOA, &GPIO_InitStructure);
        GPIO_InitStructure.GPIO_Pin = GPIO_Pin_7; // Defines which Pin to configure : PA7-MOSI, PA5-SCK
        GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP; // Defines Output Type
        GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz; // Defines speed
        GPIO_Init(GPIOA, &GPIO_InitStructure);

        GPIO_InitStructure.GPIO_Pin = GPIO_Pin_5; // Defines which Pin to configure - SCK
        GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP; // Defines Output Type
        GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz; // Defines speed
        GPIO_Init(GPIOA, &GPIO_InitStructure);

        #ifdef SET_HARD_CS
            SPI_SSOutputCmd(SPI1, ENABLE);//for hard NSS
        #else
            #ifdef SET_GEN
                AD9833_Unselect();
            #endif
            #ifdef SET_SSD1306_SPI
                SSD1306_UNSELECT();
                SSD1306_DC_CMD();
                SSD1306_RST_OFF();
            #endif
        #endif

        SPI_InitTypeDef spi_def = {0};
        spi_def.SPI_Direction = SPI_Direction_1Line_Tx;//SPI_Direction_2Lines_FullDuplex;
        spi_def.SPI_Mode = SPI_Mode_Master;
        spi_def.SPI_DataSize = SPI_DataSize_8b;
        #ifdef SET_HARD_CS
            spi_def.SPI_NSS = SPI_NSS_Hard;
        #else
            spi_def.SPI_NSS = SPI_NSS_Soft;
        #endif
        spi_def.SPI_CPOL = SPI_CPOL_High;
        spi_def.SPI_CPHA = SPI_CPHA_1Edge;
        spi_def.SPI_BaudRatePrescaler = SPI_BaudRatePrescaler_8;
        spi_def.SPI_FirstBit = SPI_FirstBit_MSB;
        spi_def.SPI_CRCPolynomial = 0;
        SPI_Init(SPI1, &spi_def);
        #ifdef SET_WITH_DMA
            SPI1->CTLR2 |= SPI_I2S_DMAReq_Tx;//SPI_I2S_DMACmd(SPI1, ENABLE);
        #endif

	    // enable SPI port
        SPI1->CTLR1 |= CTLR1_SPE_Set;//SPI_Cmd(SPI1, ENABLE);
    }
#endif
//------------------------------------------------------------------------------
void prnInfo()
{
    printf("ChipID:0x%X, SystemClk:%uHz, Seconda:%u%s", DBGMCU_GetCHIPID(), SystemCoreClock, get_sec(0), eol);
}
//------------------------------------------------------------------------------
int main(void)
{
    NVIC_PriorityGroupConfig(NVIC_PriorityGroup_1);
    SystemCoreClockUpdate();    
    GPIO_ConfigOut(LedPort, LedPin);
    Delay_Init();
    TIM1_Init();
    InitUSART4(115200);
    
    Delay_Ms(1000);

    prnInfo();

#if defined(SET_GEN) || defined(SET_SSD1306_SPI)
    spi_init();
#endif

    Delay_Ms(10);

#ifdef SET_GEN
    AD9833_Init(waveform_select, freq, phase);
    printf("Set frequency to %u Hz and form '%s' type%s", freq, formName(waveform_select), eol);
#endif 

#if defined(SET_SSD1306) || defined(SET_SSD1306_SPI)
    #ifdef SET_SSD1306
        I2C_IFS_Init(I2C_BaudRate, OLED_I2C_ADDR);
    #endif

    OLED_Init();

    Delay_Ms(50);
    OLED_text_xy(tmp, OLED_calcx(sprintf(tmp, "%s", uname)), LAST_LINE, inv);
    #ifdef SET_GEN
        Delay_Ms(100);
        float fr = freq;
		floatPart(fr, &flo);
        OLED_text_xy(tmp, OLED_calcx(sprintf(tmp, "KHz:%u.%u", flo.cel / 1000, flo.dro)), LAST_LINE - 2, inv);
        OLED_text_xy(tmp, OLED_calcx(sprintf(tmp, "%s:%s", allStep[step_idx], formName(waveform_select))), LAST_LINE - 1, inv);
    #endif
#endif

#ifdef SET_ENCODER
    TIM2_Encoder_Init();
    EXTI_PA2_Init();

    int16_t tc = 0;
    int16_t last_tc = 0;
    uint16_t count = TIM2->CNT;
	uint16_t last_count = TIM2->CNT;
    uint32_t tmr = 0;
#else
    #ifdef SET_KBD
        EXTI_PA012_Init();

        uint32_t tmr = 0;
    #endif
#endif
 

    while (1) {
        switch (evt) {
            case errEvt:
				printf("Error command !%s", eol);
				evt = noneEvt;
			break;
            case rstEvt:
#if defined(SET_SSD1306) || defined(SET_SSD1306_SPI)        
                OLED_Clear();
#endif
				printf("Restart...%s%s", eol, eol);
                Delay_Ms(100);
		        NVIC_SystemReset();
                evt = noneEvt;
            break;
            case secEvt:
#if defined(SET_SSD1306) || defined(SET_SSD1306_SPI)
			    OLED_text_xy(tmp, OLED_calcx(calcTime(get_sec(0), tmp)), 1, inv);	
#endif
        		evt = noneEvt;
        	break;
            case getEvt:
#ifdef SET_GEN			
        		printf("Time:%u sec, freq:%u Hz, form:'%s'%s", 
                       get_sec(0), freq, formName(waveform_select), eol);
#else
                prnInfo();
    #ifdef SET_SSD1306_SPI
                OLED_text_xy(tmp, OLED_calcx(sprintf(tmp, "%s", uname)), LAST_LINE, inv);
    #endif
#endif
        		evt = noneEvt;
        	break;
            case freqEvt:
#ifdef SET_GEN
                AD9833_SetFrequency(freq); 
                printf("Set frequency to %u Hz%s", freq, eol);
	#if defined(SET_SSD1306) || defined(SET_SSD1306_SPI)
				OLED_clear_line(LAST_LINE - 2, inv);
				if (freq >= 1000) {
					float fr = freq;
					floatPart(fr, &flo);
				    OLED_text_xy(tmp, OLED_calcx(sprintf(tmp, "KHz:%u.%u", flo.cel / 1000, flo.dro)), LAST_LINE - 2, inv);
				} else {
				 	OLED_text_xy(tmp, OLED_calcx(sprintf(tmp, "Hz:%u", freq)), LAST_LINE - 2, inv);
				}
	#endif
    #ifdef SET_KBD
                Delay_Ms(500);
    #endif
#endif
				evt = noneEvt;
			break;
			case formEvt:
#ifdef SET_GEN			
				AD9833_SetWaveform(waveform_select);
                printf("Set waveform:'%s', step:%u Hz%s", formName(waveform_select), allStep_bin[step_idx], eol);
	#if defined(SET_SSD1306) || defined(SET_SSD1306_SPI)
				OLED_clear_line(LAST_LINE - 1, inv);
				OLED_text_xy(tmp, OLED_calcx(sprintf(tmp, "%s:%s", allStep[step_idx], formName(waveform_select))), LAST_LINE - 1, inv);
	#endif
    #ifdef SET_KBD
                Delay_Ms(500);
    #endif
#endif
				evt = noneEvt;
			break;
            case encEvt:
#if defined(SET_ENCODER) || defined(SET_KBD)
                menu_idx++;
                if (menu_idx >= max_menu_item) menu_idx = 0;
                OLED_clear_line(LAST_LINE, inv);
                OLED_text_xy(tmp, OLED_calcx(sprintf(tmp, "%s", menu[menu_idx])), LAST_LINE, inv);
    #ifdef SET_KBD
                mkey = false;
    #endif
                tmr = get_sec(8);
                Delay_Ms(50);
#endif
                evt = noneEvt;
            break;
            case ienEvt:
#ifdef SET_ENCODER
                switch (menu_idx) {
                    case mFreq:
                        if (tc > last_tc) {
                            freq += allStep_bin[step_idx];
                            if (freq <= FMCLK >> 1) evt = freqEvt;
                        } else {
                            if (freq >= allStep_bin[step_idx]) {
                                freq -= allStep_bin[step_idx];
                                evt = freqEvt;
                            }
                        }
                    break;
                    case mForm:
                        if (tc > last_tc) {
                            waveform_select++;
                            if (waveform_select >= max_form_item) waveform_select = 0; 
                        } else {
                            if (!waveform_select) waveform_select = max_form_item - 1;
                            else
                            waveform_select--;  
                        }
                        evt = formEvt;
                    break;
                    case mStep:
                        if (tc > last_tc) {
                            step_idx++;
                            if (step_idx >= max_step_item) step_idx = 0; 
                        } else {
                            if (!step_idx) step_idx = max_step_item - 1;
                            else
                            step_idx--;  
                        }
                        evt = formEvt;
                    break;
                    default: evt = noneEvt;
                }
                last_tc = tc;
#else
    #ifdef SET_KBD
                switch (menu_idx) {
                    case mFreq:
                        evt = noneEvt;
                        if (rkey) {
                            freq += allStep_bin[step_idx];
                            if (freq <= FMCLK >> 1) evt = freqEvt;
                        } else if (lkey) {
                            if (freq >= allStep_bin[step_idx]) {
                                freq -= allStep_bin[step_idx];
                                evt = freqEvt;
                            }
                        }
                    break;
                    case mForm:
                        evt = noneEvt;
                        if (rkey) {
                            waveform_select++;
                            if (waveform_select >= max_form_item) waveform_select = 0; 
                        } else if (lkey) {
                            if (!waveform_select) waveform_select = max_form_item - 1;
                            else
                            waveform_select--;  
                        }
                        evt = formEvt;
                    break;
                    case mStep:
                        evt = noneEvt;
                        if (rkey) {
                            step_idx++;
                            if (step_idx >= max_step_item) step_idx = 0; 
                        } else if (lkey) {
                            if (!step_idx) step_idx = max_step_item - 1;
                            else
                            step_idx--;  
                        }
                        evt = formEvt;
                    break;
                        default : evt = noneEvt;
                }
                lkey = rkey = false;   
    #endif
#endif             
                if (evt != noneEvt) tmr = get_sec(8);
            break;
        }
        //
#ifdef SET_ENCODER
        count = TIM2->CNT;
	    if ((count != last_count) && (count & 1)) {
            if (count > last_count) tc++; else tc--;
		    last_count = count;
            evt = ienEvt;
	    }
        //
#endif
#if defined(SET_ENCODER) || defined(SET_KBD)
        if (tmr) {
            if (check_sec(tmr)) {
                tmr = 0;
                menu_idx = -1;
                OLED_text_xy(tmp, OLED_calcx(sprintf(tmp, "%s", uname)), LAST_LINE, inv);
            }
        }
#endif        
        //
        Delay_Ms(1);
        //
    }
}
