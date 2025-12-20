# Lightweight Embedded Serial Shell 

## Overview

This project implements a small, hardware-agnostic command-line shell for microcontrollers.  

The shell operates over any byte-oriented interface (typically USART) and provides a minimal terminal for interacting with a microcontroller without requiring a full RTOS or filesystem.

Highlights:

- **Low resource usage** 
<8kB flash and ~200B RAM on ATmega2560 with all core features enabled
No dynamic memory allocation
- **Portable pure-C core** 
No hardware assumptions; easy to configure with different serial backends
- **Configurable commands and flags** 
Easily extend shell capabilities with user-defined commands and flags
- **Non-blocking behavior** 
Can be used in interrupt context or polled periodically from state machines, main loops, etc.

The shell itself is hardware-neutral. You provide:

- A `putchar`-style function tied to your serial output
- An input buffer for incoming commands
- Command / flag tables and handlers defining your shell commands
- (Optional) platform-specific register access callbacks for system commands

Built in System Commands:
|||                         
|----------------|-----------------------|
|`help` | lists available commands and descriptions|
|`dump` | prints detailed command/flag information (execution mode, parameter mode, etc.)|
|`ver` | firmware version and build info|
|`mem` | shell memory settings|
|`peek` | read a byte from a register (supports larger widths with -16 and -32 flags) |
|`setbit` | set a  bit in a register (requires backend implementation)|
|`rstbit` | clear a bit in a register (requires backend implementation)|

These commands can be easily batch removed or individually toggled at compile-time to save space. `shell_config.h` is used to set build options. 

---

## Quick Start

### Configure the Shell

1. In `shell_config.h`, set basic limits and enable built-in commands as needed:

```c
#define LINE_BUFFER_SIZE    32      // max input line length (size for largest expected command + args + flags + terminator)
#define MAX_ARGS            8       // max number of args per command(including command and flags)
#define MAX_FLAGS           4       // max number of flags per command

#define SYS_CORE                    // enable built-in system commands (help, dump, ver, mem)
#define SYS_DEBUG                   // enable register access commands (peek, setbit, rstbit)    
```

### User Commands
2. Define user commands (optional):

If you have no user commands, you can pass `NULL` and `0` to `sh_init()` for the `user_cmds` and `cmd_count` parameters.
Otherwise, define them in a table:

```c
#include "shell.h"

static void cmd_led(shell_t* sh, const shell_cmd_t* self, const char* param, int argc, char** argv);

static const shell_cmd_t user_cmds[] = {
    { "led", "Toggle onboard LED", cmd_led, NULL, .sub = { CMD_NO_FLAG, NOPARAM, false, 0} }
};
```

Three execution modes are supported for commands:
- `CMD_NO_FLAG` - command does not accept flags and an error is returned if any are provided
- Two different handling methods for commands with flags:
    - `CMD_THEN_FLAG` - base command is executed first, the the flag handler is dispatched
    - `FLAG_ONLY` - the base command is not executed; only the flag handlers are dispatched

Four parameter modes are supported for commands and three for flags:
- `NOPARAM` - no parameter is expected; an error is returned if one is provided
- `REQ` - a parameter is required; an error is returned if none is provided
- `OPT` - parameter is optional (no error checking)
- `MULTI` - pass through that bypasses parameter validation; the handler is responsible for parsing parameters from `argv[]` **note** this     mode is not supported for flags
### Configure Input

3. Allocate an input buffer and initialize the shell:

```c
#include "shell.h"
#include "shell_sys.h" 
#include "shell_config.h"

static char in_buffer[LINE_BUFFER_SIZE];
static char* argv[MAX_ARGS];
static shell_t sh;

static void my_putc(char c, void* user) {
    (void)user;
    uart_write(c);          //your serial write function here
}

/* Implement this based on your platform and desired serial interface */
UART_RX_ISR() {
    uint8_t byte = uart_read();
    sh_poll(&sh, byte);
}

int main(void) {
    sh_init(&sh,    
            user_cmds,      //or NULL if no user commands
            in_buffer, 
            LINE_BUFFER_SIZE, 
            argv, 
            MAX_ARGS,
            my_putc, 
            NULL,           //io_user = NULL for simple backends
            sizeof(user_cmds)/sizeof(user_cmds[0]));

    while (1) {
        
    }
}
```

### Implement User Commmand Handler logic

4. Example command handler:

```c
static void cmd_led(shell_t* sh, const shell_cmd_t* self, const char* param, int argc, char** argv) {
    // Toggle LED logic here
    sh_puts(sh, "LED toggled!\r\n");
}
```

### Build

5. Build and Flash:

- Makefile is provided for AVR-GCC and ARM GCC; modify as needed for your platform and target.

```sh
#ATmega2560
make avr PLATFORM=avr BOARD=mega2560
make flash PLATFORM=avr BOARD=mega2560
#or
make avr flash PLATFORM=avr BOARD=mega2560

#ATmega328P
make avr flash PLATFORM=avr BOARD=uno

#STM32 F411RE
make stm32 flash PLATFORM=stm32 BOARD=f411re
```



At this point, you should be able to connect a serial terminal and type:
```
help
ver
peek 0xC0 
led
```


## Memory Breakdown

*This is based on an ATmega2560 reference build with 5 system commands enabled and feeding `sh_poll()` directly in ISR (no buffering).*
*The numbers below are approximate and will vary based on compiler and optimization settings. These are from avr-gcc with `-Os`.*

|Command  | Flash (bytes) | RAM (bytes) |
|---------|----------------|--------------|
|help    |  1224          |   14          |
|dump    |  1826          |   14          |
|ver     |   422          |   4           |
|mem     |   574          |   4           |
|peek    |   1150         |   22          |

**Shell Core + System Command Total:** 7768 bytes flash, 175 bytes RAM. This includes a minimal USART backend for testing.

- Just the shell core (no commands) uses ~2.9kB flash and ~90 bytes RAM.
- Adding the ring buffer holds incoming data until the shell can process it; adds ~40B flash and `RING_BUFFER_SIZE` bytes RAM.

---

## How it Works

The shell is built around three main components:

### 1. Line Buffer and Tokenizer

- Incoming bytes are fed into `sh_poll()`.
- when a newline ( `'\r'` or `'\n'` ) is detected, the shell:
    - null-terminates the input line  
    - tokenizes it into `argv[]` based on whitespace  
    - calls `sh_execute()` with `argc` and `argv`

`argv[0]` always holds the base command. `argv[1..n]` hold flags and arguments.

### 2. Command / Flag Lookup Tables

Commands live in static `shell_cmd_t` tables:

```c
static const shell_cmd_t user_cmds[] = {
    { "led", "toggle onboard LED", cmd_led, NULL, .sub = { CMD_NO_FLAG, NOPARAM, false, 0} }
};
```

System commands `help`, `dump`, `ver`, `mem`, and `peek` are defined in `sys_commands.c` and linked in when enabled via `shell_config.h`.

Each command may have an associated block of flags starting at `flag_base`, with `sub.flag_count` indicating the number of flags that belong to that command. All flags may be defined in a single table, or split across multiple tables per command. The only requirement is that, for any given command, its flags occupy a contiguous block in memory, and `flag_base` points to the first flag in that block.

Flags are defined with `flag_t` entries:

```c
static const flag_t led_flags[] = {
    { "-on", "turn LED on", "led", f_led_on, .sub = {true, OPT, 0} },
    { "-off", "turn LED off", "led", f_led_off, .sub = {true, OPT, 0} }
};
```

### 3. Handlers

Command handlers have the signature:

```c
void cmd_x(shell_t* sh, const shell_cmd_t* self, const char* param, uint8_t argc, char** argv);
```

Flag handlers have the signature:

```c
void f_x(shell_t* sh, const shell_cmd_t* parent, const char* param, uint8_t argc, char** argv);
```

`sh_execute()`:
- Looks up `argv[0]` in system + user command tables
- Validates flags/parameters according to `exe_mode_t` and `param_mode` settings
- Dispatches to the appropriate handler(s)

## Integration

### Feeding Input to the Shell

There are two typical methods to feed characters into `sh_poll()`:

1. Call `sh_poll()` directly from your serial RX interrupt handler (ISR). This is the simplest method - it minimizes RAM usage and avoids an additional buffer. 

```c
ISR(USART_RX_vect) {
    uint8_t c = UDR0;
    sh_poll(&sh, c);    /*handlers should be short/non-blocking; consider ring-buffer model 
                          if you want to run long commands outside of interrupt context */
}
```
- **Pros**: Smallest footprint, simple
- **Cons**: Command parsing and handlers will run in interrupt context; handlers should be fast and non-blocking

2. Use a ring buffer

For more complex systems, you can push bytes into a ring buffer in the ISR, and poll that buffer in your main loop or state machine with `sh_poll()`.

```c

ISR(USART_RX_vect) {
    uint8_t c = UDR0;
    buf_push(&rx_buf, c);
}

int main(void) {
    while (1) {
        uint8_t c;
        if (buf_pop(&rx_buf, &c)) {
            sh_poll(&sh, c);
        }
        //rest of loop...
    }
}
```
- **Pros**: ISR stays small, you decide when the shell processes input
- **Cons**: Extra RAM for buffer

Both input models will work identically from the shell's perspective. Choose the one that best fits your application's architecture and timing requirements.

### Backend Requirements

The shell core requires a `putchar`-style callback for output. This function should take a character and a user pointer (which can be `NULL` if not needed).

Additional `put_string` and `put_stringP` (`PROGMEM` version for AVR) can be used by the shell if provided, but they are not required. 

Wrapping an existing serial driver is straightforward. Just implement the `putchar` function to call your driver's transmit function:

```c
static void usart0_putc(char c, void* user) {
    (void)user;         //unused
    usart_write(c);     //your USART driver here
}
```

If you have object-based USART functions, like the AVR driver included in this repo, you can pass a pointer to the USART object as the `io_user` parameter in `sh_init()`, and cast it back in your `putchar` function.

```c
usart_t USART0 = {
        .put_c = usart_putc,
        .UDRn = &UDR0,
        .UCSRnA = &UCSR0A,
        .UCSRnB = &UCSR0B,
        .UCSRnC = &UCSR0C,
        .UBRRn = &UBRR0, 
        .sub = {
            .rxcie = true,
            .txcie = false,
            .rxen = true,
            .txen = true
        }
    };

    usart_init(&USART0, 19200, 8);

    void putc_wrap(char c, void* user) {
        usart_t* u = (usart_t*)user;
        u->put_c(u, c);
    }

    static char line_buf[32];
    static char* argv[8];
  
    sh_init(&sh, NULL, line_buf, 32, argv, 8, putc_wrap, &USART0, 0);

```

### Porting Requirements for Register Access (optional feature)

The shell includes optional system commands that allow reading and writing hardware registers. Platforms may implement all, none of some of these features.

The system read command, `peek` is enabled by default with numeric (hex) address access.
To enable register write system commands (`setbit` and `rstbit`) on your platform, you must provide a small set of macros and backend functions in your platform header.

If no register access features are provided:
-  `peek` will only accept numeric addresses and will return an error for all addresses unless safe ranges are defined
- Write commands (`setbit`, `rstbit`) will always return an error unless explicitly disabled in `shell_config.h`. 

#### 1. Register lookup (optional - enables name based access)

The shell supports two ways to interpret register arguments:
- Numeric addresses (always supported)
```c
peek 0xC0
>> 0x00        
```
- Symbolic register names (requires register map and lookup functions)
```c
peek PORTB 
>> 0x80
setbit DDRC 3
>> ok...     
```
To enable symbolic register access, you must provide two lookup functions and define two macros in your platform header:


```c
#define SH_FIND_REG(name)   my_find_reg(name)   
#define SH_GET_ADDR(reg)    my_get_addr(reg)
```

Signatures:
```c
int8_t my_find_reg(const char* name); 
    //returns 
        //platform-defined register ID
        //-1 if not found
volatile uint8_t* my_get_addr(int8_t reg); 
    //returns 
        //pointer to the register address (used for peek/setbit/rstbit)
```

- The return value from `my_find_reg()` does not need to be and index into any particular structure; it is an opaque ID that `my_get_addr()` needs to understand. You may use an enum, a simple integer index, or any other scheme that fits your platform.
- A register table is not required; named access only requires a way to find a register by name and get its address.
    - `my_find_reg()` must recognize reg name string and return and ID
    - `my_get_addr()` must return a valid address pointer for that ID

If these macros are not defined, `peek <REGNAME>` is disabled and only numeric addresses are accepted. `setbit` and `rstbit` commands may still work depending on your backend implementation.

Your implementation is responsible for ensuring returned pointers fall within the valid address space for your platform.

**Note:** Defining a named register lookup enables further restriction of write commands (beyond the safe address ranges described below). Write commands used with named registers will only succeed if the name exists in the register map and the specified bit is valid for that register (based on a mask you define in your register map). Numeric addresses are *only* validated against safe address ranges.

By limiting the defined registers in the register map, you can prevent accidental writes to sensitive registers.


#### 2. Safe address ranges (optional, but recommended)

The shell prevents invalid memory accesses by restricting `peek`, `setbit` and `rstbit` to a set of allowed address ranges. You must define these ranges in your platform header as an array of `safe_addr_t` structs:

```c
#define SH_ALLOWED_ADDRESS_RANGES \
    (safe_addr_t[]){{0x20u, 0xD6u}, {0x100u, 0x1FFu}}
```

- You may define one or more allowed ranges.
- These restrictions are imposed on ***both*** numeric and named register accesses for read and write commands.
- If you do not define `SH_ALLOWED_ADDRESS_RANGES`, register access commands will reject all addresses unless the feature is disabled in `shell_config.h`.



#### 3. Bit write support (optional)

Provide functions to set and clear bits and define:
```c
#define SH_SETBIT(reg, bit)    my_setbit(reg, bit)
#define SH_RSTBIT(reg, bit)    my_rstbit(reg, bit)
```
The backend implementations expect the following signatures:
```c
bool my_setbit(const char* regname, uint8_t bit);
bool my_rstbit(const char* regname, uint8_t bit);
```

- If these macros are not defined, write commands (`setbit`, `rstbit`) will return errors unless explicitly disabled in `shell_config.h`.
- The backend should perform any necessary validation (e.g. bit number in range, register writable, etc) and return `true` on success, `false` on failure.

#### Notes

The shell never includes platform headers directly. Instead, it relies on the build system to define `SH_PLATFORM_HEADER` and `SH_REGMAP_H` macros to point to the appropriate files for your platform. Your makefile or build system must ensure the correct platform header is included during compilation.
Example:
```make
CFLAGS += -include platforms/atmega2560/platform.h 
```

--- 