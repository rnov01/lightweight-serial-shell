#include <serial_shell/serial_shell.h>
#include <serial_shell/ring_buf.h>
#include "user_commands.h"
#include "uart.h"
#include <stdint.h>
#include <stdlib.h>

volatile char rx = 0;

static shell_t sh;
ringbuf_t rx_buffer = {{0}, 0, 0};

//interrupt grabs bytes from usart and feed them into buffer where they wait to be picked up by shell
void USART2_IRQHandler(void) {
    if (USART2_SR & (USART_SR_RXNE)) {
        rx = (uint8_t)USART2_DR;
        buf_push(&rx_buffer, rx);
    }
}

static void shell_putc(void* user, char c) {
    stm_putc(c, user);
}

static void led_init(void) {
    RCC_AHB1ENR |= (1U << 0);

    GPIOA_MODER &= ~(3U <<(LED_PIN * 2));
    GPIOA_MODER |= (1U << (LED_PIN * 2));

    GPIOA_OTYPER &= ~(1U << LED_PIN);

    GPIOA_ODR &= ~(1U << LED_PIN);
}

int main( void ) {

    uart2_init();
    led_init();
    sh_init(&sh, user_cmds, shell_putc, NULL, user_cmd_count);
 
    //poll buffer regularly to check for commands
    while (1) {
        uint8_t c;
        if (buf_pop(&rx_buffer, &c)) {
           sh_poll(&sh, c);
        }
    }

}