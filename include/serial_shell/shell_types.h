#pragma once
#include <serial_shell/config.h>
#include <stdint.h>
#include <stdbool.h>

struct shell;
struct shell_cmd;
struct shell_flag;

typedef struct shell        shell_t;
typedef struct shell_cmd    shell_cmd_t;
typedef struct shell_flag   shell_flag_t;

/* ===================================================================
 * Shell state
 * 
 * shell_t holds the runtime state of the shell and is initialized by the 
 * user with the sh_init() function. sh_init() only takes in 
 * the required putc() callback (the other implementations are optional, putc
 * is all the shell needs for operation). If a user has puts() and/or puts_p() 
 * (PROGMEM AVR puts function) they can be passed to the shell via this struct. 
 * 
 * An input buffer, argv array and putc() callback must be provided when calling sh_init() 
 * =================================================================== */
struct shell {
    /* Output callbacks (user-provided) */
    void (*putc)(void* user, char c);
    void (*puts)(void* user, const char* s);        /* optional; may be NULL - shell will derive its own from putc() */
    void (*puts_p)(void* user, const char* p);      /* optional for read-only (PROGMEM) strings */
    void* io_user;                                  /* optional context pointer for object-based callback functions */

    /* Input buffer */
    char buffer[SH_BUFFER_SIZE];
    uint8_t buf_len;
    uint8_t position;
    bool ovf;

    /* Token storage (argv, points into buffer)*/
    char *argv[MAX_ARGS];
    uint8_t argc;
    uint8_t max_argv;

    /* Command table */
    const shell_cmd_t *cmd;
    uint8_t cmd_count;
};

/* ===================================================================
 * Error handling
 *
 * err_t values are returned internally by the parser and are also accessible
 * for use in user commands. get_error_string() maps an err_t to a short 
 * descriptor string; error() prints a formatted error message to the shell 
 * ==================================================================== */
typedef enum err {
   NONE,
   INVALID_CMD,
   UNKNOWN_FLAG,
   MISSING_ARG,
   RUNTIME_FAILURE,
   INVALID_PARAM
}err_t;

/*===================================================================
 * Command / flag metadata
 *
 * exe_mode_t   - defines how a command interacts with its flags
 * param_mode   - defines whether a command or flag expects a parameter
 * =================================================================== */
typedef enum exe_mode {
    CMD_NO_FLAG,        /* no flags expected; if flags are present an error will be printed */
    CMD_THEN_FLAG,      /* run base command, then dispatch flags (if any) */
    FLAG_ONLY           /* run base handler or single flag handler, but not both (flags take priority if present) */
} exe_mode_t; 

typedef enum param_mode {
    NOPARAM,            /* no parameter allowed; parameter will result in error message */
    REQ,                /* required parameter; no parameter provided will result in error message */
    OPT,                /* optional parameter */
    MULTI               /* No named param, but allows multiple argv arguments */
} sh_param_t;

/*Optional color output (ANSI esc sequences)*/
typedef enum color {
    CYAN, YELLOW, GREEN, RED, RST
}color_t;
