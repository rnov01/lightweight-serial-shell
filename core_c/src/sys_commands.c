#include "shell.h"
#include "shell_sys.h"
#include "shell_config.h"
#include "../include/descriptions.h"
#include <string.h>


static void cmd_help(shell_t* sh, const shell_cmd_t* self, const char* param, int argc, char** argv);
static void cmd_dump(shell_t* sh, const shell_cmd_t* self, const char* param, int argc, char** argv);
static void cmd_ver(shell_t* sh, const shell_cmd_t* self, const char* param, int argc, char** argv);
static void cmd_mem(shell_t* sh, const shell_cmd_t* self, const char* param, int argc, char** argv);
static void cmd_peek(shell_t* sh, const shell_cmd_t* self, const char* param, int argc, char** argv);
static void f_help_c(shell_t* sh, const shell_cmd_t* parent, const char* param, int argc, char **argv);

//long flag lut
static const flag_t help_c_flag = {
    .name           = "-c",
    .description    =  help_cf_d,
    .parent         = help_n,
    .handler        = f_help_c,
    .sub = {
        .in_progmem = true,
        .param_mode = REQ,
        .reserved = 0
    }
};

static const flag_t dump_v_flag = {
    .name           = "-v",
    .description    = dump_vf_d,
    .parent         = dump_n,
    .handler        = NULL,
    .sub = {
        .in_progmem = true,
        .param_mode = NONE,
        .reserved = 0
    }
};

#if PEEK_CMD
static const flag_t peek_flags[] = {
    {
    .name = "-16",
    .description = peek16_d,
    .parent = peek_n,
    .handler = NULL,
    .sub = {
        .in_progmem = true,
        .param_mode = NONE,
        .reserved = 0
    } },
    {
    .name = "-32",
    .description = peek32_d,
    .parent = peek_n,
    .handler = NULL,
    .sub = {
        .in_progmem = true,
        .param_mode = NONE,
        .reserved = 0
    }}
};
#endif

//flag base must point to address of 1st flag in a contiguous block of flags belonging to a specific command
//i.e. if a command has multiple flags, they must be next to each other and flag_base points to the address of the first
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
    {peek_n, peek_d, cmd_peek, &peek_flags[0], .sub = { CMD_THEN_FLAG, REQ, true, 2}},
    #endif
};

const uint8_t sys_cmd_count = (uint8_t)(sizeof(sys_cmds) / sizeof(sys_cmds[0]));

#if HELP_CMD


//help flag c
void f_help_c(shell_t* sh, const shell_cmd_t* parent, const char* param, int argc, char **argv) {
    /*if (!param) {
        error(sh, MISSING_ARG, "Usage: help -c <command>\r\n");
        return;
    }*/

    //sys commands
    for(uint8_t a=0; a<sys_cmd_count; a++) {
        const shell_cmd_t* cmd = &sys_cmds[a];
        if (strcmp(param, cmd->name) == 0) {
            sh_puts_ro(sh, ROSTR("\r\nFlags available for command: ")); 
            sh_printc(sh, CYAN, cmd->name);
            LINE LINE

            if(cmd->sub.flag_count == 0) {
                sh_puts_ro(sh, ROSTR("None\r\n"));
                return;
            }
            for (uint8_t b=0; b < cmd->sub.flag_count && b < MAX_FLAGS; b++) {
                const flag_t* fl = &cmd->flag_base[b];
                sh_printc(sh, FLAGCOLOR, fl->name);
                if(fl->sub.in_progmem) {
                    sh_puts_ro(sh, ROSTR(" >> ")); sh_puts_ro(sh, fl->description); LINE
                } else {
                    sh_puts(sh, (" >> ")); sh_puts(sh, fl->description); LINE
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
                sh_puts_ro(sh, ROSTR("None\r\n"));
                return;
            }
            for (uint8_t b=0; b < cmd->sub.flag_count && b < MAX_FLAGS; b++) {
                const flag_t* fl = &cmd->flag_base[b];
                sh_printc(sh, FLAGCOLOR, fl->name);
                if(fl->sub.in_progmem) {
                    sh_puts_ro(sh, ROSTR(" >> ")); sh_puts_ro(sh, fl->description); LINE
                } else {
                    sh_puts(sh, (" >> ")); sh_puts(sh, fl->description); LINE
                }
            }
            return;
        }
    }
    error(sh, INVALID_PARAM, param);
}

void cmd_help(shell_t* sh, const shell_cmd_t* self, const char* param, int argc, char** argv) {

    uint8_t maxlen = sh_max_cmd_name_len(sh);

    if (argc == 1) {
        sh_puts_ro(sh, ROSTR("\r\nHere is a list of system commands:\r\n"));
        sh_puts_ro(sh, ROSTR("----------------------------------\r\n"));
        for(int i=0; i < sys_cmd_count; i++) {      //sys cmds
            sh_printc(sh, CYAN, sys_cmds[i].name);
            for(int k=strlen(sys_cmds[i].name);k<maxlen+2;k++) SPACE
            sh_puts_ro(sh, ROSTR("|  ")); sh_puts_ro(sh, sys_cmds[i].description); LINE
        }
       
        for(int i=0; i < sh->cmd_count;i++) {
            sh_printc(sh, YELLOW, sh->cmd[i].name);
            for(int k=strlen(sh->cmd[i].name);k<maxlen+2;k++) SPACE
            sh_puts_ro(sh, ROSTR("|  ")); sh_puts_ro(sh, sh->cmd[i].description); LINE
        }
    }
    sh_puts_ro(sh, ROSTR("\r\nUse -c <command> to list flags for a specific command\r\n"));
}
#endif

#if DUMP_CMD
/*void cmd_dump(shell_t* sh, const shell_cmd_t* self, const char* param, int argc, char **argv) {
    //no param 
        puts_ro(ROSTR("-------------\r\n"
                      "Shell dump...\r\n"
                      "-------------\r\n"
                      "Commands: "));
        
        print_uint(final_count, this); for(int z=0; z<6; z++) SPACE 
        puts_ro(ROSTR("Flags: ")); print_uint((int)total_flag_count(), this); LINE
        for(int i=0; i<final_count; i++) {
            //const command_rt_t& c = final_cmd[i];
            const flag_t* const* flags = final_cmd[i].flags;
            puts_ro(ROSTR("[")); print_uint(i, this); puts_ro(ROSTR("]")); SPACE
            printc(NAMECOLOR(i), final_cmd[i].def->name); for(int k=strlen(final_cmd[i].def->name);k<max_cname+2;k++) SPACE
            puts_ro(ROSTR("| (")); puts(final_cmd[i].def->sub.type ? "usr) " : "sys) "); 
            puts_ro(ROSTR( "| param=")); 
            switch (get_cmd_param_mode(&final_cmd[i])) {
                case CNO_PARAM: puts_ro(ROSTR("none ")); break;
                case COPT_PARAM: puts_ro(ROSTR("opt  ")); break;
                case CREQ_PARAM: puts_ro(ROSTR("req  "));}
            puts_ro(ROSTR(" | flags=")); print_uint(get_flag_count(&final_cmd[i]), this);
            puts_ro(ROSTR( "  | exe=")); 
            switch (get_flag_mode(&final_cmd[i])) {
                case BASE_ONLY_NO_FLAGS: puts_ro(ROSTR("base-no flags")); break;
                case BASE_THEN_FLAGS: puts_ro(ROSTR("base-then flags")); break;
                case FLAG_ONLY: puts_ro(ROSTR("flag only"));} LINE

            if (!get_flag_count(&final_cmd[i])) continue;
            for(int j=0; j<get_flag_count(&final_cmd[i]);j++) {
                puts_ro(ROSTR("    "));
                puts_ro((j == get_flag_count(&final_cmd[i])-1) ? ls_end_str : ls_mid_str); printc(FLAGCOLOR, flags[j]->name); 
                for(int k=strlen(flags[j]->name);k<max_cname+1;k++) SPACE puts_ro(ROSTR("|"));
                //for(int k=strlen(flags[j]->name);k<(max_cname+11);k++) SPACE
                for (int k=0; k<7;k++) SPACE puts_ro(ROSTR("| param=")); 
                switch (flags[j]->sub.param_mode) {
                    case FNO_PARAM: puts_ro(ROSTR("none")); break;
                    case FOPT_PARAM: puts_ro(ROSTR("opt ")); break;
                    case FREQ_PARAM: puts_ro(ROSTR("req "));} 
                for(int k=0;k<2;k++) SPACE puts_ro(ROSTR("|"));
                if(!is_true) LINE
                if(is_true) {
                    SPACE
                    if (flags[j]->sub.in_progmem) puts_ro(flags[j]->description); 
                    else puts(flags[j]->description);LINE
                }
            } 
        }
}
#endif*/

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
        sh_puts_ro(sh, ROSTR("| param="));
        switch (c->sub.param_mode) {
            case NOPARAM: sh_puts_ro(sh, ROSTR("none ")); break;
            case OPT:     sh_puts_ro(sh, ROSTR("opt  ")); break;
            case REQ:     sh_puts_ro(sh, ROSTR("req  ")); break;
        }
        sh_puts_ro(sh, ROSTR(" | flags=")); print_uint(sh, c->sub.flag_count); 
        sh_puts_ro(sh, ROSTR("  | exe="));
        switch (c->sub.flag_mode) {
                case CMD_NO_FLAG: sh_puts_ro(sh, ROSTR("base-no flags")); break;
                case CMD_THEN_FLAG: sh_puts_ro(sh, ROSTR("base-then flags")); break;
                case FLAG_ONLY: sh_puts_ro(sh, ROSTR("flag only"));} LINE

        // flags
        for (uint8_t j = 0; j < c->sub.flag_count; j++) {
            sh_puts_ro(sh, ROSTR("    "));
            const flag_t *fl = &c->flag_base[j];
            sh_puts_ro(sh, (j == c->sub.flag_count-1) ? ls_end_str : ls_mid_str); 
            sh_printc(sh, GREEN, fl->name);
            for(int k=strlen(fl->name);k<max_name+1;k++) { SPACE } sh_puts_ro(sh, ROSTR("|"));
            for (int k=0; k<7;k++) { SPACE } sh_puts_ro(sh, ROSTR("| param=")); 
                switch (fl->sub.param_mode) {
                    case NONE: sh_puts_ro(sh, ROSTR("none")); break;
                    case OPT: sh_puts_ro(sh, ROSTR("opt ")); break;
                    case REQ: sh_puts_ro(sh, ROSTR("req "));} 
                for(int k=0;k<2;k++) { SPACE } sh_puts_ro(sh, ROSTR("|"));
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

void cmd_dump(shell_t* sh, const shell_cmd_t* self,
              const char* param, int argc, char **argv)
{
    bool verbose = sh_has_flag(argc, argv, "-v");
    uint8_t maxlen = sh_max_cmd_name_len(sh);

    LINE
    sh_puts_ro(sh, ROSTR("-------------\r\nshell dump...\r\n-------------\r\n"));
    sh_puts_ro(sh, ROSTR("Commands: ")); print_uint(sh, sys_cmd_count + sh->cmd_count);
    for(int z=0;z<6;z++) SPACE
    sh_puts_ro(sh, ROSTR("Flags: ")); print_uint(sh, total_flag_count(sh)); LINE
    uint8_t idx = 0; idx = dump_cmd_range(sh, sys_cmds, sys_cmd_count, false, idx, maxlen, verbose);
    dump_cmd_range(sh, sh->cmd, sh->cmd_count, true,  idx, maxlen, verbose);
}

#endif


#if VER_CMD
void cmd_ver(shell_t* sh, const shell_cmd_t* self, const char* param, int argc, char **argv) {
    
    LINE
    sh_puts_ro(sh, ROSTR("version\r\n"));
    sh_puts_ro(sh, ROSTR("  Firmware   : "));
    #ifdef FIRM_VER_NAME
        sh_puts_ro(sh, ROSTR(FIRM_VER_NAME));
    #else   
        sh_puts_ro(sh, ROSTR("Serial shell"));
    #endif 

    sh_puts_ro(sh, ROSTR("  (")); sh_puts_ro(sh, ROSTR(__DATE__)); SPACE
    sh_puts_ro(sh, ROSTR(__TIME__)); sh_puts_ro(sh, ROSTR(")\r\n"));
    sh_puts_ro(sh, ROSTR("  Target     : ")); sh_puts(sh, BUILD_TARGET); LINE
    LINE
    #ifdef GIT_HASH
        sh_puts_ro(sh, ROSTR("  Revision   : ")); sh_puts_ro(sh, ROSTR(GIT_HASH));
        sh_puts_ro(sh, ROSTR(GIT_DIRTY)); LINE
    #endif
    #ifdef GIT_BRANCH
        sh_puts_ro(sh, ROSTR("  Branch     : " )); sh_puts_ro(sh, ROSTR(GIT_BRANCH)); LINE
    #endif
}
#endif

#if MEM_CMD 
void cmd_mem(shell_t* sh, const shell_cmd_t* self, const char* param, int argc, char **argv) {

    LINE
    sh_puts(sh, self->name); sh_puts_ro(sh, ROSTR("ory settings\r\n")); 
    sh_puts_ro(sh, ROSTR("  commands : ")); print_uint(sh, sys_cmd_count + sh->cmd_count); LINE
    sh_puts_ro(sh, ROSTR("  flags    : ")); print_uint(sh, total_flag_count(sh)); LINE
    sh_puts_ro(sh, ROSTR("  in buf   : ")); print_uint(sh, IN_BUFFER_SIZE); LINE
    sh_puts_ro(sh, ROSTR("  max args : ")); print_uint(sh, MAX_ARGS); 
}
#endif

#if PEEK_CMD



static void cmd_peek(shell_t* sh, const shell_cmd_t* self, const char* param, int argc, char** argv) {
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


    uint32_t addr32;
    if (!parse_uint(argv[1], &addr32)) {
        error(sh, INVALID_PARAM, NULL);
        return;
    }
    uintptr_t addr = (uintptr_t)addr32;
    for (uint8_t i=0; i<width; ++i) {
        if (!shell_is_valid_debug_addr(addr + i)) {
        error(sh, INVALID_PARAM, "addr oor");
        return;
        }
    }
    sh_puts_ro(sh, ROSTR("contents of "));
    sh_puts(sh, param); sh_puts_ro(sh, ROSTR(" :"));
    
    
    for (uint8_t i=0; i<width; ++i) {
        uint8_t v = *(volatile uint8_t*)(addr+i);
        sh_puts_ro(sh, ROSTR(" 0x"));
        print_hex8(sh, v);
    }
    LINE
}

#endif