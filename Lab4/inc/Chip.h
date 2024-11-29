#ifndef _CHIP_H_
#define _CHIP_H_

#include "XYCoord.h"
#include <vector>
#include <iostream>

#ifdef ENABLE_DEBUG_CHIP
#define DEBUG_CHIP(message) std::cout << "[CHIP] " << message << std::endl
#else
#define DEBUG_CHIP(message)
#endif

class Chip {
private:
    XYCoord LB;
    XYCoord RT;
    int width;
    int height;

    std::vector<XYCoord> bumps;
public:
    Chip();
    Chip(XYCoord inLB, int inWidth, int inHeight);
    ~Chip();

    void setBumps(std::vector<XYCoord> inBumps);
};

#endif