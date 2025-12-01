#pragma once

#include "usart.h"
#include <stdarg.h>

/*! \brief Maximum number of arguments that can be parsed from a command line. 
 *  \note Multiple flags on a single command not currently supported. */
#define MAX_ARGS 8
#define MAX_FLAGS 8

/*! \enum command_id_t
 *  \brief Enumerated list of system commands.
 *  \details Each command string parsed from user input is converted into one of these IDs. The mapping is 
 *  performed by comparing user input against the command lookup table (\ref shell::cmd[]).
 *  \note The order of this enum must match the order of the entries in \ref shell::cmd[] to ensure 
 *  correct indexing */
typedef enum {
    HELP, DEBUG, PRINT, LED, NONE, err, NUM_CMDS
}command_id_t;

/* \brief Legacy structure from previous simplified command table 
 *  \note Currently unused. Retained only for reference. */
struct str_to_cmd {
    const char* name;
    command_id_t cmd; 
};
struct _err {
    int err_id;
    const char* message;
};

// Forward declaration of shell class and runtime command struct
class shell;
struct command_rt_t;

/*! \brief Function pointer type for command handlers (cmd_* functions). 
 *  \details Command handlers execute the main command functionality and are responsible for invoking any 
 *  associated flag handlers (f_* functions) if any flags were supplied via terminal input. 
 *  \param argc Number of parsed arguments (command + flags + parameters) 
 *  \param argv Array of pointers to parsed argument strings */
typedef void (shell::*cmd_handler)(const command_rt_t* self, int argc, char **argv);
/*! \brief Function pointer type for flag handlers (f_* functions)
 *  \details These functions implement the behavior of flags/subcommands associated with a primary command. They
    are called <i>only</i> from within their respective cmd_* command handler.
 *  \param argc Number of parsed arguments 
 *  \param argv Array of pointers to parsed argument strings */
typedef void (shell::*flag_handler)(const command_rt_t* parent, int argc, char **argv);

typedef void(*cmd_handler_free)(const command_rt_t* self, int argc, char **argv);
typedef void(*flag_handler_free)(const command_rt_t* parent, int argc, char **argv);

union cmd_handler_u{
        cmd_handler mem;
        cmd_handler_free free;
    };

enum handler_type_t {       //member handler function for system commands, free handler functions for user cmds
    MEMBER_H, FREE_H
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
    flag_handler exe;
    const char* parent;
    bool in_progmem;
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
    bool in_progmem;
};

//runtime mutable struct
struct command_rt_t {               
        const command_def_t* def;   //immutable definition
        handler_type_t type;
        cmd_handler_u exe;
        const flag_t* flags[MAX_FLAGS]; 
        uint8_t flag_count;
        int id;
        bool is_user;
};





/*! \name f_lut Flag lookup table declarations 
 *  \brief One table is defined for each command that supports flags. For commands without flags, the
 *  associated pointer in \ref shell::cmd[] should be set to nullptr. 
 * @{
 */
extern const flag_t help_flags[];
extern const flag_t debug_flags[];
extern const flag_t print_flags[];
extern const flag_t led_flags[];
///@}

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
    shell(USART* interface);
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
    void puts(char* s);
    /*! \fn puts_const(const char* s)
     *  \brief Used to trasmit constant string (string literal)
     *  \param s A const string (const pointer to char array)*/
    void puts_const(const char* s);
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
    void puts_name(command_rt_t* cmd);
    void puts_err(const char* err);
    void puts_flag(const char* name);

    //user helper funcs
    void execute(const command_rt_t* cmd, int argc, char** argv);
    const flag_t* find_flag(const command_rt_t* cmd, const char* n);
    bool has_flag(int argc, char **argv, const char* f);    //check if user input has flag argument
    void dispatch_flag(const command_rt_t* parent, const flag_t* flag, int argc, char** argv);
    const char* get_opt(int argc, char** argv, const char* opt);    //returns cmd or flag parameter


    void error(const char* msg);

    /*! \name Legacy functions
     *  \brief Non-blocking/blocking char input
     * @{*/
    char nbget();   //non-blocking 
    char bget();    //blocking
    ///@}
private:
     USART* _interface;

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

    const _err* err_msg[];

    //get functions
    command_rt_t* get_cmd(const char* name);
    const flag_t* const* get_cmd_flags(const char* parent);
    //command handler functions
    void cmd_help(const command_rt_t* self, int argc, char **argv);
    void cmd_debug(const command_rt_t* self, int argc, char **argv);
    void cmd_print(const command_rt_t* self, int argc, char **argv);
    void cmd_led(const command_rt_t* self, int argc, char **argv);
    //flag handler functions
    //help flags
    void f_help_long(const command_rt_t* parent, int argc, char **argv);
    void f_help_c(const command_rt_t* parent, int argc, char **argv);
    void f_led_blink(const command_rt_t* parent, int argc, char **argv);

    command_rt_t* parse_command(char *buffer);
    int tokenizer(char *buf, char **argv, int max_args);
   
    static const command_def_t cmd[NUM_CMDS+1];
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

#define PUT(s) puts_const(s)
#define PUTLN(s) puts_many(s, "\r\n", nullptr)
#define PUTP(s) puts_ro(s)
#define PUTLNP(s) puts_many_ro(s, ROSTR("\r\n"), nullptr)
#define SH_USR_CMD(name) name(const command_rt_t* self, int argc, char** argv)