#include "../include/shell.h"
#include "../include/descriptions.h"
#include <string.h>

const flag_t shell::sys_flags[] = {
    //help
    {"-long", help_lf_d, &shell::f_help_long, help_n, true},
    {"-c", help_cf_d, &shell::f_help_c, help_n, true},
    //debug
     {"-v", debug_vf_d, nullptr, debug_n, true},
     //print
    //{"-long", "Gives a short description of each available command\r\n", nullptr, print_n};
     //led
    {"-blink", led_bf_d, &shell::f_led_blink, led_n, true},
    {nullptr, nullptr, nullptr, nullptr}
};

//command lut
//commands w/o flags get nullptr for flag_t arg
const command_def_t shell::cmd[NUM_CMDS+1] = {
    {help_n, help_d, MEMBER_H, &shell::cmd_help, true},
    {debug_n, debug_d, MEMBER_H, &shell::cmd_debug, true},
    {print_n, print_d, MEMBER_H, &shell::cmd_print, true},
    {led_n, led_d, MEMBER_H, &shell::cmd_led,true},
    {nullptr, nullptr, }
};

const _err err_msg[] RODATA = {
    {0, "Invalid command.\r\nType help -long for a list of commands and their descriptions"},
    {1, "Duplicate command entered. \r\nSee list of reserved system commands"}
};