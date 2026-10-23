#pragma once
#include "stm32f411_defs.h"

#ifdef __cplusplus
extern "C" {
#endif

#define GPIOAEN 		(1U<<0)			//clock enable port a
#define UART2EN			(1U<<17)		//clock enable uart2
#define CR1_TE  		(1U<<3)
#define CR1_RE          (1U<<2)
#define CR1_RXNEIE      (1U<<5)
#define CR1_UE			(1U<<13)
#define SR_TXE  		(1U<<7)
#define SR_RXNE         (1U<<5)
#define SYS_FREQ		16000000		//default system clock frequency
#define APB1_CLK		SYS_FREQ
#define UART_BAUDRATE	115200

void uart2_init(void);
void uart2_write(uint8_t c);
void stm_putc(char c, void* user);
void stm_puts(const char* s, void* user);

#ifdef __cplusplus
}
#endif