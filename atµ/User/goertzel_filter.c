#include "goertzel_filter.h"
#include "auto_sampler.h"
#include "arm_math.h"

//====================================================================================
//	VARIABLE DECLARATIONS
//====================================================================================
#define N 256
#define twoPI 6.28318530718


//====================================================================================
//	FUNCTION DEFINITIONS
//====================================================================================

float32_t Goertzel_Process(float32_t targetFreq) {
	float32_t s_prev = 0.0;
	float32_t s_prev2 = 0.0;    
	float32_t s;
	float32_t input;
	int i;
	
	float32_t k_bin = targetFreq / SAMPLEFREQUENCY;
	float32_t coeff = 2*cos((float32_t)twoPI*k_bin);

	AutoSampler_Start();
	for (i=0; i<N; i++) {
		// wait until theres a new sample
		while(!AutoSampler_Available()){}
		
		// get sample and normalize
		input = ((float32_t)AutoSampler_GetReading()-(float32_t)2048.0)/(float32_t)2048.0;
		
		s = input + coeff * s_prev - s_prev2;
		s_prev2 = s_prev;
		s_prev = s;
	}
	AutoSampler_Stop();
	
	// return power of bin
	return s_prev2*s_prev2+s_prev*s_prev-coeff*s_prev*s_prev2;
	
	
}
