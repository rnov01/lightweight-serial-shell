extern "C" {
  #include "core_c/include/shell.h"
  #include "core_c/include/shell_sys.h"
}


#include "legacy_cpp/include/usart.h"
#include "legacy_cpp/include/pin.h"
#include "legacy_cpp/include/ring_buffer.h"
#include <stdint.h>
#include <avr/delay.h>
#include <avr/pgmspace.h>
#include <avr/interrupt.h>
#include <string.h>
#include <stdio.h>

ringbuf GLOBAL_BUFFER = {{0}, 0, 0};
//grab usart bytes and store them in buffer
ISR(USART0_RX_vect) {
    uint8_t c = UDR0;
    buf_push(&GLOBAL_BUFFER, c);
}

int main() {

    //declare usart0 object and connect registers
    USART USART0(PIN_E1, PIN_E0, _SFR_MEM_ADDR(UCSR0A), _SFR_MEM_ADDR(UCSR0B), _SFR_MEM_ADDR(UBRR0), _SFR_MEM_ADDR(UDR0));
    //initialize usart0 with rx interrupt enabled
    USART0.init(19200, rx_int_mode::en, tx_int_mode::dis, 
        data_int_mode::dis, rx_mode::en, tx_mode::en, spd_mode::norm);
    //enable global interrupts
    sei();
   
    static char line_buf[32];
    static char* argv[8];
    uint8_t argc;
    //connect user-wrapped functions to shell interface
    static shell_t sh;
  
    //initialize the shell with user tables in user_commands.cpp
    sh_init(&sh, NULL, line_buf, 32, argv, 8, putc_wrapper, &USART0, 0);

    //poll buffer regularly to check for commands in global buffer
    while (1) {
        uint8_t c;
        if (buf_pop(&GLOBAL_BUFFER, &c)) {
            sh_poll(&sh, c);
        }
        
    
    }
    return 0;
}