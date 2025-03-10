#include "D2DGR.h"
#include <iostream>
#include <algorithm>
#include <unordered_map>
#include <unordered_set>
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
            int L_Capacity, D_Capacity;
            file >> L_Capacity >> D_Capacity;
            XYCoord LB  = this->areaLB + XYCoord(col * this->gridWidth, row * this->gridHeight);
            XYCoord pos = {int(col), int(row)};
            this->gcell_map[row][col] = new GCell(LB, pos, L_Capacity, D_Capacity);
        }
    }

    for (size_t row = 0; row < this->gcell_map.size(); row++) {
        for (size_t col = 0; col < this->gcell_map[0].size() - 1; col++) {
            int R_Capacity = this->gcell_map[row][col + 1]->get_L_Capacity();
            this->gcell_map[row][col]->set_R_Capacity(R_Capacity);
        }
    }

    for (size_t row = 0; row < this->gcell_map.size() - 1; row++) {
        for (size_t col = 0; col < this->gcell_map[0].size(); col++) {
            int U_Capacity = this->gcell_map[row + 1][col]->get_D_Capacity();
            this->gcell_map[row][col]->set_U_Capacity(U_Capacity);
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

    XYCoord gridSize(this->gridWidth, this->gridHeight);

    std::vector<int> allBumpIndecies = chip1->getBumpIndecies();
    for (int& bumpIdx : allBumpIndecies) {
        DEBUG_D2DGR("============== Pair " + std::to_string(bumpIdx) + " ==============");
        XYCoord source = chip1->getBump(bumpIdx);
        XYCoord target = chip2->getBump(bumpIdx);
        DEBUG_D2DGR("Absolute Coordinate of current pair bumps: (" + std::to_string(source.X()) + ", " + std::to_string(source.Y()) + ") -> (" + std::to_string(target.X()) + ", " + std::to_string(target.Y()) + ")");

        // Convert source and target to GCell coordinates
        XYCoord sourcePos = (source - this->areaLB) / gridSize;
        XYCoord targetPos = (target - this->areaLB) / gridSize;
        A_star_search(bumpIdx, sourcePos, targetPos);
    }
}

double D2DGR::calculate_cost(GCell *currentGCell, XYCoord direction, bool isSameDir) {
    double WL, OV, cellCost, viaCost;
    double cellCostM1 = this->cost.getCost(0, currentGCell->getPos().X(), currentGCell->getPos().Y());
    double cellCostM2 = this->cost.getCost(1, currentGCell->getPos().X(), currentGCell->getPos().Y());
    int OVNum = 0;
    if (direction == XYCoord(1, 0))
        OVNum = currentGCell->get_R_CurrentUsage() + 1 - currentGCell->get_R_Capacity();
    else if (direction == XYCoord(-1, 0))
        OVNum = currentGCell->get_L_CurrentUsage() + 1 - currentGCell->get_L_Capacity();
    else if (direction == XYCoord(0, 1))
        OVNum = currentGCell->get_U_CurrentUsage() + 1 - currentGCell->get_U_Capacity();
    else if (direction == XYCoord(0, -1))
        OVNum = currentGCell->get_D_CurrentUsage() + 1 - currentGCell->get_D_Capacity();

    OVNum = (OVNum > 0) ? OVNum : 0;
    // int OV1 = currentGCell->get_D_CurrentUsage() + 1 - currentGCell->get_D_Capacity();
    // int OV2 = currentGCell->get_L_CurrentUsage() + 1 - currentGCell->get_L_Capacity();
    // OV1 = (OV1 > 0) ? OV1 : 0;
    // OV2 = (OV2 > 0) ? OV2 : 0;

    if (isSameDir) {
        if (direction == XYCoord(1, 0) || direction == XYCoord(-1, 0)) {
            WL = this->gridWidth;
            OV = OVNum * 0.5 * cost.getMaxCellCost(1);
            cellCost = cellCostM2;
            viaCost = 0;
        }
        else {
            WL = this->gridHeight;
            OV = OVNum * 0.5 * cost.getMaxCellCost(0);
            cellCost = cellCostM1;
            viaCost = 0;
        }
    }
    else {
        if (direction == XYCoord(1, 0) || direction == XYCoord(-1, 0)) {
            WL = this->gridWidth;
            OV = OVNum * 0.5 * cost.getMaxCellCost(1);
            cellCost = (cellCostM1 + cellCostM2) / 2;
            viaCost = this->cost.ViaCost();
        }
        else {
            WL = this->gridHeight;
            OV = OVNum * 0.5 * cost.getMaxCellCost(0);
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

    // Initialize the open list and open set
    std::priority_queue<GCell*, std::vector<GCell*>, GCell::Compare> openList;
    std::unordered_map<GCell*, bool> openSetMap;
    openList.push(sourceGCell);
    openSetMap[sourceGCell] = true;


    // Initialize the closed list
    std::vector<std::vector<bool>> closedList(this->gcell_map.size(), std::vector<bool>(this->gcell_map[0].size(), false));

    // Initialize the starting GCell
    sourceGCell->setg(0);
    sourceGCell->seth(sourceGCell->manhattan_distance(sourceGCell->getLB(), targetGCell->getLB()));
    sourceGCell->setf(sourceGCell->getg() + sourceGCell->geth());

    // Create a map to restore the path
    std::vector<GCell*> cameFrom(this->gcell_map.size() * this->gcell_map[0].size(), nullptr);

    // Iterate through the open list
    while (!openList.empty()) {
        GCell *currentGCell = openList.top();
        // DEBUG_D2DGR(
        //     " ==> Current Pos: (" + std::to_string(currentGCell->getPos().X()) + ", " + std::to_string(currentGCell->getPos().Y()) + "), " +
        //     "f = " + std::to_string(static_cast<int>(currentGCell->getf())) + ", " +
        //     "g = " + std::to_string(static_cast<int>(currentGCell->getg())) + ", " +
        //     "h = " + std::to_string(static_cast<int>(currentGCell->geth()))
        // );

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
            XYCoord nextPos = currentGCell->getPos() + direction;
            if (nextPos.X() < 0 || nextPos.X() >= int(this->gcell_map[0].size()) || nextPos.Y() < 0 || nextPos.Y() >= int(this->gcell_map.size())) {
                continue;
            }
            GCell *nextGCell = this->gcell_map[nextPos.Y()][nextPos.X()];
            if (closedList[nextPos.Y()][nextPos.X()]) {
                continue;
            }

            // Calculate the tentative g score
            GCell *prevGCell = (currentGCell->getPos() == sourcePos) ? nullptr : cameFrom[currentGCell->getPos().Y() * this->gcell_map[0].size() + currentGCell->getPos().X()];
            XYCoord prevDir = (prevGCell) ? currentGCell->getPos() - prevGCell->getPos() : XYCoord(0, 1);
            bool isSameDir = (direction == prevDir);
            double tentative_g = currentGCell->getg() + this->calculate_cost(currentGCell, direction, isSameDir);

            // Check if the neighbor is in the open list
            bool inOpenList = openSetMap.find(nextGCell) != openSetMap.end();

            // Check if the path to the neighbor is better
            bool isTenativeBetter = !inOpenList || tentative_g < nextGCell->getg();

            // This path is the best until now. Record it!
            if (isTenativeBetter) {
                nextGCell->setg(tentative_g);
                nextGCell->seth(nextGCell->manhattan_distance(nextGCell->getLB(), targetGCell->getLB()));
                nextGCell->setf(nextGCell->getg() + nextGCell->geth());
                openList.push(nextGCell);
                openSetMap[nextGCell] = true;
                cameFrom[nextGCell->getPos().Y() * this->gcell_map[0].size() + nextGCell->getPos().X()] = currentGCell;
                // DEBUG_D2DGR("Push (" + std::to_string(nextGCell->getPos().X()) + ", " + std::to_string(nextGCell->getPos().Y()) + ")" + " to open list");
            }
        }
    }
}

void D2DGR::reconstruct_path(int currentIdx, std::vector<GCell*>& cameFrom, GCell *targetGCell) {
    std::vector<GCell*> totalPath;
    totalPath.push_back(targetGCell);
    while (cameFrom[targetGCell->getPos().Y() * this->gcell_map[0].size() + targetGCell->getPos().X()] != nullptr) {
        targetGCell = cameFrom[targetGCell->getPos().Y() * this->gcell_map[0].size() + targetGCell->getPos().X()];
        totalPath.push_back(targetGCell);
    }
    std::reverse(totalPath.begin(), totalPath.end());

    // Show the path
    // std::string pathStr = "Path: ";
    // for (GCell *gcell : totalPath) {
    //     pathStr += "(" + std::to_string(gcell->getPos().X()) + ", " + std::to_string(gcell->getPos().Y()) + ") ";
    // }
    // DEBUG_D2DGR(pathStr);

    // Detect via points
    std::vector<bool> isVia(totalPath.size(), false);
    for (size_t i = 1; i < totalPath.size() - 1; i++) {
        GCell *currentGCell = totalPath[i];
        GCell *prevGCell = totalPath[i - 1];
        GCell *nextGCell = totalPath[i + 1];
        XYCoord direction1 = currentGCell->getPos() - prevGCell->getPos();
        XYCoord direction2 = nextGCell->getPos() - currentGCell->getPos();
        isVia[i] = (direction1 != direction2);
    }

    // Create path segments using via points
    std::vector<std::pair<size_t, size_t>> pathSegments;
    size_t startIdx = 0;
    for (size_t i = 1; i < totalPath.size(); i++) {
        if (isVia[i] || i == totalPath.size() - 1) {
            pathSegments.push_back(std::make_pair(startIdx, i));
            startIdx = i;
        }
    }



    // Create the output format path
    std::string netName = "n" + std::to_string(currentIdx);
    Net *newNet = new Net(netName);
    std::vector<std::string> paths;

    enum Direction {Up, Down, Left, Right};
    for (auto pathSegment : pathSegments) {
        XYCoord start = totalPath[pathSegment.first]->getLB();
        XYCoord end   = totalPath[pathSegment.second]->getLB();
        // Direction direction = (start.X() == end.X()) ? Vertical : Horizontal;
        XYCoord direction = end - start;
        Direction dir = Up;

        if      (direction.X() > 0)    dir = Right;
        else if (direction.X() < 0)    dir = Left;
        else if (direction.Y() > 0)    dir = Up;
        else if (direction.Y() < 0)    dir = Down;

        // Update the capacity of the GCells
        for (size_t i = pathSegment.first; i <= pathSegment.second; i++) {
            GCell *gcell = totalPath[i];
            GCell *nextGcell = (i == pathSegment.second) ? nullptr : totalPath[i + 1];
            switch (dir) {
                case Right:
                    gcell->add_R_Usage(1);
                    if (nextGcell) nextGcell->add_L_Usage(1);
                    break;
                case Left:
                    gcell->add_L_Usage(1);
                    if (nextGcell) nextGcell->add_R_Usage(1);
                    break;
                case Up:
                    gcell->add_U_Usage(1);
                    if (nextGcell) nextGcell->add_D_Usage(1);
                    break;
                case Down:
                    gcell->add_D_Usage(1);
                    if (nextGcell) nextGcell->add_U_Usage(1);
                    break;
            }
        }

        // Create the path
        std::string metalType = (dir == Up || dir == Down) ? "M1" : "M2";
        std::string path = metalType + " " +
                           std::to_string(start.X()) + " " +
                           std::to_string(start.Y()) + " " +
                           std::to_string(end.X())   + " " +
                           std::to_string(end.Y());
        if (pathSegment == pathSegments.front() && (dir == Left || dir == Right)) {
            newNet->addPath("via");
        }
        newNet->addPath(path);
        if (pathSegment != pathSegments.back() || (pathSegment == pathSegments.back() && (dir == Left || dir == Right))) {
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