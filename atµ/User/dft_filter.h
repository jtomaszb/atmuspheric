#ifndef dft
#define dft

/* Include core modules */
#include "stm32f4xx.h"
#include "arm_math.h"


//====================================================================================
//	DFT_Init:
//	DFT_Process:
//====================================================================================

void DFT_Init(float32_t targetFreq);
float32_t DFT_Process(void);

#endif
