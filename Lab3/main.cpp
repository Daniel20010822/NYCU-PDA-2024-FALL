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

#define DEBUG 1

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
    struct Compare {
        bool operator()(const Cell* lhs, const Cell* rhs) const {
            return lhs->LB.x < rhs->LB.x;
        }
    };
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

class Row {
private:
    double LBY = 0;
    double height = 0;
    set<Cell *, Cell::Compare> mCells;
    set<Cell *, Cell::Compare> fCells;
    set<PlacementRow *, PlacementRow::Compare> PRs;
public:
    Row(vector<PlacementRow *> PRs, double LBY, double height) : LBY(LBY), height(height), PRs(PRs.begin(), PRs.end()) {}
    vector<PlacementRow *> get_PRs()       const { return vector<PlacementRow *>(PRs.begin(), PRs.end()); }
    vector<Cell *>         get_mCells()    const { return vector<Cell *>(mCells.begin(), mCells.end()); }
    vector<Cell *>         get_fCells()    const { return vector<Cell *>(fCells.begin(), fCells.end()); }
    double                 get_LBY()       const { return LBY; }
    double                 get_height()    const { return height; }
    void add_PR(PlacementRow *PR)          { PRs.insert(PR); }
    void add_mCell(Cell *mCell)            { mCells.insert(mCell); }
    void add_fCell(Cell *fCell)            { fCells.insert(fCell); }
    void erase_PR(PlacementRow *PR)        { PRs.erase(PR); }
    void erase_mCell(Cell *mCell)          { mCells.erase(mCell); }
    void clear_PRs()                       { PRs.clear(); }
    void clear_mCells()                    { mCells.clear(); }
    void clear_fCells()                    { fCells.clear(); }
};

class PlacementLegalizer {
private:
    XYCoord DieLB = {0,0};
    XYCoord DieUR = {0,0};
    double alpha = 0;
    double beta  = 0;


    map<double, Row *> allRows;
    vector<Cell *> allFCells;
    vector<Cell *> allMCells;

    unordered_map<string, Cell *> name2cell;
    unordered_map<Cell *, Row *>  cell2row;

    PlacementRow* trim_placement_row(PlacementRow *PR, double leftX, double rightX);
    PlacementRow* get_placement_row_by_point(XYCoord point);
    vector<Row*> search_local_region(Cell *mergedCell);
    bool is_mCell_inside_PR(Cell *mCell);
    // void write_post_lg(string filename);
public:
    PlacementLegalizer() {}
    void parse_init_lg(string filename);
    void parse_opt(string filename);
    void write_post_lg(string filename);
    void write_lg(string filename);
    void write_lg(string filename, vector<Row*> localRows, XYCoord LB, XYCoord UR);
    void write_lg(string filename, vector<PlacementRow *> localPRs, vector<Cell *> localMCells, vector<Cell *> localFCells, XYCoord LB, XYCoord UR);
    void place_fCells();
};

void PlacementLegalizer::write_lg(string filename) {
    ofstream f_lg(filename);

    f_lg << "Alpha " << this->alpha << endl;
    f_lg << "Beta " << this->beta << endl;
    f_lg << "DieSize " << this->DieLB.x << " " << this->DieLB.y << " " << this->DieUR.x << " " << this->DieUR.y << endl;

    for (auto &row : allRows) {
        for (auto mCell : row.second->get_mCells()) {
            f_lg << mCell->name << " " << mCell->LB.x << " " << mCell->LB.y << " " << mCell->width << " " << mCell->height << " " << "NOTFIX" << endl;
        }
        for (auto fCell : row.second->get_fCells()) {
            f_lg << fCell->name << " " << fCell->LB.x << " " << fCell->LB.y << " " << fCell->width << " " << fCell->height << " " << "FIX" << endl;
        }
        for (auto PR : row.second->get_PRs()) {
            f_lg << "PlacementRows " << int(PR->startP.x) << " " << int(PR->startP.y) << " " << int(PR->siteWidth) << " " << int(PR->siteHeight) << " " << PR->totalNumOfSites << endl;
        }
    }

    f_lg.close();
}
void PlacementLegalizer::write_lg(string filename, vector<PlacementRow *> localPRs, vector<Cell *> localMCells, vector<Cell *> localFCells, XYCoord LB, XYCoord UR) {
    ofstream f_lg(filename);

    f_lg << "Alpha " << this->alpha << endl;
    f_lg << "Beta " << this->beta << endl;
    f_lg << "DieSize " << LB.x << " " << LB.y << " " << UR.x << " " << UR.y << endl;


    for (auto cell : localMCells) {
        f_lg << cell->name << " " << cell->LB.x << " " << cell->LB.y << " " << cell->width << " " << cell->height << " " << "NOTFIX" << endl;
    }
    for (auto cell : localFCells) {
        f_lg << cell->name << " " << cell->LB.x << " " << cell->LB.y << " " << cell->width << " " << cell->height << " " << "FIX" << endl;
    }
    for (auto PR : localPRs) {
        f_lg << "PlacementRows " << int(PR->startP.x) << " " << int(PR->startP.y) << " " << int(PR->siteWidth) << " " << int(PR->siteHeight) << " " << PR->totalNumOfSites << endl;
    }

    f_lg.close();
}
void PlacementLegalizer::write_lg(string filename, vector<Row*> localRows, XYCoord LB, XYCoord UR) {
    ofstream f_lg(filename);

    f_lg << "Alpha " << this->alpha << endl;
    f_lg << "Beta " << this->beta << endl;
    f_lg << "DieSize " << LB.x << " " << LB.y << " " << UR.x << " " << UR.y << endl;

    for (auto &row : localRows) {
        for (auto mCell : row->get_mCells()) {
            f_lg << mCell->name << " " << mCell->LB.x << " " << mCell->LB.y << " " << mCell->width << " " << mCell->height << " " << "NOTFIX" << endl;
        }
        for (auto fCell : row->get_fCells()) {
            f_lg << fCell->name << " " << fCell->LB.x << " " << fCell->LB.y << " " << fCell->width << " " << fCell->height << " " << "FIX" << endl;
        }
        for (auto PR : row->get_PRs()) {
            f_lg << "PlacementRows " << int(PR->startP.x) << " " << int(PR->startP.y) << " " << int(PR->siteWidth) << " " << int(PR->siteHeight) << " " << PR->totalNumOfSites << endl;
        }
    }

    f_lg.close();
}
void PlacementLegalizer::parse_init_lg(string filename) {
    ifstream f_lg(filename);

    if (!f_lg.is_open()) {
        cerr << "Cannot open file: " << filename << endl;
        exit(1);
    }

    vector<Cell *> mCells;
    vector<Cell *> fCells;
    vector<PlacementRow *> PRs;

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
            PlacementRow* PR = new PlacementRow(startX, startY, siteWidth, siteHeight, totalNumOfSites);
            PRs.emplace_back(PR);
        }
        else {
            string cellName = type;
            string checkFIX;
            int lowerLeftX, lowerLeftY, width, height;
            f_lg >> lowerLeftX >> lowerLeftY >> width >> height >> checkFIX;

            Cell *newCell = new Cell(cellName, lowerLeftX, lowerLeftY, width, height, (checkFIX == "FIX"));
            if (newCell->isFixed)
                fCells.emplace_back(newCell);
            else
                mCells.emplace_back(newCell);
            this->name2cell[cellName] = newCell;
        }
    }

    // Initialize allRows
    for (auto &PR : PRs) {
        allRows[PR->startP.y] = new Row({PR}, PR->startP.y, PR->siteHeight);
    }
    // Add mCells and fCells to corresponding rows
    for (auto &mCell : mCells) {
        allRows[mCell->LB.y]->add_mCell(mCell);
        allMCells.emplace_back(mCell);
        this->cell2row[mCell] = allRows[mCell->LB.y];
    }
    for (auto &fCell : fCells) {
        allRows[fCell->LB.y]->add_fCell(fCell);
        allFCells.emplace_back(fCell);
        this->cell2row[fCell] = allRows[fCell->LB.y];
    }

    f_lg.close();
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
            vector<string> eraseCellList;

            // Read candidate cells until -->
            ss >> token;
            while (token != "-->") {
                eraseCellList.push_back(token);
                if (!(ss >> token)) {
                    getline(f_opt, line);
                    ss.clear();
                    ss.str(line);
                    ss >> token;
                }
            }

            // Read merged cell info
            ss >> mergedCellName >> lbx >> lby >> w >> h;

            // Remove candidate cells from its corresponding row
            for (auto cellName : eraseCellList) {
                Cell* cell = this->name2cell[cellName];
                allRows[cell->LB.y]->erase_mCell(cell);
                this->allMCells.erase(find(this->allMCells.begin(), this->allMCells.end(), cell));
                this->name2cell.erase(cellName);
                this->cell2row.erase(cell);
                delete cell;
                cell = nullptr;
            }

            for (auto erasedCell : eraseCellList) {
                cout << erasedCell << " ";
            }
            cout << "--> " << mergedCellName << endl;
        }

        // Store or process the parsed data as needed
        Cell* mergedCell = new Cell(mergedCellName, lbx, lby, w, h, false);
        allRows[lby]->add_mCell(mergedCell); //TODO: Check if lby is always on site
        allMCells.emplace_back(mergedCell);
        this->name2cell[mergedCellName] = mergedCell;
        this->cell2row[mergedCell] = allRows[lby];

        // Find local region
        vector<Row*> localRows = search_local_region(mergedCell);
        if (DEBUG) write_lg("LocalRegion.lg", localRows, XYCoord(this->DieLB), XYCoord(this->DieUR));

        // round++;
        // if (round == 2)
        //     break; // TODO: remove
    }


    f_opt.close();
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
    // // Find the first row with y-coordinate >= point.y
    // auto it = allPRs.lower_bound(point.y);

    // // If the point.y is exactly at the start of a row, use that row
    // // Otherwise, move to the previous row since rows cover [y, y+height)
    // if (it != allPRs.begin() && (it == allPRs.end() || it->first > point.y)) {
    //     --it;
    // }

    // if (it == allPRs.end()) return nullptr;

    // // Search X direction
    // for (auto PR : it->second) {
    //     if (PR->startX() <= point.x && point.x < PR->endX()) {
    //         return PR;
    //     }
    // }
    return nullptr;
}

vector<Row*> PlacementLegalizer::search_local_region(Cell *mergedCell) {

    XYCoord center = mergedCell->LB;
    double halfWidth = mergedCell->width * 20;
    double halfRowNum = 5;

    Row* centerRow = this->cell2row[mergedCell];

    double leftBound  = center.x - halfWidth;
    double rightBound = center.x + halfWidth;
    double upBound    = center.y + halfRowNum * centerRow->get_height();
    double downBound  = center.y - halfRowNum * centerRow->get_height();
    if (leftBound  < this->DieLB.x) {leftBound  = this->DieLB.x;}
    if (rightBound > this->DieUR.x) {rightBound = this->DieUR.x;}
    if (upBound    > this->DieUR.y) {upBound    = this->DieUR.y;}
    if (downBound  < this->DieLB.y) {downBound  = this->DieLB.y;}

    vector<Row*> localRows;
    vector<PlacementRow*> localPRs;
    vector<Cell*>         localMCells;
    vector<Cell*>         localFCells;

    // Find all placement rows in the local region
    for (auto it = allRows.lower_bound(downBound); it != allRows.upper_bound(upBound); ++it) {
        Row* newRow = new Row(*it->second); // Deep copy of the Row

        // Clear the copied row's containers since we'll add filtered copies
        newRow->clear_PRs();
        newRow->clear_mCells();
        newRow->clear_fCells();

        for (auto PR : it->second->get_PRs()) {
            if ((leftBound <= PR->startX() && PR->startX() <= rightBound) &&
                (downBound <= PR->startP.y && PR->startP.y + PR->siteHeight <= upBound)) {
                PlacementRow* newPR = new PlacementRow(
                    PR->startP.x, PR->startP.y,
                    PR->siteWidth, PR->siteHeight,
                    PR->totalNumOfSites
                );
                newRow->add_PR(newPR);
                localPRs.push_back(newPR);
            }
        }

        for (auto mCell : it->second->get_mCells()) {
            if (leftBound <= mCell->LB.x && mCell->LB.x <= rightBound &&
                downBound <= mCell->LB.y && mCell->LB.y + mCell->height <= upBound) {
                Cell* newMCell = new Cell(mCell->name, mCell->LB.x, mCell->LB.y, mCell->width, mCell->height, mCell->isFixed);
                newRow->add_mCell(newMCell);
                localMCells.push_back(newMCell);
            }
        }

        for (auto fCell : it->second->get_fCells()) {
            if (leftBound <= fCell->LB.x && fCell->LB.x <= rightBound &&
                downBound <= fCell->LB.y && fCell->LB.y + fCell->height <= upBound) {
                Cell* newFCell = new Cell(fCell->name, fCell->LB.x, fCell->LB.y, fCell->width, fCell->height, fCell->isFixed);
                newRow->add_fCell(newFCell);
                localFCells.push_back(newFCell);
            }
        }

        localRows.push_back(newRow);
    }

    return localRows;
}
void PlacementLegalizer::place_fCells() {
    for (auto fCell : allFCells) {
        XYCoord fCellLB = fCell->LB;
        XYCoord fCellUR = fCell->LB + XYCoord(fCell->width, fCell->height);

        auto it = allRows.lower_bound(fCellLB.y);
        while (it != allRows.end() && it->first < fCellUR.y) {
            Row* currentRow = it->second;
            for (auto PR : currentRow->get_PRs()) {
                if (fCellLB.x >= PR->endX() || fCellUR.x <= PR->startX())
                    continue;

                PlacementRow* rightPR = trim_placement_row(PR, fCellLB.x, fCellUR.x);
                if (rightPR) {
                    currentRow->add_PR(rightPR);
                }
                if (PR->totalNumOfSites == 0) {
                    currentRow->erase_PR(PR);
                    delete PR;
                    PR = nullptr;
                }
            }
            ++it;
        }
    }
}




int main(int argc, char** argv) {
    // Check command line arguments
    if (argc < 4) {
        cerr << "Usage: " << argv[0] << " <init_lg_file> <opt_file> <post_lg_file>" << endl;
        return 1;
    }

    PlacementLegalizer LG;

    LG.parse_init_lg(argv[1]);
    if (DEBUG) LG.write_lg("ParseInit.lg");
    LG.place_fCells();
    if (DEBUG) LG.write_lg("PlaceFCells.lg");
    LG.parse_opt(argv[2]);
    if (DEBUG) LG.write_lg("ParseOpt.lg");

    return 0;
}


