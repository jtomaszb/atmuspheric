#include "goertzel_filter.h"
#include "auto_sampler.h"
#include "arm_math.h"

//====================================================================================
//	VARIABLE DECLARATIONS
//====================================================================================
#define N 256
#define twoPI 6.28318530718
#define BIN_SPACING 6
#define NUM_BINS 7

float sin_tables[NUM_BINS][N];
float cos_tables[NUM_BINS][N];

//====================================================================================
//	FUNCTION DEFINITIONS
//====================================================================================

void Goertzel_Init(float32_t targetFreq) {
	int i, bin_index;

	//Calculate k-th bin
	float32_t k_bin = targetFreq / SAMPLEFREQUENCY;
	
	// Fill sin and cos tables
	for(i = 0; i < N; i++) {
		sin_tables[bin_index][i] = cos(i*twoPI*k_bin);
		cos_tables[bin_index][i] = sin(i*twoPI*k_bin);
	}
}

float32_t Goertzel_Process(float32_t targetFreq) {
	float32_t s_prev = 0.0;
	float32_t s_prev2 = 0.0;    
	float32_t s;
	float32_t input;
	float32_t freq_bins[7];
	int i;
	
//	for (i = 0; i < 7; i++)
//	{
//		freq_bins[i] = targetFreq * pow(BIN_SPACING, i);
//	}
	
	float32_t k_bin = targetFreq / SAMPLEFREQUENCY;
	float32_t coeff = 2*cos((float32_t)twoPI*k_bin);

	AutoSampler_Start();
	for (i=0; i<N; i++) {
		// wait until theres a new sample
		while(!AutoSampler_Available()){}
		
		// get sample and normalize
		input = ((float32_t)AutoSampler_GetReading()-(float32_t)2048.0)/(float32_t)2048.0;
		
		input *= 0.50 - 0.50 * cos(twoPI * (float32_t) i / (float32_t) N);
			
		s = input + coeff * s_prev - s_prev2;
		s_prev2 = s_prev;
		s_prev = s;
	}
	AutoSampler_Stop();
	
	// return power of bin
	return s_prev2*s_prev2+s_prev*s_prev-coeff*s_prev*s_prev2;
	
	
}
