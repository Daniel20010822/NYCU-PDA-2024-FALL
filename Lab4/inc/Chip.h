#ifndef _CHIP_H_
#define _CHIP_H_

#include "XYCoord.h"
#include <vector>

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


};

#endif