#ifndef _XYCOORD_H_
#define _XYCOORD_H_

class XYCoord {
public:
    double x = 0;
    double y = 0;
    XYCoord (double inX = 0, double inY = 0) : x(inX), y(inY) {}
    XYCoord operator+(const XYCoord& other) const {
        return XYCoord(x + other.x, y + other.y);
    }
};

#endif