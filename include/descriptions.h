#include "ro.h"

const char help_n[]  = "help";
const char help_d[] RODATA = "Provides a list of system commands";
    const char help_lf_d[] RODATA = "Gives a short description of each available command";
    const char help_cf_d[] RODATA = "Use -c <command> for a list of associated flags";
    
const char debug_n[]  = "debug";    
const char debug_d[] RODATA = "Activates system debug mode";
    const char debug_vf_d[] RODATA = "Verbose mode";

const char print_n[]  = "print";
const char print_d[] RODATA = "Echoes user argument";

const char led_n[] = "led";
const char led_d[] RODATA = "Toggles the onboard LED";
    const char led_bf_d[] RODATA = "Blink led for 10 seconds";