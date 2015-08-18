/* Include core modules */
#include "stm32f4xx.h"

#include "auto_sampler.h"
//#include "fft_processor.h"
#include "ws2812b.h"
#include "usart.h"
#include "dft_filter.h"
#include "cqt_filter.h"

// utility functions
void startupSequence();
void sg_delay(int count);


int height[7];
int prev_height[7];
int i, j, k, l, m;
float alpha = 0.9;

void mapToHeight(float32_t* output, float32_t* maxima, uint8_t* height);

int main(void) {

	// Initilize modules
	AutoSampler_Init();
	init_USART1(9600);
	WS2812_init();
	CQT_Init();
	
	// Run statup sequence a few times
	startupSequence();
	startupSequence();
	
	// Set initial color on tubes
	for(k = 0; k < 7; k++) {
		for(j = 0; j < 11; j++) 
			WS2812_setPixelColor(100, 10, 10, k, j);
	}

	
	while (1)
	{
		int i, temp;

		CQT_Process();

		for (i = 0; i < 7; i++)
		{
			prev_height[i] = height[i];
		}
	
		mapToHeight(cq_out, cq_max, height);
		
		for (i = 0; i < 7; i++)
		{
			if (height[i] < prev_height[i])
				height[i] = prev_height[i] - 1;
		}
		for(i = 0; i < 7; i++)
		{
			if(height[i] > 11)
				height[i] = 11;

			// turn on pixels to height
			for(j = 0; j < height[i]; j++)
				WS2812_setPixelBrightness(MAX_BRIGHTNESS, i, j);
			
			// turn off pixels past height
			for(j = height[i]; j < 11; j++)
				WS2812_setPixelBrightness(0, i, j);
		}
			
		WS2812_updateLEDs();		
	}
}

void startup_Sequence() {
	
	for(j = 0; j < 11; j++) {
		WS2812_setPixelColor(100, 10, 10, 0, j);
		WS2812_updateStrip(0);
		sg_delay(10000);
	}
	
	for(k = 1; k < 9; k++) {
		for(j = 0; j < 11; j++) {
			WS2812_setPixelColor(100, 10, 10, k, j);
			WS2812_setPixelColor(0, 0, 0, k-1, 10-j);
			WS2812_updateStrip(k-1);
			WS2812_updateStrip(k);
			sg_delay(10000);
		}
	}
	
	for(j = 0; j < 11; j++) {
		WS2812_setPixelColor(0, 0, 0, 8, j);
		WS2812_updateStrip(8);
		sg_delay(10000);
	}
}

void sg_delay(int count) {
	for(m = 0; m < count; m++)
	{}
	
}

void mapToHeight(float32_t* output, float32_t* maxima, uint8_t* height)
{
	int i;
	for (i = 0; i < NUM_STRIPS; i++)
	{
		height[i] = (uint8_t) ((output[i] / maxima[i]) * STRIP_LEN * LEVELS_PER_PIXEL);
	}
}
