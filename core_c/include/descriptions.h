#pragma once
#include "shell.h"


#if HELP_CMD
extern const char help_n[]  = "help";
extern const char help_d[] RODATA = "Provides a list of system commands";
    extern const char help_lf_d[] RODATA = "Gives a short description of each available command";
    extern const char help_cf_d[] RODATA = "Use -c <command> for a list of associated flags";
#endif

#if DUMP_CMD
const char dump_n[] = "dump";
const char dump_d[] RODATA = "Provides a list of system table data";
    const char dump_cf_d[] RODATA = "List internal command table";
    const char dump_ff_d[] RODATA = "List internal flag table";
#endif

#if VER_CMD
const char ver_n[] = "ver";
const char ver_d[] RODATA = "display firmware and build info";
#endif

#if MEM_CMD
const char mem_n[] = "mem";
const char mem_d[] RODATA = "display flashed memory settings";
#endif