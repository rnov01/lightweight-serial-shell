# Lightweight Embedded Serial Shell (C API)

## Overview

This project implements a small, hardware-agnostic command-line shell for microcontrollers.  

The shell operates over any byte-oriented interface (typically USART) and provides a minimal terminal for interacting with a microcontroller without requiring a full RTOS or filesystem.

Design Goals:

- **Low resource usage** - (`<10kB flash and ~440 bytes RAM on ATmega2560 reference build`)
- **Portable C API** - Pure C core, hardware independent; plug in any serial backend
- **Configurable commands and flags** - Easily extendable command/flag system
- **Non-blocking behavior** - simple integration into existing main loops or state machines

The shell core is hardware-neutral. You provide:

- A `putchar`-style callback for output
- A buffer for incoming lines
- Command / flag tables and handlers defining your shell commands

This lightweight build includes 5 built-in commands:

- `help` - provides info about commands and flags
- `dump` - prints command/flag lookup tables
- `ver` - show firmware version and build info
- `mem` - display shell memory settings
- `peek` - read a byte (or 2, or 4 with -16 and -32 flags) from a specified memory address

These commands can be easily batch removed or individually toggled at compile-time to save space.

---

## Quick Start

### Configure the Shell

1. In `shell_config.h`, set basic limits and enable built-in commands as needed:

```c
#define IN_BUFFER_SIZE     64   // max input line length
#define MAX_ARGS          8     // max number of args (including command and flags)
#define MAX_FLAGS        4      // max number of flags per command

#define SYS_CORE              // enable core system commands (optional)
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

#define INBUF_SIZE IN_BUFFER_SIZE
#define MAX_ARG MAX_ARGS

static char in_buffer[INBUF_SIZE];
static char* argv[MAX_ARG];
static shell_t sh;

static void my_putc(char c, void* user) {
    (void)user;
    uart_write_blocking(c);  // your UART driver here
}

int main(void) {
    sh_init(&sh,    
            user_cmds, //or NULL if no user commands
            in_buffer, 
            INBUF_SIZE, 
            argv, 
            MAX_ARG,
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



## Key Features

- **Configurable commands**  
  Commands can be added by inserting an entry in the command lookup table and creating a matching `cmd_*` handler function.

- **Flag-based subcommands**  
  Flags are defined per command in their own lookup tables and implemented with `f_*` handler functions.

- **Hardware flexibility**  
  The shell itself is hardware agnostic; only the serial driver layer is MCU-specific.

---

## Architecture

### 1. Input Buffer and Tokenizer

Characters arriving from the serial backend accumulate in a global ring buffer.  
When the system is available, `shell::poll()` transfers characters from the ring buffer into the shell’s input buffer (`sh_line[]`). Once a command terminator is received, the tokenizer splits it into:

- base command  
- flags (optional)  
- arguments (optional)

Tokens are stored in `argv[]`.  
Tokens remain stored in-place in the input buffer, with the tokenizer inserting `'\0'` to delimit substrings.

---

### 2. Command / Flag Lookup Tables

Commands are parsed from `argv[0]` and compared against the static command lookup table.  
Each entry contains:

- command name  
- help description  
- pointer to the command handler  
- command ID  
- pointer to an associated flag table (optional)

Only a command’s `cmd_*` handler invokes its `f_*` flag handlers.  
Arguments appear after flags in `argv[]` and may be interpreted as needed.

Lookup is linear (O(N)) for minimal overhead.

---

### 3. Command / Flag Handlers

Each command implements a `cmd_*` handler.  
Optional flags are implemented with `f_*` handler functions.

A command handler:

- validates arguments  
- dispatches flags if present  
- performs the command’s primary behavior  

Any number of flags may be defined per command; however, **only one flag may be used at a time**.

Flag handlers execute only the behavior associated with the flag. They are invoked exclusively by their command’s handler.

---

### Buffering Model

The shell uses two buffering layers:

1. **Global ring buffer** – fed by the serial backend (typically the RX interrupt)  
2. **Input buffer (`sh_line[]`)** – parsed and tokenized by the shell  

`argv[0]` always contains the base command.  
`argv[1..n]` contain flags and arguments.

---

## Typical Usage

Examples of supported command patterns:

```
help
help -long
help -c led

print "hello world"

debug -on
```

**Only one flag may be passed at a time.**

### Adding a New Command

1. Write a `cmd_*` handler  
2. Add a command entry in `shell::cmd[]`  
3. (Optional) Create a flag table and `f_*` handlers  

---

## Integration

A minimal integration example:

```cpp
USART serial_0;
shell sh(&serial_0);

while (1) {
    sh.poll();  // process input and execute commands
}
```

The shell is transport-agnostic.  
Any backend capable of providing characters to the shell will work.

### Typical Backend Model

In the included AVR example:

- The USART RX ISR stores incoming bytes in the global ring buffer  
- `shell::poll()` reads from the ring buffer and assembles the command in `sh_line[]`  
- When a newline is detected, the command is tokenized and executed

To use the shell without a ring buffer, `poll()` must be modified to read from another backend.

### Backend Requirements

- A function (ISR or polling function) that pushes incoming characters into the shell  
- A serial driver providing at minimum:

  ```cpp
  put_string(char*)
  ```

  and optionally:

  ```cpp
  receive()
  ```

- A main loop that repeatedly calls `poll()`

---

## Code Examples

### Command ID Enum Entry

```cpp
typedef enum {
    HELP, LED,
    NONE, err, NUM_CMDS
} command_id_t;
```

Insert new command IDs before `NONE`.  
The last three values should always be `NONE`, `err`, and `NUM_CMDS`.

---

### Command Table Entry

```cpp
const command_t shell::cmd[] = {
    {"help", "Show list of supported commands", &shell::cmd_help, HELP, help_flags},
    {"led",  "Toggle onboard LED", &shell::cmd_led, LED, nullptr},
    {nullptr, nullptr, nullptr, err, nullptr}
};
```

This table must be defined **exactly once** in `lut.cpp`.

---

### Flag Table Entry

```cpp
const flag_t help_flags[] = {
    { "-long", "Show command descriptions", &shell::f_help_long },
    { "-c",    "Show flags for a command", &shell::f_help_c },
    { nullptr, nullptr, nullptr }
};
```

Flag tables should also be defined in `lut.cpp`.

---

### Command Handler Example

```cpp
void shell::cmd_help(int argc, char **argv) {
    if (argc == 1) {
        puts_const("Available commands:\r\n");
        for (int i = 0; cmd[i].name != nullptr; i++) {
            puts_many(cmd[i].name, "\r\n", nullptr);
        }
    } else {
        for (int i = 1; i < argc; i++) {
            for (int j = 0; help_flags[j].name != nullptr; j++) {
                if (strcmp(argv[i], help_flags[j].name) == 0) {
                    (this->*(help_flags[j].exe))(argc, argv);
                    return;
                }
            }
        }
    }
}
```

---

### Flag Handler Example

```cpp
void shell::f_help_long(int argc, char **argv) {
    puts_const("Full command descriptions:\r\n");
    for (int i = 0; cmd[i].name != nullptr; i++) {
        puts_many(cmd[i].name, " - ", cmd[i].description, "\r\n", nullptr);
    }
    puts_const("\r\nUse -c <command> to list flags for a specific command\r\n");
}
```



