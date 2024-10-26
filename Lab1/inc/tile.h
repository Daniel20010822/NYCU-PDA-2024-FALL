#ifndef TILE_H
#define TILE_H
#include <iostream>
#include <string>
#include <vector>
#include <fstream>
using namespace std;

const int MAX_ITER = 100;

struct XYCoord {
    int x;
    int y;
    XYCoord (int inX = 0, int inY = 0) : x(inX), y(inY) {}
};

struct Tile {
    int block_id;
    XYCoord point_bl;
    XYCoord point_tr;
    Tile *lb;
    Tile *bl;
    Tile *rt;
    Tile *tr;
    bool isBlock;
    bool isValid;

    Tile(int bl_x = 0, int bl_y = 0, int tr_x = 0, int tr_y = 0, int block_id = 0, bool isBlock = false, bool isValid = true)
        : block_id(block_id),
          point_bl(bl_x, bl_y),
          point_tr(tr_x, tr_y),
          lb(nullptr), bl(nullptr), rt(nullptr), tr(nullptr),
          isBlock(isBlock), isValid(isValid) {}
};


Tile *point_finding(const XYCoord , Tile *);
Tile *point_finding_invert_y(const XYCoord , Tile *);
void block_insertion(Tile *, Tile *, vector<Tile *>& ,Tile *);
void hcut_top(Tile *, vector<Tile *>&, int);
void hcut_bottom(Tile *, vector<Tile *>&, int);
void vcut(Tile *, Tile *, vector<Tile *>&, int, int);
void merge_tile(Tile *);
void print_all_tiles(int WIDTH, int HEIGHT, vector<Tile *>& allSpaceTiles, vector<Tile *>& allBlockTiles);
void write_all_tiles(ofstream&, int, int, vector<Tile *>&, vector<Tile *>&);
string get_tile_info(Tile* tile);
string get_all_neighbor_tiles(Tile *tile);

#endif