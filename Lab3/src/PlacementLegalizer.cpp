#include "PlacementLegalizer.h"

void PlacementLegalizer::write_lg(std::string filename) {
    std::ofstream f_lg(filename);

    f_lg << "Alpha " << this->alpha << "\n";
    f_lg << "Beta " << this->beta << "\n";
    f_lg << "DieSize " << this->DieLB.x << " " << this->DieLB.y << " " << this->DieUR.x << " " << this->DieUR.y << "\n";

    for (auto cell: this->allFCells) {
        f_lg << cell->name << " " << cell->LB.x << " " << cell->LB.y << " " << cell->width << " " << cell->height << " " << "FIX" << "\n";
    }
    for (auto cell: this->allMCells) {
        f_lg << cell->name << " " << cell->LB.x << " " << cell->LB.y << " " << cell->width << " " << cell->height << " " << "NOTFIX" << "\n";
    }
    for (auto &row: allRows) {
        for (auto PR : row->get_PRs()) {
            f_lg << "PlacementRows " << int(PR->startP.x) << " " << int(PR->startP.y) << " " << int(PR->siteWidth) << " " << int(PR->siteHeight) << " " << PR->totalNumOfSites << "\n";
        }
    }

    f_lg.close();
}

void PlacementLegalizer::parse_init_lg(std::string filename) {
    std::ifstream f_lg(filename);

    if (!f_lg.is_open()) {
        std::cerr << "Cannot open file: " << filename << std::endl;
        exit(1);
    }

    std::vector<Cell *> mCells;
    std::vector<Cell *> fCells;
    std::vector<PlacementRow *> PRs;

    std::string type;
    while (f_lg >> type) {
        if (type == "Alpha") {
            f_lg >> this->alpha;
        }
        else if (type == "Beta") {
            f_lg >> this->beta;
        }
        else if (type == "DieSize") {
            f_lg >> this->DieLB.x >> this->DieLB.y >> this->DieUR.x >> this->DieUR.y;
        }
        else if (type == "PlacementRows") {
            int startX, startY, siteWidth, siteHeight, totalNumOfSites;
            f_lg >> startX >> startY >> siteWidth >> siteHeight >> totalNumOfSites;
            PlacementRow* PR = new PlacementRow(startX, startY, siteWidth, siteHeight, totalNumOfSites);
            PRs.emplace_back(PR);
            this->rowHeight = siteHeight;
            this->rowSiteWidth = siteWidth;
        }
        else {
            std::string cellName = type;
            std::string checkFIX;
            int lowerLeftX, lowerLeftY, width, height;
            f_lg >> lowerLeftX >> lowerLeftY >> width >> height >> checkFIX;

            Cell *newCell = new Cell(cellName, lowerLeftX, lowerLeftY, width, height, (checkFIX == "FIX"));
            if (newCell->isFixed)
                fCells.emplace_back(newCell);
            else {
                mCells.emplace_back(newCell);
            }
            this->name2cell[cellName] = newCell;
        }
    }


    sort(PRs.begin(), PRs.end(), [](PlacementRow *a, PlacementRow *b) { return a->startP.y < b->startP.y; });


    for (size_t i = 0; i < PRs.size(); ++i) {
        PlacementRow* PR = PRs[i];
        this->allRows.push_back(new Row({PR}, PR->startP.y, PR->siteHeight));
        this->rowLookup[PR->startP.y] = allRows[i];
    }

    // Add mCells and fCells to corresponding rows
    for (auto &mCell : mCells) {
        // Row *row = this->rowLookup[mCell->LB.y];
        // row->add_mCell(mCell);
        allMCells.emplace_back(mCell);
        // this->cell2row[mCell] = row;
    }
    for (auto &fCell : fCells) {
        // Row *row = this->rowLookup[fCell->LB.y];
        // row->add_fCell(fCell);
        allFCells.emplace_back(fCell);
        // this->cell2row[fCell] = row;
    }

    f_lg.close();
}

void PlacementLegalizer::parse_opt(std::string filename) {
    std::ifstream f_opt(filename);

    if (!f_opt.is_open()) {
        std::cerr << "Cannot open file: " << filename << std::endl;
        exit(1);
    }

    std::string line;
    while (getline(f_opt, line)) {
        if (line.empty()) continue;

        std::stringstream ss(line);
        std::string token;
        std::string mergedCellName;
        double lbx, lby, w, h;

        std::vector<Cell *> removeCellList;
        ss >> token;
        if (token == "Banking_Cell:") {
            std::vector<Cell *> tempList;
            // Read candidate cells until -->
            ss >> token;
            while (token != "-->") {
                tempList.push_back(this->name2cell[token]);
                if (!(ss >> token)) {
                    getline(f_opt, line);
                    ss.clear();
                    ss.str(line);
                    ss >> token;
                }
            }
            removeCellList = tempList;

            // Read merged cell info
            ss >> mergedCellName >> lbx >> lby >> w >> h;
        }

        // for (auto cell : removeCellList) {
        //     std::cout << cell->name << " ";
        // }
        // std::cout << "--> " << mergedCellName << " " << lbx << " " << lby << " " << w << " " << h << std::endl;

        // Store or process the parsed data as needed
        Cell* mergedCell = new Cell(mergedCellName, lbx, lby, w, h, false);
        this->name2cell[mergedCellName] = mergedCell;

        // Update min width
        if (mergedCell->width < this->mergedCellMinWidth) {
            this->mergedCellMinWidth = mergedCell->width;
        }

        // Add operation to queue
        OptOperation queueElement(mergedCell, removeCellList);
        this->optQueue.push(queueElement);

    }

    f_opt.close();
}

void PlacementLegalizer::init_place_cells() {
    for (auto &fCell : allFCells) {
        place_cell(fCell);
    }

    remove_redundant_PRs();

    for (auto &mCell : allMCells) {
        place_cell(mCell);
    }
}

void PlacementLegalizer::place_mergedCells(std::string filename) {
    std::ofstream f_out(filename);

    if (!f_out.is_open()) {
        std::cerr << "Cannot open file: " << filename << std::endl;
        exit(1);
    }



    while (!optQueue.empty()) {
        OptOperation opt = this->optQueue.front();
        optQueue.pop();

        // Cell *mergedCell = opt.mergedCell;
        std::vector<Cell *> removeCellList = opt.removedCells;

        for (auto cell : removeCellList) {
            recover_PR(cell);
            this->allMCells.erase(std::find(this->allMCells.begin(), this->allMCells.end(), cell));
            this->name2cell.erase(cell->name);
            delete cell;
            cell = nullptr;
        }

        if (DEBUG) std::cout << "Round: " << optQueue.size() << std::endl;
        // break; // TODO: Delete

    }





    f_out.close();

}

PlacementRow* PlacementLegalizer::get_PR_by_point(XYCoord point) {
    // Binary search to find the correct row
    auto row_it = std::lower_bound(allRows.begin(), allRows.end(), point.y,
        [](const Row* row, double y) {
            return row->get_LBY() + row->get_height() <= y;
        });

    // Check if point is within valid row range
    if (row_it == allRows.end() || point.y < (*row_it)->get_LBY())
        return nullptr;

    // Linear search within the found row's placement rows
    for (auto PR : (*row_it)->get_PRs()) {
        if (PR->startX() <= point.x && point.x <= PR->endX()) {
            return PR;
        }
    }
    return nullptr;
}

void PlacementLegalizer::remove_redundant_PRs() {
    for (auto &row : allRows) {
        for (auto PR : row->get_PRs()) {
            if (PR->getWidth() < this->mergedCellMinWidth) {
                row->erase_PR(PR);
                delete PR;
                PR = nullptr;
            }
        }
    }
}

PlacementRow* PlacementLegalizer::trim_PR(PlacementRow *PR, double leftX, double rightX) {
    PlacementRow* rightPR = nullptr;

    if (PR->startX() < leftX && rightX < PR->endX()) {
        // Right part
        int endNumOfSites = std::ceil((rightX - PR->startX()) / PR->siteWidth);
        rightPR = new PlacementRow(
            PR->getX(endNumOfSites),
            PR->startP.y,
            PR->siteWidth,
            PR->siteHeight,
            PR->totalNumOfSites - endNumOfSites
        );

        // Left part
        PR->totalNumOfSites = (leftX - PR->startX()) / PR->siteWidth;
    }
    else if (leftX <= PR->startX() && rightX < PR->endX()) {
        int endNumOfSites = std::ceil((rightX - PR->startX()) / PR->siteWidth);
        PR->startP.x = PR->getX(endNumOfSites);
        PR->totalNumOfSites -= endNumOfSites;
    }
    else if (PR->startX() < leftX && PR->endX() <= rightX) {
        PR->totalNumOfSites = (leftX - PR->startX()) / PR->siteWidth;
    }
    else {
        PR->totalNumOfSites = 0;
    }

    return rightPR;
}

void PlacementLegalizer::place_cell(Cell *cell) {
    XYCoord cellLB = cell->LB;
    XYCoord cellUR = cell->LB + XYCoord(cell->width, cell->height);

    Row *currentRow = this->rowLookup[cellLB.y];
    if (currentRow == nullptr) {
        // FIXME: Find the correct row if not
        std::cerr << "Warning: Cannot find the correct row" << std::endl;
    }

    while (currentRow->get_LBY() < cellUR.y) {

        // Use a copy of PRs instead of reference
        std::vector<PlacementRow *> PRs = currentRow->get_PRs();
        for (auto PR : PRs) {
            if (cellLB.x >= PR->endX() || cellUR.x <= PR->startX()) {
                continue;
            }

            PlacementRow *rightPR = trim_PR(PR, cellLB.x, cellUR.x);
            if (rightPR) {
                currentRow->add_PR(rightPR);
            }
            if (PR->totalNumOfSites == 0) {
                currentRow->erase_PR(PR);
                delete PR;
                PR = nullptr;
            }
        }

        currentRow = this->rowLookup[currentRow->get_LBY() + this->rowHeight];
    }
}

void PlacementLegalizer::recover_PR(Cell *cell) {
    int heightNum = std::ceil(cell->height / this->rowHeight);

    // For each row
    for (int i = 0; i < heightNum; i++) {
        Row *currentRow = this->rowLookup[cell->LB.y + i * this->rowHeight];

        double leftX   = cell->LB.x;
        double rightX  = cell->LB.x + std::ceil(cell->width / this->rowSiteWidth) * this->rowSiteWidth;
        PlacementRow *leftPR  = currentRow->get_PR_by_X(leftX);
        PlacementRow *rightPR = currentRow->get_PR_by_X(rightX);

        if (leftPR && rightPR) {
            std::cout << "case1" << "\n";
            leftPR->totalNumOfSites += rightPR->totalNumOfSites + int(std::ceil(cell->width / this->rowSiteWidth));
            currentRow->erase_PR(rightPR);
            delete rightPR;
            rightPR = nullptr;
        }
        else if (leftPR && !rightPR) {
            std::cout << "case2" << "\n";
            leftPR->totalNumOfSites += int(std::ceil(cell->width / this->rowSiteWidth));
        }
        else if (!leftPR && rightPR) {
            std::cout << "case3" << "\n";
            int addedNumOfSites = int(std::ceil(cell->width / this->rowSiteWidth));
            rightPR->totalNumOfSites += addedNumOfSites;
            rightPR->startP.x -= addedNumOfSites * rightPR->siteWidth;
        }
        else {
            std::cout << "case4" << "\n";
            PlacementRow *newPR = new PlacementRow(
                leftX,
                cell->LB.y + i * this->rowHeight,
                this->rowSiteWidth,
                this->rowHeight,
                int(std::ceil(cell->width / this->rowSiteWidth))
            );
            currentRow->add_PR(newPR);
        }

    }

}




bool PlacementLegalizer::is_placement_valid(XYCoord pointLB, Cell *cellToPlace) {
    // int heightNum = std::ceil(cellToPlace->height / this->rowHeight);
    // for (int i = 0; i < heightNum; i++) {
    //     double checkStartX = pointLB.x;
    //     double checkEndX   = pointLB.x + cellToPlace->width;
    //     double checkY      = pointLB.y + i * this->rowHeight;
    //     PlacementRow* leftPR  = get_PR_by_point(XYCoord(checkStartX, checkY));
    //     PlacementRow* rightPR = get_PR_by_point(XYCoord(checkEndX,   checkY));
    //     if (leftPR == nullptr || rightPR == nullptr || leftPR != rightPR) {
    //         return false;
    //     }

    //     std::vector<Cell *> mCellsInCurrentRow = this->rowLookup[checkY]->get_mCells();
    //     for (auto cell : mCellsInCurrentRow) {
    //         if (!(cell->LB.x + cell->width <= checkStartX || cell->LB.x >= checkEndX))
    //             return false;
    //     }
    // }
    return true;
}