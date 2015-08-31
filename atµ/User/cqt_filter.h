#ifndef cqt
#define cqt

/* Include core modules */
#include "stm32f4xx.h"
#include "arm_math.h"

extern float32_t cq_out[9];
extern float32_t cq_max[9];
extern int Nfreq[9];

//====================================================================================
//	CQT_Init:
//	CQT_Process:
// 	hamm:
//====================================================================================

void CQT_Init(void);
void CQT_Process(void);
void pin_init(void);



#endif
