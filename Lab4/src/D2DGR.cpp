#include "D2DGR.h"
#include <iostream>
#include <fstream>
#include <string>

D2DGR::D2DGR() {
    DEBUG_D2DGR("D2DGR object created");
}

D2DGR::~D2DGR() {
    DEBUG_D2DGR("D2DGR object destroyed");
}

void D2DGR::parse_input(std::string f_gmp, std::string f_gcl, std::string f_cst) {
    DEBUG_D2DGR("Parsing GridMap file: " + f_gmp);
    parse_gmp(f_gmp);
    // DEBUG_D2DGR("Parsing GridCell file: " + f_gcl);
    // parse_gcl(f_gcl);
    // DEBUG_D2DGR("Parsing Cost file: " + f_cst);
    // parse_cst(f_cst);
}

void D2DGR::parse_gmp(std::string f_gmp) {
    std::ifstream file(f_gmp);

    if (!file.is_open()) {
        std::cerr << "Error: Unable to open file " << f_gmp << std::endl;
        return;
    }

    std::string line;
    while (file >> line) {
        if (line == ".ra") {
            int LBX, LBY, width, height;
            file >> LBX >> LBY >> width >> height;
            this->LB = XYCoord(LBX, LBY);
            this->width = width;
            this->height = height;
            this->RT = this->LB + XYCoord(this->width, this->height);
        }
        else if (line == ".g") {
            int gridWidth, gridHeight;
            file >> gridWidth >> gridHeight;
            this->gridWidth = gridWidth;
            this->gridHeight = gridHeight;
        }
        else if (line == ".c") {
            int chipLBX, chipLBY, chipWidth, chipHeight;
            file >> chipLBX >> chipLBY >> chipWidth >> chipHeight;
            Chip *newChip = new Chip(XYCoord(chipLBX, chipLBY), chipWidth, chipHeight);
            this->chips.push_back(newChip);

            std::vector<XYCoord> bumps;
            while (std::getline(file, line)) {
                if (line.empty()) {
                    break;
                }
                else if (line == ".b") {
                    continue;
                }
                else {
                    int bumpIdx, bumpX, bumpY;
                    file >> bumpIdx >> bumpX >> bumpY;
                    bumps.push_back(XYCoord(bumpX, bumpY));
                }
            }
            newChip->setBumps(bumps);
        }
    }

    file.close();
}