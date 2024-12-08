#include "Cost.h"

Cost::Cost() {
    alpha = 0;
    beta  = 0;
    delta = 0;
    gamma = 0;
    viaCost = 0;
    maxCellCost.resize(2, 0);
}

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

    for (size_t layer = 0; layer < costmap.size(); layer++) {
        for (size_t i = 0; i < costmap[layer].size(); i++) {
            for (size_t j = 0; j < costmap[layer][i].size(); j++) {
                this->maxCellCost[layer] = std::max(this->maxCellCost[layer], costmap[layer][i][j]);
            }
        }
    }
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
    return this->costmap[layer][y][x];
}

const double Cost::getMaxCellCost(unsigned int layer) const {
    return this->maxCellCost[layer];
}