#ifndef CELL_H
#define CELL_H

#include <QRect>
#include <QSharedPointer>
#include <QVector>

#include "halfedge.h"

class Cell {
public:
    Cell(const Vertex& site);

    Vertex site;
    QVector<Halfedge> halfedges;
    bool closeMe;

    Cell init(const Vertex& site);

    int prepareHalfedges();

    // Return a list of the neighbor Ids
    QVector<int> getNeighborIds();

    // Compute bounding box
    QRectF getBbox();

    // Return whether a point is inside, on, or outside the cell:
    //   -1: point is outside the perimeter of the cell
    //    0: point is on the perimeter of the cell
    //    1: point is inside the perimeter of the cell
    //
    int pointIntersection(double x, double y);
}

#endif // CELL_H
