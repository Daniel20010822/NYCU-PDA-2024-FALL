#ifndef _PLACEMENT_LEGALIZER_H_
#define _PLACEMENT_LEGALIZER_H_

#include "Debug.h"
#include "XYCoord.h"
#include "Cell.h"
#include "Row.h"
#include "PR.h"

#include <cmath>
#include <string>
#include <sstream>
#include <fstream>
#include <iostream>
#include <queue>
#include <vector>
#include <unordered_map>
#include <algorithm>
#include <limits>


class PlacementLegalizer {
private:
    XYCoord DieLB = {0,0};
    XYCoord DieUR = {0,0};
    double alpha = 0;
    double beta  = 0;

    double mergedCellMinWidth = std::numeric_limits<double>::max();
    double rowHeight = 0;
    double rowSiteWidth = 0;

    struct OptOperation {
        Cell *mergedCell;
        std::vector<Cell *> removedCells;
        OptOperation(Cell *mergedCell, std::vector<Cell *> removedCells) : mergedCell(mergedCell), removedCells(removedCells) {}
    };

    std::queue<OptOperation> optQueue;


    // map<double, Row *> allRows;
    std::vector<Row *>  allRows;
    std::vector<Cell *> allFCells;
    std::vector<Cell *> allMCells;
    std::vector<Cell *> allMergedCells;
    std::vector<Cell *> allRemoveCells;

    std::unordered_map<std::string, Cell *> name2cell;
    std::unordered_map<Cell *, Row *>  cell2row;
    std::unordered_map<double, Row *>  rowLookup;


    PlacementRow* trim_PR(PlacementRow *PR, double leftX, double rightX);
    PlacementRow* get_PR_by_point(XYCoord point);
    bool is_placement_valid(XYCoord pointLB, Cell *mCell);
public:
    PlacementLegalizer() {}
    void parse_init_lg(std::string filename);
    void parse_opt(std::string filename);
    void write_lg(std::string filename);
    void place_fCells();
    void place_mCells();
    void remove_redundant_PRs();
    void place_single_cell(Cell *cell);
    void place_mergedCells(std::string filename);
};

#endif