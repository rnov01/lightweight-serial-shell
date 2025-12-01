#pragma once

#include "shell.h"

void cmd_light(const command_rt_t* self, int argc, char** argv);
void SH_USR_CMD(test_prt1);
static const flag_t usr_flags[] = {
    {"-b", "blink", nullptr, "light"}
};

static const command_def_t usr_cmds[] = {
    {"light", "Onboard led manipulation", FREE_H, {.free = cmd_light}, false},
    {nullptr, nullptr,}
};