#include "../include/shell_api.h" 

extern shell* active_shell;

void sh_error(err_t err, const char* msg) {
    active_shell->error(err, msg);
}

void sh_puts(const char* s) {
    active_shell->puts(s);
}

void sh_puts_ro(const char* s) {
    active_shell->puts_ro(s);
}

void sh_printc(color_t color, const char* s) {
    active_shell->printc(color, s);
}