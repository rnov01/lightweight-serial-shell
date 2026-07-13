#include <serial_shell/shell_types.h>
#include <serial_shell/config.h>
#include <serial_shell/shell_commands.h>
#include <serial_shell/hooks.h>
#include <stdint.h>
#include <string.h>

#include "shell.h"
//#include "str_helpers.h"
#include "shell_sys.h"

extern const shell_cmd_t sys_cmds[];
extern const uint8_t sys_cmd_count;

/* Forward Declarations */
static void sh_execute(shell_t* sh, uint8_t argc, char** argv);
static int tokenizer(char* buf, char** argv, uint8_t max_args);
static const shell_flag_t* sh_find_flag(const shell_cmd_t* cmd, const char* n);
static void sh_dispatch_handler(shell_t* sh, const shell_cmd_t* cmd, const char* param, uint8_t argc, char** argv);
static void sh_dispatch_flag(shell_t* sh, const shell_cmd_t* parent, const shell_flag_t* flag, const char* fparam, uint8_t argc, char** argv);

void sh_init(shell_t* sh, const shell_cmd_t* user_cmds, char* input_buf, uint8_t buf_len, 
    char** argv, uint8_t max_argv, void (*putc)(void* user, char c), void* io_user, uint8_t cmd_count) {
    
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

    sh_puts_ro(sh, ROSTR(" ...shell active... "));
}

int sh_poll(shell_t* sh, uint8_t c) {

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
        error_ro(sh, RUNTIME_FAILURE, ROSTR("line too long"));
    }
   
    return 3;
}

void sh_putc(shell_t* sh, char c) {
    sh->putc(sh->io_user, c);
}

void sh_puts(shell_t* sh, const char* s) {
    if (sh->puts) {
        sh->puts(sh->io_user, s);
    } else {
        while (*s) {
            sh->putc(sh->io_user, *s++);
        }
    }
}


void sh_puts_ro(shell_t* sh, const char* p) {
    if (sh->puts_p) {
        sh->puts_p(sh->io_user, p);
    } else {
        char c;
        while((c = READ_RO_BYTE(p++))) {
            sh->putc(sh->io_user, c);
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

void sh_printc_ro(shell_t* sh, color_t color, const char* s_ro) {
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
    sh_puts_ro(sh, s_ro);
    sh_puts_ro(sh, C_);
}

void print_uint(shell_t* sh, uint16_t n) {
    char buf[6];
    uint8_t i = 0;

    if(n==0) {sh_puts_ro(sh, ROSTR("0")); return;}
    while (n>0 && i < sizeof(buf)-1) {
        buf[i++] = '0' + (n%10);
        n/=10;
    }
    while(i--) sh_putc(sh, buf[i]);
}

const char* get_error_string(err_t e) {
    #if defined(__AVR__)
        return READ_RO_PTR(const char*, &err_msg[(uint8_t)e]);
    #else
        return err_msg[(uint8_t)e];
    #endif
}

void error(shell_t* sh, err_t err, const char* detail) {
    sh_puts_ro(sh, CRED);
    sh_puts_ro(sh, ROSTR("Error >> "));
    sh_puts_ro(sh, get_error_string(err));
    if(detail) {
        sh_puts_ro(sh, ROSTR(" : "));
        sh_puts(sh, detail);
    }
    sh_puts_ro(sh, C_);
    LINE
}

// PROGMEM version for internal use
void error_ro(shell_t* sh, err_t err, const char* detail_ro) {
    sh_puts_ro(sh, CRED);
    sh_puts_ro(sh, ROSTR("Error >> "));
    sh_puts_ro(sh, get_error_string(err));
    if (detail_ro) {
        sh_puts_ro(sh, ROSTR(" : "));
        sh_puts_ro(sh, detail_ro);   // PROGMEM
    }
    sh_puts_ro(sh, C_);
    LINE
}

//error strings 
static const char err_none[]          RODATA = "";
static const char err_invalid_cmd[]   RODATA = "invalid command";
static const char err_unknown_flag[]  RODATA = "unknown flag";
static const char err_missing_arg[]   RODATA = "missing argument";
static const char err_runtime_err[]   RODATA = "runtime error";
static const char err_invalid_param[] RODATA = "invalid parameter";

//table of pointers to strings
const char * const err_msg[] RODATA = {
    err_none,
    err_invalid_cmd,
    err_unknown_flag,
    err_missing_arg,
    err_runtime_err,
    err_invalid_param
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
static err_t sh_parse_args(const shell_cmd_t* cmd, uint8_t argc, char**argv, sh_parse_idx_t* out,
    const char** err_arg) {
    
    if (!cmd || !out) return RUNTIME_FAILURE;

    if (cmd->sub.param_mode == MULTI) {
        out->param_idx = -1;
        out->flag_count = 0;
        return NONE;
    }

    out->param_idx = -1; //no param
    out->flag_count = 0;
    
    for(uint8_t i=1; i<argc; i++) {
        const char*arg = argv[i];
        if (!arg) continue;
        
        if (arg[0] == '-' && arg[1] != '\0') {
            if(out->flag_count >= MAX_FLAGS) {
                if (err_arg) *err_arg = arg;
                return RUNTIME_FAILURE;
            }
            out->flag_pos[out->flag_count++] = (uint8_t)i;
        } else {
            if (i > 1) {
                const char* prev = argv[i - 1];
                if(prev && prev[0] == '-' && prev[1] != '\0') {
                    const shell_flag_t* prev_fl = sh_find_flag(cmd, prev);
                    if (prev_fl && prev_fl->sub.param_mode != NOPARAM) {
                        continue;
                    }
                }
                
            } 
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

static void sh_execute(shell_t* sh, uint8_t argc, char** argv) {
    if(!sh || argc <= 0 || !argv[0]) return;

    const shell_cmd_t *cmd = sh_find_cmd(sh, argv[0]);
    if (!cmd) {
        error(sh, INVALID_CMD, argv[0]);
        sh_puts_ro(sh, ROSTR("\r\nhelp gives cmd list"));
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
                uint8_t k = idx.flag_pos[i];
                const char* flag_name = argv[k];
                const shell_flag_t *fl = sh_find_flag(cmd, flag_name);

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
                error_ro(sh, INVALID_CMD, ROSTR("one flag per cmd"));
                return;
            } 

            uint8_t i = idx.flag_pos[0];
            const char* flag_name = argv[i];
            const shell_flag_t* fl = sh_find_flag(cmd, flag_name);
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
                error_ro(sh, INVALID_CMD, ROSTR("cmd doesn't allow flags"));
                return;  
            } 
            sh_dispatch_handler(sh, cmd, cmd_param, argc, argv);
            break;
    }
}

/*int tokenizer(char *buf, char **argv, uint8_t max_args) {
    uint8_t argc = 0;
    char *token = strtok(buf, " ");

    while ((token != NULL) && (argc < max_args)) {
        argv[argc++] = token;
        token = strtok(NULL, " ");
    }
    return argc;
}*/

static int tokenizer(char* buf, char** argv, uint8_t max_args) {
    uint8_t argc = 0;
    uint8_t i = 0;
    while(buf[i] && argc < max_args) {
        while(buf[i] == ' ') i++;
        if(!buf[i]) break;

        argv[argc++] = &buf[i];

        while(buf[i] && buf[i] != ' ') i++;
        if (buf[i]) buf[i++] = '\0';
    }
    return argc;
}

static void sh_dispatch_handler(shell_t* sh, const shell_cmd_t* cmd, const char* param, uint8_t argc, char** argv) {
    cmd->handler(sh, cmd, param, argc, argv);
}

static const shell_flag_t* sh_find_flag(const shell_cmd_t* cmd, const char* n) {
    if (!cmd || !n || !cmd->flag_base) return NULL;

    for(uint8_t i=0; i < cmd->sub.flag_count && i < MAX_FLAGS; i++) {
        const shell_flag_t *fl = &cmd->flag_base[i];
        if (!fl || !fl->name) continue;
        if (strcmp(n, fl->name) == 0) {
            return fl;
        }
    }
    return NULL;
}

bool sh_has_flag(uint8_t argc, char **argv, const char* f) {
    for(uint8_t i=1; i < argc; i++) {
        if(strcmp(argv[i], f) == 0) {
            return true;
        }
    }
    return false;
}
static void sh_dispatch_flag(shell_t* sh, const shell_cmd_t* parent, const shell_flag_t* flag, const char* fparam, uint8_t argc, char** argv) {
    if (!flag || !flag->handler) return;
    flag->handler(sh, parent, fparam, argc, argv);
}

#ifdef SYS_DEBUG
bool is_valid_addr(uintptr_t addr) {
#if SH_PEEK_REQ_SAFE_RANGES
    const safe_addr_t *ranges = SH_ADDR_RANGES;
    size_t count = SH_ADDR_RANGE_COUNT;

    if (!ranges || count == 0)
        return false;

    for (size_t i = 0; i < count; i++)
    {
        if (addr >= ranges[i].start && addr <= ranges[i].end)
            return true;
    }
    return false;
#else
    (void)addr;
    return true;
#endif
}

bool parse_uint(const char* s, uint32_t* out) {
    uint32_t value = 0;
    uint8_t base = 10;

    if (s[0] == '0' && (s[1] == 'x' || s[1] == 'X')) {
        base = 16;
        s+= 2;
    }

    if(*s == '\0') return false;

    while(*s) {
        char c = *s++;
        uint8_t d;

        if (c >= '0' && c <= '9') {
            d = (uint8_t)(c - '0');
        } else if ( base == 16 && c>= 'a' && c <='f') {
            d = (uint8_t)(c - 'a' + 10);
        } else if ( base == 16 && c >= 'A' && c <= 'F') {
            d = (uint8_t)(c - 'A'+ 10);
        } else {
            return false;
        }
        value = (value * base) + d;
    }
    *out = value;
    return true;
}

void print_hex_digit(shell_t* sh, uint8_t d) {
    char c = (d < 10) ? ('0' +d) : ('A' + (d-10));
    sh_putc(sh, c);
}

void print_hex8(shell_t* sh, uint8_t n) {
    uint8_t high = (n>>4) & 0x0F;
    uint8_t low = n & 0x0F; 
    //sh_puts_ro(sh, ROSTR("0x"));
    print_hex_digit(sh, high);
    print_hex_digit(sh, low);
}
#endif