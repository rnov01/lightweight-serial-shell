# Lightweight Embedded Serial Shell

## Overview

This project implements a lightweight command-line shell for microcontrollers.  
The shell operates over a serial interface (typically USART) and provides a minimal terminal for interacting with a microcontroller without requiring a full RTOS or filesystem.

This design focuses on:

- low resource usage  
- non-blocking behavior for simple integration  

The shell supports user-defined commands and flags/subcommands, allowing firmware developers to expose debugging tools, configuration interfaces, and system features through a familiar terminal-style syntax.

---

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



