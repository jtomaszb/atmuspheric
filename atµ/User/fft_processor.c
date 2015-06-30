#include "fft_processor.h"
#include "auto_sampler.h"

/* Global variables */
float32_t Input[SAMPLES];
float32_t Output[FFT_SIZE];

arm_cfft_radix4_instance_f32 S;	/* ARM CFFT module */
float32_t maxValue;				/* Max FFT value is stored here */
uint32_t maxIndex;				/* Index in Output array where max value is */
uint16_t i;

//====================================================================================
//	FUNCTION DEFINITIONS
//====================================================================================
void FFTProcessor_Run(void) {
	uint16_t i;
	
	// Start autosampler, grab data, then tunr off sampler
	AutoSampler_Start();
	for (i = 0; i < SAMPLES; i += 2) {
		//Wait for data to become available 
		/* Data becomes available each 28.5us ~ 35kHz sample rate */
		while(!AutoSampler_Available()){}
		
		/* Real part, must be between -1 and 1 */
		Input[(uint16_t)i] = (float32_t)((float32_t)AutoSampler_GetReading() - (float32_t)2048.0) / (float32_t)2048.0;
		/* Imaginary part */
		Input[(uint16_t)(i + 1)] = 0;
	}
	AutoSampler_Stop();
	
	/* Initialize the CFFT/CIFFT module, intFlag = 0, doBitReverse = 1 */
	arm_cfft_radix4_init_f32(&S, FFT_SIZE, 0, 1);
	
	/* Process the data through the CFFT/CIFFT module */
	arm_cfft_radix4_f32(&S, Input);
	
	/* Process the data through the Complex Magnitude Module for calculating the magnitude at each bin */
	arm_cmplx_mag_f32(Input, Output, FFT_SIZE);
	
	
}
