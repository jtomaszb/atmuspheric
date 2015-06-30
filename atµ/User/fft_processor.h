#ifndef __FFT_PROCESSOR_H
#define __FFT_PROCESSOR_H

//====================================================================================
//	VARIABLE AND FUNCTION DECLARATIONS
//====================================================================================

#include "stm32f4xx.h"

/* Include arm_math.h mathematic functions */
#include "arm_math.h"

/* FFT settings */
#define SAMPLES					512 			/* 256 real party and 256 imaginary parts */
#define FFT_SIZE				SAMPLES / 2		/* FFT size is always the same size as we have samples, so 256 in our case */
	
//====================================================================================
//	FFTProcessor_Run: gets samples and performs FFT math
//====================================================================================
void FFTProcessor_Run(void);

#endif // __FFT_PROCESSOR_H
