#include "../include/usart.h"
#include "../include/pin.h"
#include "../include/ring_buffer.h"
#include "../include/shell_api.h"
#include "../include/user_commands.h"
#include <stdint.h>
#include <avr/delay.h>
#include <avr/pgmspace.h>
#include <avr/interrupt.h>
#include <string.h>

#define USART0_tx PE1
#define USART0_rx PE0
#define BUFFER_SIZE 32
#define ADDR8(reg) (*(volatile uint8_t*))(reg)\

/*typedef enum {
    DEBUG, PRINT, err
}COMMANDS;

struct str_to_cmd {
    const char* name;
    COMMANDS cmd; 
};

static const str_to_cmd COMMAND_LUT[] = {
    {"debug", COMMANDS::DEBUG},
    {"print", COMMANDS::PRINT},
    {nullptr, COMMANDS::err}
};

COMMANDS parse_command(char *buffer) {
    for (int i=0; COMMAND_LUT[i].name != nullptr; i++) {
        if (strcmp(buffer, COMMAND_LUT[i].name) == 0) {
            return COMMAND_LUT[i].cmd;
        }
    }
    return COMMANDS::err;
}

COMMANDS receive_command(USART *USARTn) {
    char rx_buffer[BUFFER_SIZE];
    volatile uint8_t rx_index = 0;
    char rx_char;
    //char rx_char = USARTn->receive();
    do {
        rx_char = USARTn->receive();
       
    } while (rx_char == '\r' || rx_char == '\n');

    while (1) {
        if (rx_char == '\r' || rx_char == '\n') {
            rx_buffer[rx_index] = '\0';
            break;
        } else {
            rx_buffer[rx_index++] = rx_char;
        }
         rx_char = USARTn->receive();
    }
    return parse_command(rx_buffer);
}*/
ISR(USART0_RX_vect) {
    uint8_t c = UDR0;
    buf_push(&GLOBAL_BUFFER, c);
}
shell* active_shell = nullptr;
int run_usart_test() {

    USART USART0(PIN_E1, PIN_E0, _SFR_MEM_ADDR(UCSR0A), _SFR_MEM_ADDR(UCSR0B), _SFR_MEM_ADDR(UBRR0), _SFR_MEM_ADDR(UDR0));
    USART0.init(19200, rx_int_mode::en, tx_int_mode::dis, 
        data_int_mode::dis, rx_mode::en, tx_mode::en, spd_mode::norm);
    
    shell_io_t io = {
        .putc = putc_wrapper,
        .puts = puts_wrapper,
        .puts_p = putsP_wrapper,
        .self = &USART0
    };

    shell sh(&io);  //init shell
    active_shell = &sh;
   


    sei();

        char letter = 'a';
        //constexpr pin PIN_C1 = {&DDRC, &PORTC, &PINC, PC1};
        set_output(PIN_C1);
        set_output(PIN_G0);
        
        set_output(PIN_B7);

        //g_shell = &sh;
        sh.init(usr_cmds, usr_flags);
    while (1) {
        sh.poll();
       
       /* USART0.put_stringP(PSTR("\nEnter a Command: "));
        USART0.put_string("Hello");
        USART0.put_string(" Hi");
        USART0.put_string(" Hello");
        switch (receive_command(&USART0)) {
            case DEBUG:
                USART0.put_stringP(PSTR("Debug mode activated"));
                break;
            case PRINT:
                USART0.put_stringP(PSTR("Firmware v2.2"));
                break;
            case err:
                USART0.put_stringP(PSTR("Please enter a valid command"));
                break;
        }*/

        //toggle(PIN_C1);
        
        /*if (USART0.receive() == 'a') {
        //if (letter <= 'z') {
            USART0.put_stringP(PSTR("Here is a char: "));
            USART0.putc(letter);
            USART0.putc('\n');
            letter++;
        } else {
                letter = 'a';
        } 
        } else {
            USART0.put_stringP(PSTR("not a valid command\n"));
        }*/



        //_delay_ms(400);
          
    }
}