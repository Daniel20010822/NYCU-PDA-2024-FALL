#include <iostream>
#include <string>
#include <sstream>
#include <algorithm>
#include <fstream>
#include <vector>
#include <queue>
#include <map>
#include <set>
#include <unordered_map>
#include <cmath>
#include <climits>
#include <cassert>
using namespace std;

#define DEBUG 0

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

    double mergedCellMinWidth = numeric_limits<double>::max();
    double rowHeight = 0;
    double rowSiteWidth = 0;

    struct OptOperation {
        Cell *mergedCell;
        vector<Cell *> removedCells;
        OptOperation(Cell *mergedCell, vector<Cell *> removedCells) : mergedCell(mergedCell), removedCells(removedCells) {}
    };

    queue<OptOperation> optQueue;


    // map<double, Row *> allRows;
    vector<Row *>  allRows;
    vector<Cell *> allFCells;
    vector<Cell *> allMCells;
    vector<Cell *> allMergedCells;
    vector<Cell *> allRemoveCells;

    unordered_map<string, Cell *> name2cell;
    unordered_map<Cell *, Row *>  cell2row;
    unordered_map<double, Row *>  rowLookup;


    PlacementRow* trim_PR(PlacementRow *PR, double leftX, double rightX);
    PlacementRow* get_PR_by_point(XYCoord point);
    bool is_placement_valid(XYCoord pointLB, Cell *mCell);
public:
    PlacementLegalizer() {}
    void parse_init_lg(string filename);
    void parse_opt(string filename);
    void write_lg(string filename);
    void place_fCells();
    void place_mCells();
    void remove_redundant_PRs();
    void place_single_cell(Cell *cell);
    void place_mergedCells(string filename);
};

void PlacementLegalizer::write_lg(string filename) {
    ofstream f_lg(filename);

    f_lg << "Alpha " << this->alpha << endl;
    f_lg << "Beta " << this->beta << endl;
    f_lg << "DieSize " << this->DieLB.x << " " << this->DieLB.y << " " << this->DieUR.x << " " << this->DieUR.y << endl;

    for (auto &row : allRows) {
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


    sort(PRs.begin(), PRs.end(), [](PlacementRow *a, PlacementRow *b) { return a->startP.y < b->startP.y; });


    for (size_t i = 0; i < PRs.size(); ++i) {
        PlacementRow* PR = PRs[i];
        this->allRows.push_back(new Row({PR}, PR->startP.y, PR->siteHeight));
        this->rowLookup[PR->startP.y] = allRows[i];
    }

    // Add mCells and fCells to corresponding rows
    for (auto &mCell : mCells) {
        Row *row = this->rowLookup[mCell->LB.y];
        row->add_mCell(mCell);
        allMCells.emplace_back(mCell);
        this->cell2row[mCell] = row;
    }
    for (auto &fCell : fCells) {
        Row *row = this->rowLookup[fCell->LB.y];
        row->add_fCell(fCell);
        allFCells.emplace_back(fCell);
        this->cell2row[fCell] = row;
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

        vector<Cell *> removeCellList;
        ss >> token;
        if (token == "Banking_Cell:") {
            vector<Cell *> tempList;
            // Read candidate cells until -->
            ss >> token;
            while (token != "-->") {
                tempList.push_back(this->name2cell[token]);
                this->allRemoveCells.push_back(this->name2cell[token]);
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

        // Store or process the parsed data as needed
        Cell* mergedCell = new Cell(mergedCellName, lbx, lby, w, h, false);
        this->allMergedCells.push_back(mergedCell);
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
void PlacementLegalizer::place_fCells() {
    for (auto cell : this->allFCells) {
        XYCoord cellLB = cell->LB;
        XYCoord cellUR = cell->LB + XYCoord(cell->width, cell->height);

        size_t startIdx = 0;
        auto it = std::lower_bound(allRows.begin(), allRows.end(), cellLB.y,
            [](const Row* row, double y) {
                return row->get_LBY() < y;
            });

        if (it != allRows.end()) {
            startIdx = std::distance(allRows.begin(), it);
        }

        for (size_t i = startIdx; i < allRows.size(); ++i) {
            Row* currentRow = allRows[i];
            if (currentRow->get_LBY() >= cellUR.y) {
                break;
            }

            // Use a copy of PRs instead of reference
            vector<PlacementRow *> PRs = currentRow->get_PRs();
            for (auto PR : PRs) {
                if (cellLB.x >= PR->endX() || cellUR.x <= PR->startX()) {
                    continue;
                }

                PlacementRow* rightPR = trim_PR(PR, cellLB.x, cellUR.x);
                if (rightPR) {
                    currentRow->add_PR(rightPR);
                }
                if (PR->totalNumOfSites == 0) {
                    currentRow->erase_PR(PR);
                    delete PR;
                    PR = nullptr;
                }
            }
        }
    }
}
void PlacementLegalizer::place_mCells() {
    for (auto cell : this->allMCells) {
        if (find(this->allRemoveCells.begin(), this->allRemoveCells.end(), cell) != this->allRemoveCells.end()) {
            continue;
        }
        XYCoord cellLB = cell->LB;
        XYCoord cellUR = cell->LB + XYCoord(cell->width, cell->height);

        size_t startIdx = 0;
        auto it = std::lower_bound(allRows.begin(), allRows.end(), cellLB.y,
            [](const Row* row, double y) {
                return row->get_LBY() < y;
            });

        if (it != allRows.end()) {
            startIdx = std::distance(allRows.begin(), it);
        }

        for (size_t i = startIdx; i < allRows.size(); ++i) {
            Row* currentRow = allRows[i];
            if (currentRow->get_LBY() >= cellUR.y) {
                break;
            }

            // Use a copy of PRs instead of reference
            vector<PlacementRow*> PRs = currentRow->get_PRs();
            for (auto PR : PRs) {
                if (cellLB.x >= PR->endX() || cellUR.x <= PR->startX()) {
                    continue;
                }

                PlacementRow* rightPR = trim_PR(PR, cellLB.x, cellUR.x);
                if (rightPR) {
                    currentRow->add_PR(rightPR);
                }
                if (PR->totalNumOfSites == 0) {
                    currentRow->erase_PR(PR);
                    delete PR;
                    PR = nullptr;
                }
            }
        }
    }
}
void PlacementLegalizer::place_single_cell(Cell *cell) {
    XYCoord cellLB = cell->LB;
    XYCoord cellUR = cell->LB + XYCoord(cell->width, cell->height);

    size_t startIdx = 0;
    auto it = std::lower_bound(allRows.begin(), allRows.end(), cellLB.y,
        [](const Row* row, double y) {
            return row->get_LBY() < y;
        });

    if (it != allRows.end()) {
        startIdx = std::distance(allRows.begin(), it);
    }

    for (size_t i = startIdx; i < allRows.size(); ++i) {
        Row* currentRow = allRows[i];
        if (currentRow->get_LBY() >= cellUR.y) {
            break;
        }

        // Use a copy of PRs instead of reference
        vector<PlacementRow*> PRs = currentRow->get_PRs();
        for (auto PR : PRs) {
            if (cellLB.x >= PR->endX() || cellUR.x <= PR->startX()) {
                continue;
            }

            PlacementRow* rightPR = trim_PR(PR, cellLB.x, cellUR.x);
            if (rightPR) {
                currentRow->add_PR(rightPR);
            }
            // if (PR->totalNumOfSites == 0) {
            //     currentRow->erase_PR(PR);
            //     delete PR;
            //     PR = nullptr;
            // }
        }
    }
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
void PlacementLegalizer::place_mergedCells(string filename) {
    ofstream f_out(filename);

    bool isStartFromBottom = true;

    int currentNumOfMergedCell = 0;
    int totalNumOfMergedCells = this->allMergedCells.size();
    Cell *mergedCell = this->optQueue.front().mergedCell;
    double upBound = allRows.back()->get_LBY() + allRows.back()->get_height();

    while (!this->optQueue.empty()) {
        bool isAbleToContinue = false;

        for (size_t i = 0; i < allRows.size(); ++i) {
            size_t index = isStartFromBottom ? i : allRows.size() - i - 1;
            Row* currentRow = allRows[index];

            if (DEBUG) cout << "Current Row LBY = " << currentRow->get_LBY() << " " << index << "/" << allRows.size() << endl;
            auto& PRs = currentRow->get_PRs_ref(); // Access PRs directly
            auto PR_it = PRs.begin();

            while (PR_it != PRs.end()) {
                PlacementRow* PR = *PR_it; // Current PlacementRow
                XYCoord placeLB = PR->startP;

                // Start from placeLB
                while (placeLB.x + mergedCell->width <= PR->endX() && placeLB.y + mergedCell->height <= upBound) {
                    if (is_placement_valid(placeLB, mergedCell)) {
                        isAbleToContinue = true;
                        // Get removed cells from queue and remove them
                        vector<Cell *> removedCells = this->optQueue.front().removedCells;
                        for (auto cell : removedCells) {
                            // recover_PR(cell);
                            Row *row = this->cell2row[cell];
                            row->erase_mCell(cell);
                            this->allMCells.erase(find(this->allMCells.begin(), this->allMCells.end(), cell));
                            this->name2cell.erase(cell->name);
                            this->cell2row.erase(cell);
                            delete cell;
                            cell = nullptr;
                        }
                        this->optQueue.pop();

                        // Place the merged cell
                        mergedCell->LB = placeLB;
                        place_single_cell(mergedCell);
                        currentRow->add_mCell(mergedCell);

                        f_out << mergedCell->LB.x << " " << mergedCell->LB.y << endl;
                        f_out << "0" << endl;


                        // Update index to next mergedCell
                        currentNumOfMergedCell++;
                        if (DEBUG) cout << "Round: " << currentNumOfMergedCell << "/" << totalNumOfMergedCells << endl;
                        if (this->optQueue.empty()) {
                            return;
                        }

                        // Get the next merged cell and move placeLB to the next position
                        mergedCell = this->optQueue.front().mergedCell;
                        placeLB.x += int(ceil(mergedCell->width / PR->siteWidth))*PR->siteWidth;
                    }
                    else {
                        placeLB.x += this->rowSiteWidth;
                    }
                }
                // Move to the next placement row
                ++PR_it;
            }
        }

        if (!isAbleToContinue) break;
    }


    if (!this->optQueue.empty()) {
        cout << "Warning: Not all merged cells are placed. Remaining: " << this->optQueue.size() << endl;
    }
    f_out.close();

}
bool PlacementLegalizer::is_placement_valid(XYCoord pointLB, Cell *cellToPlace) {
    int heightNum = ceil(cellToPlace->height / this->rowHeight);
    for (int i = 0; i < heightNum; i++) {
        double checkStartX = pointLB.x;
        double checkEndX   = pointLB.x + cellToPlace->width;
        double checkY      = pointLB.y + i * this->rowHeight;
        PlacementRow* leftPR  = get_PR_by_point(XYCoord(checkStartX, checkY));
        PlacementRow* rightPR = get_PR_by_point(XYCoord(checkEndX,   checkY));
        if (leftPR == nullptr || rightPR == nullptr || leftPR != rightPR) {
            return false;
        }

        vector<Cell *> mCellsInCurrentRow = this->rowLookup[checkY]->get_mCells();
        for (auto cell : mCellsInCurrentRow) {
            if (!(cell->LB.x + cell->width <= checkStartX || cell->LB.x >= checkEndX))
                return false;
        }
    }
    return true;
}


int main(int argc, char** argv) {

    if (argc < 4) {
        cerr << "Usage: " << argv[0] << " <init_lg_file> <opt_file> <post_lg_file>" << endl;
        return 1;
    }

    PlacementLegalizer LG;

    LG.parse_init_lg(argv[1]);
    LG.parse_opt(argv[2]);
    if (DEBUG) LG.write_lg("00_Init.lg");

    LG.place_fCells();
    LG.remove_redundant_PRs();
    LG.place_mCells();
    if (DEBUG) LG.write_lg("01_PlaceCells.lg");

    LG.place_mergedCells(argv[3]);
    if (DEBUG) LG.write_lg("02_PlaceMergedCells.lg");

    return 0;
}
