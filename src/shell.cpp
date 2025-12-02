#include "../include/shell.h"
#include "../include/usart.h"
#include "../include/ring_buffer.h"
#include "../include/ro.h"
#include "../include/colors.h"
#include <string.h>
#include <util/delay.h>
#include <stdlib.h>

ringbuf GLOBAL_BUFFER = {{0}, 0, 0};

shell::shell(shell_io_t* interface) : io(interface) {
    puts_ro(ROSTR("\r\n... shell active ...\r\n"));
}


//#####################################################################
// #                                                                  #
// #                      Public Methods                              #  
// #                                                                  #  
//#####################################################################


void shell::init(const command_def_t* user_cmds, const flag_t* user_flags) {
    final_count = 0;
    build_cmd_table(cmd);
    build_cmd_table(user_cmds);
    build_flag_links(sys_flags);
    build_flag_links(user_flags);
}

void shell::tick() {

}

int shell::poll() {
    uint8_t c;
    if (!buf_pop(&GLOBAL_BUFFER, &c))   //no data available
        return 0;
    if (c == '\r' || c == '\n') {       //end of command   
        if(line_len > 0) {
            sh_line[line_len] = '\0';       //terminate str
            //line_len = 0;                   //reset 
            memset(_argv, 0, sizeof(_argv));
            _argc = tokenizer(sh_line, _argv, MAX_ARGS);    
            command_rt_t* cmd = parse_command(_argv[0]);
            //puts_const(sh_line);              debug readouts
            //puts_const(cmd->def->name);
            if (!cmd) {
                error(err_t::INVALID_CMD, _argv[0]);
                puts_ro(ROSTR("\r\nType help -long for a list of commands and their descriptions"));
            } else {
                execute(cmd, _argc, _argv);   
            }
        }
        line_len = 0;  
        memset(sh_line, 0, sizeof(sh_line));
        return 1;        //newline w/ no data -> ignore
    }
    if (line_len < sizeof(sh_line) - 1) {    //append single char if space available
        sh_line[line_len++] = c;
    }
    if (c == '\b' && line_len > 0) {        //delete last char if backspace is pressed
    line_len--;
    return 2;
    }
    return 3;
}

void shell::puts(const char* s) {
    if (io->puts) {
        io->puts(s, io->self);
    } else {
        while (*s) {
            io->putc(*s++, io->self);
        }
    }   
}

void shell::puts_many(const char* first, ...) {
        va_list args;
        va_start(args, first);

        const char* s = first;
        while(s != nullptr) {
            io->puts(s, io->self);
            s = va_arg(args, const char*);
        }
        va_end(args);
    }

void shell::puts_ro(const char* p) {
    if (io->puts_p) {
        io->puts_p(p, io->self);
    } else {
        char c;
        while((c = READ_RO_BYTE(p++))) {
            io->putc(c, io->self);
        }
    }  
}

void shell::puts_many_ro(const char* first, ...) {
    va_list args;
    va_start(args, first);

    const char* p = first;
    while (p != nullptr) {
        puts_ro(p);
        p = va_arg(args, const char*);
    }
    va_end(args);
}

void shell::printc(color_t color, const char* s) {
    switch (color) {
        case CYAN: puts_ro(CCYA);
            break;
        case YELLOW: puts_ro(CYEL);
            break;
        case GREEN: puts_ro(CGRN);
            break;
        case RED:puts_ro(CRED);
            break;
        default: break;  
    }
    puts(s);
    puts_ro(C_);
}

void shell::execute(const command_rt_t* cmd, int argc, char** argv) {
    if(!cmd) return;
    parsed_flag_t parsed_flags[MAX_FLAGS];
    uint8_t parsed_flags_count = 0;

    for (int i=1; i<argc; i++) {
        const char* arg = argv[i];
        
        if(arg[0] == '-' && arg[1] != '\0') {
            const flag_t* flag = find_flag(cmd, argv[i]);

            if (!flag) {
                error(err_t::UNKNOWN_FLAG, arg);
                return;
            }

            parsed_flag_t pf;
            pf.flag = flag;
            pf.param = nullptr;

            if (flag->param_mode != FNO_PARAM) {
                if (i+1 >= argc) {
                    if (flag->param_mode == FREQ_PARAM) {
                        error(err_t::MISSING_ARG, flag->name);
                        return;
                    }
                }else {
                    pf.param = argv[++i];
                }
            }
            if (parsed_flags_count >= MAX_FLAGS) {
                error(err_t::RUNTIME_FAILURE, "Too many flags");
                return;
            }
            parsed_flags[parsed_flags_count++] = pf;
           
        } else {}
        
    }

    switch (cmd->flag_mode) {
        case BASE_THEN_FLAGS:
            dispatch_handler(cmd, argc, argv);
            for(uint8_t i=0; i<parsed_flags_count; i++) {
                dispatch_flag(cmd, parsed_flags[i], argc, argv);
            }
            break;
        case FLAG_ONLY:
            if (parsed_flags_count == 0) {
                dispatch_handler(cmd, argc, argv);
            } else if (parsed_flags_count == 1) {
                dispatch_flag(cmd, parsed_flags[0], argc, argv);
            } else {   
                error(err_t::INVALID_CMD, "Only one flag allowed");
            }
            break;
        case BASE_ONLY_NO_FLAGS:
            if (parsed_flags_count == 0 ) {
                dispatch_handler(cmd, argc, argv);
            } else {
                for (uint8_t i=0; i<parsed_flags_count; i++) {
                    dispatch_flag(cmd, parsed_flags[i], argc, argv);
                }
            }
            break;
    }
}

void shell::dispatch_handler(const command_rt_t* cmd, int argc, char** argv) {
            if (cmd->type == MEMBER_H)
                (this->*(cmd->exe.mem))(cmd, argc, argv);
            else 
                cmd->exe.free(cmd, argc, argv);
}

//returns a specified flag
const flag_t* shell::find_flag(const command_rt_t* cmd, const char* flagname) {
    if (!cmd) return nullptr;
    const flag_t* const* flags = cmd->flags;
    for(int i=0; flags[i] != nullptr; i++) {
        if(strcmp(flagname, flags[i]->name) == 0) {
            return flags[i];
        }
    }
    return nullptr;
}


//check if user input command has a flag
bool shell::has_flag(int argc, char **argv, const char* flag) {
  
    for(int i=1; i < argc; i++) {
        if(strcmp(argv[i], flag) == 0) {
            return true;
        }
    }
    return false;
}

void shell::dispatch_flag(const command_rt_t* parent, const parsed_flag_t& pf, int argc, char** argv) {
    const flag_t* flag = pf.flag;
    const char* param = pf.param;
    if(flag->type == MEMBER_H) {
        if (!flag->exe.mem) {
            error(err_t::RUNTIME_FAILURE, flag->name);
            return;
        }
        (this->*(flag->exe.mem))(parent, param, argc, argv);
    } else{
        if (!flag->exe.free) {
            error(err_t::RUNTIME_FAILURE, flag->name);
            return;
        }
        (flag->exe.free)(parent, param, argc, argv);
    } 
}

//returns args
const char* shell::get_opt(int argc, char** argv, const char* opt) {
    for(int i=0; i<argc-1; i++) {
        if(strcmp(argv[i], opt) == 0) {
            return argv[i + 1];
        }
    }
    return nullptr;
}

void shell::error(err_t err, const char* detail) {
    puts_ro(CRED);
    puts_ro(ROSTR("Error >> "));
    puts(err_msg[(int)err].description);
    if(detail) {
        puts_ro(ROSTR(" : "));
        puts(detail);
    }
    puts_ro(C_);
    LINE
}


//#####################################################################
// #                                                                  #
// #                      Private Methods                             #  
// #                                                                  #  
//#####################################################################


void shell::build_cmd_table(const command_def_t* table) {
    if (!table) return;
    
    bool is_sys_table = (table == cmd);

    for(int i=0; table[i].name != nullptr; i++) {
        //check for duplicates
        bool exists = false;
        for(int j=0; j<final_count; j++) {
            if(strcmp(table[i].name, final_cmd[j].def->name) == 0) {
                exists = true;
                break;
            }
        }
        if (exists) continue;
        if (final_count < MAX_CMD) {
            final_cmd[final_count].def = &table[i];
            final_cmd[final_count].type = table[i].type;

            if (table[i].type == MEMBER_H) {
                final_cmd[final_count].exe.mem = table[i].exe.mem;
            } else {
                final_cmd[final_count].exe.free = table[i].exe.free;
            }
            
            final_cmd[final_count].flag_count = 0;
            final_cmd[final_count].id = final_count;
            final_cmd[final_count].flag_mode = table[i].flag_mode;
            final_cmd[final_count].is_user = !is_sys_table;
            
            //clr flags (populate by build flag links)
            for(int k=0; k<MAX_FLAGS; k++) {
                final_cmd[final_count].flags[k] = nullptr;
            }
            final_count++;
        }
    }
}

void shell::build_flag_links(const flag_t* table) {
    if (!table) return;

    for(int f=0; table[f].name != nullptr; f++) {
        //match with parent command
        for(int c=0; c < final_count; c++) {
            const char* cmd_name = final_cmd[c].def->name;
            if (strcmp(table[f].parent, cmd_name) == 0) {
                attach_flag_to_command(&final_cmd[c], &table[f]);
            }
        }
    }
}

void shell::attach_flag_to_command(command_rt_t* cmd, const flag_t* flag) {
    if (cmd->flag_count < MAX_FLAGS) {
        cmd->flags[cmd->flag_count++] = flag;
        cmd->flags[cmd->flag_count] = nullptr;
    }
}

command_rt_t* shell::parse_command(char *buffer) {
    for (int i=0; i < final_count; i++) {
        if (strcmp(buffer, final_cmd[i].def->name) == 0) {
            return &final_cmd[i];
        }
    }
    return nullptr;
}

int shell::tokenizer(char *buf, char **argv, int max_args) {
    int argc = 0;
    char *token = strtok(buf, " ");

    while ((token != NULL) && (argc < max_args)) {
        argv[argc++] = token;
        token = strtok(NULL, " ");
    }
    return argc;
}

command_rt_t* shell::get_cmd(const char* name) {
    for(int i=0; i<final_count; i++) {
        if (strcmp(name, final_cmd[i].def->name) == 0) {
            return &final_cmd[i];
        }
    }
    return nullptr;
}

const flag_t* const* shell::get_cmd_flags(const char* parent) {
    command_rt_t* cmd = get_cmd(parent);
    if (!cmd) return nullptr;
    return cmd->flags;
}


//#####################################################################
// #                                                                  #
// #                      Handler Functions                           #  
// #                                                                  #  
//#####################################################################


//help flags
void shell::f_help_long(const command_rt_t* parent, const char* param, int argc, char **argv) {
    puts_ro(ROSTR("\r\nFull command descriptions:\r\n"));
    for(int i=0; i < final_count; i++) {
        if (final_cmd[i].def->in_progmem) {
            printc(NAMECOLOR(i), final_cmd[i].def->name);
            puts_many_ro(ROSTR(" - "), final_cmd[i].def->description, ROSTR("\r\n"), nullptr);
        }
        else {
            printc(NAMECOLOR(i), final_cmd[i].def->name);
            puts_ro(ROSTR(" - "));
            puts(final_cmd[i].def->description);
            LINE
        }
    }
    puts_ro(ROSTR("\r\nUse -c <command> to list flags for a specific command\r\n"));
}

void shell::f_help_c(const command_rt_t* parent, const char* param, int argc, char **argv) {
    if (!param) {
        error(err_t::MISSING_ARG, "Usage: help -c <command>\r\n");
        return;
    }

    for(int a=0; a<final_count; a++) {
        if (strcmp(param, final_cmd[a].def->name) == 0) {
            puts_ro(ROSTR("\r\nFlags available for command: ")); 
            printc(NAMECOLOR(a), final_cmd[a].def->name);
            LINE LINE

            const flag_t* const* flags = final_cmd[a].flags;

            if(!flags || flags[0] == nullptr || flags[0]->name == nullptr) {
                puts_ro(ROSTR("None\r\n"));
                return;
            }
            for (int b=0; flags[b] != nullptr; b++) {
                printc(FLAGCOLOR, flags[b]->name);
                if(final_cmd[a].def->in_progmem) {
                    puts_many_ro(ROSTR(" >> "), flags[b]->description, ROSTR("\r\n"), nullptr);
                } else {
                    puts_many((" >> "), flags[b]->description, "\r\n", nullptr);
                }
            }
            return;
        }
    }
    error(err_t::INVALID_PARAM, param);
}

void shell::cmd_help(const command_rt_t* self, int argc, char **argv) {
    if (argc == 1) {
        puts_ro(ROSTR("\r\nHere is a list of system commands:\r\n"));
        puts_ro(ROSTR("----------------------------------\r\n"));
        for (int i=0; i<final_count; i++) {
            printc(NAMECOLOR(i), final_cmd[i].def->name);
            puts_ro(ROSTR("\r\n"));
            }
            return;
    } 
}

void shell::cmd_debug(const command_rt_t* self, int argc, char **argv) {
    
}

void shell::cmd_print(const command_rt_t* self, int argc, char **argv) {

}

void shell::f_led_blink(const command_rt_t* parent, const char* param, int argc, char **argv) {
    int n = atoi(param);
    if (n > 20) {
        error(err_t::INVALID_PARAM, "Must be and integer less than 20.");
        return;
    }
    puts_ro(ROSTR("LED blinking..."));
    low(PIN_B7);
    for(int i=0; i < (2*n); i++) {
        toggle(PIN_B7);
        _delay_ms(500);
    }
}

void shell::cmd_led(const command_rt_t* self, int argc, char **argv) {
    if (argc == 1) {
        toggle(PIN_B7);
    } 
}



