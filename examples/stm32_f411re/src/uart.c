#include "uart.h"
#include <stdint.h>

#define APB1_CLOCK_HZ 16000000UL
#define UART_BAUD     115200UL

void uart2_init(void) {
	RCC_AHB1ENR |= RCC_GPIOAEN;
    RCC_APB1ENR |= RCC_USART2EN;
    (void)RCC_APB1ENR;

    /* PA2 = USART2 TX, PA3 = USART2 RX. Alternate function mode = 10. */
    GPIOA_MODER = (GPIOA_MODER & ~((3UL << 4) | (3UL << 6)))
                  | (2UL << 4) | (2UL << 6);

    /* AF7 for USART2 on PA2/PA3. */
    GPIOA_AFRL = (GPIOA_AFRL & ~((15UL << 8) | (15UL << 12)))
                 | (7UL << 8) | (7UL << 12);

    USART2_CR1 = 0;
    USART2_BRR = (APB1_CLOCK_HZ + UART_BAUD / 2U) / UART_BAUD;
    USART2_CR1 = USART_CR1_TE | USART_CR1_RE | USART_CR1_UE;

    //NVIC_EnableIRQ(USART2_IRQn); //interrupt
	USART2_CR1 |= USART_CR1_RXNEIE;
	NVIC_ISER1 = (1UL << (USART2_IRQN - 32U));


}

void uart2_write(uint8_t c) {
    while(!(USART2_SR & USART_SR_TXE)){}
    USART2_DR = (c & 0xFF); 
}

void stm_putc(char c, void* user) {
	(void)user;
    uart2_write((uint8_t)c);
}

void stm_puts(const char* s, void* user) {
	(void)user;
    while(*s) {
        uart2_write(*s++);
    }
}