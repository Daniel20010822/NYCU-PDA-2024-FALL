#include <iostream>
#include <string>
#include <sstream>
#include <algorithm>
#include <fstream>
#include <vector>
#include <map>
#include <set>
#include <unordered_map>
#include <cmath>
#include <climits>
#include <cassert>
using namespace std;

class XYCoord {
public:
    double x = 0;
    double y = 0;
    XYCoord (double inX = 0, double inY = 0) : x(inX), y(inY) {}
    XYCoord operator+(const XYCoord& other) const {
        return XYCoord(x + other.x, y + other.y);
    }
};

class Cell {
public:
    string name;
    XYCoord LB = {0,0};
    double width  = 0;
    double height = 0;
    bool isFixed = false;
    Cell(string cellName, int lowerLeftX, int lowerLeftY, int width, int height, bool isFixed) :
        name(cellName), LB(lowerLeftX, lowerLeftY), width(width), height(height), isFixed(isFixed) {}
};

class PlacementRow {
public:
    XYCoord startP = {0,0};
    double siteWidth  = 1;
    double siteHeight = 0;
    int totalNumOfSites = 0;
    PlacementRow() {}
    PlacementRow(int startX, int startY, int siteWidth, int siteHeight, int totalNumOfSites) :
        startP(startX, startY), siteWidth(siteWidth), siteHeight(siteHeight), totalNumOfSites(totalNumOfSites) {}
    double startX() const { return startP.x; }
    double endX()   const { return startP.x + (siteWidth * totalNumOfSites); }
    double getX(int index) const { return startP.x + (siteWidth * index); }

    struct Compare {
        bool operator()(const PlacementRow* lhs, const PlacementRow* rhs) const {
            return lhs->startX() < rhs->startX();
        }
    };
};




class PlacementLegalizer {
private:
    XYCoord DieLB = {0,0};
    XYCoord DieUR = {0,0};
    double alpha = 0;
    double beta  = 0;

    // TODO: Use the same fashion as in allPRs for all cells
    //
    vector<Cell *> fCells; // fixed
    vector<Cell *> mCells; // movable
    unordered_map<string, Cell *> name2cell;

    map<double, set<PlacementRow *, PlacementRow::Compare>> allPRs;


    PlacementRow* trim_placement_row(PlacementRow *PR, double leftX, double rightX);
    PlacementRow* get_placement_row_by_point(XYCoord point);
    void search_local_region(Cell *mergedCell);
    void write_placement_rows(string filename);
    void write_local_region(string filename, vector<PlacementRow *> localPRs, vector<Cell *> localMCells, vector<Cell *> localFCells, XYCoord LB, XYCoord UR);
    // void write_post_lg(string filename);
public:
    PlacementLegalizer() {}
    void parse_init_lg(string filename);
    void parse_opt(string filename);
    void write_post_lg(string filename);
    void place_fCells();
};

void PlacementLegalizer::write_placement_rows(string filename) {
    ofstream f_pr(filename);
    for (const auto& pair : this->allPRs) {
        for (auto &PR : pair.second) {
            f_pr << int(PR->startP.x) << " "
                 << int(PR->startP.y) << " "
                 << int(PR->siteWidth*PR->totalNumOfSites) << " "
                 << int(PR->siteHeight) << endl;
        }
    }
    f_pr.close();
}
void PlacementLegalizer::write_local_region(string filename, vector<PlacementRow *> localPRs, vector<Cell *> localMCells, vector<Cell *> localFCells, XYCoord LB, XYCoord UR) {
    ofstream f_lr(filename);

    f_lr << "Alpha " << this->alpha << endl;
    f_lr << "Beta " << this->beta << endl;
    f_lr << "DieSize " << LB.x << " " << LB.y << " " << UR.x << " " << UR.y << endl;


    for (auto cell : localMCells) {
        f_lr << cell->name << " " << cell->LB.x << " " << cell->LB.y << " " << cell->width << " " << cell->height << " " << "NOTFIX" << endl;
    }
    for (auto cell : localFCells) {
        f_lr << cell->name << " " << cell->LB.x << " " << cell->LB.y << " " << cell->width << " " << cell->height << " " << "FIX" << endl;
    }
    for (auto PR : localPRs) {
        f_lr << "PlacementRows " << int(PR->startP.x) << " " << int(PR->startP.y) << " " << int(PR->siteWidth) << " " << int(PR->siteHeight) << " " << PR->totalNumOfSites << endl;
    }

    f_lr.close();
}
void PlacementLegalizer::parse_init_lg(string filename) {
    ifstream f_lg(filename);

    if (!f_lg.is_open()) {
        cerr << "Cannot open file: " << filename << endl;
        exit(1);
    }

    string type;
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
            PlacementRow* pr = new PlacementRow(startX, startY, siteWidth, siteHeight, totalNumOfSites);
            allPRs[startY].insert(pr);
        }
        else {
            string cellName = type;
            string checkFIX;
            int lowerLeftX, lowerLeftY, width, height;
            f_lg >> lowerLeftX >> lowerLeftY >> width >> height >> checkFIX;

            Cell *newCell = new Cell(cellName, lowerLeftX, lowerLeftY, width, height, (checkFIX == "FIX"));
            if (newCell->isFixed)
                this->fCells.emplace_back(newCell);
            else
                this->mCells.emplace_back(newCell);
            this->name2cell[cellName] = newCell;
        }
    }

    f_lg.close();
}
PlacementRow* PlacementLegalizer::trim_placement_row(PlacementRow *PR, double leftX, double rightX) {
    PlacementRow* rightPR = nullptr;

    if (PR->startX() < leftX && rightX < PR->endX()) {
        // Right part
        int endNumOfSites = ceil((rightX - PR->startX()) / PR->siteWidth);
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
        int endNumOfSites = ceil((rightX - PR->startX()) / PR->siteWidth);
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
PlacementRow* PlacementLegalizer::get_placement_row_by_point(XYCoord point) {
    // Find the first row with y-coordinate >= point.y
    auto it = allPRs.lower_bound(point.y);

    // If the point.y is exactly at the start of a row, use that row
    // Otherwise, move to the previous row since rows cover [y, y+height)
    if (it != allPRs.begin() && (it == allPRs.end() || it->first > point.y)) {
        --it;
    }

    if (it == allPRs.end()) return nullptr;

    // Search X direction
    for (auto PR : it->second) {
        if (PR->startX() <= point.x && point.x < PR->endX()) {
            return PR;
        }
    }
    return nullptr;
}
void PlacementLegalizer::search_local_region(Cell *mergedCell) {

    XYCoord center = mergedCell->LB;
    double halfWidth = mergedCell->width * 20;
    double halfRowNum = 5;
    // Retrieve the relevant PlacementRow based on the center.y coordinate
    PlacementRow* PR = get_placement_row_by_point(center);
    if (!PR) {
        cerr << "(" << center.x << "," << center.y << "): Cannot find PlacementRow for the given center point." << endl;
        return;
    }
    // cout << "PR: " << PR->startP.x << " " << PR->startP.y << " " << PR->siteWidth*PR->totalNumOfSites << " " << PR->siteHeight << endl;

    double leftBound  = center.x - halfWidth;
    double rightBound = center.x + halfWidth;
    double upBound    = center.y + halfRowNum * PR->siteHeight;
    double downBound  = center.y - halfRowNum * PR->siteHeight;
    if (leftBound  < this->DieLB.x) {leftBound  = this->DieLB.x;}
    if (rightBound > this->DieUR.x) {rightBound = this->DieUR.x;}
    if (upBound    > this->DieUR.y) {upBound    = this->DieUR.y;}
    if (downBound  < this->DieLB.y) {downBound  = this->DieLB.y;}

    vector<PlacementRow *> localPRs;
    vector<Cell *>         localMCells;
    vector<Cell *>         localFCells;

    // Find all placement rows in the local region
    for (auto it = allPRs.lower_bound(downBound); it != allPRs.upper_bound(upBound); ++it) {
        for (auto PR : it->second) {
            if ((leftBound <= PR->startX() && PR->startX() <= rightBound) &&
                (downBound <= PR->startP.y && PR->startP.y + PR->siteHeight <= upBound)) {
                localPRs.push_back(PR);
            }
        }
    }
    // Find all cells in the local region
    for (auto cell : this->mCells) {
        if (leftBound <= cell->LB.x && cell->LB.x <= rightBound &&
            downBound <= cell->LB.y && cell->LB.y + cell->height <= upBound) {
            localMCells.push_back(cell);
        }
    }
    for (auto cell : this->fCells) {
        if (leftBound <= cell->LB.x && cell->LB.x + cell->width <= rightBound &&
            downBound <= cell->LB.y && cell->LB.y + cell->height <= upBound) {
            localFCells.push_back(cell);
        }
    }

    write_local_region("LocalRegion.lg", localPRs, localMCells, localFCells, XYCoord(leftBound, downBound), XYCoord(rightBound, upBound));

    // TODO:
}
void PlacementLegalizer::place_fCells() {
    for (auto cell : this->fCells) {
        XYCoord cellLB = cell->LB;
        XYCoord cellUR = cell->LB + XYCoord(cell->width, cell->height);

        // Y of placement row:       it->first
        // vector of placement rows: it->second
        auto it = allPRs.lower_bound(cellLB.y);
        while (it != allPRs.end() && it->first < cellUR.y) {
            for (auto PR : it->second) {
                // Skip if the cell is not in the placement row
                if (cellLB.x >= PR->endX() || cellUR.x <= PR->startX())
                    continue;

                PlacementRow* rightPR = trim_placement_row(PR, cellLB.x, cellUR.x);
                if (rightPR) {
                    allPRs[it->first].insert(rightPR);
                }

                if (PR->totalNumOfSites == 0) {
                    it->second.erase(PR);
                    delete PR;
                    PR = nullptr;
                }
            }
            ++it;
        }
    }

    // // Assert that all placement rows do not overlap
    // for (const auto& pair : this->allPRs) {
    //     if (pair.second.size() < 2) continue;

    //     auto it = pair.second.begin();
    //     auto next = std::next(it);

    //     while (next != pair.second.end()) {
    //         if ((*it)->endX() > (*next)->startX()) {
    //             cout << "Overlapping placement rows: "
    //                  << (*it)->startX()   << " " << (*it)->endX()    << " "
    //                  << (*next)->startX() << " " << (*next)->endX()  << endl;
    //         }
    //         ++it;
    //         ++next;
    //     }
    // }

    // write out allPRs to "PlacementRows.blocks"

}
void PlacementLegalizer::parse_opt(string filename) {
    ifstream f_opt(filename);

    if (!f_opt.is_open()) {
        cerr << "Cannot open file: " << filename << endl;
        exit(1);
    }

    int round = 0;
    string line;
    while (getline(f_opt, line)) {
        if (line.empty()) continue;

        stringstream ss(line);
        string token;
        string mergedCellName;
        double lbx, lby, w, h;

        ss >> token;
        if (token == "Banking_Cell:") {
            vector<string> candidateCells;

            // Read candidate cells until -->
            ss >> token;
            while (token != "-->") {
                candidateCells.push_back(token);
                if (!(ss >> token)) {
                    getline(f_opt, line);
                    ss.clear();
                    ss.str(line);
                    ss >> token;
                }
            }

            // Read merged cell info
            ss >> mergedCellName >> lbx >> lby >> w >> h;

            // Remove candidate cells from mCells
            for (auto cellName : candidateCells) {
                auto it = find(this->mCells.begin(), this->mCells.end(), this->name2cell[cellName]);
                if (it != this->mCells.end()) {
                    this->mCells.erase(it);
                }
            }
            // Remove candidate cells from name2cell
            for (auto cellName : candidateCells) {
                this->name2cell.erase(cellName);
            }
        }

        // Store or process the parsed data as needed
        Cell* mergedCell = new Cell(mergedCellName, lbx, lby, w, h, false);
        this->mCells.emplace_back(mergedCell);
        this->name2cell[mergedCellName] = mergedCell;


        // Find local region
        search_local_region(mergedCell);
        round++;
        if (round == 2)
            break; // TODO: remove
    }


    f_opt.close();
}



int main(int argc, char** argv) {
    // Check command line arguments
    if (argc < 4) {
        cerr << "Usage: " << argv[0] << " <init_lg_file> <opt_file> <post_lg_file>" << endl;
        return 1;
    }

    PlacementLegalizer LG;

    LG.parse_init_lg(argv[1]);
    LG.place_fCells();
    LG.parse_opt(argv[2]);

    return 0;
}