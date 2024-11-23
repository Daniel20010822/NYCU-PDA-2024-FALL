#include <iostream>
#include "PlacementLegalizer.h"
#include "Debug.h"


int main(int argc, char** argv) {

    if (argc < 4) {
        std::cerr << "Usage: " << argv[0] << " <init_lg_file> <opt_file> <post_lg_file>" << std::endl;
        return 1;
    }

    PlacementLegalizer LG;

    LG.parse_init_lg(argv[1]);
    LG.parse_opt(argv[2]);
    if (DEBUG) LG.write_lg("00_Init.lg");

    LG.place_fCells();
    LG.remove_redundant_PRs();
    LG.place_mCells();
    if (DEBUG) LG.write_lg("01_PlaceCells.lg");

    LG.place_mergedCells(argv[3]);
    if (DEBUG) LG.write_lg("02_PlaceMergedCells.lg");

    return 0;
}
