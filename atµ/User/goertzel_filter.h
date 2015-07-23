#ifndef goertzel
#define goertzel

/* Include core modules */
#include "stm32f4xx.h"
#include "arm_math.h"

//====================================================================================
//	Goertzel_Process:
//====================================================================================

float32_t Goertzel_Process(float32_t targetFreq);

#endif
