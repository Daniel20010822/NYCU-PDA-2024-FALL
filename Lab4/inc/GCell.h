#ifndef _GCELL_H_
#define _GCELL_H_

#ifdef ENABLE_DEBUG_GCELL
#define DEBUG_GCELL(message) std::cout << "[GCELL] " << message << std::endl
#else
#define DEBUG_GCELL(message)
#endif

#include "XYCoord.h"

class GCell {
private:
    XYCoord LB;  // Actual position of the GCell in the routing area
    XYCoord pos; // Position of the GCell in the GCell map
    int leftEdgeCapacity = 0;
    int bottomEdgeCapacity = 0;

    double f = 0;
    double g = 0;
    double h = 0;

    // TODO: Think if we need to store the pointers to the neighboring GCells
    // GCell *R;
    // GCell *U;

public:
    GCell();
    GCell(XYCoord LB, XYCoord pos, int leftEdgeCapacity, int bottomEdgeCapacity);
    ~GCell();

    int manhattan_distance(XYCoord source, XYCoord target);
    void evaluate_cost(XYCoord source, XYCoord target);

    double getf() const { return f; }
    double getg() const { return g; }
    double geth() const { return h; }
    XYCoord getLB()  const { return LB; }
    XYCoord getPos() const { return pos; }
    void setf(double f) { this->f = f; }
    void setg(double g) { this->g = g; }
    void seth(double h) { this->h = h; }

    struct Compare {
        bool operator() (GCell* lhs, GCell* rhs) {
            return lhs->f > rhs->f;
        }
    };
};

#endif