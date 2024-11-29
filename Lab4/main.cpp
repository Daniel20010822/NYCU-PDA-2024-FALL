#include <iostream>
#include "D2DGR.h"

int main (int argc, char** argv) {

    if (argc != 5) {
        std::cerr << "Usage: " << argv[0] << " <gmp_file> <gcl_file> <cst_file> <lg_file>" << std::endl;
        return 1;
    }

    D2DGR GlobalRouter;
    GlobalRouter.parse_input(argv[1], argv[2], argv[3]);
    return 0;
}