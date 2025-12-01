#include "../include/shell.h"
#include "../include/descriptions.h"
#include <string.h>

/*! \name f_lut Command flag lookup tables
 * @{
 */
const flag_t shell::help_flags[] = {
    {"-long", help_lf_d, &shell::f_help_long, help_n},
    {"-c", help_cf_d, &shell::f_help_c, help_n},
    {nullptr, nullptr, nullptr}
};

const flag_t shell::debug_flags[] = {
    {"-v", "Verbose mode", debug_n},
    {nullptr, nullptr}
};

const flag_t shell::print_flags[] = {
    //{"-long", "Gives a short description of each available command\r\n"};
    {nullptr, nullptr}
};

const flag_t shell::led_flags[] = {
    {"-blink", "Gives a short description of each available command"},
    {nullptr, nullptr}
};

 ///@}
//command lut
//commands w/o flags get nullptr for flag_t arg
const command_t shell::cmd[NUM_CMDS+1] = {
    {help_n, help_d, &shell::cmd_help, HELP, help_flags},
    {debug_n, debug_d, &shell::cmd_debug, DEBUG, debug_flags},
    {"print", print_d, &shell::cmd_print, PRINT, print_flags},
    {"led", led_d, &shell::cmd_led, LED, led_flags},
    {nullptr, nullptr, nullptr, err}
};