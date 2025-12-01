#define __COLOR_ON__

#if defined(__COLOR_ON__)
    #define CCYA   "\x1B[36m"   //cyan
    #define CYEL"\x1B[33m"      //yellow
    #define CGRN  "\x1B[32m"    //green 
    #define CRED    "\x1B[31m"  //red
    #define C_ "\x1B[0m"    //rst
    #define COLOR_RESET  "\x1B[0m"
#else 
    #define COLOR_CYAN  
    #define COLOR_YELLOW 
    #define COLOR_GREEN  
    #define COLOR_RED    
    #define COLOR_RESET  
#endif
