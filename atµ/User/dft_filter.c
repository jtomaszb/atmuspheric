#include "dft_filter.h"
#include "arm_math.h"
#include "auto_sampler.h"

//====================================================================================
//	VARIABLE DECLARATIONS
//====================================================================================

#define N 256
#define twoPI 6.28318530718

float32_t cosTable[3][N];
float32_t sinTable[3][N];
float32_t freqs[3] = {50, 500, 5000};
float32_t dft_outs[3];

//====================================================================================
//	FUNCTION DEFINITIONS
//====================================================================================
void DFT_Init(float32_t targetFreq) {
	int i, j;
	float32_t k_bin[3];
	
	//Calculate k-th bin
	for (j = 0; j < 3; j++)
	{
		k_bin[j] = freqs[j] / SAMPLEFREQUENCY;
		
		// Fill sin and cos tables
		for(i = 0; i < N; i++) {
			cosTable[j][i] = cos(i*twoPI*k_bin[j]);
			sinTable[j][i] = sin(i*twoPI*k_bin[j]);
		}
	}
}

void DFT_Process(void) {
	float32_t x_real[3] = {0};
	float32_t x_imag[3] = {0};
	float32_t input;
	int i, j;
	
	// start sampler and process samples as they come
	AutoSampler_Start();
	for (i=0 ; i < N ; i++) {
		// wait until theres a new sample
		while(!AutoSampler_Available()){}
			
		// get sample and normalize
		input = ((float32_t)AutoSampler_GetReading()-(float32_t)2048.0)/(float32_t)2048.0;
		
		// calculate real and imaginary part of bin
		for (j = 0; j < 3; j++)
		{
			x_real[j] += input * cosTable[j][i];
			x_imag[j] -= input * sinTable[j][i];
		}
	}
	AutoSampler_Stop();
	
	// return power of kth frequency bin
	for (i = 0; i < 3; i++)
	{
		dft_outs[i] = (x_real[i]*x_real[i] + x_imag[i]*x_imag[i]);	
	}
}
