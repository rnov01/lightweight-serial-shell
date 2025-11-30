#pragma once

//read only wrappers allow for memory optimizations on certain devices
#if defined(__AVR__)
    //optimize lut and string literal storage for AVR chips by storing in flash
    #include <avr/pgmspace.h>
    #define RODATA PROGMEM

    

#define ROSTR(x) PSTR(x)

  
    
    #define READ_RO_PTR(type, addr) ((type)pgm_read_word(addr))
    #define READ_RO_BYTE(addr) pgm_read_byte(addr)
#else
    //for architectures w/o distinct flash and sram 
    #define RODATA
    #define ROSTR(x) (x)

    #define READ_RO_POINTER(type, addr) (*(type*)(addr))
    #define READ_RO_BYTE(addr) (*(uint8_t*)(addr))
#endif

