#include <iostream>
#include "PlacementLegalizer.h"
#include "Debug.h"


int main(int argc, char** argv) {

    if (argc < 4) {
        std::cerr << "Usage: " << argv[0] << " <init_lg_file> <opt_file> <post_lg_file>" << std::endl;
        return 1;
    }

    PlacementLegalizer LG;

    if (DEBUG) std::cout << "Step0: Parsing init_lg..." << std::endl;
    LG.parse_init_lg(argv[1]);
    if (DEBUG) std::cout << "Step0: Parsing opt..." << std::endl;
    LG.parse_opt(argv[2]);
    // if (DEBUG) LG.write_lg("00_Init.lg");

    if (DEBUG) std::cout << "Step1: Initial placing..." << std::endl;
    LG.init_place_cells();
    // if (DEBUG) LG.write_lg("01_PlaceCells.lg");

    if (DEBUG) std::cout << "Step2: Placing Merged Cells..." << std::endl;
    LG.place_mergedCells(argv[3]);
    if (DEBUG) LG.write_lg("02_PlaceMergedCells.lg");

    return 0;
}
