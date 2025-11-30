#pragma once

#include "usart.h"
#include <stdarg.h>

/*! \brief Maximum number of arguments that can be parsed from a command line. 
 *  \note Multiple flags on a single command not currently supported. */
#define MAX_ARGS 8

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

// Forward declaration of shell class
class shell;

/*! \brief Function pointer type for command handlers (cmd_* functions). 
 *  \details Command handlers execute the main command functionality and are responsible for invoking any 
 *  associated flag handlers (f_* functions) if any flags were supplied via terminal input. 
 *  \param argc Number of parsed arguments (command + flags + parameters) 
 *  \param argv Array of pointers to parsed argument strings */
typedef void (shell::*cmd_handler)(int argc, char **argv);
/*! \brief Function pointer type for flag handlers (f_* functions)
 *  \details These functions implement the behavior of flags/subcommands associated with a primary command. They
    are called <i>only</i> from within their respective cmd_* command handler.
 *  \param argc Number of parsed arguments 
 *  \param argv Array of pointers to parsed argument strings */
typedef void (shell::*flag_handler)(int argc, char **argv);

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
};

/*! /struct command_t 
 *  \brief Struct defining the lookup table entry for each system command. 
 *  \details Their should be exactly one definition for the command lookup table in \ref lut.cpp. 
 *  \param name Command name (string literal)
 *  \param description Short summary printed using \code help -long \endcode
 *  \param exe A pointer to the primary cmd_* handler for this command
 *  \param id Command ID for backend execution
 *  \param flags A pointer to an array of flag_t structs, or nullptr if the command has no flags */
struct command_t {      //order identical as enum for easy lookup
    const char* name;
    const char* description;
    cmd_handler exe;
    command_id_t id;
    const flag_t* flags;
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
    /*! \fn tick()
     *  \brief Placeholder periodic update hook
     *  \note Currently unused */
    void tick();
    /*! \fn poll()
     *  \brief Polls the global ring buffer for new characters
     *  \details When a complete command is detected (terminated by a newline), the line is tokenized, parsed, 
     *  and executed. 
     *  \return The command_id_t of the parsed command or NONE if no command is detected */
    command_id_t poll();
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

    //command handler functions
    void cmd_help(int argc, char **argv);
    void cmd_debug(int argc, char **argv);
    void cmd_print(int argc, char **argv);
    void cmd_led(int argc, char **argv);
    //flag handler functions
    //help flags
    void f_help_long(int argc, char **argv);
    void f_help_c(int argc, char **argv);

    command_id_t parse_command(char *buffer);
    int tokenizer(char *buf, char **argv, int max_args);
    void execute(command_id_t cmd_x);
   
    static const command_t cmd[NUM_CMDS+1];
    static const flag_t help_flags[];
    static const flag_t debug_flags[];
    static const flag_t print_flags[];
    static const flag_t led_flags[];
};

#define puts_many_(sh, ...) \
    do { \
        const char* const _ro_list_[] = { __VA_ARGS__, nullptr }; \
        puts_many_ro_i(sh, _ro_list_); \
    } while(0)