#include "stm32f4xx.h"

#ifndef _WS2812B_H_
#define _WS2812B_H_

#define STRIP_LEN 	11
#define NUM_STRIPS 	7

// Use update signal
// PAP: This is better than the CC1 signal; using the CC1 signal causes the timing to shift
#define PWM_TIMER	TIM3
#define DMA_STREAM	DMA1_Stream2
#define DMA_TCIF	DMA_FLAG_TCIF2
#define DMA_CHANNEL	DMA_Channel_5
#define DMA_SOURCE	TIM_DMA_Update


#define TIM_PERIOD					27
#define TIM_COMPARE_HIGH		18
#define TIM_COMPARE_LOW			8

typedef struct 
{
	uint8_t r;
	uint8_t b;
	uint8_t g;
} Pixel;

void WS2812_init(void);
void WS2812_send(const Pixel* pixel, const uint16_t _len);
void WS2812_setPixelColor(uint8_t red, uint8_t green, uint8_t blue, uint8_t strip_num, uint8_t pixel_index);
void WS2812_updateLEDs(void);

#endif // _WS2812B_H_
