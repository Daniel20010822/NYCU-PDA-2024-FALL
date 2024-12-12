#include "GCell.h"
#include <iostream>
#include <cmath>

GCell::GCell() {
    // DEBUG_GCELL("GCell object created");
}

GCell::GCell(XYCoord LB, XYCoord pos, int L_Capacity, int D_Capacity) {
    this->LB = LB;
    this->pos = pos;
    this->L_Capacity = L_Capacity;
    this->D_Capacity = D_Capacity;
    // DEBUG_GCELL("GCell object created");
}

GCell::~GCell() {
    // DEBUG_GCELL("GCell object destroyed");
}


int GCell::manhattan_distance(XYCoord source, XYCoord target) {
    return abs(source.X() - target.X()) + abs(source.Y() - target.Y());
}