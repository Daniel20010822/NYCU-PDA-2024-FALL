#ifndef _COST_H_
#define _COST_H_

#include <vector>

using CostMap2D = std::vector<std::vector<unsigned int>>;

class Cost {
private:
    double alpha;
    double beta;
    double delta;
    double gamma;
    double viaCost;

    CostMap2D layer1_costmap;
    CostMap2D layer2_costmap;

public:
    Cost() {};
    ~Cost() {};

    // Setters
    void setAlpha(double alpha);
    void setBeta(double beta);
    void setDelta(double delta);
    void setGamma(double gamma);
    void setViaCost(double viaCost);

    void setLayer1Costmap(const CostMap2D& costmap);
    void setLayer2Costmap(const CostMap2D& costmap);

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