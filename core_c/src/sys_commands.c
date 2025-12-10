#include "shell.h"
#include "shell_sys.h"
#include "../include/descriptions.h"
#include <string.h>

static void cmd_help(shell_t* sh, const shell_cmd_t* self, const char* param, int argc, char** argv);
static void cmd_dump(shell_t* sh, const char* param, int argc, char** argv);
static void cmd_ver(shell_t* sh, const char* param, int argc, char** argv);
static void cmd_mem(shell_t* sh, const char* param, int argc, char** argv);
static void f_help_c(shell_t* sh, const shell_cmd_t* parent, const char* param, int argc, char **argv);

//long flag lut
static const flag_t help_c_flag = {
    .name           = "-c",
    .description    = "",
    .parent         = help_n,
    .handler        = f_help_c,
    .sub = {
        .in_progmem = true,
        .param_mode = REQ,
        .reserved = 0
    }
};

const shell_cmd_t sys_cmds[] = {
    #if HELP_CMD
    {help_n, help_d, cmd_help, 1, &help_c_flag, .sub = {CMD_THEN_FLAG, NONE, false, true, 0} },
    #endif
    #if DUMP_CMD
    {dump_n, dump_d, cmd_dump},
    #endif
    #if VER_CMD
    {ver_n, ver_d, cmd_ver},
    #endif
    #if MEM_CMD
    {mem_n, mem_d, cmd_mem},
    #endif
};

const uint8_t sys_cmd_count = (uint8_t)(sizeof(sys_cmds) / sizeof(sys_cmds[0]));

#if HELP_CMD


//help flag c
void f_help_c(shell_t* sh, const shell_cmd_t* parent, const char* param, int argc, char **argv) {
    if (!param) {
        error(sh, MISSING_ARG, "Usage: help -c <command>\r\n");
        return;
    }

    //sys commands
    for(uint8_t a=0; a<sys_cmd_count; a++) {
        const shell_cmd_t* cmd = &sys_cmds[a];
        if (strcmp(param, cmd->name) == 0) {
            sh_puts_ro(sh, ROSTR("\r\nFlags available for command: ")); 
            sh_printc(sh, CYAN, cmd->name);
            LINE LINE

            if(cmd->flag_count == 0) {
                sh_puts_ro(sh, ROSTR("None\r\n"));
                return;
            }
            for (uint8_t b=0; cmd->flag_count && b < MAX_FLAGS; b++) {
                const flag_t* fl = cmd->flags[b];
                sh_printc(sh, FLAGCOLOR, fl->name);
                if(cmd->sub.in_progmem) {
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

            if(cmd->flag_count == 0) {
                sh_puts_ro(sh, ROSTR("None\r\n"));
                return;
            }
            for (int b=0; cmd->flag_count && b < MAX_FLAGS; b++) {
                const flag_t* fl = cmd->flags[b];
                sh_printc(sh, FLAGCOLOR, fl->name);
                if(cmd->sub.in_progmem) {
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
        for(int i=0; i < (sys_cmd_count); i++) {      //sys cmds
            sh_printc(sh, CYAN, sys_cmds[i].name);
            for(int k=strlen(sys_cmds[i].name);k<maxlen+2;k++) SPACE
            sh_puts_ro(sh, ROSTR("|  ")); sh_puts_ro(sh, sys_cmds[i].description); LINE
        }
       
        for(int i=0; i < (sh->cmd_count)-1;i++) {
            sh_printc(sh, YELLOW, sh->cmd[i].name);
            for(int k=strlen(sh->cmd[i].name);k<maxlen+2;k++) SPACE
            sh_puts_ro(sh, ROSTR("|  ")); sh_puts_ro(sh, sh->cmd[i].description); LINE
        }
    }
    sh_puts_ro(sh, ROSTR("\r\nUse -c <command> to list flags for a specific command\r\n"));
}
#endif

#if DUMP_CMD
void f_dump_v(const command_rt_t* self, const char* param, int argc, char **argv) {
    is_true = true;
    cmd_dump(self, param, argc, argv);
    is_true = false;
}

void cmd_dump(const command_rt_t* self, const char* param, int argc, char **argv) {
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
#endif

#if VERSION_CMD
void cmd_ver(const command_rt_t* self, const char* param, int argc, char **argv) {
    
    LINE
    puts_ro(ROSTR("version\r\n"));
    puts_ro(ROSTR("  Firmware   : "));
    #ifdef FIRM_VER_NAME
        puts_ro(ROSTR(FIRM_VER_NAME));
    #else   
        puts_ro(ROSTR("Serial shell"));
    #endif 

    puts_many_ro(ROSTR("  ("), ROSTR(__DATE__), ROSTR("  "), ROSTR(__TIME__), ROSTR(")\r\n"), nullptr);
    puts_ro(ROSTR("  Target     : ")); puts(BUILD_TARGET); LINE
    LINE
    #ifdef GIT_HASH
        puts_many_ro(ROSTR("  Revision   : "), ROSTR(GIT_HASH), ROSTR(GIT_DIRTY), ROSTR("\r\n"), nullptr);
    #endif
    #ifdef GIT_BRANCH
        puts_many_ro(ROSTR("  Branch     : "), ROSTR(GIT_BRANCH), ROSTR("\r\n"), nullptr);
    #endif
}
#endif

#if MEMORY_CMD
void shell::cmd_mem(const command_rt_t* self, const char* param, int argc, char **argv) {

    LINE
    puts(self->def->name); puts_ro(ROSTR("ory settings\r\n")); 
    puts_ro(ROSTR("  commands : ")); print_uint(final_count, this); LINE
    puts_ro(ROSTR("  flags    : ")); print_uint(total_flag_count(), this); LINE
    puts_ro(ROSTR("  in buf   : ")); print_uint(IN_BUFFER_SIZE, this); LINE
    puts_ro(ROSTR("  max args : ")); print_uint(MAX_ARGS, this); 
}
#endif