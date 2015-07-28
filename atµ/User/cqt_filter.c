#include "cqt_filter.h"
#include "arm_math.h"
#include "auto_sampler.h"

//====================================================================================
//	VARIABLE DECLARATIONS
//====================================================================================

#define twoPi 			6.28318530718
#define Q  					5
#define NUM_FILTERS 3

float32_t cq_out[3];
int Nfreq[3];
int filterFreq[3] = {200, 1000, 5000};
float32_t cq_real[3];
float32_t cq_imag[3];
int samplesNeeded;


//====================================================================================
//	FUNCTION DEFINITIONS
//====================================================================================
void CQT_Init(void) {
	int i;
	GPIO_InitTypeDef GPIO_InitDef;

	RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOD, ENABLE);
	 
	//Pins 13 and 14
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
	
	// Calculate N value for each filter
	for(i = 0; i < NUM_FILTERS; i++) {
		Nfreq[i] = (int) ceil(Q * SAMPLEFREQUENCY / filterFreq[i]);
	}
	
	samplesNeeded = Nfreq[0];
}

void CQT_Process(void) {
	float32_t input;
	int i;
	
	// zero out output buffer from prev iteration
	for(i = 0; i < 3; i++) {
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
			
		input = ((float32_t)AutoSampler_GetReading()-(float32_t)2048.0)/(float32_t)2048.0;
		
		// Sum each flter until Nk
		// 200 Hz
		if(i < Nfreq[0]) {
			cq_real[0] += input * hamm(i, Nfreq[0]) * cos(twoPi * i * Q / Nfreq[0]);
			cq_imag[0] -= input * hamm(i, Nfreq[0]) * sin(twoPi * i * Q / Nfreq[0]);
		}
		
		// 1000 Hz
		if(i < Nfreq[1]) {
			cq_real[1] += input * hamm(i, Nfreq[1]) * cos(twoPi * i * Q / Nfreq[1]);
			cq_imag[1] -= input * hamm(i, Nfreq[1]) * sin(twoPi * i * Q / Nfreq[1]);
		}

		// 5000 Hz
		if(i < Nfreq[2]) {
			cq_real[2] += input * hamm(i, Nfreq[2]) * cos(twoPi * i * Q / Nfreq[2]);
			cq_imag[2] -= input * hamm(i, Nfreq[2]) * sin(twoPi * i * Q / Nfreq[2]);
		}
		
		GPIO_ResetBits(GPIOD, GPIO_Pin_15);
	}
	AutoSampler_Stop();
	
	// divide real and imaginary by Nk
	for(i = 0; i < 3; i++) {
		cq_real[i] /= Nfreq[i];
		cq_imag[i] /= Nfreq[i];
	}
	
	// update output with bin power
	for(i = 0; i < 3; i++)
		cq_out[i] = (cq_real[i]*cq_real[i] + cq_imag[i]*cq_imag[i]);

}

float32_t hamm(int n, int Nk) {
	return (0.54 - 0.46*cos(twoPi*n / Nk));
}
