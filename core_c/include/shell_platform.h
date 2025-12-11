#pragma once
#include <stdint.h>

#ifndef PERIPH_START
    #define PERIPH_START    ((uintptr_t)0u)
    #define PERIPH_END      ((uintptr_t)0u)
#endif
#ifndef SRAM_START
    #define SRAM_START    ((uintptr_t)0u)
    #define SRAM_END      ((uintptr_t)0u)
#endif

#if defined (__AVR_ATmega2560__)
    #undef  PERIPH_START
    #undef  PERIPH_END
    #define PERIPH_START ((uintptr_t)0x20u)
    #define PERIPH_END  ((uintptr_t)0xD6u)
#endif

static inline void sh_read_cmd(const shell_cmd_t* src, shell_cmd_t* dst) {
    #if defined(__AVR__)
        memcpy_P(dst, src, sizeof(shell_cmd_t));
    #else
        *dst = *src;
    
    #endif
}

static inline void sh_read_flag(const flag_t* src, flag_t* dst) {
    #if defined(__AVR__)
        memcpy_P(dst, src, sizeof(flag_t));
    #else
        *dst = *src;
    #endif
}