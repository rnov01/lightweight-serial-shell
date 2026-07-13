#pragma once
//#include "tiny_reg_map_2560.h"
/* ATmega2560 Example */

/* File path for reg map header required if SH_TINY_REGMAP is enabled */
#define SH_REGMAP_H "2560_regmap.h"
#define SH_ADDR_OK(addr) is_valid_addr(addr)

/* Define Register access settings for 'peek' command */
/*const safe_addr_t SH_ALLOWED_ADDRESS_RANGES[] = {
    {0x20u, 0xD6}
};*/
#define USER_ADDR_RANGES \
    (safe_addr_t[]){{0x20u, 0xD6u}}


#define SH_SETBIT(reg, bit) setbit_2560(reg, bit)
#define SH_RSTBIT(reg, bit) rstbit_2560(reg, bit)



