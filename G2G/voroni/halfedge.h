#ifndef HALFEDGE_H
#define HALFEDGE_H

#include "edge.h"

class Halfedge {
    static int counter;
    Halfedge(const Halfedge&) = delete;
    operator=(const Halfedge&) = delete;

public:
    Halfedge(const Edge& edge, const Vertex& lSite, const Vertex rSite);

    Vertex getStartpoint();

    Vertex getEndpoint();

    Vertex site;
    Edge edge;
    double angle = 0.0;

    const int voronoiId;
}

#endif // HALFEDGE_H
