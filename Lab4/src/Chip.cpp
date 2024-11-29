#include "Chip.h"

Chip::Chip() {
    this->LB = XYCoord(0, 0);
    this->width = 0;
    this->height = 0;
    this->RT = XYCoord(0, 0);
}

Chip::Chip(XYCoord inLB, int inWidth, int inHeight) {
    this->LB = inLB;
    this->width = inWidth;
    this->height = inHeight;
    this->RT = inLB + XYCoord(inWidth, inHeight);
}

Chip::~Chip() {

}

