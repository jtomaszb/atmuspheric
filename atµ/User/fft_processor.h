#ifndef fftprocessor
#define fftprocessor

#include "stm32f4xx.h"

/* Include arm_math.h mathematic functions */
#include "arm_math.h"

/* FFT settings */ // SAMPLES/4 freqency bins, bin size = 35000/256 =  136 Hz 
#define SAMPLES					512 						/* 32 real party and 32 imaginary parts */
#define FFT_SIZE				SAMPLES / 2		/* FFT size is always the same size as we have samples, so 64 in our case */


extern float32_t Output[FFT_SIZE];

//====================================================================================
//	FFTProcessor_Run: gets samples and performs FFT math
//====================================================================================
void FFTProcessor_Run(void);

#endif
