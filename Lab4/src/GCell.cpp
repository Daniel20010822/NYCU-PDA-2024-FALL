#include "GCell.h"

GCell::GCell() {
    // DEBUG_GCELL("GCell object created");
}

GCell::GCell(int leftEdgeCapacity, int bottomEdgeCapacity) {
    this->leftEdgeCapacity = leftEdgeCapacity;
    this->bottomEdgeCapacity = bottomEdgeCapacity;
    // DEBUG_GCELL("GCell object created");
}

GCell::~GCell() {
    // DEBUG_GCELL("GCell object destroyed");
}

