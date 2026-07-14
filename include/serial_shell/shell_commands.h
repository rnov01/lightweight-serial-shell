#pragma once
#include <serial_shell/shell_types.h>
#include <stdbool.h>

/* ===================================================================
 * Flags
 * 
 * Each flag belongs to exactly one command. flag_base in shell_cmd_t 
 * points to the first flag in a contiguous block (for a given command);
 * sub.flag_count tells how many flags 
 * =================================================================== */
typedef void(*flag_handler)(
    shell_t* sh, 
    const shell_cmd_t* parent,
    const char* param, 
    uint8_t argc, 
    char **argv);

struct shell_flag {
    const char* name;           /* string that will be matched with cli args ex. "-v" */
    const char* description;    /* short description; used with "help" system command */
    #if SH_FLAG_PARENT
    const char* parent;         /* name of parent command (optional) */
    #endif
    flag_handler handler;       /* callback for flag; may be NULL */
    struct {
        bool in_progmem         : 1;    /* description in PROGMEM (AVR) */
        sh_param_t param_mode : 2;    /* flag parameter expectations */
        uint8_t reserved        : 5;
    } sub;
}; 

/* ===================================================================
 * Commands
 * 
 * Each command has a name, description, handler and optional block of 
 * flags starting at flag_base. sub.flag_count describes how many flags
 * from flag_base belong to this command 
 * =================================================================== */
typedef void(*cmd_handler)(
    shell_t* sh,
    const shell_cmd_t* self, 
    const char* param, 
    uint8_t argc, 
    char **argv);

struct shell_cmd {
    const char* name;               /* string used to identify command in terminal */
    const char* description;        /* short description; used with "help" system command */
    cmd_handler handler;            /* base command callback */
    const shell_flag_t* flag_base;        /* pointer to first flag for this command in a flag table */
    struct {
        unsigned flag_mode  : 2;    /* exe_mode_t */
        unsigned param_mode : 2;    /* sh_param_t */
        unsigned in_progmem : 1;    /* description in PROGMEM (AVR)*/
        unsigned flag_count : 3;    /* # of member flags from flag_base */
    } sub;
};