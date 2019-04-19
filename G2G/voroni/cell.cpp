#include "cell.h"
#include <limits>

Cell::Cell(const Vertex& site) //Voronoi::prototype.Cell = function(site)
    : site(site)
    //, halfedges({})
    , closeMe(false)
{
}

Cell Cell::init(const QSharedPointer<Vertex>& site) //Voronoi::prototype::Cell::prototype.init = function(site)
{
    site = site;
    halfedges.clear();
    closeMe = false;
    return *this; //shared_from_this();
}

int Cell::prepareHalfedges()
{
    //QSharedPointer<var> halfedges = halfedges;
    int iHalfedge = halfedges.length();
    // get rid of unused halfedges
    // rhill 2011-05-27: Keep it simple, no point here in trying
    // to be fancy: dangling edges are a typically a minority.

    while (iHalfedge--) {
        Edge edge = halfedges[iHalfedge].edge;
        if (!edge.vb || !edge.va) {
            halfedges.remove(iHalfedge); //.splice(iHalfedge, 1);
        }
    }

    // rhill 2011-05-26: I tried to use a binary search at insertion
    // time to keep the array sorted on-the-fly (in Cell.addHalfedge()).
    // There was no real benefits in doing so, performance on
    // Firefox 3.6 was improved marginally, while performance on
    // Opera 11 was penalized marginally.
    //halfedges.sort([](QSharedPointer<Halfedge> a, QSharedPointer<Halfedge> b) { return b.angle - a.angle; });
    std::sort(halfedges.begin(), halfedges.end(), [](QSharedPointer<Halfedge> a, QSharedPointer<Halfedge> b) { return b.angle > a.angle; });
    return halfedges.length();
}

QVector<int> Cell::getNeighborIds()
{
    QVector<int> neighbors;
    int iHalfedge = halfedges.length();
    while (iHalfedge--) {
        Edge edge = halfedges[iHalfedge].edge;
        if (edge.lSite != nullptr && edge.lSite.voronoiId != site.voronoiId)
            neighbors.push(edge.lSite.voronoiId);
        else if (edge.rSite != nullptr && edge.rSite.voronoiId != site.voronoiId)
            neighbors.push(edge.rSite.voronoiId);
    }
    return neighbors;
}

QRectF Cell::getBbox()
{
    int iHalfedge = halfedges.length();
    double xmin = std::numeric_limits<double>::max();
    double ymin = std::numeric_limits<double>::max();
    double xmax = -std::numeric_limits<double>::max();
    double ymax = -std::numeric_limits<double>::max();
    double v = 0;
    double vx = 0;
    double vy = 0;
    while (iHalfedge--) {
        Vertex v = halfedges[iHalfedge].getStartpoint();
        vx = v.x;
        vy = v.y;
        if (vx < xmin)
            xmin = vx;

        if (vy < ymin)
            ymin = vy;

        if (vx > xmax)
            xmax = vx;

        if (vy > ymax)
            ymax = vy;

        // we dont need to take into account end point,
        // since each end point matches a start point
    }
    return QRectF(xmin, ymin, xmax - xmin, ymax - ymin);
}

int Cell::pointIntersection(double x, double y) //Voronoi::prototype::Cell::prototype.pointIntersection = function(x, y)
{
    // Check if point in polygon. Since all polygons of a Voronoi
    // diagram are convex, then:
    // http://paulbourke.net/geometry/polygonmesh/
    // Solution 3 (2D):
    //   "If the polygon is convex then one can consider the polygon
    //   "as a 'path' from the first vertex. A point is on the interior
    //   "of this polygons if it is always on the same side of all the
    //   "line segments making up the path. ...
    //   "(y - y0) (x1 - x0) - (x - x0) (y1 - y0)
    //   "if it is less than 0 then P is to the right of the line segment,
    //   "if greater than 0 it is to the left, if equal to 0 then it lies
    //   "on the line segment"
    int iHalfedge = halfedges.length();
    while (iHalfedge--) {
        Halfedge halfedge = halfedges[iHalfedge];
        Vertex p0 = halfedge.getStartpoint();
        Vertex p1 = halfedge.getEndpoint();
        int r = (y - p0.y) * (p1.x - p0.x) - (x - p0.x) * (p1.y - p0.y);
        if (!r)
            return 0;
        if (r > 0)
            return -1;
    }
    return 1;
}
