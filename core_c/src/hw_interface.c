#include "hw_interface.h"
#include "shell_config.h"

#if WRITE_CMDS
#ifdef __AVR_ATmega2560__ 
#include "../../../ATmega2560_drivers/include/tiny_reg_map_2560.h"
#endif


bool hw_setbit(const char* reg_name, uint8_t bit) {
    #ifdef __AVR_ATmega2560__
    return setbit_2560(reg_name, bit);
    #else
    return false;
    #endif
}

bool hw_rstbit(const char *reg_name, uint8_t bit)
{
#ifdef __AVR_ATmega2560__
    return rstbit_2560(reg_name, bit);
#else
    return false;
#endif
}
#endif