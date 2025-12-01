#include "../include/shell.h"
#include "../include/usart.h"
#include "../include/ring_buffer.h"
#include "../include/ro.h"
#include "../include/colors.h"
#include <string.h>

ringbuf GLOBAL_BUFFER = {{0}, 0, 0};

shell::shell(USART* interface) : _interface(interface) {
    puts_ro(ROSTR("\r\n...shell active...\r\n"));
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

            _argc = tokenizer(sh_line, _argv, MAX_ARGS);    
            command_rt_t* cmd = parse_command(_argv[0]);
            //puts_const(sh_line);              debug readouts
            //puts_const(cmd->def->name);
            if (!cmd) {
               puts_ro(ROSTR("Invalid command.\r\nType help -long for a list of commands and their descriptions"));
            } else {
                if (cmd->type == MEMBER_H) {
                    (this->*(cmd->exe.mem))(_argc, _argv);
                } else {
                    cmd->exe.free(_argc, _argv);
                }
                
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

void shell::puts(char* s) {
    _interface->put_string(s);
}

void shell::puts_const(const char* s) {
     _interface->put_string((char*)s);
}
void shell::puts_many(const char* first, ...) {
        va_list args;
        va_start(args, first);

        const char* s = first;
        while(s != nullptr) {
            _interface->put_string((char*)s);
            s = va_arg(args, const char*);
        }
        va_end(args);
    }

void shell::puts_ro(const char* p) {
    char c;
    while((c = READ_RO_BYTE(p++))) {
        _interface->putc(c);
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

char shell::nbget() {
    return 0;
}

char shell::bget() {
    _interface->receive();
    return 0;
}


//#####################################################################
// #                                                                  #
// #                      Private Methods                             #  
// #                                                                  #  
//#####################################################################


void shell::build_cmd_table(const command_def_t* table) {
    if (!table) return;
    
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
void shell::f_help_long(int argc, char **argv) {
    puts_ro(ROSTR("\r\nFull command descriptions:\r\n"));
    for(int i=0; i < final_count; i++) {
        if (final_cmd[i].def->in_progmem) {
            puts_many(CCYA, final_cmd[i].def->name, C_, nullptr);
            puts_many_ro(ROSTR(" - "), final_cmd[i].def->description, ROSTR("\r\n"), nullptr);
        }
        else {
            puts_const(final_cmd[i].def->name);
            puts_ro(ROSTR(" - "));
            puts_const(final_cmd[i].def->description);
            puts_ro(ROSTR("\r\n"));
        }
         
    }
    puts_ro(ROSTR("\r\nUse -c <command> to list flags for a specific command\r\n"));
}

void shell::f_help_c(int argc, char **argv) {
    if (argc < 3) {
        puts_ro(ROSTR("Usage: help -c <command>\r\n"));
        return;
    }
    const char* target = argv[2];
    for(int a=0; a<final_count; a++) {
        if (strcmp(target, final_cmd[a].def->name) == 0) {
            puts_ro(ROSTR("\r\nFlags available for command: ")); 
            puts_const(CCYA);
            puts_const(final_cmd[a].def->name);
            puts_many(C_, "\r\n\r\n", nullptr);

            const flag_t* const* flags = final_cmd[a].flags;

            if(!flags || flags[0] == nullptr || flags[0]->name == nullptr) {
                puts_ro(ROSTR("None\r\n"));
                return;
            }
            for (int b=0; flags[b] != nullptr; b++) {
                puts_ro(ROSTR("\x1B[32m"));
                puts_const(flags[b]->name);
                puts_ro(ROSTR("\x1B[0m"));
                puts_many_ro(ROSTR(" >> "), flags[b]->description, ROSTR("\r\n"), nullptr);
            }
            return;
        }
    }
    puts_ro(ROSTR("Command ")); 
    puts_const(target);
    puts_ro(ROSTR(" not recognized"));
}

void shell::cmd_help(int argc, char **argv) {
    if (argc == 1) {
        puts_ro(ROSTR("\r\nHere is a list of system commands:\r\n"));
        puts_ro(ROSTR("----------------------------------\r\n"));
        for (int i=0; i<final_count; i++) {
            puts_many(CCYA, final_cmd[i].def->name, C_, "\r\n", nullptr);
            }
            
            return;
    } 
    
    //argc>1
    const char* flagname = argv[1];
    const flag_t* const* flags = get_cmd_flags("help");

    if (!flags) {
        puts_ro(ROSTR("Help command has no flags"));
        return;
    }

    for(int i=0; flags[i] != nullptr; i++) {
        if(strcmp(flagname, flags[i]->name) == 0) {
            (this->*(flags[i]->exe))(argc, argv);
            return;
        }
    }
    puts_ro(ROSTR("Unknown help flag\r\n"));
    /*else if (argc > 1) {
        for (int i=1; i<argc; i++) {
            for (int j=0; help_flags[j].name != nullptr; j++) {
                if (strcmp(argv[i], help_flags[j].name)==0) {
                    (this->*(help_flags[j].exe))(argc, argv);
                    return;
                }
            }
        }
    }*/
}

void shell::cmd_debug(int argc, char **argv) {
    
}

void shell::cmd_print(int argc, char **argv) {

}

void shell::cmd_led(int argc, char **argv) {
    if (argc == 1) {
        toggle(PIN_B7);
    } else if (argc > 1) {
        for (int i = 0; i<argc; i++) {
            if (strcmp(argv[i], "-d")==0) {
                toggle(PIN_B7);
                puts_ro(cmd[LED].description);
                puts_ro(ROSTR("\r\n"));
            }
        }
    }
}



