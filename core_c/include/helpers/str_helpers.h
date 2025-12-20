#pragma once

#include <stdint.h>
#include <stdbool.h>

/* Platform override macros 
 *
 * If the platform wants an optimized variant,
 * define these macros before including this header */

 #ifndef STRCMP                         //ram, ram
 #define STRCMP(a,b) tiny_strcmp((a),(b))
 #endif
 #ifndef STRCMP_P                       //ram, progmem
 #define STRCMP_P(a,b) STRCMP(a,b)
 #endif
 #ifndef STRCMP_PP                      //progmem, progmem
 #define STRCMP_PP(a,b) STRCMP((a),(b))
 #endif
 #ifndef STRLEN
 #define STRLEN(s) tiny_strlen(s)
 #endif
 #ifndef MEMSET
 #define MEMSET(p,val,n) tiny_memset((p),(val),(n))
 #endif

/*int     tiny_strcmp(const char* a, const char* b);
uint8_t tiny_strlen(const char* s);
void    tiny_memset(void* p, uint8_t val, uint8_t n);*/

static inline int tiny_strcmp(const char* a, const char* b) {
    while (*a && (*a == *b)) {
        a++;
        b++;
    }
    return (uint8_t)*a - (uint8_t)*b;
}

static inline uint8_t tiny_strlen(const char* s) {
    uint8_t n = 0;
    while (*s++) n++;
    return n;
}

static inline void tiny_memset(void* p, uint8_t val, uint8_t n) {
    uint8_t *a = (uint8_t*)p;
    while (n--) *a++ = val;
}

bool sh_streq(const char* a, const char* b);
bool sh_streq_P(const char* a, const char* p);
bool sh_streq_PP(const char* p1, const char* p2);

bool sh_parse_u8(const char* s, uint8_t* out);
bool sh_parse_u16(const char* s, uint16_t* out);