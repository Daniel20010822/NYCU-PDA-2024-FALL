#ifndef _CELL_H_
#define _CELL_H_

#include "XYCoord.h"
#include <string>

class Cell {
public:
    std::string name;
    XYCoord LB = {0,0};
    double width  = 0;
    double height = 0;
    bool isFixed = false;
    Cell(std::string cellName, int lowerLeftX, int lowerLeftY, int width, int height, bool isFixed) :
        name(cellName), LB(lowerLeftX, lowerLeftY), width(width), height(height), isFixed(isFixed) {}
    struct Compare {
        bool operator()(const Cell* lhs, const Cell* rhs) const {
            return lhs->LB.x < rhs->LB.x;
        }
    };
};

#endif