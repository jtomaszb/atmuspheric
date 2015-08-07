/* Include core modules */
#include "stm32f4xx.h"

#include "auto_sampler.h"
//#include "fft_processor.h"
#include "ws2812b.h"
#include "usart.h"
#include "dft_filter.h"
#include "cqt_filter.h"


int height[7];
int prev_height[7];
int i, j, k;
float alpha = 0.9;

int main(void) {

	AutoSampler_Init();
	init_USART1(9600);
	WS2812_init();
	
	CQT_Init();
	for(j = 0; j < 11; j++)
		WS2812_setPixelColor(100, 10, 10, 0, j);
	for(j = 0; j < 11; j++)
		WS2812_setPixelColor(100, 10, 10, 1, j);
	for(j = 0; j < 11; j++)
		WS2812_setPixelColor(100, 10, 10, 2, j);
	for(j = 0; j < 11; j++)
		WS2812_setPixelColor(100, 10, 10, 3, j);
	for(j = 0; j < 11; j++)
		WS2812_setPixelColor(100, 10, 10, 4, j);	
	for(j = 0; j < 11; j++)
		WS2812_setPixelColor(100, 10, 10, 5, j);
	for(j = 0; j < 11; j++)
		WS2812_setPixelColor(100, 10, 10, 6, j);	
	
	while (1)
	{
		int i, temp;

		CQT_Process();

		for (i = 0; i < 7; i++)
		{
			prev_height[i] = height[i];
		
			if (cq_out[i] < prev_height[i])
				height[i] = prev_height[i] - 1;
			else
				height[i] = cq_out[i];
		}
	
		// max height 64 so cast to int and right shift
//		height[0] = 10 * cq_out[0];
//		height[1] = 10 * cq_out[1];//(uint32_t)(power[1]);
//		height[2] = 10 * cq_out[2];//(uint32_t)(power[2]);

//		height[0] = (uint32_t)(alpha * height[0]) + ((1 - alpha) * Output[2]);
//		height[1] = (uint32_t)(alpha * height[1]) + ((1 - alpha) * Output[8]);
//		height[2] = (uint32_t)(alpha * height[2]) + ((1 - alpha) * Output[38]);
		
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
