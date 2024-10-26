#include <iostream>
#include <fstream>
#include <vector>
#include <string>
#include <queue>
#include <algorithm>
#include "tile.h"
#include "debug_config.h"
using namespace std;

int main(int argc, char** argv) {

    if (argc < 3) {
        cerr << "Need 2 input arguments!" << endl;
        return 1;
    }

    ifstream inputFile(argv[1]);
    ofstream outputFile(argv[2]);
    ofstream layoutFile;
    // ofstream neighborFile("neighbor.txt");

    vector<Tile *> allBlockTiles;
    vector<Tile *> allSpaceTiles;

    queue<string> answerQueue;


    int WIDTH, HEIGHT;
    inputFile >> WIDTH >> HEIGHT;
    Tile *initialTile = new Tile(0, 0, WIDTH, HEIGHT);
    allSpaceTiles.emplace_back(initialTile);

    Tile *searchStartingTile = initialTile;

    int insertRnd = 1;
    while (!inputFile.eof()) {
        string command;
        inputFile >> command;
        if (command.empty())
            continue;

        if (command == "P") {
            int searchX, searchY;
            inputFile >> searchX >> searchY;
            XYCoord searching_point(searchX, searchY);
            Tile *result_block = point_finding(searching_point, searchStartingTile);
            debug << result_block->point_bl.x << " " << result_block->point_bl.y << endl;

            string answer = to_string(result_block->point_bl.x) + " " + to_string(result_block->point_bl.y);
            answerQueue.push(answer);
        }
        else {
            int blockIdx = stoi(command);
            int blockX, blockY, blockW, blockH;
            inputFile >> blockX >> blockY >> blockW >> blockH;

            Tile *newTile = new Tile(blockX, blockY, blockX + blockW, blockY + blockH, blockIdx, true);
            allBlockTiles.emplace_back(newTile);

            debug << "================ "<< "Iter " << insertRnd << " ===============" << endl;
            debug << "Inserting block... " << blockIdx << ": " << get_tile_info(newTile) << endl;
            block_insertion(newTile, initialTile, allSpaceTiles, searchStartingTile);

            searchStartingTile = allBlockTiles[0];

            // // Print layout format for every iteration
            // if (argc == 4){
            //     string fileName = argv[3];
            //     string postFix = "-" + to_string(insertRnd);
            //     size_t dotPos = fileName.find('.');
            //     fileName.insert(dotPos, postFix);
            //     layoutFile.open(fileName, ios::out);
            //     write_all_tiles(layoutFile, WIDTH, HEIGHT, allSpaceTiles, allBlockTiles);
            //     layoutFile.close();
            // }

            insertRnd += 1;
        }
    }
    // Print neighbor information for every iteration
    // neighborFile << "================ "<< "Iter " << insertRnd << " ===============" << endl;
    // for (Tile *tile: allBlockTiles) {
    //     neighborFile << get_tile_info(tile) << " => " << get_all_neighbor_tiles(tile) << endl;
    // }
    // for (Tile *tile: allSpaceTiles) {
    //     if (tile->isValid)
    //         neighborFile << get_tile_info(tile) << " => " << get_all_neighbor_tiles(tile) << endl;
    //     else
    //         neighborFile << get_tile_info(tile) << " => " << get_all_neighbor_tiles(tile) << "    >>REMOVED<<"<< endl;
    // }

    // Print the last layout
    if (argc == 4){
        string fileName = argv[3];
        string postFix = "-" + to_string(insertRnd);
        size_t dotPos = fileName.find('.');
        fileName.insert(dotPos, postFix);
        layoutFile.open(fileName, ios::out);
        write_all_tiles(layoutFile, WIDTH, HEIGHT, allSpaceTiles, allBlockTiles);
        layoutFile.close();
    }

    int totalBlockNum = allBlockTiles.size();
    int totalSpaceNum = 0;
    for (Tile *tile: allSpaceTiles) {
        totalSpaceNum += (tile->isValid);
    }
    outputFile << totalBlockNum + totalSpaceNum << endl;

    // Sort based on block IDs
    sort(allBlockTiles.begin(), allBlockTiles.end(), [](Tile* a, Tile* b) {
        return a->block_id < b->block_id;
    });
    for (Tile *block: allBlockTiles) {
        int blockNum = 0, spaceNum = 0;
        // Top-edge traverse
        for (Tile *neiTile = block->rt; neiTile != nullptr && neiTile->point_tr.x > block->point_bl.x; neiTile = neiTile->bl) {
            blockNum += (neiTile->isBlock);
            spaceNum += (!neiTile->isBlock);
        }
        // Bottom-edge traverse
        for (Tile *neiTile = block->lb; neiTile != nullptr && neiTile->point_bl.x < block->point_tr.x; neiTile = neiTile->tr) {
            blockNum += (neiTile->isBlock);
            spaceNum += (!neiTile->isBlock);
        }
        // Left-edge traverse
        for (Tile *neiTile = block->bl; neiTile != nullptr && neiTile->point_bl.y < block->point_tr.y; neiTile = neiTile->rt) {
            blockNum += (neiTile->isBlock);
            spaceNum += (!neiTile->isBlock);
        }
        // Right-edge traverse
        for (Tile *neiTile = block->tr; neiTile != nullptr && neiTile->point_tr.y > block->point_bl.y; neiTile = neiTile->lb) {
            blockNum += (neiTile->isBlock);
            spaceNum += (!neiTile->isBlock);
        }

        outputFile << block->block_id << " " << blockNum << " " << spaceNum << endl;
    }

    while (!answerQueue.empty()) {
        string answer = answerQueue.front();
        outputFile << answer << endl;
        answerQueue.pop();
    }

    inputFile.close();
    outputFile.close();
    return 0;
}