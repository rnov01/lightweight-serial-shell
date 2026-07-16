#include "unity.h"

/* sh_find_cmd */
void test_find_cmd_normal(void);
void test_find_cmd_no_match(void);
void test_find_cmd_bad_table(void);
void test_find_cmd_null_shell(void);
void test_find_cmd_system_disabled(void);
void test_find_cmd_system_normal(void);

/* tokenizer */
void test_tokenizer_parse_cmd_arg(void);
void test_tokenizer_parse_cmd_whitespace_after(void);
void test_tokenizer_parse_cmd_whitespace_before(void);
void test_tokenizer_parse_cmd_whitespace_middle(void);
void test_tokenizer_parse_cmd_blank(void);
void test_tokenizer_exactly_max_args(void);
void test_tokenizer_max_args_zero(void);
void test_tokenizer_arg_over_max(void);

void setUp(void){}
void tearDown(void){}

int main(void)
{
    UNITY_BEGIN();
    printf("Running sh_find_cmd tests...\r\n");
    RUN_TEST(test_find_cmd_normal);
    RUN_TEST(test_find_cmd_no_match);
    RUN_TEST(test_find_cmd_bad_table);
    RUN_TEST(test_find_cmd_null_shell);
    #if !PEEK_CMD 
    RUN_TEST(test_find_cmd_system_disabled); 
    #endif
    RUN_TEST(test_find_cmd_system_normal);

    printf("Running tokenizer tests...\r\n");
    RUN_TEST(test_tokenizer_parse_cmd_arg);
    RUN_TEST(test_tokenizer_arg_over_max);
    RUN_TEST(test_tokenizer_parse_cmd_whitespace_after);
    RUN_TEST(test_tokenizer_parse_cmd_whitespace_before);
    RUN_TEST(test_tokenizer_parse_cmd_whitespace_middle);
    RUN_TEST(test_tokenizer_parse_cmd_blank);
    RUN_TEST(test_tokenizer_exactly_max_args);
    RUN_TEST(test_tokenizer_max_args_zero);
    return UNITY_END();
}