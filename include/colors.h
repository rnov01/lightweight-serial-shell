
#if defined(__COLOR_ON__)
    #define COLOR_CYAN   "\x1B[36m"
    #define COLOR_YELLOW "\x1B[33m"
    #define COLOR_GREEN  "\x1B[32m"
    #define COLOR_RED    "\x1B[31m"
    #define COLOR_RESET  "\x1B[0m"
#else 
    #define COLOR_CYAN  
    #define COLOR_YELLOW 
    #define COLOR_GREEN  
    #define COLOR_RED    
    #define COLOR_RESET  
#endif
