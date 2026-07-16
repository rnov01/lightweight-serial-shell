#include "hw_interface.h"
#include <serial_shell/config.h>
#include <serial_shell/hooks.h>

/* This interface layer allows the shell to write hardware registers, while staying platform-neutral */
#if WRITE_CMDS
/* include a hardware-specific register map of safe-writable registers for write commands */
#ifdef SH_REGMAP_H
    #include SH_REGMAP_H
#endif

/* setbit/rstbit functions must be implemented in a hardware specific driver
 * examples provided for atmega2560 */
bool hw_setbit(const char* reg_name, uint8_t bit) {
    (void)reg_name;
    (void)bit;
    return SH_SETBIT(reg_name, bit);
}

bool hw_rstbit(const char *reg_name, uint8_t bit) {
    (void)reg_name;
    (void)bit;
    return SH_RSTBIT(reg_name, bit);
}
#endif

__attribute__((weak))
int8_t find_reg(const char* name) {
    (void)name;
    return -1;
}

__attribute__((weak))
SH_ADDR_TYPE get_addr(uint8_t id) {
    (void)id;
    return 0;
}