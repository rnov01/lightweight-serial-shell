#include <serial_shell/config.h>
#include <serial_shell/shell_commands.h>
#include <stdlib.h>
#include "shell.h"
#include "unity.h"

#define ARRAY_SIZE(x) (sizeof(x) / sizeof(x[0]))

const shell_cmd_t test_cmds[] = {
    {"cmd_1", "I am command 1", NULL, .sub = {0, 0, 0, 0}},
    {"cmd_2", "I am command 2", NULL, .sub = {0, 0, 0, 0}},
    {"cmd_3", "I am command 3", NULL, .sub = {0, 0, 0, 0}},
    {"cmd_4", "I am command 4", NULL, .sub = {0, 0, 0, 0}},
    {"cmd_5", "I am command 5", NULL, .sub = {0, 0, 0, 0}}
};

void test_find_cmd_normal(void) {
    static shell_t shell = {
        .cmd = test_cmds,
        .cmd_count = ARRAY_SIZE(test_cmds)
    };
    
    const shell_cmd_t *result = sh_find_cmd(&shell, "cmd_1");
    TEST_ASSERT_NOT_NULL(result);
    TEST_ASSERT_EQUAL_STRING("cmd_1", result->name);
    TEST_ASSERT_EQUAL_STRING("I am command 1", result->description);
    TEST_ASSERT_NULL(result->handler);
}

/* expected null pointer returned */
void test_find_cmd_no_match(void) {
    static shell_t shell = {
        .cmd = test_cmds,
        .cmd_count = ARRAY_SIZE(test_cmds)
    };
    
    const shell_cmd_t *result = sh_find_cmd(&shell, "command doesn't exist");
    TEST_ASSERT_NULL(result);
}

void test_find_cmd_bad_table(void) {
    static shell_t shell = {
        .cmd = NULL,
        .cmd_count = ARRAY_SIZE(test_cmds)
    };
    
    const shell_cmd_t *result = sh_find_cmd(&shell, "command doesn't exist");
    TEST_ASSERT_NULL(result);
}

void test_find_cmd_null_shell(void) {
    const shell_cmd_t *result = sh_find_cmd(NULL, "command doesn't exist");
    TEST_ASSERT_NULL(result);
}

#if !PEEK_CMD
void test_find_cmd_system_disabled(void) {
    static shell_t shell = {
        .cmd = test_cmds,
        .cmd_count = ARRAY_SIZE(test_cmds)
    };
    
    const shell_cmd_t *result = sh_find_cmd(&shell, "peek");
    TEST_ASSERT_NULL(result);
}
#endif

void test_find_cmd_system_normal(void) {
    static shell_t shell = {
        .cmd = test_cmds,
        .cmd_count = ARRAY_SIZE(test_cmds)
    };
    
    const shell_cmd_t *result = sh_find_cmd(&shell, "help");
    TEST_ASSERT_NOT_NULL(result);
    TEST_ASSERT_EQUAL_STRING(result->name, "help");
    TEST_ASSERT_EQUAL_STRING(result->description, "Provides a list of system commands");
    TEST_ASSERT_NOT_NULL(result->handler);
}


