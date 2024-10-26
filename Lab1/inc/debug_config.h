#ifndef DEBUG_H
#define DEBUG_H


#define DEBUG 0  // Set to 0 to disable debugging
#if DEBUG == 1
    #define debug std::cout
#elif DEBUG == 2
    #define debug std::ofstream ("debug.txt")
#else
    #define debug if (false) std::cout
#endif

#endif
