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

    int L_Capacity = 0;
    int D_Capacity = 0;
    int R_Capacity = 0;
    int U_Capacity = 0;
\
    int L_Usage = 0;
    int D_Usage = 0;
    int R_Usage = 0;
    int U_Usage = 0;

    double f = 0;
    double g = 0;
    double h = 0;

public:
    GCell();
    GCell(XYCoord LB, XYCoord pos, int L_Capacity, int D_Capacity);
    ~GCell();

    XYCoord getLB()  const { return LB; }
    XYCoord getPos() const { return pos; }

    void add_L_Usage(int usage) { L_Usage += usage; }
    void add_D_Usage(int usage) { D_Usage += usage; }
    void add_R_Usage(int usage) { R_Usage += usage; }
    void add_U_Usage(int usage) { U_Usage += usage; }

    int get_L_CurrentUsage() const { return L_Usage; }
    int get_D_CurrentUsage() const { return D_Usage; }
    int get_R_CurrentUsage() const { return R_Usage; }
    int get_U_CurrentUsage() const { return U_Usage; }

    void set_L_Capacity(int capacity) { L_Capacity = capacity; }
    void set_D_Capacity(int capacity) { D_Capacity = capacity; }
    void set_R_Capacity(int capacity) { R_Capacity = capacity; }
    void set_U_Capacity(int capacity) { U_Capacity = capacity; }

    int get_L_Capacity() const { return L_Capacity; }
    int get_D_Capacity() const { return D_Capacity; }
    int get_R_Capacity() const { return R_Capacity; }
    int get_U_Capacity() const { return U_Capacity; }



    // void addLeftEdgeUsage(int capacity) { currentLeftEdgeUsage += capacity; }
    // void addBottomEdgeUsage(int capacity) { currentBottomEdgeUsage += capacity; }

    // int getLeftEdgeCapacity() const { return leftEdgeCapacity; }
    // int getBottomEdgeCapacity() const { return bottomEdgeCapacity; }

    // int getLeftEdgeUsage() const { return currentLeftEdgeUsage; }
    // int getBottomEdgeUsage() const { return currentBottomEdgeUsage; }

    // int getLeftExcessNum() const { return (currentLeftEdgeUsage > leftEdgeCapacity) ? currentLeftEdgeUsage - leftEdgeCapacity : 0; }
    // int getBottomExcessNum() const { return (currentBottomEdgeUsage > bottomEdgeCapacity) ? currentBottomEdgeUsage - bottomEdgeCapacity : 0; }

    int manhattan_distance(XYCoord source, XYCoord target);

    double getf() const { return f; }
    double getg() const { return g; }
    double geth() const { return h; }
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