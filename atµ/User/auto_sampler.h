#ifndef autosampler
#define autosampler

/* Include core modules */
#include "stm32f4xx.h"

#define ADC1_RDR 0x4001204C 					// ADC1 Regular Data Register (read only)


//====================================================================================
//	AutoSampler_Init: Inits GPIO, ADC, DMA, and Timer to sample at 35kHz. Sampling on pin PC0
//	AutoSampler_Start: Kicks off timer and enable ADC
//	AutoSampler_Stop: Stops timer and disables ADC
//	AutoSampler_getReading: Returns most recent ADC value
//	AutoSampler_Available: returns 1 if new data available flag is set and resets flag
//	ADC_IRQHandler: ADC ISR, sets new data available flag
//====================================================================================
void AutoSampler_Init(void);
void AutoSampler_Start(void);
void AutoSampler_Stop(void);
int AutoSampler_GetReading(void);
int AutoSampler_Available(void);
void ADC_IRQHandler(void);

#endif
