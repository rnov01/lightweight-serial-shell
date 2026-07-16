#pragma once
#include "shell.h"


#if HELP_CMD
extern const char help_n[];
extern const char help_d[] RODATA;
   extern const char help_cf_d[];
#endif

#if DUMP_CMD
extern const char dump_n[];
extern const char dump_d[] RODATA;
    extern const char dump_vf_d[] RODATA;
#endif

#if VER_CMD
extern const char ver_n[];
extern const char ver_d[] RODATA;
#endif

#if MEM_CMD
extern const char mem_n[];
extern const char mem_d[] RODATA;
#endif

#if PEEK_CMD
extern const char peek_n[];
extern const char peek_d[] RODATA;
    extern const char peek16_d[] RODATA;
    extern const char peek32_d[] RODATA;

#endif

#if WRITE_CMDS
extern const char set_n[];
extern const char set_d[] RODATA;
extern const char rst_n[];
extern const char rst_d[] RODATA;
#endif