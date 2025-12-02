#include "../include/user_commands.h"
#include "../include/shell.h"
#include <string.h>
#include <util/delay.h>

void cmd_light(const command_rt_t* self, int argc, char** argv) {
     toggle(PIN_B7);
}

void f_light_b(const command_rt_t* self, const char* param, int argc, char** argv) {
    for (int k=0; k<10; k++) {
                    toggle(PIN_B7);
                    _delay_ms(300);
                }
}

void SH_USR_CMD(test_prt1) {

}