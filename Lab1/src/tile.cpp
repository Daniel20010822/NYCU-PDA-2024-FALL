#include "tile.h"
#include "debug_config.h"
#include <algorithm>

Tile *point_finding(const XYCoord searchPoint, Tile *startingTile) {
    // Q: SearchPoint out of range may cause Segmentation fault?
    Tile *currentTile = startingTile;
    int searchRnd = 0;
    debug << "point_finding(" << searchPoint.x << "," << searchPoint.y << "): " << get_tile_info(startingTile) << " -> ";
    while (!(currentTile->point_bl.x <= searchPoint.x && searchPoint.x < currentTile->point_tr.x &&
             currentTile->point_bl.y <= searchPoint.y && searchPoint.y < currentTile->point_tr.y)) {

        if (searchRnd > MAX_ITER){
            cerr << "point_finding(): Stop at " << MAX_ITER << " iterations." << endl;
            exit(1);
        }

        // 1. Move up/down
        while (!(currentTile->point_bl.y <= searchPoint.y && searchPoint.y < currentTile->point_tr.y)) {
            if (searchPoint.y >= currentTile->point_tr.y) {
                currentTile = currentTile->rt;
                debug << "U -> "<< get_tile_info(currentTile) << " -> ";
            }
            else if (searchPoint.y < currentTile->point_bl.y) {
                currentTile = currentTile->lb;
                debug << "D -> "<< get_tile_info(currentTile) << " -> ";
            }
        }

        // 2. Move left/right
        while(!(currentTile->point_bl.x <= searchPoint.x && searchPoint.x < currentTile->point_tr.x)) {
            if (searchPoint.x < currentTile->point_bl.x) {
                currentTile = currentTile->bl;
                debug << "L -> "<< get_tile_info(currentTile) << " -> ";
            }
            else if (searchPoint.x >= currentTile->point_tr.x) {
                currentTile = currentTile->tr;
                debug << "R -> "<< get_tile_info(currentTile) << " -> ";
            }
        }
        searchRnd += 1;
    }
    debug << "done" << endl;
    return currentTile;
}

Tile *point_finding_invert_y(const XYCoord searchPoint, Tile *startingTile) {
    Tile *currentTile = startingTile;
    int searchRnd = 0;
    debug << "point_finding_invert_y(" << searchPoint.x << "," << searchPoint.y << "): " << get_tile_info(startingTile) << " -> ";
    while (!(currentTile->point_bl.x <= searchPoint.x && searchPoint.x <  currentTile->point_tr.x &&
             currentTile->point_bl.y <  searchPoint.y && searchPoint.y <= currentTile->point_tr.y)) {

        if (searchRnd > MAX_ITER){
            cerr << "point_finding_invert_y(): Stop at " << MAX_ITER << " iterations." << endl;
            exit(1);
        }

        // 1. Move up/down
        while (!(currentTile->point_bl.y < searchPoint.y && searchPoint.y <= currentTile->point_tr.y)) {
            if (searchPoint.y > currentTile->point_tr.y) {
                currentTile = currentTile->rt;
                debug << "U -> "<< get_tile_info(currentTile) << " -> ";
            }
            else if (searchPoint.y <= currentTile->point_bl.y) {
                currentTile = currentTile->lb;
                debug << "D -> "<< get_tile_info(currentTile) << " -> ";
            }
        }

        // 2. Move left/right
        while(!(currentTile->point_bl.x <= searchPoint.x && searchPoint.x < currentTile->point_tr.x)) {
            if (searchPoint.x < currentTile->point_bl.x) {
                currentTile = currentTile->bl;
                debug << "L -> "<< get_tile_info(currentTile) << " -> ";
            }
            else if (searchPoint.x >= currentTile->point_tr.x) {
                currentTile = currentTile->tr;
                debug << "R -> "<< get_tile_info(currentTile) << " -> ";
            }
        }
        searchRnd += 1;
    }
    debug << "done" << endl;
    return currentTile;
}

void block_insertion(Tile *newTile, Tile *initialTile, vector<Tile *>& allSpaceTiles, Tile *startingTile) {
    Tile *beCutTile;

    // Cut top edge horizontally
    debug << "-------1-------" << endl;
    XYCoord point_tl(newTile->point_bl.x, newTile->point_tr.y);
    beCutTile = point_finding_invert_y(point_tl, startingTile);
    if (beCutTile->point_tr.y > point_tl.y) {
        debug << "hcut_top: beCutTile = " << get_tile_info(beCutTile) << endl;
        hcut_top(beCutTile, allSpaceTiles, point_tl.y);
    }

    // Start vertical cut from top
    debug << "-------2-------" << endl;
    int finalY = newTile->point_bl.y;
    while (beCutTile != nullptr && beCutTile->point_bl.y > finalY) {
        debug << "vcut: beCutTile = " << get_tile_info(beCutTile) << endl;
        vcut(beCutTile, newTile, allSpaceTiles, newTile->point_bl.x, newTile->point_tr.x);
        // debug << "beCutTile" << get_tile_info(beCutTile) << ", newTile" << get_tile_info(newTile) << endl;
        beCutTile = newTile->lb; // newTile size is modified in vcut
    }

    //
    debug << "-------3-------" << endl;
    if (beCutTile != nullptr && beCutTile->point_bl.y < finalY) {
        debug << "final hcut_top: beCutTile = " << get_tile_info(beCutTile) << endl;
        hcut_top(beCutTile, allSpaceTiles, finalY);
        beCutTile = beCutTile->rt;
    }

    debug << "-------4-------" << endl;
    debug << "final vcut: beCutTile = " << get_tile_info(beCutTile) << endl;
    vcut(beCutTile, newTile, allSpaceTiles, newTile->point_bl.x, newTile->point_tr.x);

}

void hcut_top(Tile *origFullTile, vector<Tile *>& allSpaceTiles, int cut_y) {
    // Create a new upperTile
    Tile *upperTile = new Tile(origFullTile->point_bl.x, cut_y, origFullTile->point_tr.x, origFullTile->point_tr.y, allSpaceTiles.size());
    upperTile->bl = origFullTile->bl;
    upperTile->lb = origFullTile;
    upperTile->tr = origFullTile->tr;
    upperTile->rt = origFullTile->rt;
    while (upperTile->bl != nullptr && cut_y >= upperTile->bl->point_tr.y) {
        upperTile->bl = upperTile->bl->rt;
    }
    allSpaceTiles.emplace_back(upperTile);

    // Modify the origFullTile
    origFullTile->rt = upperTile;
    origFullTile->point_tr.y = cut_y;
    while (origFullTile->tr != nullptr && origFullTile->tr->point_bl.y >= cut_y) {
        origFullTile->tr = origFullTile->tr->lb;
    }


    // Update pointers of surrounding tiles due to this new added upperTile
    // left-side
    for (Tile *neiTile = upperTile->bl; neiTile != nullptr && neiTile->point_tr.y <= upperTile->point_tr.y; neiTile = neiTile->rt) {
        debug << get_tile_info(neiTile) << "->R from " << get_tile_info(neiTile->tr) << " to ";
        neiTile->tr = upperTile;
        debug << get_tile_info(neiTile->tr) << endl;
    }
    // right-side
    for (Tile *neiTile = upperTile->tr; neiTile != nullptr && neiTile->point_bl.y >= upperTile->point_bl.y; neiTile = neiTile->lb) {
        debug << get_tile_info(neiTile) << "->L from " << get_tile_info(neiTile->bl) << " to ";
        neiTile->bl = upperTile;
        debug << get_tile_info(neiTile->bl) << endl;
    }
    // upper-side
    for (Tile *neiTile = upperTile->rt; neiTile != nullptr && neiTile->point_bl.x >= upperTile->point_bl.x; neiTile = neiTile->bl) {
        debug << get_tile_info(neiTile) << "->B from " << get_tile_info(neiTile->lb) << " to ";
        neiTile->lb = upperTile;
        debug << get_tile_info(neiTile->lb) << endl;
    }

    debug << "upper: " << "\t" << get_tile_info(upperTile)    << "\t" << get_all_neighbor_tiles(upperTile) << endl;
    debug << "lower: " << "\t" << get_tile_info(origFullTile) << "\t" << get_all_neighbor_tiles(origFullTile) << endl;
}

void hcut_bottom(Tile *origFullTile, vector<Tile *>& allSpaceTiles, int cut_y) {
    // Create a new upperTile
    Tile *lowerTile = new Tile(origFullTile->point_bl.x, origFullTile->point_bl.y, origFullTile->point_tr.x, cut_y, allSpaceTiles.size());
    lowerTile->bl = origFullTile->bl;
    lowerTile->lb = origFullTile->lb;
    lowerTile->tr = origFullTile->tr;
    lowerTile->rt = origFullTile;
    while (lowerTile->tr != nullptr && lowerTile->tr->point_bl.y >= cut_y) {
        lowerTile->tr = lowerTile->tr->lb;
    }
    allSpaceTiles.emplace_back(lowerTile);

    // Modify the lowerTile
    origFullTile->lb = lowerTile;
    origFullTile->point_bl.y = cut_y;
    while (origFullTile->bl != nullptr && origFullTile->bl->point_tr.y < cut_y) {
        origFullTile->bl = origFullTile->bl->tr;
    }

    // Update pointers of surrounding tiles due to this new added lowerTile
    // left-side
    for (Tile *neiTile = lowerTile->bl; neiTile != nullptr && neiTile->point_tr.y < lowerTile->point_tr.y; neiTile = neiTile->rt) {
        neiTile->tr = lowerTile;
    }
    // right-side
    for (Tile *neiTile = lowerTile->tr; neiTile != nullptr && neiTile->point_bl.y >= lowerTile->point_bl.y; neiTile = neiTile->rt) {
        neiTile->bl = lowerTile;
    }
    // bottom-side
    for (Tile *neiTile = lowerTile->lb; neiTile != nullptr && neiTile->point_tr.x < lowerTile->point_tr.x; neiTile = neiTile->tr) {
        neiTile->rt = lowerTile;
    }
}

void vcut(Tile *beCutTile, Tile *middleTile , vector<Tile *>& allSpaceTiles, int cut_xl, int cut_xr) {
    // Change middleTile's height to fit beCutTile
    middleTile->point_bl.x = cut_xl;
    middleTile->point_bl.y = beCutTile->point_bl.y;


    // ================
    // Middle & Right
    // ================
    Tile *rightTile;
    if (cut_xr == beCutTile->point_tr.x) {
        rightTile = middleTile;

        if (middleTile->point_tr.y == beCutTile->point_tr.y) { // Only happens at 1st vcut
            middleTile->tr = beCutTile->tr;
            middleTile->rt = beCutTile->rt;
        }
        middleTile->lb = beCutTile->lb;
        while (middleTile->lb != nullptr && middleTile->lb->point_tr.x <= middleTile->point_bl.x) {
            middleTile->lb = middleTile->lb->tr;
        }
    }
    else {
        rightTile = new Tile(cut_xr, beCutTile->point_bl.y, beCutTile->point_tr.x, beCutTile->point_tr.y, allSpaceTiles.size());
        rightTile->lb = beCutTile->lb;
        rightTile->bl = middleTile;
        rightTile->rt = beCutTile->rt;
        rightTile->tr = beCutTile->tr;
        while (rightTile->lb != nullptr && rightTile->lb->point_tr.x <= rightTile->point_bl.x) {
            rightTile->lb = rightTile->lb->tr;
        }
        allSpaceTiles.emplace_back(rightTile);

        if (middleTile->point_tr.y == beCutTile->point_tr.y) {
            middleTile->tr = rightTile;
            middleTile->rt = beCutTile->rt;
            while (middleTile->rt != nullptr && middleTile->rt->point_bl.x >= middleTile->point_tr.x) {
                middleTile->rt = middleTile->rt->bl;
            }
        }
        middleTile->lb = beCutTile->lb;
        while (middleTile->lb != nullptr && middleTile->lb->point_tr.x <= middleTile->point_bl.x) {
            middleTile->lb = middleTile->lb->tr;
        }
    }


    // ================
    // Update neighbor pointers
    // ================
    // Top-edge neighbors (fix lb)
    for (Tile *neiTile = beCutTile->rt; neiTile != nullptr && neiTile->point_bl.x >= beCutTile->point_bl.x; neiTile = neiTile->bl) {
        if (neiTile != middleTile) {
            debug << get_tile_info(neiTile) << "->B from " << get_tile_info(neiTile->lb) << " to ";
            if (neiTile->point_bl.x >= cut_xr)
                neiTile->lb = rightTile;
            else if (neiTile->point_bl.x >= cut_xl)
                neiTile->lb = middleTile;
            else
                neiTile->lb = beCutTile;
            debug << get_tile_info(neiTile->lb) << endl;
        }
    }
    // bottom-edge neighbors (fix rt)
    for (Tile *neiTile = beCutTile->lb; neiTile != nullptr && neiTile->point_tr.x <= rightTile->point_tr.x; neiTile = neiTile->tr) {
        debug << get_tile_info(neiTile) << "->T from " << get_tile_info(neiTile->rt) << " to ";
        if (neiTile->point_tr.x <= cut_xl)
            neiTile->rt = beCutTile;
        else if (neiTile->point_tr.x <= cut_xr)
            neiTile->rt = middleTile;
        else
            neiTile->rt = rightTile;
        debug << get_tile_info(neiTile->rt) << endl;
    }


    // right-edge neighbors (fix bl)
    for (Tile *neiTile = rightTile->tr; neiTile != nullptr && neiTile->point_bl.y >= rightTile->point_bl.y; neiTile = neiTile->lb) {
        debug << get_tile_info(neiTile) << "->L from " << get_tile_info(neiTile->bl) << " to ";
        neiTile->bl = rightTile;
        debug << get_tile_info(neiTile->bl) << endl;
    }

    // ================
    // Left (beCutTile)
    // Note: beCutTile have to be processed at the end, otherwise it will cause error!!!
    // ================
    if (cut_xl != beCutTile->point_bl.x) {
        beCutTile->point_tr.x = cut_xl;
        beCutTile->point_tr.y = beCutTile->point_tr.y;
        beCutTile->tr = middleTile;
        while (beCutTile->rt != nullptr && beCutTile->rt->point_bl.x >= beCutTile->point_tr.x) {
            beCutTile->rt = beCutTile->rt->bl;
        }

        middleTile->bl = beCutTile;
    }
    else {
        middleTile->bl = beCutTile->bl;

        beCutTile->isValid = false;
        beCutTile = middleTile;
    }

    // left-edge neighbors (fix tr)
    for (Tile *neiTile = beCutTile->bl; neiTile != nullptr && neiTile->point_tr.y <= beCutTile->point_tr.y; neiTile = neiTile->rt) {
        debug << get_tile_info(neiTile) << "->R from " << get_tile_info(neiTile->tr) << " to ";
        neiTile->tr = beCutTile;
        debug << get_tile_info(neiTile->tr) << endl;
    }

    debug << "left:   " << "\t" << get_tile_info(beCutTile)  << "\t" << get_all_neighbor_tiles(beCutTile)  << endl;
    debug << "middle: " << "\t" << get_tile_info(middleTile) << "\t" << get_all_neighbor_tiles(middleTile) << endl;
    debug << "right:  " << "\t" << get_tile_info(rightTile)  << "\t" << get_all_neighbor_tiles(rightTile)  << endl;

    // ================
    // Merge those mergable tiles
    // ================
    if (middleTile != rightTile) {
        merge_tile(rightTile);
        if (rightTile->lb) {
            merge_tile(rightTile->lb);
        }
    }
    if (beCutTile != middleTile) {
        merge_tile(beCutTile);
        if (beCutTile->lb) {
            merge_tile(beCutTile->lb);
        }
    }
}

void merge_tile(Tile *lowerTile) {
    Tile *upperTile = lowerTile->rt;
    if (upperTile) {
        debug << "merge_tile(): " << "upperTile = " << get_tile_info(upperTile) << endl;
        if (!upperTile->isBlock && !lowerTile->isBlock && upperTile->point_bl.x == lowerTile->point_bl.x && upperTile->point_tr.x == lowerTile->point_tr.x) {
            debug << get_tile_info(lowerTile) << " " << get_tile_info(upperTile);
            lowerTile->point_tr.x = upperTile->point_tr.x;
            lowerTile->point_tr.y = upperTile->point_tr.y;
            lowerTile->tr = upperTile->tr;
            lowerTile->rt = upperTile->rt;
            debug << " -> " << get_tile_info(lowerTile) << endl;

            // left-side (fix tr)
            for (Tile *neiTile = lowerTile->bl; neiTile != nullptr && neiTile->point_tr.y <= upperTile->point_tr.y; neiTile = neiTile->rt) {
                debug << get_tile_info(neiTile) << "->R from " << get_tile_info(neiTile->tr) << " to ";
                neiTile->tr = lowerTile;
                debug << get_tile_info(neiTile->tr) << endl;
            }
            // right-side (fix bl)
            for (Tile *neiTile = upperTile->tr; neiTile != nullptr && neiTile->point_bl.y >= lowerTile->point_bl.y; neiTile = neiTile->lb) {
                debug << get_tile_info(neiTile) << "->L from " << get_tile_info(neiTile->bl) << " to ";
                neiTile->bl = lowerTile;
                debug << get_tile_info(neiTile->bl) << endl;
            }
            // upper-side (fix lb)
            for (Tile *neiTile = upperTile->rt; neiTile != nullptr && neiTile->point_bl.x >= upperTile->point_bl.x; neiTile = neiTile->bl) {
                debug << get_tile_info(neiTile) << "->B from " << get_tile_info(neiTile->lb) << " to ";
                neiTile->lb = lowerTile;
                debug << get_tile_info(neiTile->lb) << endl;
            }

            // Invalidate upperTile
            upperTile->isValid = false;
        }
    }
}





void print_all_tiles(int WIDTH, int HEIGHT, vector<Tile *>&allSpaceTiles, vector<Tile *>& allBlockTiles) {
    cout << "==========================" << endl;
    cout << allBlockTiles.size() + allSpaceTiles.size() << endl;
    cout << WIDTH << " " << HEIGHT << endl;
    for (Tile *tile: allBlockTiles) {
        int id = tile->block_id;
        int x  = tile->point_bl.x;
        int y  = tile->point_bl.y;
        int w  = tile->point_tr.x - x;
        int h  = tile->point_tr.y - y;
        cout << id << " " << x << " " << y << " " << w << " " << h << endl;
    }
    for (size_t i = 0; i < allSpaceTiles.size(); i++) {
        Tile *tile = allSpaceTiles[i];
        // int id = tile->block_id;
        int x  = tile->point_bl.x;
        int y  = tile->point_bl.y;
        int w  = tile->point_tr.x - x;
        int h  = tile->point_tr.y - y;
        cout << -(signed(i)+1) << " " << x << " " << y << " " << w << " " << h << endl;
    }
}

void write_all_tiles(ofstream& layoutFile, int WIDTH, int HEIGHT, vector<Tile *>&allSpaceTiles, vector<Tile *>& allBlockTiles) {
    int spaceTileNum = 0;
    for (Tile *tile: allSpaceTiles) {
        spaceTileNum += (tile->isValid);
    }
    layoutFile << allBlockTiles.size() + spaceTileNum << endl;
    layoutFile << WIDTH << " " << HEIGHT << endl;
    for (Tile *tile: allBlockTiles) {
        int id = tile->block_id;
        int x  = tile->point_bl.x;
        int y  = tile->point_bl.y;
        int w  = tile->point_tr.x - x;
        int h  = tile->point_tr.y - y;
        layoutFile << id << " " << x << " " << y << " " << w << " " << h << endl;
    }
    for (size_t i = 0; i < allSpaceTiles.size(); i++) {
        Tile *tile = allSpaceTiles[i];
        if (tile->isValid) {
            // int id = tile->block_id;
            int x  = tile->point_bl.x;
            int y  = tile->point_bl.y;
            int w  = tile->point_tr.x - x;
            int h  = tile->point_tr.y - y;
            layoutFile << -(signed(i)+1) << " " << x << " " << y << " " << w << " " << h << endl;
        }
    }
}

string get_tile_info(Tile* tile) {
    if (tile) {
        string a1 = tile->isBlock ? "B" : "S";
        string a2 = to_string(tile->block_id);
        string b = to_string(tile->point_bl.x);
        string c = to_string(tile->point_bl.y);
        string d = to_string(tile->point_tr.x - tile->point_bl.x);
        string e = to_string(tile->point_tr.y - tile->point_bl.y);
        return "(" + a1 + a2 + "," + b + "," + c + "," + d + "," + e + ")";
    }
    else {
        return "NULL";
    }
}

string get_all_neighbor_tiles(Tile *tile) {
    string bl = (tile->bl) ? ((tile->bl->isBlock ? "B" : "S") + to_string(tile->bl->block_id)) : "NULL";
    string lb = (tile->lb) ? ((tile->lb->isBlock ? "B" : "S") + to_string(tile->lb->block_id)) : "NULL";
    string tr = (tile->tr) ? ((tile->tr->isBlock ? "B" : "S") + to_string(tile->tr->block_id)) : "NULL";
    string rt = (tile->rt) ? ((tile->rt->isBlock ? "B" : "S") + to_string(tile->rt->block_id)) : "NULL";
    return "L=" + bl + ", B=" + lb + ", R=" + tr + ", T=" + rt;
}