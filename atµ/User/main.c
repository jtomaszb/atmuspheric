#include <math.h>
#include "stm32f4xx.h"
#include "ws2812b.h"
#include "usart.h"
#include "tm_stm32f4_swo.h"
#include "tm_stm32f4_delay.h"

int main(void) {
	int i;
	uint8_t j = 0;
	
	TM_SWO_Init();
	TM_DELAY_Init();
	
	WS2812_init();
	init_USART1(9600); // initialize USART1 @ 9600 baud

  USART_puts(USART1, "Init complete! Hello World!\r\n"); // just send a message to indicate that it works

	TM_SWO_Printf("Hello from MCU via SWO\n");
	
	while (1)
	{	
//		/* Print via SWO */
//		TM_SWO_Printf("%d\n", TM_DELAY_Time());
//		
//		/* Delay some time */
//		Delayms(500);
		
		for (i = 0; i < STRIP_LEN; i++)
		{
			WS2812_setPixelColor( 110 - 10 * i, 0, 10 * i, 0, i); 
		}
		
		WS2812_updateStrip(0);
		Delay(10000L);
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
}
