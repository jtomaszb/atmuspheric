#include <math.h>
#include "stm32f4xx.h"
#include "ws2812b.h"
#include <string.h>

TIM_TimeBaseInitTypeDef  TIM_TimeBaseStructure[NUM_STRIPS];
TIM_OCInitTypeDef  TIM_OCInitStructure[NUM_STRIPS];
GPIO_InitTypeDef GPIO_InitStructure[NUM_STRIPS];
DMA_InitTypeDef DMA_InitStructure[NUM_STRIPS];

/* Buffer that holds one complete DMA transmission
 * 
 * Ensure that this buffer is big enough to hold
 * all data bytes that need to be sent
 * 
 * The buffer size can be calculated as follows:
 * number of LEDs * 24 bytes + 42 bytes
 * 
 * This leaves us with a maximum string length of
 * (2^16 bytes per DMA stream - 42 bytes)/24 bytes per LED = 2728 LEDs
 */
uint16_t LED_BYTE_Buffer[NUM_STRIPS][STRIP_LEN * 24 + 42];	

TIM_TypeDef* pwm_outputs[7] = {PWM_TIMER_0, PWM_TIMER_1, PWM_TIMER_2, PWM_TIMER_3, PWM_TIMER_4, PWM_TIMER_5, PWM_TIMER_6};
DMA_Stream_TypeDef* dma_streams[7] = {DMA_STREAM_0, DMA_STREAM_1, DMA_STREAM_2, DMA_STREAM_3, DMA_STREAM_4, DMA_STREAM_5, DMA_STREAM_6};
uint32_t dma_channels[7] = {DMA_CHANNEL_0, DMA_CHANNEL_1, DMA_CHANNEL_2, DMA_CHANNEL_3, DMA_CHANNEL_4, DMA_CHANNEL_5, DMA_CHANNEL_6};
uint32_t dma_interrupt_flags[7] = {DMA_FLAG_TCIF5, DMA_FLAG_TCIF5, DMA_FLAG_TCIF5, DMA_FLAG_TCIF5, 0, 0, 0};

uint16_t led_output_pins[NUM_STRIPS] = {0};//GPIO_Pin_4, GPIO_Pin_13, GPIO_Pin_14, GPIO_Pin_15, 0, 0, 0};

uint8_t gPixels[NUM_STRIPS * STRIP_LEN * PIXEL_SIZE];
uint8_t gPixelBrightness[NUM_STRIPS * STRIP_LEN];

uint8_t curr_strip = 0;

uint8_t transfer_in_progress = 0;
uint8_t current_strip = 4;

static void DMA_init(void);

void WS2812_init(void)
{
	uint8_t i;
	
	/* Compute the prescaler value */
	RCC_ClocksTypeDef clocks;
	
	RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOA, ENABLE);
	RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOB, ENABLE);
	
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM3, ENABLE);
//	RCC_APB1PeriphClockCmd(RCC_AP	B1Periph_TIM4, ENABLE);
	
	GPIO_InitStructure[0].GPIO_Pin = GPIO_Pin_0 | GPIO_Pin_1 | GPIO_Pin_4 | GPIO_Pin_5; // | GPIO_Pin_14 | GPIO_Pin_15;
	GPIO_InitStructure[0].GPIO_Mode = GPIO_Mode_AF;
	GPIO_InitStructure[0].GPIO_Speed = GPIO_Speed_50MHz;
	
//	GPIO_InitStructure[1].GPIO_Pin = led_output_pins[2] | led_output_pins[3];
//	GPIO_InitStructure[1].GPIO_Mode = GPIO_Mode_AF;
//	GPIO_InitStructure[1].GPIO_Speed = GPIO_Speed_50MHz;

	GPIO_Init(GPIOB, &GPIO_InitStructure[0]);

	GPIO_PinAFConfig(GPIOB, GPIO_PinSource0, GPIO_AF_TIM3);
	GPIO_PinAFConfig(GPIOB, GPIO_PinSource1, GPIO_AF_TIM3);
	GPIO_PinAFConfig(GPIOB, GPIO_PinSource4, GPIO_AF_TIM3);
	GPIO_PinAFConfig(GPIOB, GPIO_PinSource5, GPIO_AF_TIM3);

//	GPIO_PinAFConfig(GPIOD, GPIO_PinSource12, GPIO_AF_TIM4);
//	GPIO_PinAFConfig(GPIOD, GPIO_PinSource13, GPIO_AF_TIM4);
//	GPIO_PinAFConfig(GPIOD, GPIO_PinSource14, GPIO_AF_TIM4);
//	GPIO_PinAFConfig(GPIOD, GPIO_PinSource15, GPIO_AF_TIM4);

	RCC_GetClocksFreq(&clocks);

	for (i = 0; i < NUM_STRIPS; i++)
	{
		/* Time base configuration */
		TIM_TimeBaseStructure[i].TIM_Period = TIM_PERIOD; // 800kHz 
		TIM_TimeBaseStructure[i].TIM_Prescaler = 3;
		TIM_TimeBaseStructure[i].TIM_ClockDivision = 0;
		TIM_TimeBaseStructure[i].TIM_CounterMode = TIM_CounterMode_Up;

		//TIM_TimeBaseInit(pwm_outputs[i], &TIM_TimeBaseStructure[i]);
	}

		TIM_TimeBaseInit(TIM3, &TIM_TimeBaseStructure[0]);
	
	/* PWM1 Mode configuration: Channel1 */


	for (i = 0; i < NUM_STRIPS; i++)
	{
		TIM_OCInitStructure[i].TIM_OCMode = TIM_OCMode_PWM1;
		TIM_OCInitStructure[i].TIM_OutputState = TIM_OutputState_Enable;
		TIM_OCInitStructure[i].TIM_Pulse = 0;
		TIM_OCInitStructure[i].TIM_OCPolarity = TIM_OCPolarity_High;
	}

		TIM_OC1Init(pwm_outputs[0], &TIM_OCInitStructure[0]);
//		TIM_OC2Init(pwm_outputs[1], &TIM_OCInitStructure[1]);
//		TIM_OC3Init(pwm_outputs[2], &TIM_OCInitStructure[2]);

	/***
	 * Must enable reload for PWM (STMicroelectronicd RM0090 section 18.3.9
	 * "PWM mode":
	 *
	 *   You must enable the corresponding preload register by setting the
	 *   OCxPE bit in the TIMx_CCMRx register.
	 *
	 * This is part of the fix for the pulse corruption (the runt pulse at
	 * the start and the extra pulse at the end).
	 */
		TIM_OC1PreloadConfig(pwm_outputs[0], TIM_OCPreload_Enable);
		TIM_OC2PreloadConfig(pwm_outputs[1], TIM_OCPreload_Enable);
		TIM_OC3PreloadConfig(pwm_outputs[2], TIM_OCPreload_Enable);
	
	DMA_init();
}

static void DMA_init(void)
{
	uint8_t i;
	
	/* configure DMA */
	/* DMA clock enable */
	RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_DMA1, ENABLE);

	for (i = 0; i < NUM_STRIPS; i++)
	{
		/* DMA1 Channel6 Config */
		DMA_DeInit(dma_streams[i]);

		DMA_InitStructure[i].DMA_BufferSize = 42;
		DMA_InitStructure[i].DMA_Channel = dma_channels[i];
		DMA_InitStructure[i].DMA_DIR = DMA_DIR_MemoryToPeripheral;					// data shifted from memory to peripheral
		DMA_InitStructure[i].DMA_FIFOMode = DMA_FIFOMode_Disable;
		DMA_InitStructure[i].DMA_FIFOThreshold = DMA_FIFOThreshold_HalfFull;

		DMA_InitStructure[i].DMA_Memory0BaseAddr = (uint32_t)LED_BYTE_Buffer[i];		// this is the buffer memory 
		DMA_InitStructure[i].DMA_MemoryDataSize = DMA_MemoryDataSize_HalfWord;
		DMA_InitStructure[i].DMA_MemoryInc = DMA_MemoryInc_Enable;					// automatically increase buffer index
		DMA_InitStructure[i].DMA_MemoryBurst = DMA_MemoryBurst_Single;

		DMA_InitStructure[i].DMA_Mode = DMA_Mode_Normal;							// stop DMA feed after buffer size is reached

		if (i == 0)
			DMA_InitStructure[i].DMA_PeripheralBaseAddr = (uint32_t)&TIM3->CCR1;	// physical address of Timer 3 CCR1
		else if (i == 1)
			DMA_InitStructure[i].DMA_PeripheralBaseAddr = (uint32_t)&TIM3->CCR2;	// physical address of Timer 3 CCR1
		else if (i == 2)
			DMA_InitStructure[i].DMA_PeripheralBaseAddr = (uint32_t)&TIM3->CCR3;	// physical address of Timer 3 CCR1
		else if (i == 3)
			DMA_InitStructure[i].DMA_PeripheralBaseAddr = (uint32_t)&TIM3->CCR4;	// physical address of Timer 3 CCR1
		
		DMA_InitStructure[i].DMA_PeripheralDataSize = DMA_PeripheralDataSize_HalfWord;
		DMA_InitStructure[i].DMA_PeripheralInc = DMA_PeripheralInc_Disable;
		DMA_InitStructure[i].DMA_PeripheralBurst = DMA_PeripheralBurst_Single;

		DMA_InitStructure[i].DMA_Priority = DMA_Priority_High;
		
		DMA_Init(dma_streams[i], &DMA_InitStructure[i]);
		
		/* PWM_TIMER CC1 DMA Request enable */
		TIM_DMACmd(pwm_outputs[i], DMA_SOURCE, ENABLE);
	}
}

//void WS2812_updateStrip(uint8_t strip_index)
//{
//	if (strip_index != curr_strip && strip_index < NUM_STRIPS)
//	{
//		curr_strip = strip_index;

//		// UNDO PWM PIN CONFIG FOR ALL PINS
//		GPIO_InitStructure.GPIO_Pin = GPIO_Pin_6;
//		GPIO_InitStructure.GPIO_Mode = GPIO_Mode_OUT;
//		GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_DOWN;
//		GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
//		GPIO_Init(GPIOA, &GPIO_InitStructure);

//		GPIO_InitStructure.GPIO_Pin = GPIO_Pin_4;
//		GPIO_Init(GPIOB, &GPIO_InitStructure);

//		GPIO_InitStructure.GPIO_Pin = GPIO_Pin_6;
//		GPIO_Init(GPIOC, &GPIO_InitStructure);

//		// SET UP TO CONFIG PIN FOR PWM
//		GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF;
//		GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
//		
//		switch (curr_strip)
//		{
//			case 0:
//				GPIO_InitStructure.GPIO_Pin = GPIO_Pin_4;
//				GPIO_Init(GPIOB, &GPIO_InitStructure);
//				GPIO_PinAFConfig(GPIOB, GPIO_PinSource4, GPIO_AF_TIM3);
//				break;

//			case 1:
//				GPIO_InitStructure.GPIO_Pin = GPIO_Pin_6;
//				GPIO_Init(GPIOA, &GPIO_InitStructure);
//				GPIO_PinAFConfig(GPIOA, GPIO_PinSource6, GPIO_AF_TIM3);
//				break;

//			case 2:
//				GPIO_InitStructure.GPIO_Pin = GPIO_Pin_6;
//				GPIO_Init(GPIOC, &GPIO_InitStructure);
//				GPIO_PinAFConfig(GPIOC, GPIO_PinSource6, GPIO_AF_TIM3);
//				break;
//			
//			default:
//				break;
//		}
//	}
//	else if (strip_index >= NUM_STRIPS)
//	{
//		return;
//	}
//	
//	WS2812_send(gPixels + STRIP_LEN * PIXEL_SIZE * strip_index, gPixelBrightness + STRIP_LEN * strip_index, STRIP_LEN);
//}

void WS2812_updateLEDs(int strip_index)
{
	uint8_t i;
	
	WS2812_send(&gPixels[STRIP_LEN * PIXEL_SIZE * strip_index], &gPixelBrightness[STRIP_LEN * strip_index], STRIP_LEN, strip_index);
}

void WS2812_clearPixel(uint8_t strip_num, uint8_t pixel_index)
{
	WS2812_setPixelColor(0, 0, 0, strip_num, pixel_index);
}	

void WS2812_setPixelColor(uint8_t red, uint8_t green, uint8_t blue,  uint8_t strip_num, uint8_t pixel_index)
{
	gPixels[(strip_num * STRIP_LEN * PIXEL_SIZE) + (PIXEL_SIZE * pixel_index) + R_OFFSET] = red;
	gPixels[(strip_num * STRIP_LEN * PIXEL_SIZE) + (PIXEL_SIZE * pixel_index) + G_OFFSET] = green;
	gPixels[(strip_num * STRIP_LEN * PIXEL_SIZE) + (PIXEL_SIZE * pixel_index) + B_OFFSET] = blue;
}

void WS2812_setPixelBrightness(uint8_t brightness, uint8_t strip_num, uint8_t pixel_index)
{
	gPixelBrightness[STRIP_LEN * strip_num + pixel_index] = brightness;
}

static void WS2812_colorToBitArray(uint8_t color_val, uint16_t* byte_array)
{
	uint8_t memaddr = 0;
	uint8_t j = 0;
	
	for (j = 0; j < 8; j++)
	{
		if ( (color_val << j) & 0x80 )	// data sent MSB first, j = 0 is MSB j = 7 is LSB
		{
			byte_array[memaddr] = TIM_COMPARE_HIGH;	// compare value for logical 1
		}
		else
		{
			byte_array[memaddr] = TIM_COMPARE_LOW;		// compare value for logical 0
		}
		memaddr++;
	}
}

/* This function sends data bytes out to a string of WS2812s
 * The first argument is a pointer to the first RGB triplet to be sent
 * The seconds argument is the number of LEDs in the chain
 * 
 * This will result in the RGB triplet passed by argument 1 being sent to 
 * the LED that is the furthest away from the controller (the point where
 * data is injected into the chain)
 */

void WS2812_send(const uint8_t* pixels, const uint8_t* pixel_brightness, const uint16_t _len, uint8_t strip_num)
{
	uint8_t led;
	uint16_t buffersize;
	uint16_t len = _len;

	buffersize = (len*24)+42;	// number of bytes needed is #LEDs * 24 bytes + 42 trailing bytes
	led = 0;					// reset led index

	// fill transmit buffer with correct compare values to achieve
	// correct pulse widths according to color values
	while (len)
	{
		WS2812_colorToBitArray(apply_brightness(pixels[led * PIXEL_SIZE + G_OFFSET], pixel_brightness[led]), &LED_BYTE_Buffer[strip_num][led * 24]); 	
		WS2812_colorToBitArray(apply_brightness(pixels[led * PIXEL_SIZE + R_OFFSET], pixel_brightness[led]), &LED_BYTE_Buffer[strip_num][led * 24 + 8]); 	
		WS2812_colorToBitArray(apply_brightness(pixels[led * PIXEL_SIZE + B_OFFSET], pixel_brightness[led]), &LED_BYTE_Buffer[strip_num][led * 24 + 16]); 	
		
		led++;
		len--;
	}
	
	// add needed delay at end of byte cycle, pulsewidth = 0
	memset((void *)&LED_BYTE_Buffer[strip_num][led * 24], 0, 42 * sizeof(uint16_t));

	DMA_SetCurrDataCounter(dma_streams[strip_num], buffersize); 	// load number of bytes to be transferred

	// PAP: Clear the timer's counter and set the compare value to 0. This
	// sets the output low on start and gives us a full cycle to set up DMA.
	TIM_SetCounter(pwm_outputs[strip_num], 0);

	if (strip_num == 0)
		TIM_SetCompare1(pwm_outputs[strip_num], 0);
	else if (strip_num == 1)
		TIM_SetCompare2(pwm_outputs[strip_num], 0);
	else if (strip_num == 2)
		TIM_SetCompare3(pwm_outputs[strip_num], 0);
		
	TIM_Cmd(pwm_outputs[strip_num], ENABLE); 						// enable Timer 3
	
	// PAP: Start DMA transfer after starting the timer. This prevents the
	// DMA/PWM from dropping the first bit.	
	transfer_in_progress = 1;
	DMA_Cmd(dma_streams[strip_num], ENABLE); 			// enable DMA channel 6
	
//	while(!DMA_GetFlagStatus(dma_streams[strip_num], dma_interrupt_flags[strip_num])); 	// wait until transfer complete
//	TIM_Cmd(pwm_outputs[strip_num], DISABLE); 					// disable Timer 3
//	DMA_Cmd(dma_streams[strip_num], DISABLE); 			// disable DMA channel 6
//	DMA_ClearFlag(dma_streams[strip_num], DMA_TCIF); 				// clear DMA1 Channel 6 transfer complete flag
}

