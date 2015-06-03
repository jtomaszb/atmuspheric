#include <math.h>
#include "stm32f4xx.h"
#include "ws2812b.h"

void Delay(__IO uint32_t nCount) {
  while(nCount--) {
  }
}

int main(void) {
	int i;
	uint8_t j = 0;

	WS2812_init();

	while (1)
	{	
		for (i = 0; i < STRIP_LEN; i++)
		{
			WS2812_setPixelColor( 110 - 10 * i, 0, 10 * i, 0, i); 
		}
		
		WS2812_updateStrip(0);
		Delay(10000L);
	}
}
