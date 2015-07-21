#include "auto_sampler.h"

//====================================================================================
//	VARIABLE AND FUNCTION DECLARATIONS
//====================================================================================
volatile uint16_t ADCvalue = 0;       // Updated 35000 times per second by DMA
int newDataAvailable = 0;							// Flag set by ADC ISR when new data available


//====================================================================================
//	FUNCTION DEFINITIONS
//====================================================================================
void AutoSampler_Init(void) {
	
	// Declare Timer, ADC, DMA, and NVIC init structs	
	TIM_TimeBaseInitTypeDef 	TIM2_TimeBase;
	ADC_InitTypeDef       		ADC_INIT;
	ADC_CommonInitTypeDef 		ADC_COMMON;
	DMA_InitTypeDef       		DMA_INIT;
	GPIO_InitTypeDef 					gpio;
	NVIC_InitTypeDef 					NVIC_ADC1;


	//====================================================================================
	//   Configuring TIM2 to trigger at 35kHz, or the ADC sampling rate
	//====================================================================================
  RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM2, ENABLE);
 
  TIM_TimeBaseStructInit(&TIM2_TimeBase); 
  TIM2_TimeBase.TIM_Period        = (uint16_t)11; // Trigger = CK_CNT/(11+1) = 35kHz
  TIM2_TimeBase.TIM_Prescaler     = 200;          // CK_CNT = 84MHz/200 = 420kHz
  TIM2_TimeBase.TIM_ClockDivision = 0;
  TIM2_TimeBase.TIM_CounterMode   = TIM_CounterMode_Up;  
  TIM_TimeBaseInit(TIM2, &TIM2_TimeBase);
  TIM_SelectOutputTrigger(TIM2, TIM_TRGOSource_Update);

  //TIM_Cmd(TIM2, ENABLE);	// this is done in the start function
	
	
	
	
	//====================================================================================
	//   Configuring ADC with DMA
	//====================================================================================
  RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_DMA2, ENABLE);  
  RCC_APB2PeriphClockCmd(RCC_APB2Periph_ADC1, ENABLE);

  DMA_INIT.DMA_Channel = DMA_Channel_0; 
  DMA_INIT.DMA_PeripheralBaseAddr = (uint32_t)ADC1_RDR;
  DMA_INIT.DMA_Memory0BaseAddr    = (uint32_t)&ADCvalue; 
  DMA_INIT.DMA_DIR                = DMA_DIR_PeripheralToMemory;
  DMA_INIT.DMA_BufferSize         = 1;
  DMA_INIT.DMA_PeripheralInc      = DMA_PeripheralInc_Disable;
  DMA_INIT.DMA_MemoryInc          = DMA_MemoryInc_Enable;
  DMA_INIT.DMA_PeripheralDataSize = DMA_PeripheralDataSize_HalfWord;
  DMA_INIT.DMA_MemoryDataSize     = DMA_MemoryDataSize_HalfWord;
  DMA_INIT.DMA_Mode               = DMA_Mode_Circular;
  DMA_INIT.DMA_Priority           = DMA_Priority_High;
  DMA_INIT.DMA_FIFOMode           = DMA_FIFOMode_Disable;         
  DMA_INIT.DMA_FIFOThreshold      = DMA_FIFOThreshold_HalfFull;
  DMA_INIT.DMA_MemoryBurst        = DMA_MemoryBurst_Single;
  DMA_INIT.DMA_PeripheralBurst    = DMA_PeripheralBurst_Single;
  DMA_Init(DMA2_Stream4, &DMA_INIT);
  DMA_Cmd(DMA2_Stream4, ENABLE);

  ADC_COMMON.ADC_Mode             = ADC_Mode_Independent;
  ADC_COMMON.ADC_Prescaler        = ADC_Prescaler_Div2;
  ADC_COMMON.ADC_DMAAccessMode    = ADC_DMAAccessMode_Disabled;
  ADC_COMMON.ADC_TwoSamplingDelay = ADC_TwoSamplingDelay_5Cycles;
  ADC_CommonInit(&ADC_COMMON);

  ADC_INIT.ADC_Resolution           = ADC_Resolution_12b;
  ADC_INIT.ADC_ScanConvMode         = DISABLE;
  ADC_INIT.ADC_ContinuousConvMode   = DISABLE; // ENABLE for max ADC sampling frequency
  ADC_INIT.ADC_ExternalTrigConvEdge = ADC_ExternalTrigConvEdge_Rising;
  ADC_INIT.ADC_ExternalTrigConv     = ADC_ExternalTrigConv_T2_TRGO;
  ADC_INIT.ADC_DataAlign            = ADC_DataAlign_Right;
  ADC_INIT.ADC_NbrOfConversion      = 1;
  ADC_Init(ADC1, &ADC_INIT);

  ADC_RegularChannelConfig(ADC1, ADC_Channel_11, 1, ADC_SampleTime_3Cycles);
  ADC_DMARequestAfterLastTransferCmd(ADC1, ENABLE);
  
	// These are done in start function
	//ADC_ITConfig(ADC1, ADC_IT_EOC, ENABLE);
  //ADC_DMACmd(ADC1, ENABLE);
  //ADC_Cmd(ADC1, ENABLE);
	
	
	//====================================================================================
	//   Configuring ADC global interrupt (for testing)
	//==================================================================================== 
  NVIC_ADC1.NVIC_IRQChannel    = ADC_IRQn;
  NVIC_ADC1.NVIC_IRQChannelCmd = ENABLE;
  NVIC_Init(&NVIC_ADC1);
	
	
	//====================================================================================
	//   Configuring GPIO PC1, Channel 11
	//====================================================================================
  RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOC, ENABLE);
 
	gpio.GPIO_Pin   = GPIO_Pin_1;
	gpio.GPIO_Mode  = GPIO_Mode_AIN;
	gpio.GPIO_Speed = GPIO_Speed_50MHz;
	gpio.GPIO_PuPd  = GPIO_PuPd_NOPULL;
  GPIO_Init(GPIOC, &gpio);
	
}

//====================================================================================
//   Return the reading so it can be copied to a non-volatile variable
//====================================================================================
int AutoSampler_GetReading(void) {
	return ADCvalue;
}

void AutoSampler_Start() {
	ADC_ITConfig(ADC1, ADC_IT_EOC, ENABLE);
	ADC_DMACmd(ADC1, ENABLE);
	ADC_Cmd(ADC1, ENABLE);
	TIM_Cmd(TIM2, ENABLE);
}

void AutoSampler_Stop() {
	ADC_ITConfig(ADC1, ADC_IT_EOC, DISABLE);
	ADC_DMACmd(ADC1, DISABLE);
	ADC_Cmd(ADC1, DISABLE);
	TIM_Cmd(TIM2, DISABLE);
}

int AutoSampler_Available() {
	if(newDataAvailable) {
		newDataAvailable = 0;
		return 1;
	}
	
	return 0;
}

void ADC_IRQHandler(void)
{
  newDataAvailable = 1;
  ADC_ClearITPendingBit(ADC1, ADC_IT_EOC);
}
