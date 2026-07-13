//#include "../include/shell.h"
#include "../include/user_commands.h"
#include "../shell_utils/shell_validate.h"
#include "../include/shell_api.h"
#include <string.h>
#include <util/delay.h>
#include <stdlib.h>


/*extern shell* g_shell;   
shell* g_shell = nullptr;*/
void cmd_light(const command_rt_t* self, const char* param, int argc, char** argv) {
     toggle(PIN_B7);
}

void f_light_b(const command_rt_t* self, const char* param, int argc, char** argv) {
    if (!is_int(param)) {
        sh_error(err_t::INVALID_PARAM, "Must enter an integer");
    }
    int n = atoi(param);
    if(n > 10) return;
    low(PIN_B7);
    for (int k=0; k<n; k++) {
                    toggle(PIN_B7);
                    _delay_ms(300);
                }
}

void SH_USR_CMD(test_prt1) {

}