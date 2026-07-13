#include "avr_usart.h"

/* init for commonly adjusted USART parameters. 
 * other parameters can be configured manually if necessary */
void usart_init(usart_t* self, uint16_t baud, uint8_t char_size) {
    /* for instanced putc function */
    #if USART_USE_VTABLE
    self->put_c = usart_putc;
    #endif

    if (self->sub.rxen){ *self->UCSRnB |= (1<<RXEN);}
    if (self->sub.txen){ *self->UCSRnB |= (1<<TXEN);}
    if (self->sub.rxcie){*self->UCSRnB |= (1<<RXCIE);}
    if (self->sub.txcie){*self->UCSRnB |= (1<<TXCIE);}

    *self->UCSRnC &= ~((1<<UCSZ0) | (1<<UCSZ1));
    *self->UCSRnB &= ~(1<<UCSZ2);
    switch(char_size) {
        case 5: //already cleared
            break;
        case 6:
            *self->UCSRnC |= (1<<UCSZ0);
            break;
        case 7:
            *self->UCSRnC |= (1<<UCSZ1);
            break;
        case 8:
            *self->UCSRnC |= (1<<UCSZ0) | (1<<UCSZ1);
            break;
        case 9:
            *self->UCSRnC |= (1<<UCSZ0) | (1<<UCSZ1);
            *self->UCSRnB |= (1<<UCSZ2);
            break;
    }
    *self->UBRRn = (uint16_t)(F_CPU / ( 16UL * (uint32_t)baud)) - 1;
}

void usart_putc(usart_t* self, char c) {
    while (!(*self->UCSRnA & (1<<UDRE)));
    *self->UDRn = (uint8_t)c;
}

void usart_puts(usart_t* self, const char* s) {
    while (*s) {
        usart_putc(self, *s++);
    }
}

void usart_puts_P(usart_t* self, const char* s) {
    char c;

    while(1) {
        __asm__ __volatile__ (
            "lpm %0, Z+" 
            : "=r"(c), "+z"(s)
        );

        if (c=='\0') break;

        usart_putc(self, c);
    }
}