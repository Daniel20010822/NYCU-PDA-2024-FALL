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

// TODO: 先塞完 -> 把opt裡面mCell要刪掉的刪掉 -> 把mCell空下的位置復原 -> 繼續擺剩下的mergedCell

// Strategy 1: mCell和fCell都要切PR
// recover_placement_row

// Strategy 2: mCell不要切PR，只有fCell切PR
// MCell不要切PR，把他存在Row裡面
// 但是要找到可以確定不會Overlap的機制 -> line: 531

// // TODO:
// 1. Delete merged cells
// 2. Push all single height cells to the left to make room
// 3. Place again

// TODO: Integrate parse_opt and this
// TODO: Add recover_PR() when deleting candidate cells
// bool isPlaceDone = false;

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
    double getWidth() const { return siteWidth*totalNumOfSites; }

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
    // Getter
    set<PlacementRow *, PlacementRow::Compare>& get_PRs_ref() { return PRs; }
    vector<PlacementRow *> get_PRs()       const { return vector<PlacementRow *>(PRs.begin(), PRs.end()); }
    vector<Cell *>         get_mCells()    const { return vector<Cell *>(mCells.begin(), mCells.end()); }
    vector<Cell *>         get_fCells()    const { return vector<Cell *>(fCells.begin(), fCells.end()); }
    double                 get_LBY()       const { return LBY; }
    double                 get_height()    const { return height; }
    // Adder
    void add_PR(PlacementRow *PR)          { PRs.insert(PR); }
    void add_mCell(Cell *mCell)            { mCells.insert(mCell); }
    void add_fCell(Cell *fCell)            { fCells.insert(fCell); }
    // Eraser
    void erase_PR(PlacementRow *PR)        { PRs.erase(PR); }
    void erase_mCell(Cell *mCell)          { mCells.erase(mCell); }
    // Clear
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
    int movedCells = 0;

    double rowHeight = 0;
    double rowSiteWidth = 0;

    map<double, Row *> allRows;
    vector<Cell *> allFCells;
    vector<Cell *> allMCells;
    vector<Cell *> allmergedCells;

    unordered_map<string, Cell *> name2cell;
    unordered_map<Cell *, Row *>  cell2row;

    vector<Cell *> removeCellList;


    PlacementRow* trim_placement_row(PlacementRow *PR, double leftX, double rightX);
    void recover_placement_row(Cell *removedCell);
    PlacementRow* get_placement_row_by_point(XYCoord point);
    bool is_placement_valid(XYCoord pointLB, Cell *mCell);
public:
    PlacementLegalizer() {}
    void parse_init_lg(string filename);
    void parse_opt(string filename);
    void write_lg(string filename);
    void write_post_lg(string filename);
    void place_cells();
    void place_single_cell(Cell *cell);
    void place_mergedCells();
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
            this->rowHeight = siteHeight;
            this->rowSiteWidth = siteWidth;
        }
        else {
            string cellName = type;
            string checkFIX;
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

    string line;
    while (getline(f_opt, line)) {
        if (line.empty()) continue;

        stringstream ss(line);
        string token;
        string mergedCellName;
        double lbx, lby, w, h;

        ss >> token;
        if (token == "Banking_Cell:") {
            // Read candidate cells until -->
            ss >> token;
            while (token != "-->") {
                this->removeCellList.push_back(this->name2cell[token]);
                if (!(ss >> token)) {
                    getline(f_opt, line);
                    ss.clear();
                    ss.str(line);
                    ss >> token;
                }
            }

            // Read merged cell info
            ss >> mergedCellName >> lbx >> lby >> w >> h;

            // // Remove candidate cells from its corresponding row
            // for (auto cellName : eraseCellList) {
            //     Cell* cell = this->name2cell[cellName];
            //     allRows[cell->LB.y]->erase_mCell(cell);
            //     this->allMCells.erase(find(this->allMCells.begin(), this->allMCells.end(), cell));
            //     this->name2cell.erase(cellName);
            //     this->cell2row.erase(cell);
            //     delete cell;
            //     cell = nullptr;
            // }

            // for (auto erasedCell : eraseCellList) {
            //     cout << erasedCell << " ";
            // }
            // cout << "--> " << mergedCellName << endl;
        }

        // Store or process the parsed data as needed
        Cell* mergedCell = new Cell(mergedCellName, lbx, lby, w, h, false);
        this->allmergedCells.push_back(mergedCell);
        this->name2cell[mergedCellName] = mergedCell;
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
void PlacementLegalizer::place_cells() {
    for (auto cell : allFCells) {
        XYCoord cellLB = cell->LB;
        XYCoord cellUR = cell->LB + XYCoord(cell->width, cell->height);

        auto it = allRows.lower_bound(cellLB.y);
        while (it != allRows.end() && it->first < cellUR.y) {
            Row* currentRow = it->second;
            for (auto PR : currentRow->get_PRs()) {
                if (cellLB.x >= PR->endX() || cellUR.x <= PR->startX())
                    continue;

                PlacementRow* rightPR = trim_placement_row(PR, cellLB.x, cellUR.x);
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
    for (auto cell : allMCells) {
        XYCoord cellLB = cell->LB;
        XYCoord cellUR = cell->LB + XYCoord(cell->width, cell->height);

        auto it = allRows.lower_bound(cellLB.y);
        while (it != allRows.end() && it->first < cellUR.y) {
            Row* currentRow = it->second;
            for (auto PR : currentRow->get_PRs()) {
                if (cellLB.x >= PR->endX() || cellUR.x <= PR->startX())
                    continue;

                PlacementRow* rightPR = trim_placement_row(PR, cellLB.x, cellUR.x);
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
void PlacementLegalizer::place_single_cell(Cell *cell) {
    XYCoord cellLB = cell->LB;
    XYCoord cellUR = cell->LB + XYCoord(cell->width, cell->height);

    auto it = allRows.lower_bound(cellLB.y);
    while (it != allRows.end() && it->first < cellUR.y) {
        Row* currentRow = it->second;
        for (auto PR : currentRow->get_PRs()) {
            if (cellLB.x >= PR->endX() || cellUR.x <= PR->startX())
                continue;

            PlacementRow* rightPR = trim_placement_row(PR, cellLB.x, cellUR.x);
            if (rightPR) {
                currentRow->add_PR(rightPR);
            }
            // if (PR->totalNumOfSites == 0) {
            //     currentRow->erase_PR(PR);
            //     delete PR;
            //     PR = nullptr;
            // }
        }
        ++it;
    }
}
void PlacementLegalizer::recover_placement_row(Cell *removedCell) {
    // Row *row = this->cell2row[removedCell];
    // XYCoord LB = removedCell->LB;
    // XYCoord RB = removedCell->LB + XYCoord(removedCell->width, 0);
}




PlacementRow* PlacementLegalizer::get_placement_row_by_point(XYCoord point) {
    // Find the first row with y-coordinate >= point.y
    auto it = allRows.begin();
    if (point.y < it->second->get_LBY())
        return nullptr;
    while (point.y >= it->second->get_LBY() + it->second->get_height() && it != allRows.end()) {
        ++it;
    }
    if (point.y >= it->second->get_LBY() + it->second->get_height())
        return nullptr;
    // Search X direction
    for (auto PR : it->second->get_PRs()) {
        if (PR->startX() <= point.x && point.x <= PR->endX()) {
            return PR;
        }
    }
    return nullptr;
}

void PlacementLegalizer::place_mergedCells() {


    size_t mergedCellIndex = 0;
    Cell *mergedCell = this->allmergedCells[mergedCellIndex];
    double upBound = allRows.rbegin()->second->get_LBY() + allRows.rbegin()->second->get_height();

    // Start from bottom
    auto it = allRows.begin();
    for (size_t i = 0; i < allRows.size(); ++i, ++it) {
        if (DEBUG) cout << "Current Row LBY = " << it->second->get_LBY() << " " << i + 1 << "/" << allRows.size() << endl;
        auto& PRs = it->second->get_PRs_ref(); // Access PRs directly
        auto PR_it = PRs.begin();
        while (PR_it != PRs.end()) { // Dynamic iteration
            PlacementRow* PR = *PR_it; // Current PlacementRow
            XYCoord placeLB = PR->startP;
            // Start from placeLB
            while (placeLB.x + mergedCell->width <= PR->endX() && placeLB.y + mergedCell->height <= upBound) {
                if (is_placement_valid(placeLB, mergedCell)) {
                    // Place the merged cell
                    mergedCell->LB = placeLB;
                    place_single_cell(mergedCell);
                    it->second->add_mCell(mergedCell);

                    // Move placeLB to the next position
                    placeLB.x += ceil(mergedCell->width / PR->siteWidth)*PR->siteWidth;

                    // Update index to next mergedCell
                    mergedCellIndex++;
                    if (DEBUG) cout << "Round: " << mergedCellIndex << "/" << this->allmergedCells.size() << endl;
                    // if (mergedCellIndex == 5) {
                    //     return;
                    // }
                    if (mergedCellIndex >= this->allmergedCells.size()) {
                        for (auto cell : this->removeCellList) {
                            allRows[cell->LB.y]->erase_mCell(cell);
                            this->allMCells.erase(find(this->allMCells.begin(), this->allMCells.end(), cell));
                            this->name2cell.erase(cell->name);
                            this->cell2row.erase(cell);
                            delete cell;
                            cell = nullptr;
                        }
                        return;
                    }
                    mergedCell = this->allmergedCells[mergedCellIndex];
                }
                else {
                    placeLB.x += this->rowSiteWidth;
                }
            }
            // Move to the next placement row
            ++PR_it;
        }
    }

    if (mergedCellIndex < this->allmergedCells.size()) {
        cout << "Warning: Not all merged cells are placed" << endl;
    }
    for (auto cell : this->removeCellList) {
        allRows[cell->LB.y]->erase_mCell(cell);
        this->allMCells.erase(find(this->allMCells.begin(), this->allMCells.end(), cell));
        this->name2cell.erase(cell->name);
        this->cell2row.erase(cell);
        delete cell;
        cell = nullptr;
    }


}
bool PlacementLegalizer::is_placement_valid(XYCoord pointLB, Cell *mCell) {
    int heightNum = ceil(mCell->height / this->rowHeight);
    for (int i = 0; i < heightNum; i++) {
        XYCoord leftCheckPoint  = XYCoord(pointLB.x,                pointLB.y + i * this->rowHeight);
        XYCoord rightCheckPoint = XYCoord(pointLB.x + mCell->width, pointLB.y + i * this->rowHeight);
        PlacementRow* leftPR  = get_placement_row_by_point(leftCheckPoint);
        PlacementRow* rightPR = get_placement_row_by_point(rightCheckPoint);
        // TODO: After recovering PR -> if leftPR.endX() == rightPR.startX() will consider true
        if (leftPR == nullptr || rightPR == nullptr || leftPR != rightPR) {
            return false;
        }
    }
    return true;
}


void PlacementLegalizer::write_post_lg(string filename) {
    ofstream f_out(filename);

    for (auto mergedCell : this->allmergedCells) {
        f_out << mergedCell->LB.x << " " << mergedCell->LB.y << endl;
        f_out << "0" << endl;
    }
    f_out.close();
}


int main(int argc, char** argv) {
    // Check command line arguments
    if (argc < 4) {
        cerr << "Usage: " << argv[0] << " <init_lg_file> <opt_file> <post_lg_file>" << endl;
        return 1;
    }

    PlacementLegalizer LG;

    LG.parse_init_lg(argv[1]);
    LG.parse_opt(argv[2]);
    if (DEBUG) LG.write_lg("00_Init.lg");

    LG.place_cells();
    if (DEBUG) LG.write_lg("01_PlaceCells.lg");

    LG.place_mergedCells();
    if (DEBUG) LG.write_lg("02_PlaceMergedCells.lg");

    LG.write_post_lg(argv[3]);
    return 0;
}
