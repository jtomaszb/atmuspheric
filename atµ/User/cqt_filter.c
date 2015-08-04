#include "cqt_filter.h"
#include "arm_math.h"
#include "auto_sampler.h"

//====================================================================================
//	VARIABLE DECLARATIONS
//====================================================================================

#define twoPi 			6.28318530718
#define Q  					2
#define NUM_FILTERS 5

#define FREQ0				168
#define FREQ1				353
#define FREQ2				741
#define FREQ3				1556
#define FREQ4				3267

#define FREQ0_N			(Q * SAMPLEFREQUENCY / FREQ0)
#define FREQ1_N			(Q * SAMPLEFREQUENCY / FREQ1)
#define FREQ2_N			(Q * SAMPLEFREQUENCY / FREQ2)
#define FREQ3_N			(Q * SAMPLEFREQUENCY / FREQ3)
#define FREQ4_N			(Q * SAMPLEFREQUENCY / FREQ4)

// Utility arrays with precalculated filter frequencies and corresponding sample size
int filterFreq[5] = {FREQ0, FREQ1, FREQ2, FREQ3, FREQ4};
int Nfreq[5] = {FREQ0_N, FREQ1_N, FREQ2_N, FREQ3_N, FREQ4_N};

// The lowest frequency needs the largets number of samples
int samplesNeeded = FREQ0_N;

// Intermediate value holders
float32_t cq_real[5];
float32_t cq_imag[5];

// Sine/Cosine and hamming tables
// Table length depends on frequency
float32_t cosSinTableF0[2][FREQ0_N];			//Index 0 for cos, 1 for sin
float32_t cosSinTableF1[2][FREQ1_N];			//Index 0 for cos, 1 for sin
float32_t cosSinTableF2[2][FREQ2_N];			//Index 0 for cos, 1 for sin
float32_t cosSinTableF3[2][FREQ3_N];			//Index 0 for cos, 1 for sin
float32_t cosSinTableF4[2][FREQ4_N];			//Index 0 for cos, 1 for sin
float32_t hammTableF0[FREQ0_N];
float32_t hammTableF1[FREQ1_N];
float32_t hammTableF2[FREQ2_N];
float32_t hammTableF3[FREQ3_N];
float32_t hammTableF4[FREQ4_N];

// Output variable that holds bin power of selected frequencies
float32_t cq_out[5];


//====================================================================================
//	FUNCTION DEFINITIONS
//====================================================================================
void CQT_Init(void) {
	int i;

	pin_init();
	
	// Fill up sin/cos and hamm tables for each freq
	for(i = 0; i < Nfreq[0]; i++) {
		cosSinTableF0[0][i] = cos(twoPi * i * Q / Nfreq[0]);
		cosSinTableF0[1][i] = sin(twoPi * i * Q / Nfreq[0]);
		
		hammTableF0[i] = 0.54 - 0.46*cos(twoPi * i / Nfreq[0]);
		
	}
	for(i = 0; i < Nfreq[1]; i++) {
		cosSinTableF1[0][i] = cos(twoPi * i * Q / Nfreq[1]);
		cosSinTableF1[1][i] = sin(twoPi * i * Q / Nfreq[1]);
		
		hammTableF1[i] = 0.54 - 0.46*cos(twoPi * i / Nfreq[1]);
		
	}
	for(i = 0; i < Nfreq[2]; i++) {
		cosSinTableF2[0][i] = cos(twoPi * i * Q / Nfreq[2]);
		cosSinTableF2[1][i] = sin(twoPi * i * Q / Nfreq[2]);
			
		hammTableF2[i] = 0.54 - 0.46*cos(twoPi * i / Nfreq[2]);
			
	}
	for(i = 0; i < Nfreq[3]; i++) {
		cosSinTableF3[0][i] = cos(twoPi * i * Q / Nfreq[3]);
		cosSinTableF3[1][i] = sin(twoPi * i * Q / Nfreq[3]);
			
		hammTableF3[i] = 0.54 - 0.46*cos(twoPi * i / Nfreq[3]);
			
	}
	for(i = 0; i < Nfreq[4]; i++) {
		cosSinTableF4[0][i] = cos(twoPi * i * Q / Nfreq[4]);
		cosSinTableF4[1][i] = sin(twoPi * i * Q / Nfreq[4]);
			
		hammTableF4[i] = 0.54 - 0.46*cos(twoPi * i / Nfreq[4]);
			
	}

}

void CQT_Process(void) {
	float32_t input, input0, input1, input2, input3, input4;
	int i;
	
	// zero out output buffer from prev iteration
	for(i = 0; i < 5; i++) {
		cq_real[i] = 0;
		cq_imag[i] = 0;
	}
	
	// start sampler and process samples as they come
	AutoSampler_Start();
	for (i=0 ; i < samplesNeeded; i++) {
		
		// wait until theres a new sample
		// get new sample and normalize it to -1 to 1
		while(!AutoSampler_Available()){}
			
		GPIO_SetBits(GPIOD, GPIO_Pin_15);
			
		// get input and go hamm
		input = ((float32_t)AutoSampler_GetReading()-(float32_t)2048.0)/(float32_t)2048.0;
		//input *=	hammTableF0[i];
		
		// Sum each flter until Nfreq, or the samples needed for the filter
		// 200 Hz
		if(i < Nfreq[0]) {
			input0 = 3*input * hammTableF0[i];
			cq_real[0] += input0 * cosSinTableF0[0][i];
			cq_imag[0] -= input0 * cosSinTableF0[1][i];
		}
		
		// 600 Hz
		if(i < Nfreq[1]) {
			input1 = 5*input * hammTableF1[i];
			cq_real[1] += input1 * cosSinTableF1[0][i];
			cq_imag[1] -= input1 * cosSinTableF1[1][i];
		}

		// 1800 Hz
		if(i < Nfreq[2]) {
			input2 = 7*input * hammTableF2[i];
			cq_real[2] += input2 * cosSinTableF2[0][i];
			cq_imag[2] -= input2 * cosSinTableF2[1][i];
		}

		// 600 Hz
		if(i < Nfreq[3]) {
			input3 = 9*input * hammTableF3[i];
			cq_real[3] += input3 * cosSinTableF3[0][i];
			cq_imag[3] -= input3 * cosSinTableF3[1][i];
		}

		// 1800 Hz
		if(i < Nfreq[4]) {
			input4 = 13*input * hammTableF4[i];
			cq_real[4] += input4 * cosSinTableF4[0][i];
			cq_imag[4] -= input4 * cosSinTableF4[1][i];
		}		
		GPIO_ResetBits(GPIOD, GPIO_Pin_15);
	}
	AutoSampler_Stop();
	
	// divide real and imaginary by Nfreq, or the samples needed for the filter
//	for(i = 0; i < 3; i++) {
//		cq_real[i] /= Nfreq[i];
//		cq_imag[i] /= Nfreq[i];
//	}
	
	// update output with bin power
	for(i = 0; i < 5; i++)
		cq_out[i] = (cq_real[i]*cq_real[i] + cq_imag[i]*cq_imag[i])/(Nfreq[i]);

}

void pin_init() {
	
	GPIO_InitTypeDef GPIO_InitDef;

	RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOD, ENABLE);
	
	//Pins 15D
	GPIO_InitDef.GPIO_Pin = GPIO_Pin_15;
	//Mode output
	GPIO_InitDef.GPIO_Mode = GPIO_Mode_OUT;
	//Output type push-pull
	GPIO_InitDef.GPIO_OType = GPIO_OType_PP;
	//Without pull resistors
	GPIO_InitDef.GPIO_PuPd = GPIO_PuPd_NOPULL;
	//50MHz pin speed
	GPIO_InitDef.GPIO_Speed = GPIO_Speed_50MHz;
	 
	//Initialize pins on GPIOG port
	GPIO_Init(GPIOD, &GPIO_InitDef);
}
