#pragma once 
#include <stdint.h>
#include <stdbool.h>

#if WRITE_CMDS
bool hw_setbit(const char* reg_name, uint8_t bit);
bool hw_rstbit(const char* reg_name, uint8_t bit);
#endif
