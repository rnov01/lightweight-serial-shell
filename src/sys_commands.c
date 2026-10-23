#include <serial_shell/shell_types.h>
#include <serial_shell/config.h>
#include <serial_shell/shell_commands.h>
#include <serial_shell/hooks.h>
#include <string.h>

#include <serial_shell/serial_shell.h>
#include "shell.h"
#include "shell_sys.h"
#include "hw_interface.h"
#include "descriptions.h"
#if WRITE_CMDS
#include <stdlib.h>
#endif

#ifndef BUILD_TARGET
#define BUILD_TARGET "Unknown"
#endif

#if HELP_CMD
static void cmd_help(shell_t* sh, const shell_cmd_t* self, const char* param, uint8_t argc, char** argv);
static void f_help_c(shell_t* sh, const shell_cmd_t* parent, const char* param, uint8_t argc, char **argv);
#endif
#if DUMP_CMD
static void cmd_dump(shell_t* sh, const shell_cmd_t* self, const char* param, uint8_t argc, char** argv);
#endif
#if VER_CMD
static void cmd_ver(shell_t* sh, const shell_cmd_t* self, const char* param, uint8_t argc, char** argv);
#endif
#if MEM_CMD
static void cmd_mem(shell_t* sh, const shell_cmd_t* self, const char* param, uint8_t argc, char** argv);
#endif
#if PEEK_CMD
static void cmd_peek(shell_t* sh, const shell_cmd_t* self, const char* param, uint8_t argc, char** argv);
#endif
#if WRITE_CMDS
static void cmd_setbit(shell_t* sh, const shell_cmd_t* self, const char* param, uint8_t argc, char** argv);
static void cmd_rstbit(shell_t* sh, const shell_cmd_t* self, const char* param, uint8_t argc, char** argv);
#endif

/* shared description helpers */
#if (SYS_CORE)
static const char s_cmds[] RODATA   = "commands: ";
static const char s_flags[] RODATA  = "flags: ";
static const char s_param[] RODATA  = "| param=";
static const char s_none[] RODATA   = "none";
static const char s_opt[] RODATA    = "opt ";
static const char s_req[] RODATA    = "req ";
static const char s_multi[] RODATA = "mult";
static const char s_arr[] RODATA    = " >> ";
#endif

#if HELP_CMD
static const shell_flag_t help_c_flag = {
    .name           = "-c",
    .description    =  help_cf_d,
    #if SH_FLAG_PARENT
    .parent         = help_n,
    #endif
    .handler        = f_help_c,
    .sub = {
        .in_progmem = true,
        .param_mode = REQ,
        .reserved = 0
    }
};
#endif

#if DUMP_CMD
static const shell_flag_t dump_v_flag = {
    .name           = "-v",
    .description    = dump_vf_d,
    #if SH_FLAG_PARENT
    .parent         = dump_n,
    #endif
    .handler        = NULL,
    .sub = {
        .in_progmem = true,
        .param_mode = NOPARAM,
        .reserved = 0
    }
};
#endif

#if SH_PEEK_WIDE
static const shell_flag_t peek_flags[] = {
    {
    .name = "-16",
    .description = peek16_d,
    #if SH_FLAG_PARENT
    .parent = peek_n,
    #endif
    .handler = NULL,
    .sub = {
        .in_progmem = true,
        .param_mode = NOPARAM,
        .reserved = 0
    } },
    {
    .name = "-32",
    .description = peek32_d,
    #if SH_FLAG_PARENT
    .parent = peek_n,
    #endif
    .handler = NULL,
    .sub = {
        .in_progmem = true,
        .param_mode = NOPARAM,
        .reserved = 0
    }}
};
#endif

//flag base must point to address of 1st flag in a contiguous block of flags belonging to a specific command
//i.e. if a command has multiple flags, they must be next to each other and flag_base points to the address of the first
#if (SYS_CORE) || (HELP_CMD) || (DUMP_CMD) || (VER_CMD) || (MEM_CMD) || PEEK_CMD
const shell_cmd_t sys_cmds[] = {
    #if HELP_CMD
    {help_n, help_d, cmd_help, &help_c_flag, .sub = {FLAG_ONLY, NONE, true, 1} },
    #endif
    #if DUMP_CMD
    {dump_n, dump_d, cmd_dump, &dump_v_flag, .sub = {CMD_THEN_FLAG, NONE, true, 1}},
    #endif
    #if VER_CMD
    {ver_n, ver_d, cmd_ver, NULL, .sub = { CMD_NO_FLAG, NONE, true, 0}},
    #endif
    #if MEM_CMD
    {mem_n, mem_d, cmd_mem, NULL, .sub = { CMD_NO_FLAG, NONE, true, 0}},
    #endif
    #if PEEK_CMD
    {peek_n, peek_d, cmd_peek, 
        #if SH_PEEK_WIDE
            &peek_flags[0], .sub = { CMD_THEN_FLAG, REQ, true, 2}
        #else
            NULL, .sub = { CMD_NO_FLAG, REQ, true, 0}},
        #endif          
    #endif
    #if WRITE_CMDS
    {set_n, set_d, cmd_setbit, NULL, .sub = { CMD_NO_FLAG, MULTI, true, 0}},
    {rst_n, rst_d, cmd_rstbit, NULL, .sub = { CMD_NO_FLAG, MULTI, true, 0}}
    #endif
};

const uint8_t sys_cmd_count = (uint8_t)(sizeof(sys_cmds) / sizeof(sys_cmds[0]));
#else 
    const shell_cmd_t sys_cmds[] = {0};
    const uint8_t sys_cmd_count = 0;
#endif

/* Private Helpers */

static uint8_t sh_max_cmd_name_len(shell_t *sh)
{
    uint8_t max = 0;

    /* system commands */
    for (uint8_t i = 0; i < sys_cmd_count; i++) {
        const shell_cmd_t *c = &sys_cmds[i];
        if (!c->name) continue;

        uint8_t len = (uint8_t)strlen(c->name);
        if (len > max) max = len;
    }

    /* user commands */
    for (uint8_t i = 0; i < sh->cmd_count; i++) {
        const shell_cmd_t *c = &sh->cmd[i];
        if (!c->name) continue;

        uint8_t len = (uint8_t)strlen(c->name);
        if (len > max) max = len;
    }

    return max;
}

static uint8_t total_flag_count(shell_t* sh) {
    uint8_t total_flags = 0;
    for (uint8_t i=0;i<sys_cmd_count;i++) {
        total_flags += sys_cmds[i].sub.flag_count;
    }
    for (uint8_t k=0; k<sh->cmd_count; k++) {
        total_flags += sh->cmd[k].sub.flag_count;
    }
    return total_flags;
}

/* System handler definitions */

#if HELP_CMD

void f_help_c(shell_t* sh, const shell_cmd_t* parent, const char* param, uint8_t argc, char **argv) {
    (void)parent; (void)argc; (void)argv;

    //sys commands
    for(uint8_t a=0; a<sys_cmd_count; a++) {
        const shell_cmd_t* cmd = &sys_cmds[a];
        if (strcmp(param, cmd->name) == 0) {
            sh_puts_ro(sh, ROSTR("\r\nFlags for: ")); 
            sh_printc(sh, CYAN, cmd->name);
            LINE LINE

            if(cmd->sub.flag_count == 0) {
                sh_puts_ro(sh, s_none); LINE
                return;
            }
            for (uint8_t b=0; b < cmd->sub.flag_count && b < MAX_FLAGS; b++) {
                const shell_flag_t* fl = &cmd->flag_base[b];
                sh_printc(sh, FLAGCOLOR, fl->name);
                if(fl->sub.in_progmem) {
                    sh_puts_ro(sh, s_arr); sh_puts_ro(sh, fl->description); LINE
                } else {
                    sh_puts(sh, s_arr); sh_puts(sh, fl->description); LINE
                }
            }
            return;
        }
    }
    //user commands
    for(uint8_t a=0; a<(sh->cmd_count); a++) {
        const shell_cmd_t* cmd = &sh->cmd[a];
        if (strcmp(param, cmd->name) == 0) {
            sh_puts_ro(sh, ROSTR("\r\nFlags available for command: ")); 
            sh_printc(sh, CYAN, cmd->name);
            LINE LINE

            if(cmd->sub.flag_count == 0) {
                sh_puts_ro(sh, s_none); LINE
                return;
            }
            for (uint8_t b=0; b < cmd->sub.flag_count && b < MAX_FLAGS; b++) {
                const shell_flag_t* fl = &cmd->flag_base[b];
                sh_printc(sh, FLAGCOLOR, fl->name);
                if(fl->sub.in_progmem) {
                    sh_puts_ro(sh, s_arr); sh_puts_ro(sh, fl->description); LINE
                } else {
                    sh_puts(sh, s_arr); sh_puts(sh, fl->description); LINE
                }
            }
            return;
        }
    }
    error(sh, INVALID_PARAM, param);
}

void cmd_help(shell_t* sh, const shell_cmd_t* self, const char* param, uint8_t argc, char** argv) {
    (void)self; (void)param; (void)argv;

    uint8_t maxlen = sh_max_cmd_name_len(sh);

    if (argc == 1) {
        sh_puts_ro(sh, ROSTR("\r\nCommand list:\r\n"));
        //sh_puts_ro(sh, ROSTR("----------------------------------\r\n"));
        for(uint8_t i=0; i < sys_cmd_count; i++) {      //sys cmds
            sh_printc(sh, CYAN, sys_cmds[i].name);
            for(uint8_t k=strlen(sys_cmds[i].name);k<maxlen+2;k++) SPACE
            sh_puts_ro(sh, ROSTR("|  ")); sh_puts_ro(sh, sys_cmds[i].description); LINE
        }
       
        for(uint8_t i=0; i < sh->cmd_count;i++) {
            sh_printc(sh, YELLOW, sh->cmd[i].name);
            for(uint8_t k=strlen(sh->cmd[i].name);k<maxlen+2;k++) SPACE
            sh_puts_ro(sh, ROSTR("|  ")); 
            if (sh->cmd[i].sub.in_progmem) {
                sh_puts_ro(sh, sh->cmd[i].description); LINE
            } else {
                sh_puts(sh, sh->cmd[i].description); LINE
            }
        }
    }
    sh_puts_ro(sh, ROSTR("\r\nUse -c <command> to list flags for a specific command\r\n"));
}
#endif

#if DUMP_CMD
static uint8_t dump_cmd_range(shell_t *sh, const shell_cmd_t *cmds, uint8_t count, bool is_user,
    uint8_t start_index, uint8_t max_name, bool verbose) {

    for (uint8_t i = 0; i < count; i++) {
        const shell_cmd_t *c = &cmds[i];

        // index in global list (if you care)
        uint8_t idx = start_index + i;

        // name + basic info
        sh_puts_ro(sh, ROSTR("[")); print_uint(sh, idx); sh_puts_ro(sh, ROSTR("] "));

        sh_printc(sh, is_user ? YELLOW : CYAN, c->name);
        for (uint8_t k = strlen(c->name); k < max_name + 2; k++) SPACE
        sh_puts_ro(sh, ROSTR("| (")); sh_puts_ro(sh, is_user ? ROSTR("usr) ") : ROSTR("sys) "));
        sh_puts_ro(sh, s_param);
        switch (c->sub.param_mode) {
            case NOPARAM: sh_puts_ro(sh, s_none); break;
            case OPT:     sh_puts_ro(sh, s_opt); break;
            case REQ:     sh_puts_ro(sh, s_req); break;
            case MULTI:   sh_puts_ro(sh, s_multi); break;
        }
        sh_puts_ro(sh, ROSTR("  | flags=")); print_uint(sh, c->sub.flag_count); 
        sh_puts_ro(sh, ROSTR("  | exe="));
        switch (c->sub.flag_mode) {
                case CMD_NO_FLAG: sh_puts_ro(sh, ROSTR("b-")); break;
                case CMD_THEN_FLAG: sh_puts_ro(sh, ROSTR("bf")); break;
                case FLAG_ONLY: sh_puts_ro(sh, ROSTR("-f")); break;} LINE

        // flags
        for (uint8_t j = 0; j < c->sub.flag_count; j++) {
            sh_puts_ro(sh, ROSTR("    "));
            const shell_flag_t *fl = &c->flag_base[j];
            sh_puts_ro(sh, (j == c->sub.flag_count-1) ? ls_end_str : ls_mid_str); 
            sh_printc(sh, GREEN, fl->name);
            for(uint8_t k=strlen(fl->name);k<max_name+1;k++) { SPACE } sh_puts_ro(sh, ROSTR("|"));
            for (uint8_t k=0; k<7;k++) { SPACE } sh_puts_ro(sh, s_param); 
                switch (fl->sub.param_mode) {
                    case NONE:  sh_puts_ro(sh, s_none); break;
                    case OPT:   sh_puts_ro(sh, s_opt); break;
                    case REQ:   sh_puts_ro(sh, s_req); break;
                    case MULTI: sh_puts_ro(sh, s_multi); break;} 
                for(uint8_t k=0;k<2;k++) { SPACE } sh_puts_ro(sh, ROSTR("|"));
                if(!verbose) LINE
                else {
                    SPACE
                    if (fl->sub.in_progmem) { sh_puts_ro(sh, fl->description); }
                    else { sh_puts(sh, fl->description); } LINE
                }
        }
    }

    return (uint8_t)(start_index + count);
}

void cmd_dump(shell_t* sh, const shell_cmd_t* self, const char* param, uint8_t argc, char **argv) {
    (void)self; (void)param;

    bool verbose = sh_has_flag(argc, argv, "-v");
    uint8_t maxlen = sh_max_cmd_name_len(sh);

    LINE
    sh_puts_ro(sh, ROSTR("\r\nshell dump...\r\n"));
    sh_puts_ro(sh, s_cmds); print_uint(sh, sys_cmd_count + sh->cmd_count);
    for(uint8_t z=0;z<6;z++) SPACE
    sh_puts_ro(sh, s_flags); print_uint(sh, total_flag_count(sh)); LINE
    uint8_t idx = 0; idx = dump_cmd_range(sh, sys_cmds, sys_cmd_count, false, idx, maxlen, verbose);
    dump_cmd_range(sh, sh->cmd, sh->cmd_count, true,  idx, maxlen, verbose);
}

#endif


#if VER_CMD
void cmd_ver(shell_t* sh, const shell_cmd_t* self, const char* param, uint8_t argc, char **argv) {
    (void)param; (void)self; (void)argc; (void)argv;
    LINE
    sh_puts_ro(sh, ROSTR("version\r\n"));
    sh_puts_ro(sh, ROSTR(" Firmware : "));
    #ifdef FIRM_VER_NAME
        sh_puts_ro(sh, ROSTR(FIRM_VER_NAME));
    #else   
        sh_puts_ro(sh, ROSTR("shell"));
    #endif 

    sh_puts_ro(sh, ROSTR("  (")); sh_puts_ro(sh, ROSTR(__DATE__)); SPACE
    sh_puts_ro(sh, ROSTR(__TIME__)); sh_puts_ro(sh, ROSTR(")\r\n"));
    sh_puts_ro(sh, ROSTR(" Target   : ")); sh_puts_ro(sh, ROSTR(BUILD_TARGET)); LINE
    LINE
    #ifdef GIT_HASH
        sh_puts_ro(sh, ROSTR(" Revision : ")); sh_puts_ro(sh, ROSTR(GIT_HASH));
        sh_puts_ro(sh, ROSTR(GIT_DIRTY)); LINE
    #endif
    #ifdef GIT_BRANCH
        sh_puts_ro(sh, ROSTR(" Branch   : " )); sh_puts_ro(sh, ROSTR(GIT_BRANCH)); LINE
    #endif
}
#endif

#if MEM_CMD 
void cmd_mem(shell_t* sh, const shell_cmd_t* self, const char* param, uint8_t argc, char **argv) {
    (void)param; (void)argc; (void)argv;
    LINE
    sh_puts(sh, self->name); sh_puts_ro(sh, ROSTR("ory settings\r\n")); 
    sh_puts_ro(sh, ROSTR(" cmds    : ")); print_uint(sh, sys_cmd_count + sh->cmd_count); LINE
    sh_puts_ro(sh, ROSTR(" flags   : ")); print_uint(sh, total_flag_count(sh)); LINE
    sh_puts_ro(sh, ROSTR(" in buf  : ")); print_uint(sh, SH_BUFFER_SIZE); LINE
    sh_puts_ro(sh, ROSTR(" max arg : ")); print_uint(sh, MAX_ARGS); LINE
}
#endif


#if PEEK_CMD

static void cmd_peek(shell_t* sh, const shell_cmd_t* self, const char* param, uint8_t argc, char** argv) {
    (void)self;
    if (!param) { sh_puts_ro(sh, ROSTR("Usage: peek <addr> [-16|-32]\r\n")); return;}

    uint8_t width = 1;
    bool f16 = sh_has_flag(argc, argv, "-16");
    bool f32 = sh_has_flag(argc, argv, "-32");

    if (f16 && f32) {
        error(sh, INVALID_PARAM, NULL);
        return;
    } else if (f32) {
        width = 4;
    } else if (f16) {
        width = 2;
    }

    volatile uint8_t* addr_ptr = NULL;
    uint32_t addr32;
    /* Numeric parse */
    if (parse_uint(argv[1], &addr32)) {
        addr_ptr = (volatile uint8_t*)(uintptr_t)addr32;
    } 
    /* Fallback reg name may be present */
    #if defined(SH_REGMAP_H)
    else {
        int8_t id = SH_FIND_REG(argv[1]);
        if (id>=0) {
            addr_ptr = SH_REG_ADDR(id);
        } else {
            error(sh, INVALID_PARAM, NULL);
            return;
        }
    }
    #else
        else {
        error(sh, INVALID_PARAM, NULL);
        return;
        }
    #endif
    //uintptr_t addr = (uintptr_t)addr32;
    for (uint8_t i=0; i<width; ++i) {
        if (!SH_ADDR_OK((uintptr_t)(addr_ptr + i))) {
        error(sh, INVALID_PARAM, "addr oor");
        return;
        }
    }
    sh_puts_ro(sh, ROSTR("contents of "));
    sh_puts(sh, param); sh_puts_ro(sh, ROSTR(" :"));
    
    
    for (uint8_t i=0; i<width; ++i) {
        //uint8_t v = *(volatile uint8_t*)(addr+i);
        uint8_t v = ((volatile uint8_t*)addr_ptr)[i];
        sh_puts_ro(sh, ROSTR(" 0x"));
        print_hex8(sh, v);
    }
    LINE
}

#endif

#if WRITE_CMDS
    void cmd_setbit(shell_t* sh, const shell_cmd_t* self, const char* param, uint8_t argc, char** argv) {
        (void)self; (void)param;

        if (argc < 2) {
            error_ro(sh, INVALID_PARAM, ROSTR("Usage: setbit <REG> <bit>\r\n"));
            return;
        }

        const char *reg = argv[1];
        uint8_t bit = (uint8_t)atoi(argv[2]);
        /*uint8_t bit;
        if(!sh_parse_u8(argv[2], &bit)) {
            error_ro(sh, INVALID_PARAM, NULL);
            return;
        }*/

        if (!hw_setbit(reg, bit)) {
            error_ro(sh, INVALID_PARAM, NULL);
            return;
        }

        sh_puts_ro(sh, ROSTR("ok.."));
    }

    void cmd_rstbit(shell_t* sh, const shell_cmd_t* self, const char* param, uint8_t argc, char** argv) {
        (void)self; (void)param;

        if (argc < 2) {
            error_ro(sh, INVALID_PARAM, ROSTR("Usage: rstbit <REG> <bit>\r\n"));
            return;
        }

        const char *reg = argv[1];
        uint8_t bit = (uint8_t)atoi(argv[2]);
       /*uint8_t bit;
        if(!sh_parse_u8(argv[2], &bit)) {
            error_ro(sh, INVALID_PARAM, NULL);
            return;
        }*/
       
        if (!hw_rstbit(reg, bit)) {
            error_ro(sh, INVALID_PARAM, NULL);
            return;
        }

        sh_puts_ro(sh, ROSTR("ok.."));
    }
#endif