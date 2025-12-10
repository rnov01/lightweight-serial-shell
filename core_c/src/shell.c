#include "shell.h"
#include "shell_sys.h"
//#include "../include/descriptions.h"
#include <stdint.h>
#include <string.h>

extern const shell_cmd_t sys_cmds[];
extern const uint8_t sys_cmd_count;




void sh_init(shell_t* sh, const shell_cmd_t* user_cmds, char* input_buf, uint8_t buf_len, 
    char** argv, uint8_t max_argv, void (*putc)(char c, void* user), void* io_user, uint8_t cmd_count) {
    
    sh->putc = putc;
    sh->puts = NULL;
    sh->puts_p = NULL;
    sh->io_user = io_user;

    sh->buffer = input_buf;
    sh->buf_len = buf_len;
    sh->position = 0;

    sh->argv = argv;
    sh->argc = 0;
    sh->max_argv = max_argv;

    sh->cmd = user_cmds;
    sh->cmd_count = cmd_count;
}

int sh_poll(shell_t* sh, uint8_t c) {
    /*uint8_t c;
    if (!buf_pop(&GLOBAL_BUFFER, &c))   //no data available
        return 0;*/

    if ((c == '\b' || c == 0x7F) && sh->position > 0) {  //handle backspace
        sh->position--;
        return 2;
    }

    if (c == '\r' || c == '\n') {       //end of command   
        if(sh->position > 0) {
            sh->buffer[sh->position] = '\0';       //terminate str
    
            //clr argv array
            for (uint8_t i=0; i<sh->max_argv; i++) {
                sh->argv[i] = NULL;
            }

            sh->argc = tokenizer(sh->buffer, sh->argv, sh->max_argv);

            if (sh->argc > 0 && sh->argv[0]) {
                sh_execute(sh, sh->argc, sh->argv);
            } 
        }
        sh->position = 0;  
        return 1;        
    }
    if (sh->position < sh->buf_len - 1) {    //append single char if space available
        sh->buffer[sh->position++] = (char)c;
    } else {
        error(sh, RUNTIME_FAILURE, "line too long");
    }
   
    return 3;
}


void sh_puts(shell_t* sh, const char* s) {
    if (sh->puts) {
        sh->puts(s, sh->io_user);
    } else {
        while (*s) {
            sh->putc(*s++, sh->io_user);
        }
    }
}


void sh_puts_ro(shell_t* sh, const char* p) {
    if (sh->puts_p) {
        sh->puts_p(p, sh->io_user);
    } else {
        char c;
        while((c = READ_RO_BYTE(p++))) {
            sh->putc(c, sh->io_user);
        }
    }
}

void sh_printc(shell_t* sh, color_t color, const char* s) {
     switch (color) {
        case CYAN:      sh_puts_ro(sh, CCYA);
            break;
        case YELLOW:    sh_puts_ro(sh, CYEL);
            break;
        case GREEN:     sh_puts_ro(sh, CGRN);
            break;
        case RED:       sh_puts_ro(sh, CRED);
            break;
        default: break;  
    }
    sh_puts(sh, s);
    sh_puts_ro(sh, C_);
}

void error(shell_t* sh, err_t err, const char* detail) {
    sh_puts_ro(sh, CRED);
    sh_puts_ro(sh, ROSTR("Error >> "));
    sh_puts(sh, err_msg[(int)err].description);
    if(detail) {
        sh_puts_ro(sh, ROSTR(" : "));
        sh_puts(sh, detail);
    }
    sh_puts_ro(sh, C_);
    LINE
}

const _err err_msg[] RODATA =  {
    {NONE,              ""},
    {INVALID_CMD,       "invalid command"},
    {UNKNOWN_FLAG,      "unknown flag"},
    {MISSING_ARG,       "missing argument"},
    {RUNTIME_FAILURE,   "runtime error"},
    {INVALID_PARAM,     "invalid parameter"}
};

static const shell_cmd_t* sh_find_cmd(shell_t* sh, const char *name) {
    if (!sh ||!name) return NULL;

    for (uint8_t i=0; i < sys_cmd_count; i++) {
        if (sys_cmds[i].name && strcmp(name, sys_cmds[i].name) == 0) {
            return &sys_cmds[i];
        }
    }

    for (uint8_t i = 0; i < sh->cmd_count; i++) {
        const shell_cmd_t *c = &sh->cmd[i];
        if (c->name && strcmp(name, c->name) == 0) {
            return c;
        } 
    }
    return NULL;
}

/*first pass finds command params and counts flags*/
static err_t sh_parse_args(const shell_cmd_t* cmd, int argc, char**argv, sh_parse_idx_t* out,
    const char** err_arg) {
    
    if (!cmd || !out) return RUNTIME_FAILURE;

    out->param_idx = -1; //no param
    out->flag_count = 0;
    
    for(int i=1; i<argc; i++) {
        const char*arg = argv[i];
        if (!arg) continue;
        
        if (arg[0] == '-' && arg[1] != '\0') {
            if(out->flag_count >= MAX_FLAGS) {
                if (err_arg) *err_arg = arg;
                return RUNTIME_FAILURE;
            }
            out->flag_pos[out->flag_count++] = (uint8_t)i;
        } else {
            if (out->param_idx >= 0) {
                if(err_arg) *err_arg = arg;
                return INVALID_PARAM;
            }
            out->param_idx = (int8_t)i;
        }
    }

    switch (cmd->sub.param_mode) {
        case REQ:
            if(out->param_idx < 0) {
                if(err_arg) *err_arg = cmd->name;
                return MISSING_ARG;
            }
            break;
        case NONE:
            if (out->param_idx >= 0) {
                if(err_arg) *err_arg = argv[out->param_idx];
                return INVALID_PARAM;
            }
            break;
        case OPT:
            break;
    }
    return NONE;
}

void sh_execute(shell_t* sh, int argc, char** argv) {
    if(!sh || argc <= 0 || !argv[0]) return;

    const shell_cmd_t *cmd = sh_find_cmd(sh, argv[0]);
    if (!cmd) {
        error(sh, INVALID_CMD, argv[0]);
        sh_puts_ro(sh, ROSTR("\r\nType help -long for a list of commands and their descriptions"));
        return;
    }
  
    sh_parse_idx_t idx;
    const char* err_arg = NULL;

    err_t e = sh_parse_args(cmd, argc, argv, &idx, &err_arg);
    if (e != NONE) {
        error(sh, e, err_arg);
        return;
    }

    const char* cmd_param = (idx.param_idx >= 0) ? argv[idx.param_idx] : NULL;

    switch (cmd->sub.flag_mode) {
        case CMD_THEN_FLAG:
            sh_dispatch_handler(sh, cmd, cmd_param, argc, argv);
            for(uint8_t i=0; i<idx.flag_count; i++) {
                int k = idx.flag_pos[i];
                const char* flag_name = argv[k];
                const flag_t *fl = sh_find_flag(cmd, flag_name);

                if (!fl) {
                    error(sh, UNKNOWN_FLAG, flag_name);
                    return;
                }
                const char* fparam = NULL;

                if (fl->sub.param_mode ==  REQ || fl->sub.param_mode == OPT) {
                    if((k+1) < argc && argv[k+1] && argv[k+1][0] != '-') {
                        fparam = argv[k+1];
                    } else if (fl->sub.param_mode == REQ) {
                        error(sh, MISSING_ARG, fl->name);
                        return;
                    }
                } 
                sh_dispatch_flag(sh, cmd, fl, fparam, argc, argv);
            }
            break;

        case FLAG_ONLY: {
            if (idx.flag_count == 0) {
                sh_dispatch_handler(sh, cmd, cmd_param, argc, argv);
                break;
            } 
            
            if (idx.flag_count >1) {
                error(sh, INVALID_CMD, "Only one flag allowed");
                return;
            } 

            int i = idx.flag_pos[0];
            const char* flag_name = argv[i];
            const flag_t* fl = sh_find_flag(cmd, flag_name);
            if(!fl)  {
                error(sh, UNKNOWN_FLAG, flag_name);
                return;
            }  
            const char* fparam = NULL;
            
            if (fl->sub.param_mode == REQ || fl->sub.param_mode == OPT) {
                if ((i + 1) < argc && argv[i + 1] && argv[i + 1][0] != '-') {
                    fparam = argv[i + 1];
                } else if (fl->sub.param_mode == REQ) {
                    error(sh, MISSING_ARG, fl->name);
                    return;
                }
            }

            sh_dispatch_flag(sh, cmd, fl, fparam, argc, argv);
            break;
        }

        case CMD_NO_FLAG:
            if (idx.flag_count != 0 ) {
                error(sh, INVALID_CMD, "flags not allowed for this command");
                return;  
            } 
            sh_dispatch_handler(sh, cmd, cmd_param, argc, argv);
            break;
    }
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

void sh_dispatch_handler(shell_t* sh, const shell_cmd_t* cmd, const char* param, int argc, char** argv) {
    cmd->handler(sh, cmd, param, argc, argv);
}

const flag_t* sh_find_flag(const shell_cmd_t* cmd, const char* n) {
    if (!cmd) return NULL;
    const flag_t* const* flags = cmd->flags;
    for(int i=0; i < cmd->flag_count && i < MAX_FLAGS; i++) {
        const flag_t *fl = cmd->flags[i];
        if (!fl || !fl->name) continue;
        if (strcmp(n, fl->name) == 0) {
            return fl;
        }
    }
    return NULL;
}

bool sh_has_flag(int argc, char **argv, const char* f) {
    for(int i=1; i < argc; i++) {
        if(strcmp(argv[i], f) == 0) {
            return true;
        }
    }
    return false;
}
void sh_dispatch_flag(shell_t* sh, const shell_cmd_t* parent, const flag_t* flag, const char* fparam, int argc, char** argv) {
    if (!flag || !flag->handler) return;
    flag->handler(sh, parent, fparam, argc, argv);
}


uint8_t sh_max_cmd_name_len(shell_t *sh)
{
    uint8_t max = 0;

    // system commands
    for (uint8_t i = 0; i < sys_cmd_count; i++) {
        const shell_cmd_t *c = &sys_cmds[i];
        if (!c->name) continue;

        uint8_t len = (uint8_t)strlen(c->name);
        if (len > max) max = len;
    }

    // user commands
    for (uint8_t i = 0; i < sh->cmd_count; i++) {
        const shell_cmd_t *c = &sh->cmd[i];
        if (!c->name) continue;

        uint8_t len = (uint8_t)strlen(c->name);
        if (len > max) max = len;
    }

    return max;
}
