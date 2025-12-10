#pragma once

static inline bool is_int(const char* s) {
    if (!s || *s == '\0') return false;
    if (*s == '-' || *s == '+') s++;
    bool has_digit = false;
    while(*s) {
        if (*s >= '0' && *s <= '9') has_digit = true;
        else return false;
        s++;
    }
    return has_digit;
}

