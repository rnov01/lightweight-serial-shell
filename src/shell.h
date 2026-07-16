#pragma once
#include <serial_shell/config.h>
#include <serial_shell/shell_types.h>
#include <serial_shell/shell_commands.h>

#include <stdbool.h>
#include <stdint.h>

/**
 * @file shell.h
 * @brief Lightweight, portable serial shell core
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

extern const shell_cmd_t sys_cmds[];
extern const uint8_t sys_cmd_count;

/* Parse indices
 * sh_parse_idx keeps track of the location of command parameters and 
 * flags in the input buffer. It stores which indices in the buffer correspond
 * to flags. This structure is internal to the parser and not normally needed
 * by user code */
typedef struct {
    int8_t param_idx;
    uint8_t flag_pos[MAX_FLAGS];
    uint8_t flag_count;
} sh_parse_idx_t;

#if defined(SH_COLOR_ON)
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

/* Error helpers */
const char*         get_error_string(err_t e);

/* Flag related helpers */
bool                sh_has_flag(uint8_t argc, char **argv, const char* f);    //check if user input has flag argument

int tokenizer(char* buf, char** argv, uint8_t max_args);
void sh_execute(shell_t* sh, uint8_t argc, char** argv);
err_t sh_parse_args(const shell_cmd_t* cmd, uint8_t argc, char**argv, sh_parse_idx_t* out,
    const char** err_arg);
const shell_cmd_t* sh_find_cmd(shell_t* sh, const char *name);

/* Optional debug helpers used for register-reading (peek) */
bool                is_valid_addr(uintptr_t addr);
void                print_uint(shell_t* sh, uint16_t n); 
bool                parse_uint(const char* s, uint32_t* out);   //parse hex
void                print_hex_digit(shell_t* sh, uint8_t d);
void                print_hex8(shell_t* sh, uint8_t n);

static inline void sh_read_cmd(const shell_cmd_t* src, shell_cmd_t* dst) {
    #if (SH_AVR_FLASH_TABLES)
        memcpy_P(dst, src, sizeof(shell_cmd_t));
    #else
        *dst = *src;
    
    #endif
}

static inline void sh_read_flag(const shell_flag_t* src, shell_flag_t* dst) {
    #if (SH_AVR_FLASH_TABLES)
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



