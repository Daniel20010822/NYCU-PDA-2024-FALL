#ifndef _D2DGR_H_
#define _D2DGR_H_

#ifdef ENABLE_DEBUG_D2DGR
#define DEBUG_CHECKER(message) std::cout << "[D2DGR] " << message << std::endl
#else
#define DEBUG_CHECKER(message)
#endif

#include <string>
#include <vector>



class D2DGR {
private:

public:
    D2DGR();
    ~D2DGR();

    void parse_cst(std::string filename);
};

#endif