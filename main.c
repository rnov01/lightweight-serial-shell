
#include "core_c/src/shell.h"
#include "ring_buf.h"
#include "core_c/src/shell_sys.h"
#include "avr_usart.h"
//#include "oled_ssd1306.h"
//#include "2560_twi.h"

#include <stdint.h>
#include <util/delay.h>
#include <avr/interrupt.h>
#include <avr/io.h>
#include <string.h>
#include <stdio.h>
#include <stdbool.h>

static shell_t sh;
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

/*static int twi_write_wrap(void* instance, uint8_t addr7, const uint8_t* data, size_t len) {
    (void)instance;
    i2c_start();
    i2c_write((uint8_t)(addr7 << 1));

    for(size_t i=0; i<len;i++) {
        i2c_write(data[i]);
    }

    i2c_stop();
    return 0;
}*/

/*void cmd_led(shell_t* sh, const shell_cmd_t* self, const char* param, int argc, char** argv);

static const shell_cmd_t user_commands[] = {
    {"led", "toggle led", &cmd_led, NULL, .sub = { CMD_NO_FLAG, NOPARAM, false, 0}},
};

void cmd_led(shell_t* sh, const shell_cmd_t* self, const char* param, int argc, char** argv) {
    
    PINB = (1<<PB7);
}*/

int main() {

    
   // i2c_init();
    

    /*i2c_bus_t twi_bus = {
        .instance = NULL,
        .write = twi_write_wrap
    };*/

    
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

    /*ssd1306_t oled = {
        .bus = twi_bus,
        .addr7 = 0x3C,
        //.framebuf = v_buf,
        .width = 128,
        .height = 64,
        .rotation = 0
    };*/



    usart_init(&USART0, 19200, 8);
    _delay_ms(300);
    //ssd1306_init(&oled);

   /*memset(oled.framebuf, 0x00, 1024);
    memset(&oled.framebuf[0], 0xFF, 128);
    ssd1306_update(&oled);

    memset(oled.framebuf, 0x00, sizeof(oled.framebuf));
    ssd1306_draws(&oled, 18, 3, "Merry Christmas!");
    ssd1306_draw_rect(&oled, 5, 16, 120, 20);
    ssd1306_update(&oled);*/

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