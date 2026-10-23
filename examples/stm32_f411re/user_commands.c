#include "user_commands.h"
#include "stm32f411_defs.h"
#include <stddef.h>

/* toggle led example */
static void cmd_led(shell_t* sh, const shell_cmd_t* self, const char* param, uint8_t argc, char** argv) {
    (void)self;
    (void)param;
    (void)argc;
    (void)argv;

    GPIOA_ODR ^= (1U << LED_PIN);

    if (GPIOA_ODR & (1U << LED_PIN)) {
        sh_puts(sh, "LED ON\r\n");
    } else {
        sh_puts(sh, "LED OFF\r\n");
    }
}

const shell_cmd_t user_cmds[] = {
    {"led", "toggle user LED", cmd_led, NULL, .sub = {CMD_NO_FLAG, NOPARAM, false, 0}}
};

const uint8_t user_cmd_count = (sizeof(user_cmds) / sizeof(user_cmds[0]));

void SystemInit(void)
{

}