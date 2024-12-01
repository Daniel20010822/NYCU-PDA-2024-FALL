#ifndef _D2DGR_H_
#define _D2DGR_H_

#ifdef ENABLE_DEBUG_D2DGR
#define DEBUG_D2DGR(message) std::cout << "[D2DGR] " << message << std::endl
#else
#define DEBUG_D2DGR(message)
#endif

#include "Cost.h"
#include "GCell.h"
#include "XYCoord.h"
#include "Chip.h"
#include <string>
#include <vector>

using GCellMap2D = std::vector<std::vector<GCell*>>;

class D2DGR {
private:
    XYCoord areaLB; // Routing Area
    XYCoord areaRT; // Routing Area
    int width, height;
    int gridWidth, gridHeight;

    Cost cost;
    std::vector<Chip*> chips;
    GCellMap2D gcell_map;

    void parse_gmp(std::string f_gmp);
    void parse_gcl(std::string f_gcl);
    void parse_cst(std::string f_cst);
    void A_star_search(XYCoord source, XYCoord target);

public:
    D2DGR();
    ~D2DGR();

    void parse_input(std::string f_gmp, std::string f_gcl, std::string f_cst);
    void global_route();
};

#endif