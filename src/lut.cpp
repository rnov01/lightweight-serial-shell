#include "../include/shell.h"
#include "../include/descriptions.h"
#include <string.h>

const flag_t shell::sys_flags[] = {
    //help
    {"-long", help_lf_d, MEMBER_H,{.mem = &shell::f_help_long}, help_n, true, FNO_PARAM},
    {"-c", help_cf_d, MEMBER_H, {.mem = &shell::f_help_c}, help_n, true, FREQ_PARAM},
    //debug
     {"-v", debug_vf_d, MEMBER_H, {.mem = nullptr}, debug_n, true},
     //print
    //{"-long", "Gives a short description of each available command\r\n", nullptr, print_n};
     //led
    {"-blink", led_bf_d, MEMBER_H,{.mem = &shell::f_led_blink}, led_n, true, FREQ_PARAM},
    {nullptr, nullptr, MEMBER_H, {.mem =nullptr}, nullptr, false, FNO_PARAM}
};

//command lut
//commands w/o flags get nullptr for flag_t arg
const command_def_t shell::cmd[] = {
    {help_n, help_d, MEMBER_H, &shell::cmd_help, FLAG_ONLY, true},
    {debug_n, debug_d, MEMBER_H, &shell::cmd_debug, BASE_THEN_FLAGS, true},
    {print_n, print_d, MEMBER_H, &shell::cmd_print, BASE_THEN_FLAGS, true},
    {led_n, led_d, MEMBER_H, &shell::cmd_led, FLAG_ONLY,true},
    {nullptr, nullptr, MEMBER_H, nullptr, BASE_THEN_FLAGS, false}
};

const _err shell::err_msg[] = {
    {err_t::NONE, ""},
    {err_t::INVALID_CMD, "Invalid command"},
    {err_t::UNKNOWN_FLAG, "Unknown flag"},
    {err_t::MISSING_ARG, "Missing argument"},
    {err_t::RUNTIME_FAILURE, "Runtime error"},
    {err_t::INVALID_PARAM, "Invalid parameter"}
};