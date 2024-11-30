#ifndef _CHIP_H_
#define _CHIP_H_

#include "XYCoord.h"
#include <vector>
#include <unordered_map>
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

    std::unordered_map<int, XYCoord> bumps; //FIXME: Change to grid coord
public:
    Chip();
    Chip(XYCoord inLB, int inWidth, int inHeight);
    ~Chip();

    XYCoord getBump(int idx);
    std::vector<int> getBumpIndecies();
    void setBump(int idx, XYCoord inBump);
};

#endif