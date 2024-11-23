#ifndef _PR_H_
#define _PR_H_

#include "XYCoord.h"

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

#endif