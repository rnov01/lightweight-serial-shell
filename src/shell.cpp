#include "../include/shell.h"
#include "../include/usart.h"
#include "../include/ring_buffer.h"
#include "../include/ro.h"
#include <string.h>

ringbuf GLOBAL_BUFFER = {{0}, 0, 0};

shell::shell(USART* interface) : _interface(interface) {
    puts_ro(ROSTR("\r\n...shell active...\r\n"));
}

void shell::init(const command_t* user_cmds = nullptr, const flag_t* user_flags = nullptr) {
    final_count = 0;
    build_cmd_table(cmd);
    build_cmd_table(user_cmds);
    build_flag_links(user_flags);
}

void shell::tick() {

}

command_id_t shell::poll() {
    uint8_t c;

    if (!buf_pop(&GLOBAL_BUFFER, &c))   //no data available
        return NONE;
    if (c == '\r' || c == '\n') {       //end of command
        if(line_len > 0) {
            sh_line[line_len] = '\0';       //terminate str
            line_len = 0;                   //reset 

            _argc = tokenizer(sh_line, _argv, MAX_ARGS);    
            command_id_t cmd = parse_command(sh_line);

            if (cmd == err) {
                puts_ro(ROSTR("Invalid command.\r\nType help -long for a list of commands and their descriptions"));
            }
            if(true) {              //change this to monitor system status (if not busy)
                execute(cmd);
            }
        }
        return NONE;        //newline w/ no data -> ignore
    }

    if (line_len < sizeof(sh_line) - 1) {    //append single char if space available
        sh_line[line_len++] = c;
    }

    if (c == '\b' && line_len > 0) {        //delete last char if backspace is pressed
    line_len--;
    return NONE;
    }

    return NONE;
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

static const str_to_cmd COMMAND_LUT[] = {
    {"help", command_id_t::HELP},
    {"debug", command_id_t::DEBUG},
    {"print", command_id_t::PRINT},
    {"led", command_id_t::LED},
    {nullptr, command_id_t::err}
};

void shell::build_cmd_table(const command_t* table) {
    if (!table) return;

    for(int i=0; table[i].name != nullptr; i++) {
        if (final_count < MAX_CMD) {
            final_cmd[final_count++] = &table[i];
        }
    }
}

void shell::build_flag_links(const flag_t* table) {
    if (!table) return;

    for(int f=0; table[f].name != nullptr) {
        //match with parent command
        for(int c=0; c < final_count; c++) {
            if (strcmp(table[f].parent, final_cmd[c]->name) == 0) {
                attach_flag_to_command(final_cmd[c], &table[f]);
            }
        }
    }
}

void shell::attach_flag_to_command(command_t* cmd, const flag_t* flag) {
    if (cmd->flag_count < MAX_FLAGS) {
        cmd->flags[cmd->flag_count++] = flag;
    }
}

command_id_t shell::parse_command(char *buffer) {
    for (int i=0; cmd[i].name != nullptr; i++) {
        if (strcmp(buffer, cmd[i].name) == 0) {
            return cmd[i].id;
        }
    }
    return command_id_t::err;
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

void shell::execute(command_id_t cmd_x) {
    switch (cmd_x) {
        case HELP:
            (this->*(cmd[cmd_x].exe))(_argc, _argv);
            break;
        case LED:
            //toggle(PIN_B7);
            (this->*(cmd[cmd_x].exe))(_argc, _argv);
            break;
   
    }
}

//help flags
void shell::f_help_long(int argc, char **argv) {
    puts_ro(ROSTR("Full command descriptions:\r\n"));
    for(int i=0; cmd[i].name != nullptr; i++) {
        puts_const(cmd[i].name);
        puts_many_ro(ROSTR(" - "), cmd[i].description, ROSTR("\r\n"), nullptr); 
    }
    puts_const(ROSTR("\r\nUse -c <command> to list flags for a specific command\r\n"));
}

void shell::f_help_c(int argc, char **argv) {
    if (argc < 3) {
        puts_ro(ROSTR("Usage: help -c <command>\r\n"));
        return;
    }
    const char* target = argv[2];
    for(int a=0; cmd[a].name != nullptr; a++) {
        if (strcmp(target, cmd[a].name)==0) {
            puts_ro(ROSTR("\r\nFlags available for command: ")); 
            puts_ro(ROSTR("\x1B[34m"));
            puts_const(cmd[a].name);
            puts_ro(ROSTR("\x1B[0m\r\n\r\n"));
    

            const flag_t* flags = cmd[a].flags;

            if(!flags || flags[0].name == nullptr) {
                puts_ro(ROSTR("None\r\n"));
                return;
            }
            for (int b=0; flags[b].name != nullptr; b++) {
                puts_ro(ROSTR("\x1B[32m"));
                puts_const(flags[b].name);
                puts_ro(ROSTR("\x1B[0m"));
                puts_many_ro(ROSTR(" >> "), flags[b].description, ROSTR("\r\n"), nullptr);
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
        puts_ro(ROSTR("Here is a list of system commands:\r\n"));
        puts_ro(ROSTR("----------------------------------\r\n"));
        for (int i=0; cmd[i].name != nullptr; i++) {
                puts((char*)cmd[i].name);
                puts_ro(ROSTR("\r\n"));
            }
    } else if (argc > 1) {
        for (int i=1; i<argc; i++) {
            for (int j=0; help_flags[j].name != nullptr; j++) {
                if (strcmp(argv[i], help_flags[j].name)==0) {
                    (this->*(help_flags[j].exe))(argc, argv);
                    return;
                }
            }
        }
    }
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



