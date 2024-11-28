#ifndef _COST_H_
#define _COST_H_

#include <vector>

typedef std::vector<std::vector<unsigned int>> costmap2d;

class Cost {
private:
    double alpha;
    double beta;
    double delta;
    double gamma;

    double viaCost;

    costmap2d layer1_costmap;
    costmap2d layer2_costmap;

public:
    Cost();
};

#endif