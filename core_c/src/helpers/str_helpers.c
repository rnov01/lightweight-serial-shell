#include "str_helpers.h"
#include <stdint.h>

/*int tiny_strcmp(const char* a, const char* b) {
    while (*a && (*a == *b)) {
        a++;
        b++;
    }
    return (uint8_t)*a - (uint8_t)*b;
}

uint8_t tiny_strlen(const char* s) {
    uint8_t n = 0;
    while (*s++) n++;
    return n;
}

void tiny_memset(void* p, uint8_t val, uint8_t n) {
    uint8_t *a = (uint8_t*)p;
    while (n--) *a++ = val;
}*/

bool sh_streq(const char* a, const char* b) {
    return (STRCMP(a,b)==0);
}
bool sh_streq_P(const char* a, const char* p) {
    return (STRCMP_P(a,p)==0);
}
bool sh_streq_PP(const char* p1, const char* p2) {
    return (STRCMP(p1,p2)==0);
}

/*bool sh_parse_u8(const char* s, uint8_t* out) {
    uint16_t v = 0;
    if (!s || !*s) return false;

    while (*s) {
        if(*s < '0' || *s > '9') return false;
        v = v * 10 + (*s - '0');
        if (v > 255) return false;
        s++;
    }
    *out = (uint8_t)v;
    return true;
}

bool sh_parse_u16(const char* s, uint16_t* out) {
    uint32_t v = 0;
    if (!s || !*s) return false;

    while (*s) {
        if(*s < '0' || *s > '9') return false;
        v = v * 10 + (*s - '0');
        if (v > 65535) return false;
        s++;
    }
    *out = (uint16_t)v;
    return true;
}*/