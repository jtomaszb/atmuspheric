#include "stm32f4xx.h"

#ifndef _WS2812B_H_
#define _WS2812B_H_

#define STRIP_LEN 	11
#define NUM_STRIPS 	3

// Use update signal
// PAP: This is better than the CC1 signal; using the CC1 signal causes the timing to shift
#define PWM_TIMER_0	TIM3
#define PWM_TIMER_1	TIM3
#define PWM_TIMER_2	TIM3
#define PWM_TIMER_3	TIM3
#define PWM_TIMER_4	TIM1
#define PWM_TIMER_5	TIM10
#define PWM_TIMER_6	TIM11

#define DMA_STREAM_0	DMA1_Stream4
#define DMA_STREAM_1	DMA1_Stream5
#define DMA_STREAM_2	DMA1_Stream7
#define DMA_STREAM_3	DMA1_Stream2
#define DMA_STREAM_4	DMA1_Stream5
#define DMA_STREAM_5	DMA2_Stream2
#define DMA_STREAM_6	DMA2_Stream3

#define DMA_TCIF	DMA_FLAG_TCIF2

#define DMA_CHANNEL_0	DMA_Channel_5
#define DMA_CHANNEL_1	DMA_Channel_5
#define DMA_CHANNEL_2	DMA_Channel_5
#define DMA_CHANNEL_3	DMA_Channel_5
#define DMA_CHANNEL_4	DMA_Channel_4
#define DMA_CHANNEL_5	DMA_Channel_5
#define DMA_CHANNEL_6	DMA_Channel_6

#define DMA_SOURCE	TIM_DMA_Update

#define TIM_PERIOD					27
#define TIM_COMPARE_HIGH		18
#define TIM_COMPARE_LOW			8

#define R_OFFSET 0
#define G_OFFSET 1
#define B_OFFSET 2

#define PIXEL_SIZE 3

#define MAX_BRIGHTNESS 0xFF

#define apply_brightness(color, brightness) (uint8_t) (((uint16_t)color * (uint16_t)brightness)/ MAX_BRIGHTNESS)

typedef struct 
{
	uint8_t r;
	uint8_t g;
	uint8_t b;
} Pixel;

extern uint8_t gPixels[NUM_STRIPS * STRIP_LEN * PIXEL_SIZE];

void WS2812_init(void);
void WS2812_updateStrip(uint8_t strip_index);
void WS2812_send(const uint8_t* pixels, const uint8_t* pixel_brightness, const uint16_t _len, uint8_t strip_num);
void WS2812_setPixelColor(uint8_t red, uint8_t green, uint8_t blue, uint8_t strip_num, uint8_t pixel_index);
void WS2812_setPixelBrightness(uint8_t brightness, uint8_t strip_num, uint8_t pixel_index);
void WS2812_updateLEDs(int strip_index);

#endif // _WS2812B_H_
