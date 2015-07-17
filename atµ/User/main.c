/* Include core modules */
#include "stm32f4xx.h"

#include "auto_sampler.h"
#include "fft_processor.h"
#include "ws2812b.h"

int height;
int j;

int main(void) {
	AutoSampler_Init();
  AutoSampler_Start();
//	WS2812_init();
	
	while (1)
	{
//		FFTProcessor_Run();
//		
//		// max height 64 so cast to int and right shift
//		height = ((uint32_t) Output[4]);
//		if(height > 11)
//			height = 11;
//		
//		
//		// turn on pixels to height
//		for(j = 0; j < height; j++)
//			WS2812_setPixelColor(1, 0, 1, 0, j);
//		
//		// turn off pixels past height
//		for(j = height; j < 11; j++)
//			WS2812_setPixelColor(0, 0, 0, 0, j);
//		
//		WS2812_updateLEDs();
		
	}
}
