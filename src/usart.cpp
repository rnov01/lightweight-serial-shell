#include "../include/usart.h"
#include "../include/pin.h"

#include <stdint.h>

USART::USART(
        const pin& tx_x,                //tx pin
        const pin& rx_x,                //rx pin
        uint16_t ucsrnA_addr,           //A reg addr
        uint16_t ucsrnB_addr,           //B reg addr
        volatile uint16_t ubrrn_addr,   //BR reg addr
        uint16_t udrn_addr)             //data reg addr
    : tx_pin(tx_x),
      rx_pin(rx_x),
      UCSRnA(reinterpret_cast<volatile uint8_t*>(ucsrnA_addr)),
      UCSRnB(reinterpret_cast<volatile uint8_t*>(ucsrnB_addr)),
      UBRRn(reinterpret_cast<volatile uint16_t*>(ubrrn_addr)),
      UDRn(reinterpret_cast<volatile uint8_t*>(udrn_addr)) {}

void USART::init(unsigned long baud_rate, 
                rx_int_mode rxInt, 
                tx_int_mode txInt, 
                data_int_mode datInt,
                rx_mode rxEn,
                tx_mode txEn,
                spd_mode speed) {
    
    //config i/o
    set_output(tx_pin);
    set_input(rx_pin);

    if (rxInt == rx_int_mode::en)
        *UCSRnB |= (1<<RXCIE0);
    else 
        *UCSRnB &= ~(1<<RXCIE0);
    
    if (txInt == tx_int_mode::en)
        *UCSRnB |= (1<<TXCIE0);
    else 
        *UCSRnB &= ~(1<<TXCIE0);

    if (datInt == data_int_mode::en)
        *UCSRnB |= (1<<UDRIE0);
    else
        *UCSRnB &= ~(1<<UDRIE0);

    if (rxEn == rx_mode::en)
        *UCSRnB |= (1<<RXEN0);
    else 
        *UCSRnB &= ~(1<<RXEN0);

    if (txEn == tx_mode::en)
        *UCSRnB |= (1<<TXEN0);
    else 
        *UCSRnB &= ~(1<<TXEN0);

    if (speed == spd_mode::doub) {
        *UCSRnA |= (1<<U2X0);
        BAUD_prescaler = 8;
    } else if (speed == spd_mode::norm) {
        *UCSRnA &= ~(1<<U2X0);
        BAUD_prescaler = 16;
    }
    //calculate baud rate
    *UBRRn = (unsigned int)((F_CPU / (BAUD_prescaler * baud_rate)) - 1);
}

bool USART::ready(void) { 
    return (*UCSRnA & (1 << UDRE0)); 
} 

void USART::write(uint8_t data) { 
    while (!(*UCSRnA & (1 << UDRE0))); 
    *UDRn = data; 
} 

uint8_t USART::receive(void) { 
    while (!(*UCSRnA & (1<<RXC0)));
    return *UDRn; 
} 

void USART::put_string(const char* str) { 
    while (*str) { 
        while (!ready()); 
        write(*str++); 
    } 
    //clear_buffer(str); 
} 
void USART::put_stringP(PGM_P str) { 
    while(1) { 
        uint8_t b = pgm_read_byte_near(str++); 
        if(!b) 
        break; 
        putc(b); 
    } 
} 

void putc_wrapper(char c, void* self) {
    USART* u = (USART*)self;
    u->putc(c);
}
void puts_wrapper(const char* s, void* self) {
    USART* u = (USART*)self;
    u->put_string(s);
}
void putsP_wrapper(const char* s, void* self) {
    USART* u = (USART*)self;
    u->put_stringP(s);
}