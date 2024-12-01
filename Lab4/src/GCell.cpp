#include "GCell.h"
#include <iostream>
#include <cmath>

GCell::GCell() {
    // DEBUG_GCELL("GCell object created");
}

GCell::GCell(XYCoord LB, XYCoord pos, int leftEdgeCapacity, int bottomEdgeCapacity) {
    this->LB = LB;
    this->pos = pos;
    this->leftEdgeCapacity = leftEdgeCapacity;
    this->bottomEdgeCapacity = bottomEdgeCapacity;
    // DEBUG_GCELL("GCell object created");
}

GCell::~GCell() {
    // DEBUG_GCELL("GCell object destroyed");
}


int GCell::manhattan_distance(XYCoord source, XYCoord target) {
    return abs(source.X() - target.X()) + abs(source.Y() - target.Y());
}