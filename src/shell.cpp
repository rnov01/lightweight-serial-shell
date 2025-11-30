#include "../include/shell.h"
#include "../include/usart.h"
#include "../include/ring_buffer.h"
#include <string.h>

ringbuf GLOBAL_BUFFER = {{0}, 0, 0};

/*static const str_to_cmd COMMAND_LUT[] = {
    {"help", COMMAND::HELP},
    {"debug", COMMAND::DEBUG},
    {"print", COMMAND::PRINT},
    {"led", COMMAND::LED},
    {nullptr, COMMAND::err}
};

COMMAND parse_command(char *buffer) {
    for (int i=0; COMMAND_LUT[i].name != nullptr; i++) {
        if (strcmp(buffer, COMMAND_LUT[i].name) == 0) {
            return COMMAND_LUT[i].cmd;
        }
    }
    return COMMAND::err;
}*/


shell::shell(USART* interface) : _interface(interface) {
    puts_const("\r\n...shell active...\r\n");
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
    puts_const("Full command descriptions:\r\n\r\n");
    for(int i=0; cmd[i].name != nullptr; i++) {
        puts_many(cmd[i].name, " - ", cmd[i].description, "\r\n", nullptr); 
    }
    puts_const("\r\nUse -c <command> to list flags for a specific command\r\n");
}

void shell::f_help_c(int argc, char **argv) {
    if (argc < 3) {
        puts_const("Usage: help -c <command>\r\n");
        return;
    }

    const char* target = argv[2];

    for(int a=0; cmd[a].name != nullptr; a++) {
        if (strcmp(target, cmd[a].name)==0) {
            puts_many("Flags available for command: ", cmd[a].name, "\r\n\r\n", nullptr);

            const flag_t* flags = cmd[a].flags;

            if(!flags || flags[0].name == nullptr) {
                puts_const("None\r\n");
                return;
            }
            for (int b=0; flags[b].name != nullptr; b++) {
                puts_many(flags[b].name, " >> ", flags[b].description, "\r\n", nullptr);
            }
            return;
        }
    }

    puts_many("Command ", target, " not recognized");
}

void shell::cmd_help(int argc, char **argv) {
    if (argc == 1) {
        puts_const("Here is a list of system commands:\r\n");
        puts_const("----------------------------------\r\n");
        for (int i=0; cmd[i].name != nullptr; i++) {
                puts((char*)cmd[i].name);
                puts_const("\r\n");
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
                puts_const(cmd[LED].description);
                puts_const("\r\n");
            }
        }
    }
}



