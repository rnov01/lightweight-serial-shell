#pragma once

#include <serial_shell/shell_types.h>
#include <serial_shell/config.h>
#include <serial_shell/hooks.h>
#include <serial_shell/shell_commands.h>

/* Public API */

void sh_init(shell_t* sh, const shell_cmd_t* user_cmds, char* input_buf, uint8_t buf_len, 
        char** argv, uint8_t max_argv, void (*putc)(void* user, char c), void* io_user, uint8_t cmd_count);

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