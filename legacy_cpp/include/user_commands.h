#pragma once

#include "shell.h"

void cmd_light(const command_rt_t* self, const char* param, int argc, char** argv);
void f_light_b(const command_rt_t* self, const char* param, int argc, char** argv);
void SH_USR_CMD(test_prt1);
static const flag_t usr_flags[] = {
    FLAG("light", "-b", "blink", f_light_b, FREQ_PARAM),
    FLAG_END
};

static const command_def_t usr_cmds[] = {
    CMD("light", "Onboard led", cmd_light, FLAG_ONLY, CNO_PARAM),
    CMD_END
};

