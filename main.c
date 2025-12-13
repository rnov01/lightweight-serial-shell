
#include "core_c/include/shell.h"
#include "ring_buf.h"
#include "core_c/include/shell_sys.h"
#include "../usart_hal/c/include/avr_usart.h"

//#include "legacy_cpp/include/ring_buffer.h"
#include <stdint.h>
#include <util/delay.h>
#include <avr/pgmspace.h>
#include <avr/interrupt.h>
#include <avr/io.h>
#include <string.h>
#include <stdio.h>
#include <stdbool.h>
static shell_t sh;
//static usart_t USART0;
ringbuf_t GLOBAL_BUFFER = {{0}, 0, 0};
//grab usart bytes and store them in buffer
#if defined(__AVR_ATmega2560__)
ISR(USART0_RX_vect) {
    uint8_t c = UDR0;
    buf_push(&GLOBAL_BUFFER, c);
     
    //sh_poll(&sh, c);
}
#elif defined (__AVR_ATmega328P__)
ISR(USART_RX_vect) {
    uint8_t c = UDR0;
    //buf_push(&GLOBAL_BUFFER, c);
     
    sh_poll(&sh, c);
}
#endif

 void putc_wrap(void* user, char c) {
        usart_t* u = (usart_t*)user;
        //u->put_c(u, c);
        usart_putc(u, c);
    }

/*void cmd_led(shell_t* sh, const shell_cmd_t* self, const char* param, int argc, char** argv);

static const shell_cmd_t user_commands[] = {
    {"led", "toggle led", &cmd_led, NULL, .sub = { CMD_NO_FLAG, NOPARAM, false, 0}},
};

void cmd_led(shell_t* sh, const shell_cmd_t* self, const char* param, int argc, char** argv) {
    
    PINB = (1<<PB7);
}*/


int main() {

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

    usart_init(&USART0, 19200, 8);

    sei();
    DDRB |= (1<<PB7);
    static char line_buf[32];
    static char* argv[8];
  
    //initialize the shell with user tables in user_commands.cpp
    sh_init(&sh, NULL, line_buf, 32, argv, 8, putc_wrap, &USART0, 0);

    //poll buffer regularly to check for commands in global buffer
    while (1) {
        uint8_t c;
        if (buf_pop(&GLOBAL_BUFFER, &c)) {
            sh_poll(&sh, c);
        }
        
    
    }
    return 0;
}