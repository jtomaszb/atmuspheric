#include "stm32f4xx_usart.h" // under Libraries/STM32F4xx_StdPeriph_Driver/inc and src

#ifndef _USART_H_
#define _USART_H_

void init_USART1(uint32_t baudrate);
void USART_puts(USART_TypeDef* USARTx, volatile char *s);
void USART1_readBuffer(void);

#endif // _USART_H_
