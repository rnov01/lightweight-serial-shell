#include "../include/shell.h"
#include <string.h>

/*! \name f_lut Command flag lookup tables
 * @{
 */
const flag_t shell::help_flags[] = {
    {"-long", "Gives a short description of each available command", &shell::f_help_long},
    {"-c", "-c <command> provides a list of flags available for the specified command", &shell::f_help_c},
    {nullptr, nullptr}
};

const flag_t shell::debug_flags[] = {
    {"-v", "Verbose mode\r\n"},
    {nullptr, nullptr}
};

const flag_t shell::print_flags[] = {
    //{"-long", "Gives a short description of each available command\r\n"};
    {nullptr, nullptr}
};

const flag_t shell::led_flags[] = {
    {"-long", "Gives a short description of each available command\r\n"},
    {nullptr, nullptr}
};

 ///@}
//command lut
//commands w/o flags get nullptr for flag_t arg
const command_t shell::cmd[NUM_CMDS+1] = {
    {"help", "Provides a list of system commands", &shell::cmd_help, HELP, help_flags},
    {"debug", "Activates system debug mode", &shell::cmd_debug, DEBUG, debug_flags},
    {"print", "Prints system status", &shell::cmd_print, PRINT, print_flags},
    {"led", "Toggles the onboard LED", &shell::cmd_led, LED, led_flags},
    {nullptr, nullptr, nullptr, err}
};