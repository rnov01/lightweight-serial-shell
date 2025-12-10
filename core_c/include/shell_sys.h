
#pragma once
#include "shell.h"

extern const shell_cmd_t sys_cmds[];
extern const uint8_t sys_cmd_count;

//#define SYS_CORE
#if defined(SYS_CORE) 
    #define HELP_CMD    1
    #define DUMP_CMD    1
    #define VER_CMD     1
    #define MEM_CMD     1
#else
    #define HELP_CMD    0
    #define DUMP_CMD    0
    #define VER_CMD     0
    #define MEM_CMD     0
#endif

#define HELP_CMD    1