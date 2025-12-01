#include "ro.h"
#define __COLOR_ON__

#if defined(__COLOR_ON__)
    #define CCYA   ROSTR("\x1B[36m")   //cyan
    #define CYEL    ROSTR("\x1B[33m")      //yellow
    #define CGRN    ROSTR("\x1B[32m")    //green 
    #define CRED    ROSTR("\x1B[31m")  //red
    #define C_      ROSTR("\x1B[0m")    //rst
    #define COLOR_RESET  "\x1B[0m"
#else 
    #define COLOR_CYAN  
    #define COLOR_YELLOW 
    #define COLOR_GREEN  
    #define COLOR_RED    
    #define COLOR_RESET  
#endif
