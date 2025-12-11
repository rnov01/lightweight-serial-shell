#include "descriptions.h"

#if HELP_CMD
const char help_n[]  = "help";
const char help_d[] RODATA = "Provides a list of system commands";
   const char help_cf_d[] RODATA = "Use -c <command> for a list of associated flags";
#endif

#if DUMP_CMD
const char dump_n[] = "dump";
const char dump_d[] RODATA = "provides a list of system table data";
    const char dump_vf_d[] RODATA = "verbose option with additional info";
#endif

#if VER_CMD
const char ver_n[] = "ver";
const char ver_d[] RODATA = "display firmware and build info";
#endif

#if MEM_CMD
const char mem_n[] = "mem";
const char mem_d[] RODATA = "display flashed memory settings";
#endif

#if PEEK_CMD
const char peek_n[] = "peek";
const char peek_d[] RODATA = "read peripheral register address. use with caution.";
    const char peek16_d[] RODATA = "read 2 bytes up from base addr";
    const char peek32_d[] RODATA = "read 4 bytes up from base addr";
#endif