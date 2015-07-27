#ifndef cqt
#define cqt

/* Include core modules */
#include "stm32f4xx.h"
#include "arm_math.h"

extern float32_t cq_out[3];

//====================================================================================
//	CQT_Init:
//	CQT_Process:
// 	hamm:
//====================================================================================

void CQT_Init(float32_t targetFreq);
void CQT_Process(void);
float32_t hamm(int n, int Nk);



#endif
