#pragma once

/* Data sizes can be configured to suit project needs
 * Line buffer (bytes) must be sized for the maximum number of input characters for a single command;
        this includes base command + any command params + flags + flag params */
#define SH_BUFFER_SIZE 16
#define MAX_ARGS 5
#define MAX_FLAGS 4

/* Core system commands can be batch or individually toggled */
#define SYS_CORE 1
#if SYS_CORE
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

/* Enable/disable peek/poke commands */
#define SYS_DEBUG 0
#if SYS_DEBUG
    #define PEEK_CMD 1
    #define POKE_CMD 0      //unimplemented, use setbit/rstbit instead
    #define WRITE_CMDS 1
#else 
    #define PEEK_CMD 0 
    #define POKE_CMD 0      //unimplemented, use setbit/rstbit instead
    #define WRITE_CMDS 0 
#endif

/* If enabled, peek will only allow addresses inside SH_ALLOWED_ADDRESS_RANGES */
/* If disables, peek will allow any address to be read */
#define SH_PEEK_REQ_SAFE_RANGES 1

#ifndef SH_PEEK_WIDE
#define SH_PEEK_WIDE 0
#endif

/* Enable/disable flags for 16 and 32 bit register reading */
#if SH_PEEK_WIDE
#define PEEK16_CMD 1
#define PEEK32_CMD 1
#else
#define PEEK16_CMD 0
#define PEEK32_CMD 0
#endif
//#define HELP_CMD  

//enable color coded command and flag names
//default cyan - sys cmd, yellow - user command, green - flag, red - error
#ifndef SH_COLOR_ON
#define SH_COLOR_ON 1
#endif

//future task
//optional store all command/flag tables in progmem on AVR for ram-starved applications
//this will save ~100bytes off of the core build (~450->350bytes RAM)
#define SH_AVR_FLASH_TABLES 0 

//attaches parent command name to flag entry
/* This is generally not needed unless you make a flag handler that references the parent command by name */
#define SH_FLAG_PARENT 0