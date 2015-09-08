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

#define FREQ0_N			(Q * SAMPLEFREQUENCY / FREQ0)
#define FREQ1_N			(Q * SAMPLEFREQUENCY / FREQ1)
#define FREQ2_N			(Q * SAMPLEFREQUENCY / FREQ2)
#define FREQ3_N			(Q * SAMPLEFREQUENCY / FREQ3)
#define FREQ4_N			(Q * SAMPLEFREQUENCY / FREQ4)
#define FREQ5_N			(Q * SAMPLEFREQUENCY / FREQ5)
#define FREQ6_N			(Q * SAMPLEFREQUENCY / FREQ6)
#define FREQ7_N			(Q * SAMPLEFREQUENCY / FREQ7)
#define FREQ8_N			(Q * SAMPLEFREQUENCY / FREQ8)

#define CQ_ALPHA		0.5f
#define MAXIMA_ALPHA 0.99f

#define MAXIMA_WINDOW_SIZE 256

// Utility arrays with precalculated filter frequencies and corresponding sample size
int filterFreq[9] = {FREQ0, FREQ1, FREQ2, FREQ3, FREQ4, FREQ5, FREQ6, FREQ7, FREQ8};
int Nfreq[9] = {FREQ0_N, FREQ1_N, FREQ2_N, FREQ3_N, FREQ4_N, FREQ5_N, FREQ6_N, FREQ7_N, FREQ8_N};

// The lowest frequency needs the largets number of samples
int samplesNeeded = FREQ0_N;

int maxima_window_counter = 0;

// Intermediate value holders
float32_t cq_real[9];
float32_t cq_imag[9];

float32_t cq_last[9];
float32_t cq_curr[9];

float32_t cq_avg[9];

// Sine/Cosine and hamming tables
// Table length depends on frequency
float32_t cosSinTableF0[2][FREQ0_N];			//Index 0 for cos, 1 for sin
float32_t cosSinTableF1[2][FREQ1_N];			//Index 0 for cos, 1 for sin
float32_t cosSinTableF2[2][FREQ2_N];			//Index 0 for cos, 1 for sin
float32_t cosSinTableF3[2][FREQ3_N];			//Index 0 for cos, 1 for sin
float32_t cosSinTableF4[2][FREQ4_N];			//Index 0 for cos, 1 for sin
float32_t cosSinTableF5[2][FREQ5_N];			//Index 0 for cos, 1 for sin
float32_t cosSinTableF6[2][FREQ6_N];			//Index 0 for cos, 1 for sin
float32_t cosSinTableF7[2][FREQ7_N];			//Index 0 for cos, 1 for sin
float32_t cosSinTableF8[2][FREQ8_N];			//Index 0 for cos, 1 for sin

float32_t hammTableF0[FREQ0_N];
float32_t hammTableF1[FREQ1_N];
float32_t hammTableF2[FREQ2_N];
float32_t hammTableF3[FREQ3_N];
float32_t hammTableF4[FREQ4_N];
float32_t hammTableF5[FREQ5_N];
float32_t hammTableF6[FREQ6_N];
float32_t hammTableF7[FREQ7_N];
float32_t hammTableF8[FREQ8_N];

// Output variable that holds bin power of selected frequencies
float32_t cq_out[9];
float32_t cq_max[9];
float32_t cq_raw[9];
float32_t cq_out_last[9];

float32_t maxima_window[NUM_FILTERS][MAXIMA_WINDOW_SIZE];

void updateMaxima();
void updateAvg(int freq, int sample_num);


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
	
	for(i = 0; i < Nfreq[5]; i++) {
		cosSinTableF5[0][i] = cos(twoPi * i * Q / Nfreq[5]);
		cosSinTableF5[1][i] = sin(twoPi * i * Q / Nfreq[5]);
			
		hammTableF5[i] = 0.54 - 0.46*cos(twoPi * i / Nfreq[5]);		
	}
	
	for(i = 0; i < Nfreq[6]; i++) {
		cosSinTableF6[0][i] = cos(twoPi * i * Q / Nfreq[6]);
		cosSinTableF6[1][i] = sin(twoPi * i * Q / Nfreq[6]);
			
		hammTableF6[i] = 0.54 - 0.46*cos(twoPi * i / Nfreq[6]);	
	}
	for(i = 0; i < Nfreq[7]; i++) {
		cosSinTableF7[0][i] = cos(twoPi * i * Q / Nfreq[7]);
		cosSinTableF7[1][i] = sin(twoPi * i * Q / Nfreq[7]);
			
		hammTableF7[i] = 0.54 - 0.46*cos(twoPi * i / Nfreq[7]);		
	}
	
	for(i = 0; i < Nfreq[8]; i++) {
		cosSinTableF8[0][i] = cos(twoPi * i * Q / Nfreq[8]);
		cosSinTableF8[1][i] = sin(twoPi * i * Q / Nfreq[8]);
			
		hammTableF8[i] = 0.54 - 0.46*cos(twoPi * i / Nfreq[8]);	
	}
}

void CQT_Process(void) {
	float32_t input, input0, input1, input2, input3, input4, input5, input6, input7, input8;
	int i;
	
	// zero out output buffer from prev iteration
	for(i = 0; i < 8; i++) {
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
			
		// get input and go hamm
		input = ((float32_t)AutoSampler_GetReading()-(float32_t)2048.0)/(float32_t)2048.0;
		
		// Sum each flter until Nfreq, or the samples needed for the filter
		// 200 Hz
		if(i < Nfreq[0]) {
			input0 = input * hammTableF0[i];
			cq_real[0] += input0 * cosSinTableF0[0][i];
			cq_imag[0] -= input0 * cosSinTableF0[1][i];
		}
		
		input1 = input * hammTableF1[i % Nfreq[1]];
		cq_real[1] += input1 * cosSinTableF1[0][i % Nfreq[1]];
		cq_imag[1] -= input1 * cosSinTableF1[1][i % Nfreq[1]];		
		updateAvg(1, i);

		input2 = input * hammTableF2[i % Nfreq[2]];
		cq_real[2] += input2 * cosSinTableF2[0][i % Nfreq[2]];
		cq_imag[2] -= input2 * cosSinTableF2[1][i % Nfreq[2]];
		updateAvg(2, i);

		input3 = input * hammTableF3[i % Nfreq[3]];
		cq_real[3] += input3 * cosSinTableF3[0][i % Nfreq[3]];
		cq_imag[3] -= input3 * cosSinTableF3[1][i % Nfreq[3]];
		updateAvg(3, i);

		input4 = input * hammTableF4[i % Nfreq[4]];
		cq_real[4] += input4 * cosSinTableF4[0][i % Nfreq[4]];
		cq_imag[4] -= input4 * cosSinTableF4[1][i % Nfreq[4]];
		updateAvg(4, i);
		
		input5 = input * hammTableF5[i % Nfreq[5]];
		cq_real[5] += input5 * cosSinTableF5[0][i % Nfreq[5]];
		cq_imag[5] -= input5 * cosSinTableF5[1][i % Nfreq[5]];
		updateAvg(5, i);
				
		input6 = input * hammTableF6[i % Nfreq[6]];
		cq_real[6] += input6 * cosSinTableF6[0][i % Nfreq[6]];
		cq_imag[6] -= input6 * cosSinTableF6[1][i % Nfreq[6]];
		updateAvg(6, i);
		
		input7 = input * hammTableF7[i % Nfreq[7]];
		cq_real[7] += input7 * cosSinTableF7[0][i % Nfreq[7]];
		cq_imag[7] -= input7 * cosSinTableF7[1][i % Nfreq[7]];
		updateAvg(7, i);
				
		input8 = input * hammTableF8[i % Nfreq[8]];
		cq_real[8] += input8 * cosSinTableF8[0][i % Nfreq[8]];
		cq_imag[8] -= input8 * cosSinTableF8[1][i % Nfreq[8]];
		updateAvg(8, i);
		
		GPIO_ResetBits(GPIOD, GPIO_Pin_15);
	}
	AutoSampler_Stop();
	
	cq_raw[0] = (cq_real[0]*cq_real[0] + cq_imag[0]*cq_imag[0])/(Nfreq[0]);

	for (i = 0; i < 9; i++)
	{
		cq_out_last[i] = cq_out[i];
		cq_out[i] = CQ_ALPHA * cq_out_last[i] + (1.0f - CQ_ALPHA) * cq_raw[i];
	}

	updateMaxima();
}

void updateMaxima(void)
{
	int i, j;
	
	maxima_window_counter++;
	
	for (i = 0; i < 9; i++)
	{
		float32_t temp = 0;

		maxima_window[i][maxima_window_counter % MAXIMA_WINDOW_SIZE] = cq_out[i];

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
