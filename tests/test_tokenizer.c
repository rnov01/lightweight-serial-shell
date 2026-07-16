#include <serial_shell/config.h>
#include <stdlib.h>
#include "shell.h"
#include "unity.h"

/* Normal cmd + argument */
void test_tokenizer_parse_cmd_arg(void) {
    char input[] = "peek 0x300";
    char *argv[MAX_ARGS];

    int argc = tokenizer(input, argv, MAX_ARGS);

    TEST_ASSERT_EQUAL(2, argc);
    TEST_ASSERT_EQUAL_STRING("peek", argv[0]);
    TEST_ASSERT_EQUAL_STRING("0x300", argv[1]);
}

/* Expected ignore trailing whitespace*/
void test_tokenizer_parse_cmd_whitespace_after(void) {
    char input[] = "peek           ";
    char *argv[MAX_ARGS];

    int argc = tokenizer(input, argv, MAX_ARGS);

    TEST_ASSERT_EQUAL(1, argc);
    TEST_ASSERT_EQUAL_STRING("peek", argv[0]); 
}

/* Expected ignore leading whitespace */
void test_tokenizer_parse_cmd_whitespace_before(void) {
    char input[] = "      peek";
    char *argv[MAX_ARGS];

    int argc = tokenizer(input, argv, MAX_ARGS);

    TEST_ASSERT_EQUAL(1, argc);
    TEST_ASSERT_EQUAL_STRING("peek", argv[0]);  
}

/* Expected ignore whitespace between command/arg */
void test_tokenizer_parse_cmd_whitespace_middle(void) {
    char input[] = "peek              0x300";
    char *argv[MAX_ARGS];

    int argc = tokenizer(input, argv, MAX_ARGS);

    TEST_ASSERT_EQUAL(2, argc);
    TEST_ASSERT_EQUAL_STRING("peek", argv[0]);
    TEST_ASSERT_EQUAL_STRING("0x300", argv[1]);  
}

/* Expected argv not modified*/
void test_tokenizer_parse_cmd_blank(void) {
    char input[] = "";
    char *argv[MAX_ARGS] = {0};

    int argc = tokenizer(input, argv, MAX_ARGS);

    TEST_ASSERT_EQUAL(0, argc);
    for (int i=0; i<MAX_ARGS; i++) {
        TEST_ASSERT_NULL(argv[i]);
    }

    argv[0] = "0x55";
    argv[1] = "0xAA";

    argc = tokenizer(input, argv, MAX_ARGS);
    TEST_ASSERT_EQUAL(0, argc);
    TEST_ASSERT_EQUAL_STRING("0x55", argv[0]);
    TEST_ASSERT_EQUAL_STRING("0xAA", argv[1]);
    for (int i=2; i<MAX_ARGS; i++) {
        TEST_ASSERT_NULL(argv[i]);
    }
}

/* Expected all args captured */
void test_tokenizer_exactly_max_args(void) {
    char input[] = "1 2 3 4 5";
    char *argv[MAX_ARGS] = {0};

    int argc = tokenizer(input, argv, MAX_ARGS);

    TEST_ASSERT_EQUAL(5, argc);
    TEST_ASSERT_EQUAL_STRING("1", argv[0]);
    TEST_ASSERT_EQUAL_STRING("2", argv[1]);
    TEST_ASSERT_EQUAL_STRING("3", argv[2]);
    TEST_ASSERT_EQUAL_STRING("4", argv[3]);
    TEST_ASSERT_EQUAL_STRING("5", argv[4]);
    
}

/* Expected argv untouched */
void test_tokenizer_max_args_zero(void) {
    char input[] = "peek 0x300";
    char *argv[MAX_ARGS] = {0};

    int argc = tokenizer(input, argv, 0);

    TEST_ASSERT_EQUAL(0, argc);
    TEST_ASSERT_NULL(argv[0]);
}

/* Expected truncate args > MAX_ARGS *guarded at higher level* */
void test_tokenizer_arg_over_max(void) {
    char input[] = "1 2 3 4 5 6 7 8 9";
    char *argv[MAX_ARGS] = {0};

    int argc = tokenizer(input, argv, MAX_ARGS);

    TEST_ASSERT_EQUAL(5, argc);
    TEST_ASSERT_EQUAL_UINT(5, argc);
    TEST_ASSERT_EQUAL_STRING("1", argv[0]);
    TEST_ASSERT_EQUAL_STRING("2", argv[1]);
    TEST_ASSERT_EQUAL_STRING("3", argv[2]);
    TEST_ASSERT_EQUAL_STRING("4", argv[3]);
    TEST_ASSERT_EQUAL_STRING("5", argv[4]);   
}
