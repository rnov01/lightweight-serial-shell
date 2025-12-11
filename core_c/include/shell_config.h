#pragma once
//if defined this will display when "ver" command is called
#define FIRMWARE_VER_NAME ROSTR("v1.0")

//configure data sizes
#define IN_BUFFER_SIZE 16
#define MAX_ARGS 5
#define MAX_FLAGS 4

//enable system commands
#define SYS_CORE
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
#define SYS_DEBUG
#if defined(SYS_DEBUG)
    #define PEEK_CMD 1
    #define POKE_CMD 1
#else 
    #define PEEK_CMD 0 
    #define POKE_CMD 0 
#endif

#define PEEK16_CMD 1
#define PEEK32_CMD 1

//#define HELP_CMD  

//enable color coded command and flag names
//default cyan - sys cmd, yellow - user command, green - flag, red - error
#define __COLOR_ON__


//future task
//optional store all command/flag tables in progmem on AVR for ram-starved applications
//this will save ~100bytes off of the core build (~450->350bytes RAM)
#define SH_AVR_TINY_TABLES 0 