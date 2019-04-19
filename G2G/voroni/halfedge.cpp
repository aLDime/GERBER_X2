#include "halfedge.h"
int Halfedge::counter = 0;

Halfedge::Halfedge(const Edge& edge, const Vertex& lSite, const Vertex rSite) //Voronoi::prototype.Halfedge = function(edge, lSite, rSite)
    : edge(edge)
    , site(lSite)
    , voronoiId(counter++)
{
    // 'angle' is a value to be used for properly sorting the
    // halfsegments counterclockwise. By convention, we will
    // use the angle of the line defined by the 'site to the left'
    // to the 'site to the right'.
    // However, border edges have no 'site to the right': thus we
    // use the angle of line perpendicular to the halfsegment (the
    // edge should have both end points defined in such case.)
    if (rSite) {
        angle = std::atan2(rSite.y - lSite.y, rSite.x - lSite.x);
    } else {
        Vertex va = edge.va, vb = edge.vb;
        // rhill 2011-05-31: used to call getStartpoint()/getEndpoint(),
        // but for performance purpose, these are expanded in place here.
        angle = edge.lSite == lSite ? std::atan2(vb.x - va.x, va.y - vb.y) : std::atan2(va.x - vb.x, vb.y - va.y);
    }
}

Vertex Halfedge::getStartpoint() //Voronoi::prototype::Halfedge::prototype.getStartpoint = function()
{
    return edge.lSite == site ? edge.va : edge.vb;
}

Vertex Halfedge::getEndpoint() //Voronoi::prototype::Halfedge::prototype.getEndpoint = function()
{
    return edge.lSite == site ? edge.vb : edge.va;
}
