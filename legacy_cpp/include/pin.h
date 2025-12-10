#pragma once

#include <stdint.h>
#include <avr/io.h>

#define MEM_CAST (volatile uint8_t*)
struct pin {
    volatile uint8_t *DDR;
    volatile uint8_t *PORT;
    volatile uint8_t *PIN;
    uint8_t bit_number;
};

#define set_output(pin) do{ (*(pin.DDR) |= (1<<pin.bit_number)); } while(0)
#define set_input(pin)  do{ (*(pin.DDR) &= ~(1<<pin.bit_number)); } while(0)
#define high(pin)       do{ (*(pin.PORT) |= (1<<pin.bit_number)); } while(0)
#define low(pin)        do{ (*(pin.PORT) &= ~(1<<pin.bit_number)); } while(0)
#define toggle(pin)    do{ (*(pin.PIN) = (1<<pin.bit_number)); } while(0)

#define MAKE_PIN(port_letter, bit)          \
    static const pin PIN_##port_letter##bit = {   \
        (volatile uint8_t*)(_SFR_MEM_ADDR(DDR##port_letter)),  \
        (volatile uint8_t*)(_SFR_MEM_ADDR(PORT##port_letter)), \
        (volatile uint8_t*)(_SFR_MEM_ADDR(PIN##port_letter)),  \
        bit                 \
    };

#define X MAKE_PIN
// Master list of all 86 pins on the ATmega2560
// Format: X(PORT_LETTER, BIT_NUMBER)
#define PIN_LIST \
    /* PORT A */ \
    /*X(A,0) X(A,1) X(A,2) X(A,3) X(A,4) X(A,5) X(A,6) X(A,7) \*/
    /* PORT B */ \
    X(B,0) X(B,1) X(B,2) X(B,3) X(B,4) X(B,5) X(B,6) X(B,7) \
    /* PORT C */ \
    X(C,0) X(C,1) X(C,2) X(C,3) X(C,4) X(C,5) X(C,6) X(C,7) \
    /* PORT D */ \
    X(D,0) X(D,1) X(D,2) X(D,3) X(D,4) X(D,5) X(D,6) X(D,7) \
    /* PORT E */ \
    X(E,0) X(E,1) X(E,2) X(E,3) X(E,4) X(E,5) X(E,6) X(E,7) \
    /* PORT F */ \
    X(F,0) X(F,1) X(F,2) X(F,3) X(F,4) X(F,5) X(F,6) X(F,7) \
    /* PORT G (only bits 0–5 exist) */ \
    X(G,0) X(G,1) X(G,2) X(G,3) X(G,4) X(G,5) \
    /* PORT H */ \
    X(H,0) X(H,1) X(H,2) X(H,3) X(H,4) X(H,5) X(H,6) X(H,7) \
    /* PORT J */ \
    X(J,0) X(J,1) X(J,2) X(J,3) X(J,4) X(J,5) X(J,6) X(J,7) \
    /* PORT K */ \
    X(K,0) X(K,1) X(K,2) X(K,3) X(K,4) X(K,5) X(K,6) X(K,7) \
    /* PORT L */ \
    X(L,0) X(L,1) X(L,2) X(L,3) X(L,4) X(L,5) X(L,6) X(L,7)

    PIN_LIST    //expand and create pins

#undef X