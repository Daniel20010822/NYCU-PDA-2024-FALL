#include "Cost.h"

// Setters
void Cost::setAlpha(double alpha) {
    this->alpha = alpha;
}

void Cost::setBeta(double beta) {
    this->beta = beta;
}

void Cost::setDelta(double delta) {
    this->delta = delta;
}

void Cost::setGamma(double gamma) {
    this->gamma = gamma;
}

void Cost::setViaCost(double viaCost) {
    this->viaCost = viaCost;
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