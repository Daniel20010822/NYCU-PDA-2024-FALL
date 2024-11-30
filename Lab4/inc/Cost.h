#ifndef _COST_H_
#define _COST_H_

#ifdef ENABLE_DEBUG_COST
#define DEBUG_COST(message) std::cout << "[COST] " << message << std::endl
#else
#define DEBUG_COST(message)
#endif

#include <iostream>
#include <vector>

using CostMap2D = std::vector<std::vector<double>>;
using CostMap3D = std::vector<std::vector<std::vector<double>>>;

class Cost {
private:
    double alpha;
    double beta;
    double delta;
    double gamma;
    double viaCost;

    CostMap3D costmap;
public:
    Cost() {};
    ~Cost() {};

    // Setters
    void setAlpha(double alpha);
    void setBeta(double beta);
    void setDelta(double delta);
    void setGamma(double gamma);
    void setViaCost(double viaCost);

    void setCostmap(CostMap3D costmap);

    // Getters
    const CostMap2D& getLayer1Costmap() const;
    const CostMap2D& getLayer2Costmap() const;

    double Alpha() const;
    double Beta() const;
    double Delta() const;
    double Gamma() const;
    double ViaCost() const;

    const CostMap2D& getCostmap(unsigned int layer) const;
    const double getCost(unsigned int layer, unsigned int x, unsigned int y) const;
};

#endif