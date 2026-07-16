#pragma once

#include <serial_shell/shell_types.h>
#include <serial_shell/config.h>
#include <serial_shell/hooks.h>
#include <serial_shell/shell_commands.h>

/* Public API */


/**
 * @brief           Initialize a shell instance. 
 * @param sh        Pointer to user allocated shell state
 * @param user_cmds Array of user commands (may be NULL)
 * @param input_buf Line buffer used for command parsing
 * @param buf_len   Byte size of input buffer
 * @param argv      Array of char* ptrs used for tokens
 * @param max_argv  Max. number of tokens; set by @ref MAX_ARGS in shell_config.h
 * @param putc      Character output callback
 * @param io_user   User ptr passed back to output callbacks (may be NULL, depending on serial interface config)
 * @param cmd_count Number of entries in user_cmds
 * */
void sh_init(shell_t* sh, const shell_cmd_t* user_cmds, void (*putc)(void* user, char c), void* io_user, uint8_t cmd_count);


/** 
 * @brief Feed one character into the shell state machine
 * 
 * Call this whenever a new character arrives from the serial backend.
 * When a full line is received and parsed, the corresponding command
 * handler is executed. 
 * 
 * @return Status code 
 */
int sh_poll(shell_t* sh, uint8_t c);


/* Basic output helpers */
void sh_putc(shell_t* sh, char c);
void sh_puts(shell_t* sh, const char* s);
void sh_puts_ro(shell_t* sh, const char* p);
void sh_printc(shell_t* sh, color_t color, const char* s);
void sh_printc_ro(shell_t* sh, color_t color, const char* s_ro);

/* Error helpers */
void error(shell_t* sh, err_t err, const char* detail);
void error_ro(shell_t* sh, err_t err, const char* detail_ro);