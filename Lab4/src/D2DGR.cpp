#include "D2DGR.h"
#include <iostream>
#include <algorithm>
#include <unordered_map>
#include <fstream>
#include <string>
#include <sstream>
#include <queue>
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
            this->areaLB = XYCoord(LBX, LBY);
            this->width = width;
            this->height = height;
            this->areaRT = this->areaLB + XYCoord(this->width, this->height);
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
            XYCoord chipLB = XYCoord(chipLBX, chipLBY) + this->areaLB;
            Chip *newChip = new Chip(chipLB, chipWidth, chipHeight);
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
                    XYCoord bumpPos = XYCoord(bumpX, bumpY) + chipLB;
                    newChip->setBump(bumpIdx, bumpPos);
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

    for (size_t row = 0; row < this->gcell_map.size(); row++) {
        for (size_t col = 0; col < this->gcell_map[0].size(); col++) {
            int leftEdgeCapacity, bottomEdgeCapacity;
            file >> leftEdgeCapacity >> bottomEdgeCapacity;
            XYCoord LB  = this->areaLB + XYCoord(col * this->gridWidth, row * this->gridHeight);
            XYCoord pos = {int(col), int(row)};
            this->gcell_map[row][col] = new GCell(LB, pos, leftEdgeCapacity, bottomEdgeCapacity);
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
        DEBUG_D2DGR("============== Pair " + std::to_string(bumpIdx) + " ==============");
        XYCoord source = chip1->getBump(bumpIdx);
        XYCoord target = chip2->getBump(bumpIdx);
        DEBUG_D2DGR("Absolute Coordinate of current pair bumps: (" + std::to_string(source.X()) + ", " + std::to_string(source.Y()) + ") -> (" + std::to_string(target.X()) + ", " + std::to_string(target.Y()) + ")");

        // Convert source and target to GCell coordinates
        XYCoord sourcePos = (source - this->areaLB) / XYCoord(this->gridWidth, this->gridHeight);
        XYCoord targetPos = (target - this->areaLB) / XYCoord(this->gridWidth, this->gridHeight);
        A_star_search(bumpIdx, sourcePos, targetPos);
    }
}

double D2DGR::calculate_cost(GCell *currentGCell, Direction dir, bool isSameDir) {
    double WL, OV, cellCost, viaCost;
    double cellCostM1 = this->cost.getCost(0, currentGCell->getPos().X(), currentGCell->getPos().Y());
    double cellCostM2 = this->cost.getCost(1, currentGCell->getPos().X(), currentGCell->getPos().Y());
    int OV1 = currentGCell->getBottomEdgeUsage()  + 1 - currentGCell->getBottomEdgeCapacity();
    int OV2 = currentGCell->getLeftEdgeUsage()    + 1 - currentGCell->getLeftEdgeCapacity();
    OV1 = (OV1 > 0) ? OV1 : 0;
    OV2 = (OV2 > 0) ? OV2 : 0;

    if (isSameDir) {
        if (dir == Horizontal) {
            WL = this->gridWidth;
            OV = OV2 * 0.5 * cost.getMaxCellCost(1);
            cellCost = cellCostM2;
            viaCost = 0;
        }
        else {
            WL = this->gridHeight;
            OV = OV1 * 0.5 * cost.getMaxCellCost(0);
            cellCost = cellCostM1;
            viaCost = 0;
        }
    }
    else {
        if (dir == Horizontal) {
            WL = this->gridWidth;
            OV = OV2 * 0.5 * cost.getMaxCellCost(1);
            cellCost = (cellCostM1 + cellCostM2) / 2;
            viaCost = this->cost.ViaCost();
        }
        else {
            WL = this->gridHeight;
            OV = OV1 * 0.5 * cost.getMaxCellCost(0);
            cellCost = (cellCostM1 + cellCostM2) / 2;
            viaCost = this->cost.ViaCost();
        }
    }

    double resultCost = this->cost.Alpha() * WL +
                        this->cost.Beta()  * OV +
                        this->cost.Gamma() * cellCost +
                        this->cost.Delta() * viaCost;

    return resultCost;
}


void D2DGR::A_star_search(int currentIdx, XYCoord sourcePos, XYCoord targetPos) {
    DEBUG_D2DGR("A* search from (" + std::to_string(sourcePos.X()) + ", " + std::to_string(sourcePos.Y()) + ") to (" + std::to_string(targetPos.X()) + ", " + std::to_string(targetPos.Y()) + ")");
    GCell *sourceGCell = this->gcell_map[sourcePos.Y()][sourcePos.X()];
    GCell *targetGCell = this->gcell_map[targetPos.Y()][targetPos.X()];

    // Initialize the open list
    std::priority_queue<GCell*, std::vector<GCell*>, GCell::Compare> openList;
    openList.push(sourceGCell);

    // Initialize the closed list
    std::vector<std::vector<bool>> closedList(this->gcell_map.size(), std::vector<bool>(this->gcell_map[0].size(), false));

    // Initialize the starting GCell
    sourceGCell->setg(0);
    sourceGCell->seth(sourceGCell->manhattan_distance(sourceGCell->getLB(), targetGCell->getLB()));
    sourceGCell->setf(sourceGCell->getg() + sourceGCell->geth());

    // Create a map to restore the path
    std::unordered_map<GCell*, GCell*> cameFrom;

    // Iterate through the open list
    while (!openList.empty()) {
        GCell *currentGCell = openList.top();
        DEBUG_D2DGR(
            " ==> Current Pos: (" + std::to_string(currentGCell->getPos().X()) + ", " + std::to_string(currentGCell->getPos().Y()) + "), " +
            "f = " + std::to_string(static_cast<int>(currentGCell->getf())) + ", " +
            "g = " + std::to_string(static_cast<int>(currentGCell->getg())) + ", " +
            "h = " + std::to_string(static_cast<int>(currentGCell->geth()))
        );

        openList.pop();
        closedList[currentGCell->getPos().Y()][currentGCell->getPos().X()] = true;

        // Check if we have reached the target
        if (currentGCell->getPos() == targetPos) {
            reconstruct_path(currentIdx, cameFrom, currentGCell);
            return;
        }

        // Iterate through the neighbors
        XYCoord directions[] = {XYCoord(1, 0), XYCoord(0, 1), XYCoord(-1, 0), XYCoord(0, -1)}; // Right, Up, Left, Down
        for (XYCoord direction : directions) {
            Direction dir = (direction.X() == 0) ? Vertical : Horizontal;
            XYCoord nextPos = currentGCell->getPos() + direction;
            if (nextPos.X() < 0 || nextPos.X() >= int(this->gcell_map[0].size()) || nextPos.Y() < 0 || nextPos.Y() >= int(this->gcell_map.size())) {
                continue;
            }
            GCell *nextGCell = this->gcell_map[nextPos.Y()][nextPos.X()];
            if (closedList[nextPos.Y()][nextPos.X()]) {
                continue;
            }

            // Calculate the tentative g score
            GCell *prevGCell = (currentGCell->getPos() == sourcePos) ? nullptr : cameFrom[currentGCell];
            Direction prevDir = (prevGCell == nullptr) ? Vertical : (prevGCell->getPos().X() == currentGCell->getPos().X()) ? Vertical : Horizontal;
            bool isSameDir = (dir == prevDir);
            double tentative_g = currentGCell->getg() + this->calculate_cost(currentGCell, dir, isSameDir);

            // Check if the neighbor is in the open list
            bool inOpenList = false;
            std::priority_queue<GCell*, std::vector<GCell*>, GCell::Compare> tempQueue = openList;
            while (!tempQueue.empty()) {
                GCell *openGCell = tempQueue.top();
                tempQueue.pop();
                if (openGCell->getPos() == nextGCell->getPos()) {
                    inOpenList = true;
                    break;
                }
            }

            // Check if the path to the neighbor is better
            bool isTenativeBetter = false;
            if (!inOpenList) {
                isTenativeBetter = true;
            }
            else if (tentative_g < nextGCell->getg()) {
                isTenativeBetter = true;
            }

            // This path is the best until now. Record it!
            if (isTenativeBetter) {
                nextGCell->setg(tentative_g);
                nextGCell->seth(nextGCell->manhattan_distance(nextGCell->getLB(), targetGCell->getLB()));
                nextGCell->setf(nextGCell->getg() + nextGCell->geth());
                openList.push(nextGCell);
                cameFrom[nextGCell] = currentGCell;
                // DEBUG_D2DGR("Push (" + std::to_string(nextGCell->getPos().X()) + ", " + std::to_string(nextGCell->getPos().Y()) + ")" + " to open list");
            }
        }

        // // Show the open list after each iteration
        // std::priority_queue<GCell*, std::vector<GCell*>, GCell::Compare> tempQueue = openList;
        // std::string openListStr = "Open List: ";
        // while (!tempQueue.empty()) {
        //     GCell *openGCell = tempQueue.top();
        //     openListStr += "(" + std::to_string(openGCell->getPos().X()) + ", " + std::to_string(openGCell->getPos().Y()) + ", " + std::to_string(static_cast<int>(openGCell->getf())) + ") ";
        //     tempQueue.pop();
        // }
        // DEBUG_D2DGR(openListStr);
    }
}

void D2DGR::reconstruct_path(int currentIdx, std::unordered_map<GCell*, GCell*>& cameFrom, GCell *targetGCell) {
    std::vector<GCell*> totalPath;
    totalPath.push_back(targetGCell);
    while (cameFrom.find(targetGCell) != cameFrom.end()) {
        targetGCell = cameFrom[targetGCell];
        totalPath.push_back(targetGCell);
    }
    std::reverse(totalPath.begin(), totalPath.end());

    // Show the path
    std::string pathStr = "Path: ";
    for (GCell *gcell : totalPath) {
        pathStr += "(" + std::to_string(gcell->getPos().X()) + ", " + std::to_string(gcell->getPos().Y()) + ") ";
    }
    DEBUG_D2DGR(pathStr);

    // Detect via points
    std::vector<bool> isVia(totalPath.size(), false);
    for (size_t i = 1; i < totalPath.size() - 1; i++) {
        GCell *currentGCell = totalPath[i];
        GCell *prevGCell = totalPath[i - 1];
        GCell *nextGCell = totalPath[i + 1];
        Direction dir1 = (currentGCell->getPos().X() == prevGCell->getPos().X()) ? Vertical : Horizontal;
        Direction dir2 = (currentGCell->getPos().X() == nextGCell->getPos().X()) ? Vertical : Horizontal;
        isVia[i] = (dir1 != dir2);
    }

    // Create path segments using via points
    std::vector<std::pair<size_t, size_t>> pathSegments;
    size_t startIdx = 0;
    for (size_t i = 1; i < totalPath.size(); i++) {
        if (isVia[i]) {
            pathSegments.push_back(std::make_pair(startIdx, i));
            // DEBUG_D2DGR("Segment from " + std::to_string(startIdx) + " to " + std::to_string(i));
            startIdx = i;
        }
        else if (i == totalPath.size() - 1) {
            pathSegments.push_back(std::make_pair(startIdx, i));
            // DEBUG_D2DGR("Segment from " + std::to_string(startIdx) + " to " + std::to_string(i));
        }
    }



    // Create the output format path
    std::string netName = "n" + std::to_string(currentIdx);
    Net *newNet = new Net(netName);
    std::vector<std::string> paths;

    for (auto pathSegment : pathSegments) {
        XYCoord start = totalPath[pathSegment.first]->getLB();
        XYCoord end   = totalPath[pathSegment.second]->getLB();
        Direction direction = (start.X() == end.X()) ? Vertical : Horizontal;

        // Update the capacity of the GCells
        for (size_t i = pathSegment.first; i <= pathSegment.second; i++) {
            GCell *gcell = totalPath[i];
            if (direction == Horizontal) {
                gcell->addLeftEdgeUsage(1);
            }
            else {
                gcell->addBottomEdgeUsage(1);
            }
        }

        // Create the path
        std::string metalType = (direction == Vertical) ? "M1" : "M2";
        std::string path = metalType + " " +
                           std::to_string(start.X()) + " " +
                           std::to_string(start.Y()) + " " +
                           std::to_string(end.X())   + " " +
                           std::to_string(end.Y());
        if (pathSegment == pathSegments.front() && direction == Horizontal) {
            newNet->addPath("via");
        }
        newNet->addPath(path);
        if (pathSegment != pathSegments.back() || (pathSegment == pathSegments.back() && direction == Horizontal)) {
            newNet->addPath("via");
        }
    }
    newNet->addPath(".end");

    // Add this net to the netlist
    this->netlist.push_back(newNet);
}

void D2DGR::write_output(std::string f_lg) {
    std::ofstream file(f_lg);

    if (!file.is_open()) {
        std::cerr << "Error: Unable to open file " << f_lg << std::endl;
        return;
    }

    for (Net *net : this->netlist) {
        file << net->getName() << "\n";
        for (std::string path : net->getPaths()) {
            file << path << "\n";
        }
    }

    file.close();
}