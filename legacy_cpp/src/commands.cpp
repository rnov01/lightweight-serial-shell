#include "../include/commands.h"
#include "../include/shell.h"
#include <string.h>

static const str_to_cmd COMMAND_LUT[] = {
    {"help", command_id_t::HELP},
    {"debug", command_id_t::DEBUG},
    {"print", command_id_t::PRINT},
    {"led", command_id_t::LED},
    {nullptr, command_id_t::err}
};

command_id_t parse_command(char *buffer) {
    for (int i=0; COMMAND_LUT[i].name != nullptr; i++) {
        if (strcmp(buffer, COMMAND_LUT[i].name) == 0) {
            return COMMAND_LUT[i].cmd;
        }
    }
    return command_id_t::err;
}

int tokenizer(char *buf, char **argv, int max_args) {
    int argc = 0;
    char *token = strtok(buf, " ");

    while ((token != NULL) && (argc < max_args)) {
        argv[argc++] = token;
        token = strtok(NULL, " ");
    }
    return argc;
}

void cmd_help(int argc, char **argv) {
    if (argc == 1) {
        shell::puts_const(cmd[HELP].description);
    }
}