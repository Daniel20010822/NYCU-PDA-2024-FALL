#include "Row.h"

PlacementRow* Row::get_PR_by_X(double X) const {
    for (auto PR : PRs) {
        if (PR->startX() <= X && X <= PR->endX()) {
            return PR;
        }
        // else if (X > PR->endX()) {
        //     break;
        // }
    }
    return nullptr;
}