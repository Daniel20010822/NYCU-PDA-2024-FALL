#ifndef _GCELL_H_
#define _GCELL_H_

class GCell {
private:
    XYCoord LB;
    int leftEdgeCapacity = 0;
    int bottomEdgeCapacity = 0;

    // TODO: Think if we need to store the pointers to the neighboring GCells
    // GCell *R;
    // GCell *U;


    bool isOpen = true;
public:
    GCell();
    GCell(int leftEdgeCapacity, int bottomEdgeCapacity);
    ~GCell();

    // void
};

#endif