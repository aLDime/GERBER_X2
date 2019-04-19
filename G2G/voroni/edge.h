#ifndef EDGE_H
#define EDGE_H

#include "halfedge.h"

#include <QSharedPointer>

class Vertex {
public:
    Vertex(double x = 0, double y = 0)
        : x(x)
        , y(y)
    {
    }
    void reset()
    {
        x = 0;
        y = 0;
    }
    double x;
    double y;
};

class Edge {
public:
    Edge(const QSharedPointer<Halfedge>& lSite, const QSharedPointer<Halfedge>& rSite) //Voronoi::prototype->Edge = function(lSite, rSite)
        : lSite(lSite)
        , rSite(rSite)
    {
        //this->va = this->vb = nullptr;
    }

    QSharedPointer<Halfedge> lSite; // the Voronoi site object at the left of this Voronoi.Edge object.
    QSharedPointer<Halfedge> rSite; //  the Voronoi site object at the right of this Voronoi.Edge object (can be null).
    Vertex va; //  an object with an 'x' and a 'y' property defining the start point   (relative to the Voronoi site on the left) of this Voronoi.Edge object.
    Vertex vb; //  an object with an 'x' and a 'y' property defining the end point     (relative to Voronoi site on the left) of this Voronoi.Edge object.
};

class CircleEvent {
public:
    CircleEvent() { site.reset(); }
    void* arc = nullptr;
    void* rbLeft = nullptr;
    void* rbNext = nullptr;
    void* rbParent = nullptr;
    void* rbPrevious = nullptr;
    void* rbRed = false;
    void* rbRight = nullptr;

    Vertex site;

    double x = 0.0;
    double y = 0.0;
    double ycenter = 0.0;
};

#endif // EDGE_H
