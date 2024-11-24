#ifndef _ROW_H_
#define _ROW_H_

#include "Cell.h"
#include "PR.h"
#include <set>
#include <vector>


class Row {
private:
    double LBY    = 0;
    double height = 0;
    // std::set<Cell *, Cell::Compare> mCells;
    // std::set<Cell *, Cell::Compare> fCells;
    std::set<PlacementRow *, PlacementRow::Compare> PRs;
public:
    Row(std::vector<PlacementRow *> PRs, double LBY, double height) : LBY(LBY), height(height), PRs(PRs.begin(), PRs.end()) {}

    // Getter
    std::set<PlacementRow *, PlacementRow::Compare>& get_PRs_ref() { return PRs; }
    std::vector<PlacementRow *> get_PRs()             const { return std::vector<PlacementRow *>(PRs.begin(), PRs.end()); }
    PlacementRow*               get_PR_by_X(double X) const;
    double                      get_LBY()             const { return LBY; }
    double                      get_height()          const { return height; }

    // Adder
    void add_PR(PlacementRow *PR)          { PRs.insert(PR); }

    // Eraser
    void erase_PR(PlacementRow *PR)        { PRs.erase(PR); }

    // Clear
    void clear_PRs()                       { PRs.clear(); }

    // std::vector<Cell *>         get_mCells()    const { return std::vector<Cell *>(mCells.begin(), mCells.end()); }
    // std::vector<Cell *>         get_fCells()    const { return std::vector<Cell *>(fCells.begin(), fCells.end()); }
    // void add_mCell(Cell *mCell)            { mCells.insert(mCell); }
    // void add_fCell(Cell *fCell)            { fCells.insert(fCell); }
    // void erase_mCell(Cell *mCell)          { mCells.erase(mCell);  }
    // void clear_mCells()                    { mCells.clear(); }
    // void clear_fCells()                    { fCells.clear(); }

};

#endif