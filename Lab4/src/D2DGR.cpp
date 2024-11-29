#include "D2DGR.h"
#include <iostream>
#include <fstream>

D2DGR::D2DGR() {
    DEBUG_D2DGR("D2DGR object created");
}

D2DGR::~D2DGR() {
    DEBUG_D2DGR("D2DGR object destroyed");
}

void D2DGR::parse_input(std::string f_gmp, std::string f_gcl, std::string f_cst) {
    DEBUG_D2DGR("Parsing GridMap file: " + f_gmp);


}