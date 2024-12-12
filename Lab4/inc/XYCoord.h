#ifndef _XYCOORD_H_
#define _XYCOORD_H_

class XYCoord {
private:
    int x;
    int y;
public:
    XYCoord (int inX = 0, int inY = 0) : x(inX), y(inY) {}
    ~XYCoord () {}

    int X() const { return x; }
    int Y() const { return y; }

    XYCoord operator+ (const XYCoord& rhs) {
        return XYCoord(x + rhs.x, y + rhs.y);
    }
    XYCoord operator- (const XYCoord& rhs) {
        return XYCoord(x - rhs.x, y - rhs.y);
    }
    XYCoord operator* (const XYCoord& rhs) {
        return XYCoord(x * rhs.x, y * rhs.y);
    }
    XYCoord operator/ (const XYCoord& rhs) {
        return XYCoord(x / rhs.x, y / rhs.y);
    }
    bool operator== (const XYCoord& rhs) {
        return x == rhs.x && y == rhs.y;
    }
    bool operator!= (const XYCoord& rhs) {
        return x != rhs.x || y != rhs.y;
    }
};

#endif