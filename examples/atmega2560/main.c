#include <serial_shell/serial_shell.h>
#include <serial_shell/ring_buf.h>
#include <stdint.h>
#include <stdlib.h>
#include <avr/interrupt.h>
#include <avr/io.h>

#include "avr_usart.h"

static shell_t sh;
ringbuf_t rx_buffer = {{0}, 0, 0};

ISR(USART0_RX_vect) {
    uint8_t c = UDR0;
    buf_push(&rx_buffer, c);
}


static void shell_putc(void* user, char c) {
    usart_t* u = (usart_t*)user;
    usart_putc(u, c);
}

 usart_t USART0 = {
    /*.put_c = usart_putc,*/
    .UDRn = &UDR0,
    .UCSRnA = &UCSR0A,
    .UCSRnB = &UCSR0B,
    .UCSRnC = &UCSR0C,
    .UBRRn = &UBRR0, 
    .sub = {
        .rxcie = true,
        .txcie = false,
        .rxen = true,
        .txen = true
    }
};

int main() {

    usart_init(&USART0, 19200, 8);
    sei();

    sh_init(&sh, NULL, shell_putc, &USART0, 0);

    //poll buffer regularly to check for commands in global buffer
    while (1) {
    
        uint8_t c;
        if (buf_pop(&rx_buffer, &c)) {
           sh_poll(&sh, c);
        }

    }

    return 0;
}
