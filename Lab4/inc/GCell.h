#ifndef _GCELL_H_
#define _GCELL_H_

class GCell {
private:
    int leftEdgeCapacity = 0;
    int bottomEdgeCapacity = 0;

    bool isOpen = true;
public:
    GCell();
    GCell(int leftEdgeCapacity, int bottomEdgeCapacity);
    ~GCell();

    // void
};

#endif