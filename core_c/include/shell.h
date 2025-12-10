#pragma once
#include <stdbool.h>
#include <stdint.h>

#define MAX_ARGS 8
#define MAX_FLAGS 8


struct shell;
struct shell_cmd;
struct flag;

typedef struct shell        shell_t;
typedef struct shell_cmd    shell_cmd_t;
typedef struct flag         flag_t;

/*I/O Wrapper*/
/*typedef struct serial_io_s {
    void (*putc)(char c, void* self);
    void (*puts)(const char* s, void* self);
    void (*puts_p)(const char* p, void* self);
    void* self;
}shell_io_t;

extern shell_io_t* user_io;*/

/*Error handling*/
typedef enum err {
   NONE,
   INVALID_CMD,
   UNKNOWN_FLAG,
   MISSING_ARG,
   RUNTIME_FAILURE,
   INVALID_PARAM
}err_t;

typedef struct err_s {
    err_t type; 
    const char* description;
}_err;

typedef struct shell_err {
    err_t       type; 
    const char* description;
}shell_err_t;

extern const _err err_msg[];

/*Execution and parameter modes*/
typedef enum exe_mode {
    CMD_NO_FLAG, CMD_THEN_FLAG, FLAG_ONLY
} exe_mode_t; 

typedef enum param_mode {
    NOPARAM, REQ, OPT
} cmd_param_t, flag_param_t;

/*Flags*/
typedef void(*flag_handler)(
    shell_t* sh, 
    const shell_cmd_t* parent,
    const char* param, 
    int argc, 
    char **argv);

struct flag {
    const char* name;
    const char* description; 
    const char* parent;
    flag_handler handler;
    struct {
        bool in_progmem         : 1;
        flag_param_t param_mode : 2;
        uint8_t reserved        : 5;
    } sub;
}; 

/*Commands*/
typedef void(*cmd_handler)(
    shell_t* sh,
    const shell_cmd_t* self, 
    const char* param, 
    int argc, 
    char **argv);

struct shell_cmd {
    const char* name;
    const char* description;
    cmd_handler handler;
    const flag_t* flag_base; //points to first flag for command in flag table 
    struct {
        unsigned flag_mode  : 2;
        unsigned param_mode : 2;
        unsigned in_progmem : 1;
        unsigned flag_count : 3; //tells you how many up from base belong to command
    } sub;
};

/*Parse indices*/
typedef struct {
    int8_t param_idx;
    uint8_t flag_pos[MAX_FLAGS];
    uint8_t flag_count;
} sh_parse_idx_t;

/*Shell state*/
struct shell {
    void (*putc)(char c, void* user);
    void (*puts)(const char* s, void* user);
    void (*puts_p)(const char* p, void* user);
    void* io_user;

    char *buffer;
    uint8_t buf_len;
    uint8_t position;

    char **argv;
    uint8_t argc;
    uint8_t max_argv;

    const shell_cmd_t *cmd;
    uint8_t cmd_count;

};

/*Optional color output*/
typedef enum color {
    CYAN, YELLOW, GREEN, RED, RST
}color_t;



#define __COLOR_ON__

#if defined(__COLOR_ON__)
    #define CCYA   ROSTR("\x1B[36m")   //cyan
    #define CYEL    ROSTR("\x1B[33m")      //yellow
    #define CGRN    ROSTR("\x1B[32m")    //green 
    #define CRED    ROSTR("\x1B[31m")  //red
    #define C_      ROSTR("\x1B[0m")    //rst
    #define COLOR_RESET  "\x1B[0m"
#else 
    #define COLOR_CYAN  
    #define COLOR_YELLOW 
    #define COLOR_GREEN  
    #define COLOR_RED    
    #define COLOR_RESET  
#endif


void                sh_init(shell_t* sh, const shell_cmd_t* user_cmds, char* input_buf, uint8_t buf_len, 
                        char** argv, uint8_t max_argv, void (*putc)(char c, void* user), void* io_user, 
                        uint8_t cmd_count);
int                 sh_poll(shell_t* sh, uint8_t c);

void                sh_puts(shell_t* sh, const char* s);
void                sh_puts_ro(shell_t* sh, const char* p);
void                sh_printc(shell_t* sh, color_t color, const char* s);
void                error(shell_t* sh, err_t err, const char* detail);

//err_t               sh_parse_args(const shell_cmd_t* cmd, int argc, char**argv, sh_parse_idx_t* out,
//                        const char** err_arg);
//const shell_cmd_t*  sh_find_cmd(shell_t* sh, const char *name);
void                sh_execute(shell_t* sh, int argc, char** argv);
int                 tokenizer(char *buf, char **argv, int max_args);

void                sh_dispatch_handler(shell_t* sh, const shell_cmd_t* cmd, const char* param, int argc, char** argv);
const flag_t*       sh_find_flag(const shell_cmd_t* cmd, const char* n);
bool                sh_has_flag(int argc, char **argv, const char* f);    //check if user input has flag argument
void                sh_dispatch_flag(shell_t* sh, const shell_cmd_t* parent, const flag_t* flag, const char* fparam, int argc, char** argv);

uint8_t sh_max_cmd_name_len(shell_t *sh);








//read only wrappers allow for memory optimizations on certain devices
#if defined(__AVR__)
    //optimize lut and string literal storage for AVR chips by storing in flash
    #include <avr/pgmspace.h>
    #define RODATA PROGMEM
    

    

#define ROSTR(x) PSTR(x)

  
    
    #define READ_RO_PTR(type, addr) ((type)pgm_read_word(addr))
    #define READ_RO_BYTE(addr) pgm_read_byte(addr)
#else
    //for architectures w/o distinct flash and sram 
    #define RODATA
    #define ROSTR(x) (x)

    #define READ_RO_POINTER(type, addr) (*(type*)(addr))
    #define READ_RO_BYTE(addr) (*(uint8_t*)(addr))
#endif

#define LINE sh_puts_ro(sh, ROSTR("\r\n"));
#define NAMECOLOR(s) (final_cmd[s].is_user ? YELLOW : CYAN)
#define FLAGCOLOR GREEN
#define SPACE sh_puts_ro(sh, " ");