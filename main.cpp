extern "C" {
  #include "core_c/include/shell.h"
  #include "core_c/include/shell_sys.h"
}


#include "legacy_cpp/include/usart.h"
#include "legacy_cpp/include/pin.h"
#include "legacy_cpp/include/ring_buffer.h"
#include <stdint.h>
#include <util/delay.h>
#include <avr/pgmspace.h>
#include <avr/interrupt.h>
//#include <avr/io.h>
#include <string.h>
#include <stdio.h>
#include <stdbool.h>
static shell_t sh;
//ringbuf GLOBAL_BUFFER = {{0}, 0, 0};
//grab usart bytes and store them in buffer
ISR(USART0_RX_vect) {
    uint8_t c = UDR0;
    //buf_push(&GLOBAL_BUFFER, c);
     
    sh_poll(&sh, c);

}

/*void cmd_led(shell_t* sh, const shell_cmd_t* self, const char* param, int argc, char** argv);

static const shell_cmd_t user_commands[] = {
    {"led", "toggle led", &cmd_led, NULL, .sub = { CMD_NO_FLAG, NOPARAM, false, 0}},
};

void cmd_led(shell_t* sh, const shell_cmd_t* self, const char* param, int argc, char** argv) {
    
    PINB = (1<<PB7);
}*/


int main() {

    //declare usart0 object and connect registers
   USART USART0(PIN_E1, PIN_E0, _SFR_MEM_ADDR(UCSR0A), _SFR_MEM_ADDR(UCSR0B), _SFR_MEM_ADDR(UBRR0), _SFR_MEM_ADDR(UDR0));
    //initialize usart0 with rx interrupt enabled
    USART0.init(19200, rx_int_mode::en, tx_int_mode::dis, 
        data_int_mode::dis, rx_mode::en, tx_mode::en, spd_mode::norm);
    //enable global interrupts

    sei();
    DDRB |= (1<<PB7);
    static char line_buf[32];
    static char* argv[8];

    //connect user-wrapped functions to shell interface
    //static shell_t sh;
  
    //initialize the shell with user tables in user_commands.cpp
    sh_init(&sh, NULL, line_buf, 32, argv, 8, putc_wrapper, &USART0, 0);

    //poll buffer regularly to check for commands in global buffer
    while (1) {
        uint8_t c;
        /*if (buf_pop(&GLOBAL_BUFFER, &c)) {
            sh_poll(&sh, c);
        }*/
        
    
    }
    return 0;
}