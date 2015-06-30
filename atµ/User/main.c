#include <math.h>
#include "stm32f4xx.h"
#include "ws2812b.h"
#include "usart.h"
#include "tm_stm32f4_swo.h"
#include "tm_stm32f4_delay.h"
#include "auto_sampler.h"
#include "fft_processor.h"

int main(void) {
	int i;
	uint8_t j = 0;
	uint8_t k = 0;
	uint8_t ascending = 1;
	
	TM_SWO_Init();
	TM_DELAY_Init();
	AutoSampler_Init();	
	
	WS2812_init();
	init_USART1(9600); // initialize USART1 @ 9600 baud

  USART_puts(USART1, "Init complete! Hello World!\r\n"); // just send a message to indicate that it works

	TM_SWO_Printf("Hello from MCU via SWO\n");
	
	FFTProcessor_Run();
	
	while (1)
	{	
		for (j = 0; j < NUM_STRIPS; j++)
		{
			for (i = 0; i < STRIP_LEN; i++)
			{
				WS2812_setPixelColor( 255 - k, k, 0, j, i); 
			}
			
			
			if (ascending == 1)
			{
				if (k < 255)
					k++;
				else
					ascending = 0;
			}
			else if (ascending == 0)
			{
				if (k > 0)
					k--;
				else
					ascending = 1;
			}
			
		}
		Delay(10000UL);
		WS2812_updateLEDs();
	}
}
	/* workloop */
	
	/*
	while (1)
	{	
		process_audio()
			-- perform FFT, etc. on ADC data
		
		read_serial_bt_buffer()
			-- read in any commands received and update state accordingly
		
		update_strip_colors()
			-- update RGB structs (wait to store in PWM DMA, creating DMA buffers for all strips = ~7k of space!)
			
		kickoff_strip_update()
			-- load first strip color info into DMA buffer
			-- reset current strip index to first strip
			-- maybe queue should be used instead??
	}
	*/
	
	/* interrupt/timer-based events */
	
	/*
			sample_adc()
				-- requires rigid timing
	
			update_next_strip()
				-- translate RGB values to PWM delays
				-- start DMA
	*/
