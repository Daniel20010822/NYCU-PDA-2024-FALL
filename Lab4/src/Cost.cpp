#include "Cost.h"

// Setters
void Cost::setAlpha(double alpha) {
    this->alpha = alpha;
    DEBUG_COST("Alpha set to " + std::to_string(this->alpha));
}

void Cost::setBeta(double beta) {
    this->beta = beta;
    DEBUG_COST("Beta set to " + std::to_string(this->beta));
}

void Cost::setDelta(double delta) {
    this->delta = delta;
    DEBUG_COST("Delta set to " + std::to_string(this->delta));
}

void Cost::setGamma(double gamma) {
    this->gamma = gamma;
    DEBUG_COST("Gamma set to " + std::to_string(this->gamma));
}

void Cost::setViaCost(double viaCost) {
    this->viaCost = viaCost;
    DEBUG_COST("ViaCost set to " + std::to_string(this->viaCost));
}

void Cost::setCostmap(CostMap3D costmap) {
    this->costmap = costmap;
    DEBUG_COST("Costmap set");
}


// Getters
double Cost::Alpha() const {
    return this->alpha;
}

double Cost::Beta() const {
    return this->beta;
}

double Cost::Delta() const {
    return this->delta;
}

double Cost::Gamma() const {
    return this->gamma;
}

double Cost::ViaCost() const {
    return this->viaCost;
}

const double Cost::getCost(unsigned int layer, unsigned int x, unsigned int y) const {
    return this->costmap[layer][x][y];
}