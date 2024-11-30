#include "Chip.h"

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



void Chip::setBumps(std::vector<XYCoord> inBumps) {
    this->bumps = inBumps;
    for (auto bump : inBumps) {
        DEBUG_CHIP("Added bump at (" + std::to_string(bump.X()) + ", " + std::to_string(bump.Y()) + ")");
    }
}