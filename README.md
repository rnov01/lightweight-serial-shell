# Lightweight Embedded Serial Shell (C API)

## Overview

This project implements a small, hardware-agnostic command-line shell for microcontrollers.  

The shell operates over any byte-oriented interface (typically USART) and provides a minimal terminal for interacting with a microcontroller without requiring a full RTOS or filesystem.

Highlights:

- **Low resource usage** - (`<10kB flash and ~330 bytes RAM on ATmega2560 for all core features and input buffering; can be reduced further`)
- **Portable C API** - Pure C core, hardware independent; plug in any serial backend
- **Configurable commands and flags** - Easily extendable command/flag system
- **Non-blocking behavior** - Simple integration into existing main loops or state machines

The shell core is hardware-neutral. You provide:

- A `putchar`-style callback for output
- A buffer for incoming lines
- Command / flag tables and handlers defining your shell commands

This lightweight build includes 5 built-in commands:

`help` - provides info about commands and flags
`dump` - prints command/flag lookup tables
`ver` - show firmware version and build info
`mem` - display shell memory settings
`peek` - read a byte (or 2, or 4 with -16 and -32 flags) from a specified memory address

These commands can be easily batch removed or individually toggled at compile-time to save space.

---

## Quick Start

### Configure the Shell

1. In `shell_config.h`, set basic limits and enable built-in commands as needed:

```c
#define LINE_BUFFER_SIZE    32      // max input line length
#define MAX_ARGS            8       // max number of args (including command and flags)
#define MAX_FLAGS           4       // max number of flags per command

#define SYS_CORE                    // enable core system commands (optional)
```
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
    uart_write_blocking(c);  // your UART driver here
}

int main(void) {
    sh_init(&sh,    
            user_cmds, //or NULL if no user commands
            in_buffer, 
            LINE_BUFFER_SIZE, 
            argv, 
            MAX_ARGS,
            my_putc, 
            NULL,       //io_user = NULL for simple backends
            sizeof(user_cmds)/sizeof(user_cmds[0]));

    while (1) {
        int c = uart_read_nonblocking(); // returns -1 if no data
        if (c >= 0) {
            sh_poll(&sh, (uint8_t)c);  
        }
        //rest of main loop...
    }
}
```

4. Example command handler:

```c
static void cmd_led(shell_t* sh, const shell_cmd_t* self, const char* param, int argc, char** argv) {
    // Toggle LED logic here
    sh_puts(sh, "LED toggled!\r\n");
}
```
5. Build and Flash:

- Makefile is provided for AVR-GCC and ARM GCC; modify as needed for your platform and target.

```sh
make avrflash
make stm32flash
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

|Command  | Flash (bytes) | RAM (bytes) |
|---------|----------------|--------------|
|help    |  1358          |   30          |
|dump    |  1402          |   26          |
|ver     |   418          |   30          |
|mem     |   346          |   14          |
|peek    |   846          |   50          |

**Shell Core Total:** 8180 bytes flash, 291 bytes RAM

- Just the shell core (no commands) uses ~5kB flash and ~230 bytes RAM.
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

Each command may have an associated block of flags starting at `flag_base`, with `sub.flag_count` indicating the number of flags that beong to that command. All flags may be defined in a single table, or split across multiple tables per command. The only requirement is that commands with multiple flags have their flags defined in contiguous entries.

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
void cmd_x(shell_t* sh, const shell_cmd_t* self, const char* param, int argc, char** argv);
```

Flag handlers have the signature:

```c
void f_x(shell_t* sh, const shell_cmd_t* parent, const char* param, int argc, char** argv);
```

`sh_execute()`:
- Looks up `argv[0]` in system + user command tables
- Validates flags/paramaters accordign to `exe_mode_t` and `param_mode` settings
- Dispatches to the appropriate handler(s)

## Integration

### Feeding Input to the Shell

There are two typical methods to feed characters into `sh_poll()`:

1. Call `sh_poll()` directly from your serial RX interrupt handler (ISR). This is the simplest method - it minimizes RAM usage and avoids an additional buffer. 

```c
ISR(USART_RX_vect) {
    uint8_t c = UDR0;
    sh_poll(&sh, c);
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

--- 