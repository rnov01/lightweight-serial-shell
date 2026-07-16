#pragma once
#include <stdint.h>

/* find_reg() and get_addr() can be optionally implemented to allow "peek" to accept
 *  either name or hex argument */
#ifndef SH_ADDR_TYPE
#define SH_ADDR_TYPE volatile uint8_t*
#endif
#ifndef SH_FIND_REG
#define SH_FIND_REG(name) find_reg(name)
#endif
#ifndef SH_REG_ADDR
#define SH_REG_ADDR(id) get_addr(id)
#endif

/* Optional hook that can be used to validate addresses for reading with 'peek' command 
 * define as is_valid_addr(addr) to use the shells internal validation against provided SH_GET_ALLOWED_RANGES 
 * define as true to disable address validation for peek comands
 * alternatively, define your own validation scheme */
#ifndef SH_ADDR_OK
#define SH_ADDR_OK(addr) true
#endif

/* Define acceptable ranges for register reading (peek command) */
typedef struct {
    uintptr_t start;
    uintptr_t end;
} safe_addr_t;

/* You *must* define SH_ALLOWED_ADDRESS_RANGES if you want to use the 'peek' command */
#ifdef USER_ADDR_RANGES
#define SH_ADDR_RANGES (USER_ADDR_RANGES)
#define SH_ADDR_RANGE_COUNT (sizeof(USER_ADDR_RANGES) / sizeof(safe_addr_t))
#else 
#define SH_ADDR_RANGES (NULL)
#define SH_ADDR_RANGE_COUNT (0)
#endif

/* Default hooks for set/rst bit commands (must be implemented if you want to use them) */
#ifndef SH_SETBIT
#define SH_SETBIT(reg, bit) (false)
#endif
#ifndef SH_RSTBIT
#define SH_RSTBIT(reg, bit) (false)
#endif

__attribute__((weak))
int8_t find_reg(const char* name);

__attribute__((weak))
SH_ADDR_TYPE get_addr(uint8_t id);