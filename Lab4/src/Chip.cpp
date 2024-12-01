#include "Chip.h"
#include <algorithm>

Chip::Chip() {
    this->LB = XYCoord(0, 0);
    this->width = 0;
    this->height = 0;
    this->RT = XYCoord(0, 0);
    DEBUG_CHIP("Created new chip at (0, 0) with width 0 and height 0");
}

Chip::Chip(XYCoord inLB, int inWidth, int inHeight) {
    this->LB = inLB;
    this->width = inWidth;
    this->height = inHeight;
    this->RT = inLB + XYCoord(inWidth, inHeight);
    DEBUG_CHIP("Created new chip at (" + std::to_string(inLB.X()) + ", " + std::to_string(inLB.Y()) + "), width = " + std::to_string(inWidth) + ", height = " + std::to_string(inHeight));
}

Chip::~Chip() {

}

XYCoord Chip::getBump(int idx) {
    return this->bumps[idx];
}

std::vector<int> Chip::getBumpIndecies() {
    std::vector<int> indecies;
    for (auto const& bump : this->bumps) {
        indecies.push_back(bump.first);
    }
    std::sort(indecies.begin(), indecies.end());
    return indecies;
}

void Chip::setBump(int idx, XYCoord inBump) {
    this->bumps[idx] = inBump;
    DEBUG_CHIP("Added bump " + std::to_string(idx) + " at (" + std::to_string(inBump.X()) + ", " + std::to_string(inBump.Y()) + ")");
}