#include "shell.h"
#pragma once

typedef enum {
    HELP, DEBUG, PRINT, LED, NONE, err, NUM_CMDS
}command_id_t;

struct str_to_cmd {
    const char* name;
    command_id_t cmd; 
};

command_id_t parse_command(char *buffer);

typedef void (*cmd_handler)(int argc, char **argv);
int tokenizer(char *buf, char **argv, int max_args);

typedef struct command_t {
    char* name;
    char* description;
    cmd_handler exe;
    command_id_t cmd;
};

extern void cmd_help(int argc, char **argv);


const command_t cmd[NUM_CMDS] = {
    [HELP] = {"help", "Here is a list of system commands:\r\n", cmd_help, HELP}
};

void cmd_help(int argc, char **argv);

char *_argv[8];
int _argc = 0;

