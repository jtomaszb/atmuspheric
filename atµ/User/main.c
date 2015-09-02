/* Include core modules */
#include "stm32f4xx.h"

#include "auto_sampler.h"
#include "ws2812b.h"
#include "usart.h"
#include "dft_filter.h"
#include "cqt_filter.h"

// utility functions
void startup1(void);
void startup2(void);
void sg_delay(int count);

uint8_t height[7];
int prev_height[7];
uint8_t i, j, k, l, m;
float alpha = 0.9;
int direction  = 0;
int avg_height = 0;


void mapToHeight(float32_t* output, float32_t* maxima, uint8_t* height);

int main(void) {

	// Initilize modules
	AutoSampler_Init();
	init_USART1(9600);
	WS2812_init();
	CQT_Init();
	
	// Run statup sequence
//	startup1();
	
	// clear pixels
	for(k = 0; k < 7; k++) {
		for(j = 0; j < 11; j++) 
			WS2812_setPixelColor(0, 0, 0, k, j);
	}
	WS2812_updateLEDs();
	
	// run second startup
//	startup2();
	
	// Set initial color on tubes
	for(k = 0; k < 7; k++) {
		for(j = 0; j < STRIP_LEN; j++) 
			WS2812_setPixelColor(100, 10, 10, k, j);
	}
	
	k = 0;
	
	while (1)
	{
		int i;

		CQT_Process();
			
		// Update counter k that goes from 0 -> 255 -> 0 -> 255 and so on
		if (direction == 0)
		{
			if (k == (UINT8_MAX - 1))
				direction = 1;
			else
				k++;
		}
		else
		{
			if (k == 1)
				direction  = 0;
			else
				k--;
		}
		
		// Use counter to fade between blue and red, with green gradient along strips
		for (i = 0; i < NUM_STRIPS; i++)
		{
			for (l = 0; l < STRIP_LEN; l++)
			{
					WS2812_setPixelColor(UINT8_MAX - k, l * 10, k - UINT8_MAX, i, l);	
			}				
		}

		
		for (i = 0; i < NUM_STRIPS; i++)
		{
			prev_height[i] = height[i];
		}
	
		mapToHeight(cq_out, cq_max, height);
		
		for (i = 0; i < NUM_STRIPS; i++)
		{
			if (height[i] < prev_height[i])
				height[i] = prev_height[i] - 1;
		}
		
		for(i = 0; i < NUM_STRIPS; i++)
		{
			if(height[i] > STRIP_LEN * LEVELS_PER_PIXEL)
				height[i] = STRIP_LEN * LEVELS_PER_PIXEL;
			
			WS2812_setStripLevel(i, height[i]);
		}
		
		// compute average height among tubes for fun lighting stuff
		avg_height = 0;
		
		for (i = 0; i < NUM_STRIPS; i++)
		{
			avg_height += height[i];
		}
		avg_height /= NUM_STRIPS;
		
		j = avg_height;
			
		WS2812_updateLEDs();		
	}
}

void startup1() {
	
	for(j = 0; j < STRIP_LEN; j++) {
		WS2812_setPixelColor(100, 10, 10, 0, j);
		WS2812_updateStrip(0);
		sg_delay(10000);
	}
	
	for(k = 1; k < 9; k++) {
		for(j = 0; j < STRIP_LEN; j++) {
			WS2812_setPixelColor(100, 10, 10, k, j);
			WS2812_setPixelColor(0, 0, 0, k-1, 10-j);
			WS2812_updateStrip(k-1);
			WS2812_updateStrip(k);
			sg_delay(10000);
		}
	}
	
	for(j = 0; j < NUM_STRIPS; j++) {
		WS2812_setPixelColor(0, 0, 0, 8, j);
		WS2812_updateStrip(8);
		sg_delay(10000);
	}
}

void startup2() {
	
	// light up first strip
	for(j = 0; j < 11; j++) {
		WS2812_setPixelColor(100, 10, 10, 0, j);
		WS2812_updateStrip(0);
		sg_delay(10000);
	}
	// slow down light up second
	for(j = 0; j < 8; j++) {
		WS2812_setPixelColor(100, 10, 10, 1, j);
		WS2812_updateStrip(1);
		sg_delay(10000*(j+1)/2);
	}
	// flicker next light
	WS2812_setPixelColor(0, 10, 10, 1, 8);
	WS2812_updateStrip(1);
	sg_delay(1000);
	WS2812_setPixelColor(100, 10, 10, 1, 8);
	WS2812_updateStrip(1);
	sg_delay(1000);
	WS2812_setPixelColor(50, 10, 10, 1, 8);
	WS2812_updateStrip(1);
	sg_delay(1000);
	WS2812_setPixelColor(0, 10, 10, 1, 8);
	WS2812_updateStrip(1);
	sg_delay(1000);
	WS2812_setPixelColor(100, 10, 10, 1, 8);
	WS2812_updateStrip(1);
	sg_delay(1000);
	WS2812_setPixelColor(0, 10, 10, 1, 8);
	WS2812_updateStrip(1);
	sg_delay(1000);
	WS2812_setPixelColor(60, 10, 10, 1, 8);
	WS2812_updateStrip(1);
	sg_delay(1000);
	WS2812_setPixelColor(100, 10, 10, 1, 8);
	WS2812_updateStrip(1);
	sg_delay(1000);
	WS2812_setPixelColor(50, 10, 10, 1, 8);
	WS2812_updateStrip(1);
	sg_delay(1000);
	WS2812_setPixelColor(0, 10, 10, 1, 8);
	WS2812_updateStrip(1);
	sg_delay(1000);
	WS2812_setPixelColor(100, 10, 10, 1, 8);
	WS2812_updateStrip(1);
	sg_delay(1000);
	WS2812_setPixelColor(20, 10, 10, 1, 8);
	WS2812_updateStrip(1);
	sg_delay(1000);
	WS2812_setPixelColor(0, 10, 10, 1, 8);
	WS2812_updateStrip(1);
	sg_delay(1000);
	WS2812_setPixelColor(100, 10, 10, 1, 8);
	WS2812_updateStrip(1);
	sg_delay(1000);
	// big delay, make it look like stuck
	sg_delay(100000);
	// quickly light up the rest, gottem
	for(j = 9; j < 11; j++) {
		WS2812_setPixelColor(100, 10, 10, 1, j);
		WS2812_updateStrip(1);
		sg_delay(10000);
	}
	for(k = 2; k < 9; k++) {
		for(j = 0; j < 11; j++) {
			WS2812_setPixelColor(100, 10, 10, k, j);
			WS2812_updateStrip(k);
			sg_delay(10000/4);
		}
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
