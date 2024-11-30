#include "D2DGR.h"
#include <iostream>
#include <fstream>
#include <string>
#include <sstream>
#include <cassert>

D2DGR::D2DGR() {
    DEBUG_D2DGR("D2DGR object created");
}

D2DGR::~D2DGR() {
    DEBUG_D2DGR("D2DGR object destroyed");
}

void D2DGR::parse_input(std::string f_gmp, std::string f_gcl, std::string f_cst) {
    DEBUG_D2DGR("Parsing GridMap file: " + f_gmp);
    parse_gmp(f_gmp);
    DEBUG_D2DGR("Parsing GridCell file: " + f_gcl);
    parse_gcl(f_gcl);
    DEBUG_D2DGR("Parsing Cost file: " + f_cst);
    parse_cst(f_cst);
}

void D2DGR::parse_gmp(std::string f_gmp) {
    std::ifstream file(f_gmp);

    if (!file.is_open()) {
        std::cerr << "Error: Unable to open file " << f_gmp << std::endl;
        return;
    }

    std::string line;
    while (std::getline(file, line)) {
        if (line == ".ra") {
            std::getline(file, line);
            std::istringstream iss(line);
            int LBX, LBY, width, height;
            iss >> LBX >> LBY >> width >> height;
            this->LB = XYCoord(LBX, LBY);
            this->width = width;
            this->height = height;
            this->RT = this->LB + XYCoord(this->width, this->height);
        }
        else if (line == ".g") {
            std::getline(file, line);
            std::istringstream iss(line);
            int gridWidth, gridHeight;
            iss >> gridWidth >> gridHeight;
            this->gridWidth = gridWidth;
            this->gridHeight = gridHeight;
        }
        else if (line == ".c") {
            std::getline(file, line);
            std::istringstream iss(line);
            int chipLBX, chipLBY, chipWidth, chipHeight;
            iss >> chipLBX >> chipLBY >> chipWidth >> chipHeight;
            Chip *newChip = new Chip(XYCoord(chipLBX, chipLBY), chipWidth, chipHeight);
            this->chips.push_back(newChip);

            std::vector<XYCoord> bumps;
            while (std::getline(file, line)) {
                if (line.empty()) {
                    break;
                }
                if (line == ".b") {
                    continue;
                }
                else {
                    std::istringstream iss(line);
                    int bumpIdx, bumpX, bumpY;
                    iss >> bumpIdx >> bumpX >> bumpY;
                    newChip->setBump(bumpIdx, XYCoord(bumpX, bumpY));
                }
            }
        }
    }

    // Create GCell map
    int numGCellsX = this->width / this->gridWidth;
    int numGCellsY = this->height / this->gridHeight;
    this->gcell_map.resize(numGCellsY);
    for (int i = 0; i < numGCellsY; i++) {
        this->gcell_map[i].resize(numGCellsX);
    }
    DEBUG_D2DGR("Creating GCell map with dimensions: " + std::to_string(this->gcell_map[0].size()) + "x" + std::to_string(this->gcell_map.size()));

    file.close();
}

void D2DGR::parse_gcl(std::string f_gcl) {
    std::ifstream file(f_gcl);

    if (!file.is_open()) {
        std::cerr << "Error: Unable to open file " << f_gcl << std::endl;
        return;
    }

    std::string temp;
    file >> temp; // Skip .ec

    for (size_t i = 0; i < this->gcell_map.size(); i++) {
        for (size_t j = 0; j < this->gcell_map[0].size(); j++) {
            int leftEdgeCapacity, bottomEdgeCapacity;
            file >> leftEdgeCapacity >> bottomEdgeCapacity;
            this->gcell_map[i][j] = new GCell(leftEdgeCapacity, bottomEdgeCapacity);
        }
    }

    file.close();
}

void D2DGR::parse_cst(std::string f_cst) {
    std::ifstream file(f_cst);

    if (!file.is_open()) {
        std::cerr << "Error: Unable to open file " << f_cst << std::endl;
        return;
    }

    std::vector<std::vector<std::vector<double>>> costmap(2, std::vector<std::vector<double>>(this->gcell_map.size(), std::vector<double>(this->gcell_map[0].size())));

    size_t layer = 0;
    std::string temp;
    while (file >> temp) {
        if (temp == ".alpha") {
            double alpha;
            file >> alpha;
            this->cost.setAlpha(alpha);
        }
        else if (temp == ".beta") {
            double beta;
            file >> beta;
            this->cost.setBeta(beta);
        }
        else if (temp == ".delta") {
            double delta;
            file >> delta;
            this->cost.setDelta(delta);
        }
        else if (temp == ".gamma") {
            double gamma;
            file >> gamma;
            this->cost.setGamma(gamma);
        }
        else if (temp == ".v") {
            double viaCost;
            file >> viaCost;
            this->cost.setViaCost(viaCost);
        }
        else if (temp == ".l") {
            for (size_t i = 0; i < this->gcell_map.size(); i++) {
                for (size_t j = 0; j < this->gcell_map[0].size(); j++) {
                    double cost;
                    file >> cost;
                    costmap[layer][i][j] = cost;
                }
            }
            layer++;
        }
    }

    this->cost.setCostmap(costmap);

    file.close();
}

void D2DGR::global_route() {
    Chip *chip1 = this->chips[0];
    Chip *chip2 = this->chips[1];

    std::vector<int> allBumpIndecies = chip1->getBumpIndecies();
    for (int& bumpIdx : allBumpIndecies) {
        XYCoord source = chip1->getBump(bumpIdx);
        XYCoord target = chip2->getBump(bumpIdx);
        DEBUG_D2DGR("Routing bump " + std::to_string(bumpIdx) + ": from (" + std::to_string(source.X()) + ", " + std::to_string(source.Y()) + ") to (" + std::to_string(target.X()) + ", " + std::to_string(target.Y()) + ")");

        // Convert source and target to GCell
        source = (source - this->LB) / XYCoord(this->gridWidth, this->gridHeight);
        target = (target - this->LB) / XYCoord(this->gridWidth, this->gridHeight);
    }
    // A_star_search();
}