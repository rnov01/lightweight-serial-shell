#include "../include/user_commands.h"
#include "../include/shell.h"
#include <string.h>
#include <util/delay.h>

void cmd_light(const command_rt_t* self, int argc, char** argv) {
    if (argc == 1) {
        toggle(PIN_B7);
    } else if (argc > 1) {
        for (int i = 0; i<argc; i++) {
            if (strcmp(argv[i], "-b")==0) {
                for (int k=0; k<10; k++) {
                    toggle(PIN_B7);
                    _delay_ms(300);
                }
            }
        }
    }
}

void SH_USR_CMD(test_prt1) {
    
}