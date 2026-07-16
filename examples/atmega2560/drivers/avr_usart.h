#pragma once 
#include <stdint.h>

#ifndef PROGMEM
#define PROGMEM __attribute__((progmem))
#endif

#ifndef F_CPU
#define F_CPU 16000000UL 
#endif

/* Toggle usart_t function pointers */
#define USART_USE_VTABLE 0

//#define __AVR_atmega2560__
#if defined(__AVR_atmega2560__)
#define F_CPU   16000000UL
#define UDR0    (*(volatile uint8_t*)0xC6)
#define UCSR0A  (*(volatile uint8_t*)0xC0)
#define UCSR0B  (*(volatile uint8_t*)0xC1)
#define UCSR0C  (*(volatile uint8_t*)0xC2)
#define UBRR0   (*(volatile uint16_t*)0xC4)

#define UDR1    (*(volatile uint8_t*)0xCE)
#define UCSR1A  (*(volatile uint8_t*)0xC8)
#define UCSR1B  (*(volatile uint8_t*)0xC9)
#define UCSR1C  (*(volatile uint8_t*)0xCA)
#define UBRR1   (*(volatile uint16_t*)0xCC)

#define UDR2    (*(volatile uint8_t*)0xD6)
#define UCSR2A  (*(volatile uint8_t*)0xD0)
#define UCSR2B  (*(volatile uint8_t*)0xD1)
#define UCSR2C  (*(volatile uint8_t*)0xD2)
#define UBRR2   (*(volatile uint16_t*)0xD4)

#define UDR3    (*(volatile uint8_t*)0x136)
#define UCSR3A  (*(volatile uint8_t*)0x130)
#define UCSR3B  (*(volatile uint8_t*)0x131)
#define UCSR3C  (*(volatile uint8_t*)0x132)
#define UBRR3   (*(volatile uint16_t*)0x134)
#endif 

typedef struct usart usart_t;
typedef enum {
    MPCM,          /* Multi-processor communication mode */
    U2X,           /* Double USART transmission speed */
    UPE,           /* USART parity error*/
    DOR,           /* Data overrun */
    FE,            /* Fram error */
    UDRE,          /* USART data reg empty */
    TXC,           /* USART transmit complete */
    RXC            /* USART receive complete */
} UCSRnA_t;

typedef enum {
    TXB8,          /* Transmit data bit 8 (9th)*/
    RXB8,          /* Receive data bit 8 (9th)*/
    UCSZ2,         /* Character size*/
    TXEN,          /* Transmit enable */
    RXEN,          /* Receiver enable */
    UDRIE,         /* USART data reg empty interrupt enable */
    TXCIE,         /* TX complete interrupt enable */
    RXCIE,         /* RX complete interrupt enable */
} UCSRnB_t;

typedef enum {
    UCPOL,          /* Clock polarity (synchronous mode) */
    UCSZ0,          /* Character size */
    UCSZ1,          /* Character size */
    USBS,           /* Stop bit select */
    UPM0,           /* Parity mode */
    UPM1,           /* Parity mode */
    UMSEL0,         /* USART mode select */
    UMSEL1,         /* USART mode select */
} UCSRnC_t;

void usart_init(usart_t* self, uint16_t baud, uint8_t char_size);
void usart_putc(usart_t* self, char c);
void usart_puts(usart_t* self, const char* s);
void usart_puts_P(usart_t* self, const char* s);

struct usart {
    #if USART_USE_VTABLE
    void     (*put_c)(usart_t* self, char c);
    #endif
    volatile uint8_t*   UDRn;
    volatile uint8_t*   UCSRnA;
    volatile uint8_t*   UCSRnB;
    volatile uint8_t*   UCSRnC;
    volatile uint16_t*   UBRRn;
    struct {
        unsigned rxen   : 1;
        unsigned txen   : 1;
        unsigned rxcie  : 1;
        unsigned txcie  : 1;
    } sub;
};


