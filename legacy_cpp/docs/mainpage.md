# Lightweight Embedded Serial Shell

---
\section intro Overview

This project implements a lightweight command-line shell for microcontrollers. The shell operates over a serial interface (typically USART) and provides a minimal terminal for interacting with a microcontroller without the need for a full RTOS or filesystem.

This design focuses on:
- low resource usage
- non-blocking for simple integration

The shell supports user-defined commands and flags/subcommands, allowing firmware developers to expose debugging tools, configuration interfaces and system features through a familiar terminal-style syntax. 

---
\section features Key Features

**Configurable commands** -  Commands can be easily added by adding an entry in the command lookup table and creating a corresponding cmd_* handler function.

**Flag-based subcommands** -  Flags are defined in per-command lookup tables and implemented with f_* handler functions.

**Hardware flexibility** -  The shell itself is hardware agnostic; only the serial driver layer is MCU-specific.

---
\section design Architecture

The shell consists of three main components:

### 1. Input Buffer and Tokenizer
 When the system is available, the shell's input buffer is populated with characters from the global buffer. Once a command is detected, the tokenizer breaks it into 3 parts:
    
- base command
- flags (optional)
- arguments (optional)

Token pointers are stored in an argument buffer 'argv[]'. This buffer points to the strings that still "live" in the input buffer. The tokenizer modifies the input buffer in place using null terminators ('\0') to separate tokens, and argv[] pointers reference string positions within thie same buffer. 

### 2. Command / Flag Lookup Tables
Commands are parsed from argv[0] and matched against the static command lookup table, where each entry contains:
- command name
- help description
- pointer to the command handler 
- command id
- pointer to the associated flag table (if any)

This approach provides O(N) lookup for minimal code overhead.

Only the cmd_* handler invokes f_* flag handlers. For example, cmd_help() checks argv[] for flags, looks up the matching entry in help_flags[] and dispatches the corresponding f_help_* handler.

Arguments (non-flag parameters) are stored after flags in argv[], and command handlers may interpret them as needed.
### 3. Command / Flag Handlers
Each command implements a corresponding cmd_* handler function.
Optional flags are implemented as f_* handler functions.

The command handler validates arguments, dispatches flags if present, and executes the command's primary behavior.

The shell supports an arbitrary amount of flags per command. However, only one flag can be passed at a time. 

The flag handler is only responsible for executing the behavior of the corresponding flag. A f_* flag handler function should **only** be called by its command handler

#### Note
The shell uses two buffers:
1. A global ring buffer updated by the serial backend
2. An input buffer (sh_line[]) where complete commands are parsed and tokenized

- argv[0] always contains the primary (base) command.
- argv[1] ... argv[n] contain flags and/or positional arguments. 

---
\section usage Typical Usage

Example command patterns supported by the shell:

\code
help
help -long
help -c led

print "hello world"

debug -on
\endcode

\warning Only one flag may be passed at a time. Additional flags will be ignored.

Adding a new command:
 1. Write a cmd_* handler to perform a task associated with the command
 2. Add a matching lookup table entry to shell::cmd[]
 3. Define flag tables and f_* subcommand handlers (optional)

 ---
 \section integration Integration

 To use the shell, instantiate it with a serial interface:

 \code
 USART serial_0;
 shell sh(&serial_0);

 while (1) {
    sh.poll();  //process input and execute commands
 }
\endcode

The shell is transport-agnostic. Characters must be supplied by a backend interface. The example included with this project uses an interrupt-driven USART interface. In the ISR, received characters are stored in the global ring buffer while the system is busy. Then, shell::poll() fetches characters from the ring buffer into the shell's input buffer. This set-up is not required, any backend that pushes characters into the shell's input pipeline will work. 

A description of the ring buffer operation is provided below:

Characters received from the serial interface accumulate in the global ring buffer. The global ring buffer is updated in the isr for an rx interrupt. The buffer is designed to wrap and reject new characters if full - it acts as a fifo for the shell input buffer and provides a temporary storage preventing data loss when the system is busy with other tasks. shell::poll() checks the ring buffer and grabs characters which then accumulate in the shell's input buffer until a command is detected.

To use shell::poll() without the ring buffer, it will need to be modified to accept characters from another location.

#### Backend Requirements

- A function is needed to push incoming bytes into the shell's input buffer. This is performed by the USART Rx interrupt in the provided example. 
- A serial interface driver that implements at minimum \code put_string(char*) \endcode and, optionally, a blocking \code receive() \endcode function or ISR
- A main loop calling shell::poll() regularly

\section ex Code Examples

Code examples are provided to illustrate the necessary steps to implement unique commands and flags.

### Command ID Enum Entry

\code
typedef enum {
    HELP, LED, 
    
    NONE, err, NUM_CMDS
}command_id_t;
\endcode

- Create a one-word command ID in the command_id_t enum. The index in the ID enum must match the lookup table index in the following step. 
\note Insert command ID into the list before the NONE ID. The last three IDs in the list should always be NONE, err, NUM_CMDS

### Command Table Entry

\code
const command_t shell::cmd[] = {
    {"help", "Show list of supported commands", &shell::cmd_help, HELP, help_flags },
    {"led", "Toggle onboard LED", &shell::cmd_led, LED, nullptr },
    {nullptr, nullptr, nullptr, err, nullptr }
};
\endcode

- Create another entry in the command lookup table. The entry index must correspond the the ID enum index. 
- For example if above, a PWM command was added after LED, the PWM entry in the command table should also go after the led entry. 
- Provide the name (this is the exact text matched from the terminal - i.e. expected input), a short description, address of cmd_* handler, enum ID, and pointer to flag lookup table (described in next step) or nullptr if no flags
\note In this example, the LED command has no flags so the flag table pointer is nullptr. 
User entries should be inserted before the last nullptr entry and the last entry should not be modified.

\warning This table should be defined exactly once in \ref lut.cpp 

### Flag Table Entry

\code 
const flag_t help_flags[] = {
    { "-long", "Show command descriptions", &shell::f_help_long },
    { "-c",    "Show flags for a command", &shell::f_help_c },
    { nullptr, nullptr, nullptr }
};
\endcode

- Create a flag_t lookup table with members for each flag you want to associate with your command. 
\note User entries should be inserted before the last nullptr entry and the last entry should not be modified. Flag tables should be defined in \ref lut.cpp, not in a header, to avoid multiple definition errors.

### Command Handler Function

\code 
void shell::cmd_help(int argc, char **argv) {
    if (argc == 1) {
        puts_const("Available commands:\r\n");
        for (int i=0; cmd[i].name != nullptr; i++) {
                puts_many(cmd[i].name, "\r\n", nullptr);
        }
    } else if (argc > 1) {
        for (int i=1; i < argc; i++) {
            for (int j=0; help_flags[j].name != nullptr; j++) {
                if (strcmp(argv[i], help_flags[j].name) == 0) {
                    (this->*(help_flags[j].exe))(argc, argv);
                    return;
                }
            }
        }
    }
}
\endcode

- Create cmd_* handler function to execute your command's behavior. The base command is executed in the if argc == 1 block. Calls to any flag handlers are handled in the if argc > 1 block. This can be omitted if no flags are used. 

### Flag Handler Function (optional)

\code
void shell::f_help_long(int argc, char **argv) {
    puts_const("Full command descriptions:\r\n");
    for(int i=0; cmd[i].name != nullptr; i++) {
        puts_many(cmd[i].name, " - ", cmd[i].description, "\r\n", nullptr); 
    }
    puts_const("\r\nUse -c <command> to list flags for a specific command\r\n");
}
\endcode 

- Create f_* handler function to execute flag-specific behavior. 

\section compinfo Compilation Info

The test code for this software was compiled on an ATMEGA2560 using:
- **avr-gcc**
- **avrdude**
- **GNU make**

The following memory usage statistics are generated automatically using `avr-size` during the build process:

\include ./memusage.txt

Memory usage output shown here is AVR-specific. Other architectures will produce different output depending on their toolchains.



