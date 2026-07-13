#pragma once
#include "shell.h"

#ifdef __cplusplus
extern "C" {
#endif

void sh_error(err_t err, const char* msg);
void sh_puts(const char* s);
void sh_puts_ro(const char* s);
void sh_printc(color_t color, const char* s);

#ifdef __cplusplus
}
#endif