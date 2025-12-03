#pragma once

#include "usart.h"
#include "colors.h"
#include <stdarg.h>

/*! \brief Maximum number of arguments that can be parsed from a command line. 
 *  \note Multiple flags on a single command not currently supported. */
#define MAX_ARGS 8
#define MAX_FLAGS 8

// Forward declaration of shell class and runtime command struct
class shell;
struct command_rt_t;


/*! \brief Function pointer type for command handlers (cmd_* functions). 
 *  \details Command handlers execute the main command functionality and are responsible for invoking any 
 *  associated flag handlers (f_* functions) if any flags were supplied via terminal input. 
 *  \param argc Number of parsed arguments (command + flags + parameters) 
 *  \param argv Array of pointers to parsed argument strings */
typedef void (shell::*cmd_handler)(const command_rt_t* self, const char* param, int argc, char **argv);
/*! \brief Function pointer type for flag handlers (f_* functions)
 *  \details These functions implement the behavior of flags/subcommands associated with a primary command. They
    are called <i>only</i> from within their respective cmd_* command handler.
 *  \param argc Number of parsed arguments 
 *  \param argv Array of pointers to parsed argument strings */
typedef void (shell::*flag_handler)(const command_rt_t* parent, const char* param, int argc, char **argv);

typedef void(*cmd_handler_free)(const command_rt_t* self, const char* param, int argc, char **argv);
typedef void(*flag_handler_free)(const command_rt_t* parent, const char* param, int argc, char **argv);

typedef struct serial_io_s {
    void (*putc)(char c, void* self);
    void (*puts)(const char* s, void* self);
    void (*puts_p)(const char* p, void* self);
    void* self;
}shell_io_t;

union cmd_handler_u{
        cmd_handler mem;
        cmd_handler_free free;
    };

union flag_handler_u {
    flag_handler mem;
    flag_handler_free free;
};

enum handler_type_t {       //member handler function for system commands, free handler functions for user cmds
    MEMBER_H, FREE_H
};

enum flag_mode_t {
    BASE_THEN_FLAGS, FLAG_ONLY, BASE_ONLY_NO_FLAGS
};

enum flag_param_t {
    FNO_PARAM, FREQ_PARAM, FOPT_PARAM
};

enum cmd_param_t {
    CNO_PARAM, CREQ_PARAM, COPT_PARAM
};

/*! \struct flag_t 
 *  \brief Describes a single flag (subcommand) associated with a primary command
 *  \details Each command that accepts flags must have an associated flag_t lookup table containing one 
 *  entry for each supported flag. 
 *  \param name The flag string (e.g. "-long")
 *  \param description A short description of the flag's behavior. This can be displayed using the \code help -c
 *  <command> \endcode command 
 *  \param exe A pointer to the flag handler function */
struct flag_t {
    const char* name;
    const char* description;
    handler_type_t type;
    flag_handler_u exe;
    const char* parent;
    bool in_progmem;
    flag_param_t param_mode;
};

//struct to store parsed flags and their parameters
struct parsed_flag_t {
    const flag_t* flag;
    const char* param;
};

/*! /struct command_t 
 *  \brief Struct defining the lookup table entry for each system command. 
 *  \details Their should be exactly one definition for the command lookup table in \ref lut.cpp. 
 *  \param name Command name (string literal)
 *  \param description Short summary printed using \code help -long \endcode
 *  \param exe A pointer to the primary cmd_* handler for this command
 *  \param id Command ID for backend execution
 *  \param flags A pointer to an array of flag_t structs, or nullptr if the command has no flags */
struct command_def_t {      //order identical as enum for easy lookup
    const char* name;
    const char* description;
    handler_type_t type;
    cmd_handler_u exe;
    flag_mode_t flag_mode;
    bool in_progmem;
    cmd_param_t param_mode;
};

//runtime mutable struct
struct command_rt_t {               
        const command_def_t* def;   //immutable definition
        handler_type_t type;
        cmd_handler_u exe;
        flag_mode_t flag_mode;
        const flag_t* flags[MAX_FLAGS]; 
        uint8_t flag_count;
        const char* param;
        int id;
        bool is_user;
};

enum class err_t {
   NONE,
   INVALID_CMD,
   UNKNOWN_FLAG,
   MISSING_ARG,
   RUNTIME_FAILURE,
   INVALID_PARAM
};

typedef struct _err {
    err_t type; 
    const char* description;
};


/*! \class shell 
 *  \brief Driver class for the serial shell interface. 
 *  \details The shell provides a minimal cli over a serial communication link (typically USART). It supports:
 *  \n- command parsing \n-argument tokenization \n-flag-based subcommmands \n-automatic help generation \n \n
 *  The shell is designed to be lightweight, non-blocking, and easily adapted to different communication drivers.
 *  */
class shell {

public:
    /*! \fn shell(USART* interface)
     * \brief Construct a shell object using the specified serial interface. */
    shell(shell_io_t* interface);
    /*!
    */
    void init(const command_def_t* user_cmds = nullptr, const flag_t* user_flags = nullptr);
    /*! \fn tick()
     *  \brief Placeholder periodic update hook
     *  \note Currently unused */
    void tick();
    /*! \fn poll()
     *  \brief Polls the global ring buffer for new characters
     *  \details When a complete command is detected (terminated by a newline), the line is tokenized, parsed, 
     *  and executed. 
     *  \return The command_id_t of the parsed command or NONE if no command is detected */
    int poll();
    /*! \fn puts(char* s) 
     *  \brief Used to transmit a mutable string
      * \param s A string (pointer to char array)*/
    void puts(const char* s);
    /*! \fn puts_many(const char* first, ...)
     *  \brief Print multiple concatenated strings separated by commmas 
     *  \details This function accepts a variable-length list of string arguments. Strings are transmitted sequentially
     *  until a final nullptr arguments is encountered Usage as follows: \n
     *  \code *char name[] = {name};
     *  puts_many("Hello", name, "how are you today?"); \endcode 
     *  \warning The last string argument passed MUST be nullptr. \n consider a macro to automatically 
     *  append a nullptr string to the function arguments for safety */
    void puts_many(const char* first, ...);
    //flexible for flash storage on some hardware
    void puts_ro(const char* p);
    void puts_many_ro(const char* first, ...);

    //color coded puts for special identifiers
    void printc(color_t color, const char* s);         //color string
  
    //user helper funcs
    void execute(command_rt_t* cmd, int argc, char** argv);
    void dispatch_handler(const command_rt_t* cmd, int argc, char** argv);
    const flag_t* find_flag(const command_rt_t* cmd, const char* n);
    bool has_flag(int argc, char **argv, const char* f);    //check if user input has flag argument
    void dispatch_flag(const command_rt_t* parent, const parsed_flag_t& pf, int argc, char** argv);
    const char* get_opt(int argc, char** argv, const char* opt);    //returns cmd or flag parameter


    void error(err_t err, const char* msg);

    /*! \name Legacy functions
     *  \brief Non-blocking/blocking char input
     * @{*/
    char nbget();   //non-blocking 
    char bget();    //blocking
    ///@}
private:
    shell_io_t* io;

    char cmd_buffer[64];
    char sh_line[64];
    uint8_t line_len = 0;

    char *_argv[MAX_ARGS];
    int _argc = 0;

    void build_cmd_table(const command_def_t* table);   //combines built-in lut with any user defined commands
    void build_flag_links(const flag_t* table);
    void build_sys_flag_links(void);
    void attach_flag_to_command(command_rt_t* cmd, const flag_t* flag);
    static const int MAX_CMD = 50;
    command_rt_t final_cmd[MAX_CMD];
    int final_count = 0;

    static const _err err_msg[];

    //get functions
    command_rt_t* get_cmd(const char* name);
    const flag_t* const* get_cmd_flags(const char* parent);
    //command handler functions
    void cmd_help(const command_rt_t* self, const char* param, int argc, char **argv);
    void cmd_debug(const command_rt_t* self, const char* param, int argc, char **argv);
    void cmd_print(const command_rt_t* self, const char* param, int argc, char **argv);
    void cmd_led(const command_rt_t* self, const char* param, int argc, char **argv);
    //flag handler functions
    //help flags
    void f_help_long(const command_rt_t* parent, const char* param, int argc, char **argv);
    void f_help_c(const command_rt_t* parent, const char* param, int argc, char **argv);
    void f_led_blink(const command_rt_t* parent, const char* param, int argc, char **argv);

    command_rt_t* parse_command(char *buffer);
    int tokenizer(char *buf, char **argv, int max_args);
   
    static const command_def_t cmd[];
    //struct command_runtime_t 
    static const flag_t help_flags[];
    static const flag_t debug_flags[];
    static const flag_t print_flags[];
    static const flag_t led_flags[];
    static const flag_t sys_flags[];
};

#define puts_many_(sh, ...) \
    do { \
        const char* const _ro_list_[] = { __VA_ARGS__, nullptr }; \
        puts_many_ro_i(sh, _ro_list_); \
    } while(0)

#define SHELL_CMD(name, desc, handler, flags)   \
    const command_t __shell_cmd_##handler       \
    __attribute__((section(".shell_cmds"))) = { \
        ROSTR(name),                            \
        ROSTR(desc),                            \
        handler,                                \
        NONE,                                   \
        flags,                                  \
        nullptr                                 \
    };

#define PRINTLN(s) do {this->puts(s); this->puts("\r\n");} while(0)
#define LINE this->puts_ro(ROSTR("\r\n"));
#define PUTP(s) this->puts_ro(ROSTR(s));
#define NAMECOLOR(s) (final_cmd[s].is_user ? YELLOW : CYAN)
#define FLAGCOLOR GREEN
#define PUTLNP(s) puts_many_ro(s, ROSTR("\r\n"), nullptr)

//user table macro helpers

#define CMD(name, description, handler, mode, param_mode) \
    {name, description, FREE_H, {.free = handler}, mode, false, param_mode}
#define CMD_END {nullptr, nullptr, FREE_H, {.free = nullptr}, FLAG_ONLY, false, CNO_PARAM}

#define FLAG(parent, flagname, description, handler, param_mode) \
    {flagname, description, FREE_H, {.free = handler}, parent, false, param_mode}
#define FLAG_END {nullptr, nullptr, FREE_H, {.free = nullptr}, nullptr, false, FNO_PARAM}

#define SH_USR_CMD(name) name(const command_rt_t* self, int argc, char** argv)