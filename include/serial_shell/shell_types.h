#pragma once
#include <stdint.h>

struct shell;
struct shell_cmd;
struct shell_flag;

typedef struct shell        shell_t;
typedef struct shell_cmd    shell_cmd_t;
typedef struct shell_flag   shell_flag_t;

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
} cmd_param_t, flag_param_t;

/*Optional color output (ANSI esc sequences)*/
typedef enum color {
    CYAN, YELLOW, GREEN, RED, RST
}color_t;
