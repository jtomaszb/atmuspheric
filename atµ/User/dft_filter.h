#ifndef dft
#define dft

/* Include core modules */
#include "stm32f4xx.h"
#include "arm_math.h"

extern float32_t dft_outs[3];

//====================================================================================
//	DFT_Init:
//	DFT_Process:
//====================================================================================

void DFT_Init(float32_t targetFreq);
void DFT_Process(void);

#endif
