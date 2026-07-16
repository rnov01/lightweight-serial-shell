# Unit Tests

Unit tests for all major functions in shell call path:

sh_poll -> tokenizer -> sh_execute
                            |-> sh_find_cmd
                            |-> sh_parse_args
                            |-> sh_dispatch_handler
                            |-> sh_find_flag
                            |-> sh_dispatch_flag