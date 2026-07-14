#pragma once
#include <serial_shell/config.h>
#include <serial_shell/shell_types.h>
#include <serial_shell/shell_commands.h>

#include <stdbool.h>
#include <stdint.h>

/**
 * @file shell.h
 * @brief Lightweight, portable serial shell core
 * 
 *  This header defines the C API for the serial shell. The shell parses 
 *  text commands from a provided character stream, dispatches user-defined handlers,
 *  and optionally supports flags and parameters.
 * 
 *  Hardware-specific details (USART, USB, etc.) are kept out of the core and must be 
 *  provided by the user via callback functions. 
 */

/* ===================================================================
 * Read-only storage helpers
 * 
 *  On AVR devices, constant data like strings and lookup tables can be
 *  stored in flash memory instead of SRAM using PROGMEM. On other architectures, 
 *  these macros collapse to normal access.
 * 
 *  RODATA  -  annotates objects that may live in flash/PROGMEM
 *  ROSTR   -  wrapper for string literals that may live in flash/PROGMEM
 *  READ_RO_PTR / READ_RO_BYTE  -  read a ptr/byte from read-only (flash) storage 
 *  =================================================================== */
#if defined(__AVR__)
    #include <avr/pgmspace.h>
    #define RODATA PROGMEM
    #define ROSTR(x) PSTR(x)
    #define READ_RO_PTR(type, addr) ((type)pgm_read_word(addr))
    #define READ_RO_BYTE(addr) pgm_read_byte(addr)
#else
    #define RODATA
    #define ROSTR(x) (x)
    #define READ_RO_PTR(type, addr) (*(type*)(addr))
    #define READ_RO_BYTE(addr) (*(uint8_t*)(addr))
#endif

/* Small formatting helpers used inside handler functions */
#define LINE sh_puts_ro(sh, ROSTR("\r\n"));
#define FLAGCOLOR GREEN     /* System default flag color */
#define SPACE sh_puts_ro(sh, ROSTR(" "));


/* Table of error descriptions strings (*in flash on AVR)*/
extern const char* const err_msg[];

/* ===================================================================
 * Shell state
 * 
 * shell_t holds the runtime state of the shell and is initialized by the 
 * user with the sh_init() function. sh_init() only takes in 
 * the required putc() callback (the other implementations are optional, putc
 * is all the shell needs for operation). If a user has puts() and/or puts_p() 
 * (PROGMEM AVR puts function) they can be passed to the shell via this struct. 
 * 
 * An input buffer, argv array and putc() callback must be provided when calling sh_init() 
 * =================================================================== */
struct shell {
    /* Output callbacks (user-provided) */
    void (*putc)(void* user, char c);
    void (*puts)(void* user, const char* s);        /* optional; may be NULL - shell will derive its own from putc() */
    void (*puts_p)(void* user, const char* p);      /* optional for read-only (PROGMEM) strings */
    void* io_user;                                  /* optional context pointer for object-based callback functions */

    /* Input buffer */
    char *buffer;
    uint8_t buf_len;
    uint8_t position;

    /* Token storage (argv, points into buffer)*/
    char **argv;
    uint8_t argc;
    uint8_t max_argv;

    /* Command table */
    const shell_cmd_t *cmd;
    uint8_t cmd_count;
};



/* ===================================================================
 * Parse indices
 * 
 * sh_parse_idx keeps track of the location of command parameters and 
 * flags in the input buffer. It stores which indices in the buffer correspond
 * to flags. This structure is internal to the parser and not normally needed
 * by user code 
 * =================================================================== */
typedef struct {
    int8_t param_idx;
    uint8_t flag_pos[MAX_FLAGS];
    uint8_t flag_count;
} sh_parse_idx_t;





#if defined(__COLOR_ON__)
    #define CCYA   ROSTR("\x1B[36m")   //cyan
    #define CYEL    ROSTR("\x1B[33m")      //yellow
    #define CGRN    ROSTR("\x1B[32m")    //green 
    #define CRED    ROSTR("\x1B[31m")  //red
    #define C_      ROSTR("\x1B[0m")    //rst
    #define COLOR_RESET  "\x1B[0m"
#else 
    #define CCYA  ROSTR("")
    #define CYEL ROSTR("")
    #define CGRN  ROSTR("")
    #define CRED   ROSTR("")
    #define C_ ROSTR("")
#endif

/* ===================================================================
 * Public API
* =================================================================== */

/**
 * @brief           Initialize a shell instance. 
 * @param sh        Pointer to user allocated shell state
 * @param user_cmds Array of user commands (may be NULL)
 * @param input_buf Line buffer used for command parsing
 * @param buf_len   Byte size of input buffer
 * @param argv      Array of char* ptrs used for tokens
 * @param max_argv  Max. number of tokens; set by @ref MAX_ARGS in shell_config.h
 * @param putc      Character output callback
 * @param io_user   User ptr passed back to output callbacks (may be NULL, depending on serial interface config)
 * @param cmd_count Number of entries in user_cmds
 * */
void                sh_init(shell_t* sh, const shell_cmd_t* user_cmds, char* input_buf, uint8_t buf_len, 
                        char** argv, uint8_t max_argv, void (*putc)(void* user, char c), void* io_user, 
                        uint8_t cmd_count);
/** 
 * @brief Feed one character into the shell state machine
 * 
 * Call this whenever a new character arrives from the serial backend.
 * When a full line is received and parsed, the corresponding command
 * handler is executed. 
 * 
 * @return Status code 
 */
int                 sh_poll(shell_t* sh, uint8_t c);

/* Basic output helpers */
void                sh_putc(shell_t* sh, char c);
void                sh_puts(shell_t* sh, const char* s);
void                sh_puts_ro(shell_t* sh, const char* p);
void                sh_printc(shell_t* sh, color_t color, const char* s);
void                sh_printc_ro(shell_t* sh, color_t color, const char* s_ro);

/* Error helpers */
const char*         get_error_string(err_t e);
void                error(shell_t* sh, err_t err, const char* detail);
void                error_ro(shell_t* sh, err_t err, const char* detail_ro);

/* Command execution & parsing */


/* Flag related helpers */
bool                sh_has_flag(uint8_t argc, char **argv, const char* f);    //check if user input has flag argument


/* Debug helpers */


/* Optional debug helpers used for register-reading (peek) */
bool                is_valid_addr(uintptr_t addr);
void                print_uint(shell_t* sh, uint16_t n); 
bool                parse_uint(const char* s, uint32_t* out);   //parse hex
void                print_hex_digit(shell_t* sh, uint8_t d);
void                print_hex8(shell_t* sh, uint8_t n);

static inline void sh_read_cmd(const shell_cmd_t* src, shell_cmd_t* dst) {
    #if (__SH_AVR_TINY_TABLES__)
        memcpy_P(dst, src, sizeof(shell_cmd_t));
    #else
        *dst = *src;
    
    #endif
}

static inline void sh_read_flag(const shell_flag_t* src, shell_flag_t* dst) {
    #if (__SH_AVR_TINY_TABLES__)
        memcpy_P(dst, src, sizeof(shell_flag_t));
    #else
        *dst = *src;
    #endif
}

/* Line-drawing characters for flag tree in dump command */
#if defined(NULL)
    #define ls_mid_str ROSTR("\xE2\x94\x9C")
    #define ls_end_str ROSTR("\xE2\x94\x94")
#else 
    #define ls_mid_str ROSTR("|")
    #define ls_end_str ROSTR("\\")
#endif



