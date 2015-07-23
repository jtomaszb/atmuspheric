#include "dft_filter.h"
#include "arm_math.h"
#include "auto_sampler.h"

//====================================================================================
//	VARIABLE DECLARATIONS
//====================================================================================

#define N 256
#define twoPI 6.28318530718

float32_t cosTable[N];
float32_t sinTable[N];


//====================================================================================
//	FUNCTION DEFINITIONS
//====================================================================================
void DFT_Init(float32_t targetFreq) {
	int i;

	//Calculate k-th bin
	float32_t k_bin = targetFreq / SAMPLEFREQUENCY;
	
	// Fill sin and cos tables
	for(i = 0; i < N; i++) {
		cosTable[i] = cos(i*twoPI*k_bin);
		sinTable[i] = sin(i*twoPI*k_bin);
	}
}

float32_t DFT_Process(void) {
	float32_t x_real = 0;
	float32_t x_imag = 0;
	float32_t input;
	int i;
	
	// start sampler and process samples as they come
	AutoSampler_Start();
	for (i=0 ; i < N ; i++) {
		// wait until theres a new sample
		while(!AutoSampler_Available()){}
			
		// get sample and normalize
		input = ((float32_t)AutoSampler_GetReading()-(float32_t)2048.0)/(float32_t)2048.0;
		
		// calculate real and imaginary part of bin
		x_real += input * cosTable[i];
		x_imag -= input * sinTable[i];
		
	}
	AutoSampler_Stop();
	
	// return power of kth frequency bin
	return x_real*x_real + x_imag*x_imag;
	
}
