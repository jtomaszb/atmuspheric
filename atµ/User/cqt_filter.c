#include "cqt_filter.h"
#include "arm_math.h"
#include "auto_sampler.h"

//====================================================================================
//	VARIABLE DECLARATIONS
//====================================================================================

#define twoPi 			6.28318530718f
#define Q  					2
#define NUM_FILTERS 9

#define FREQ0				100 
#define FREQ1				182 
#define FREQ2				331 
#define FREQ3				603 
#define FREQ4				1097
#define FREQ5				1997
#define FREQ6				3634
#define FREQ7				6615 
#define FREQ8				12038 

#define FREQ0_N			(Q * SAMPLEFREQUENCY / FREQ0) 	// 700
#define FREQ1_N			(Q * SAMPLEFREQUENCY / FREQ1)
#define FREQ2_N			(Q * SAMPLEFREQUENCY / FREQ2)
#define FREQ3_N			(Q * SAMPLEFREQUENCY / FREQ3)
#define FREQ4_N			(Q * SAMPLEFREQUENCY / FREQ4)
#define FREQ5_N			(Q * SAMPLEFREQUENCY / FREQ5)
#define FREQ6_N			(Q * SAMPLEFREQUENCY / FREQ6)
#define FREQ7_N			(Q * SAMPLEFREQUENCY / FREQ7)
#define FREQ8_N			(Q * SAMPLEFREQUENCY / FREQ8)

#define CQ_ALPHA		0.01f
#define MAXIMA_ALPHA 0.9f

#define MAXIMA_WINDOW_SIZE 256

// Utility arrays with precalculated filter frequencies and corresponding sample size
int filterFreq[9] = {FREQ0, FREQ1, FREQ2, FREQ3, FREQ4, FREQ5, FREQ6, FREQ7, FREQ8};
int Nfreq[9] = {FREQ0_N, FREQ1_N, FREQ2_N, FREQ3_N, FREQ4_N, FREQ5_N, FREQ6_N, FREQ7_N, FREQ8_N};

// The lowest frequency needs the largets number of samples
int samplesNeeded = FREQ0_N;

int maxima_window_counter = 0;

// Intermediate value holders
float32_t cq_real[NUM_FILTERS];
float32_t cq_imag[NUM_FILTERS];

float32_t cq_last[NUM_FILTERS];
float32_t cq_curr[NUM_FILTERS];

float32_t cq_avg[NUM_FILTERS];

// hamming-windowed cosine and sine tables
// Table length depends on frequency
float32_t cosSinHammF0[2][FREQ0_N];			//Index 0 for cos, 1 for sin, 2 for hamm
float32_t cosSinHammF1[2][FREQ1_N];			//Index 0 for cos, 1 for sin, 2 for hamm
float32_t cosSinHammF2[2][FREQ2_N];			//Index 0 for cos, 1 for sin, 2 for hamm
float32_t cosSinHammF3[2][FREQ3_N];			//Index 0 for cos, 1 for sin, 2 for hamm
float32_t cosSinHammF4[2][FREQ4_N];			//Index 0 for cos, 1 for sin, 2 for hamm
float32_t cosSinHammF5[2][FREQ5_N];			//Index 0 for cos, 1 for sin, 2 for hamm
float32_t cosSinHammF6[2][FREQ6_N];			//Index 0 for cos, 1 for sin, 2 for hamm
float32_t cosSinHammF7[2][FREQ7_N];			//Index 0 for cos, 1 for sin, 2 for hamm
float32_t cosSinHammF8[2][FREQ8_N];			//Index 0 for cos, 1 for sin, 2 for hamm

//Declare array of pointers to the tables above
float32_t **cosSinHammTable[NUM_FILTERS] = {(float**)cosSinHammF0, (float**)cosSinHammF1, 
																						(float**)cosSinHammF2, (float**)cosSinHammF3, 
																						(float**)cosSinHammF4, (float**)cosSinHammF5, 
																						(float**)cosSinHammF6, (float**)cosSinHammF7, 
																						(float**)cosSinHammF8 };

// Output variable that holds bin power of selected frequencies
float32_t cq_out[NUM_FILTERS];
float32_t cq_max[NUM_FILTERS];
float32_t cq_raw[NUM_FILTERS];
float32_t cq_out_last[NUM_FILTERS];

float32_t maxima_window[NUM_FILTERS][MAXIMA_WINDOW_SIZE];
void updateMaxima(void);
void updateAvg(int freq, int sample_num);

//====================================================================================
//	FUNCTION DEFINITIONS
//====================================================================================
void CQT_Init(void) {
	int i, j;

	pin_init();
	
	// Fill up sin/cos and hamm tables for each freq
	for(j = 0; j < NUM_FILTERS; j++) {
		for(i = 0; i < Nfreq[j]; i++) {
			cosSinHammTable[j][0][i] = (0.54 - 0.46*cos(twoPi * i / Nfreq[j])) * cos(twoPi * i * Q / Nfreq[j]);
			cosSinHammTable[j][1][i] = (0.54 - 0.46*cos(twoPi * i / Nfreq[j])) * sin(twoPi * i * Q / Nfreq[j]);
		}
	}
}

void CQT_Process(void) {
	float32_t input;
	int i, j;
	
	// zero out output buffer from prev iteration
	for(i = 0; i < NUM_FILTERS; i++) {
		cq_real[i] = 0.0f;
		cq_imag[i] = 0.0f;
	}
	
	// start sampler and process samples as they come
	AutoSampler_Start();
	for (i=0 ; i < samplesNeeded; i++) {
		
		// wait until theres a new sample
		// get new sample and normalize it to -1 to 1
		while(!AutoSampler_Available()){}
			
		GPIO_SetBits(GPIOD, GPIO_Pin_15);
			
		// get input and normalize to (-1 , 1)
		input = ((float32_t)AutoSampler_GetReading()-(float32_t)2048.0)/(float32_t)2048.0;
		
		// Sum each flter until Nfreq, or the samples needed for the filter
//		if(i < Nfreq[0]) {
//			cq_real[0] += input * cosSinHammTable[0][0][i];
//			cq_imag[0] -= input * cosSinHammTable[0][1][i];
//		}
		
		for(j = 0; j < NUM_FILTERS; j++) {
			cq_real[j] += input * cosSinHammTable[j][0][i % Nfreq[j]];
			cq_imag[j] -= input * cosSinHammTable[j][1][i % Nfreq[j]];		
			updateAvg(j, i);
		}
		
		GPIO_ResetBits(GPIOD, GPIO_Pin_15);
	}
	AutoSampler_Stop();
	
	cq_raw[0] = (cq_real[0]*cq_real[0] + cq_imag[0]*cq_imag[0])/(Nfreq[0]);

	for (i = 0; i < NUM_FILTERS; i++)
	{
		cq_out_last[i] = cq_out[i];
		cq_out[i] = CQ_ALPHA * cq_out_last[i] + (1.0f - CQ_ALPHA) * cq_raw[i];
	}

	updateMaxima();
}

void updateMaxima(void)
{
	int i, j;
	
	if (maxima_window_counter < (MAXIMA_WINDOW_SIZE - 1))
	{
		maxima_window_counter++;
	}
	else
	{	
		maxima_window_counter = 0;
	}		
	
	for (i = 0; i < NUM_FILTERS; i++)
	{
		float32_t temp = 0;

		maxima_window[i][maxima_window_counter] = cq_out[i];

		for (j = 0; j < MAXIMA_WINDOW_SIZE; j++)
		{
			if (temp < maxima_window[i][j])
			{
				temp = maxima_window[i][j];
			}				
		}

		cq_max[i] = MAXIMA_ALPHA * cq_max[i] + (1 - MAXIMA_ALPHA) * temp;
	}
}

void updateAvg(int freq, int sample_num)
{
	if ( sample_num > 1 && ((sample_num - 1) % Nfreq[freq]) == 0 )
	{
		cq_last[freq] = cq_avg[freq];
		cq_curr[freq] = (cq_real[freq] * cq_real[freq] + cq_imag[freq] * cq_imag[freq]) / (Nfreq[freq]);
		
		cq_avg[freq] = cq_last[freq] + cq_curr[freq] - cq_last[freq] / ( (float32_t) samplesNeeded / (float32_t) Nfreq[freq]);
		
		cq_raw[freq] = cq_avg[freq] / (float32_t) ( (float32_t)samplesNeeded / (float32_t)Nfreq[freq]); 
		
		cq_real[freq] = 0.0f;
		cq_imag[freq] = 0.0f;	
	}	
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
