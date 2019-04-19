#ifndef VORONOI_H
#define VORONOI_H
#include "cell.h"
#include "edge.h"
#include "rbtree.h"

#include <QSharedPointer>
#include <QVector>
#include <cmath>
#include <memory>

/*!
Copyright (C) 2010-2013 Raymond Hill: https://github.com/gorhill/Javascript-Voronoi
MIT License: See https://github.com/gorhill/Javascript-Voronoi/LICENSE.md
*/
/*
Author: Raymond Hill (rhill@raymondhill.net)
Contributor: Jesse Morgan (morgajel@gmail.com)
File: rhill-voronoi-core.js
Version: 0.98
Date: January 21, 2013
Description: This is my personal Javascript implementation of
Steven Fortune's algorithm to compute Voronoi diagrams.

License: See https://github.com/gorhill/Javascript-Voronoi/LICENSE.md
Credits: See https://github.com/gorhill/Javascript-Voronoi/CREDITS.md
History: See https://github.com/gorhill/Javascript-Voronoi/CHANGELOG.md

## Usage:

  var sites = [{x:300,y:300}, {x:100,y:100}, {x:200,y:500}, {x:250,y:450}, {x:600,y:150}];
  // xl, xr means x left, x right
  // yt, yb means y top, y bottom
  var bbox = {xl:0, xr:800, yt:0, yb:600}
  var voronoi = new Voronoi();
  // pass an object which exhibits xl, xr, yt, yb properties. The bounding
  // box will be used to connect unbound edges, and to close open cells
  result = voronoi.compute(sites, bbox);
  // render, further analyze, etc.

Return value:
  An object with the following properties:

  result.vertices = an array of unordered, unique Voronoi.Vertex objects making
    up the Voronoi diagram.
  result.edges = an array of unordered, unique Voronoi.Edge objects making up
    the Voronoi diagram.
  result.cells = an array of Voronoi.Cell object making up the Voronoi diagram.
    A Cell object might have an empty array of halfedges, meaning no Voronoi
    cell could be computed for a particular cell.
  result.execTime = the time it took to compute the Voronoi diagram, in
    milliseconds.

Voronoi.Vertex object:
  x: The x position of the vertex.
  y: The y position of the vertex.

Voronoi.Edge object:
  lSite: the Voronoi site object at the left of this Voronoi.Edge object.
  rSite: the Voronoi site object at the right of this Voronoi.Edge object (can be null).
  va: an object with an 'x' and a 'y' property defining the start point   (relative to the Voronoi site on the left) of this Voronoi.Edge object.
  vb: an object with an 'x' and a 'y' property defining the end point     (relative to Voronoi site on the left) of this Voronoi.Edge object.
  For edges which are used to close open cells (using the supplied bounding box), the rSite property will be null.

Voronoi.Cell object:
  site: the Voronoi site object associated with the Voronoi cell.
  halfedges: an array of Voronoi.Halfedge objects, ordered counterclockwise,
    defining the polygon for this Voronoi cell.

Voronoi.Halfedge object:
  site: the Voronoi site object owning this Voronoi.Halfedge object.
  edge: a reference to the unique Voronoi.Edge object underlying this
    Voronoi.Halfedge object.
  getStartpoint(): a method returning an object with an 'x' and a 'y' property
    for the start point of this halfedge. Keep in mind halfedges are always
    countercockwise.
  getEndpoint(): a method returning an object with an 'x' and a 'y' property
    for the end point of this halfedge. Keep in mind halfedges are always
    countercockwise.

TODO: Identify opportunities for performance improvement.

TODO: Let the user close the Voronoi cells, do not do it automatically. Not only let
      him close the cells, but also allow him to close more than once using a different
      bounding box for the same Voronoi diagram.
*/

/*global Math */

// ---------------------------------------------------------------------------

class Voronoi {

public:
    Voronoi();

    // ---------------------------------------------------------------------------

    int vertices = nullptr;
    QVector<Edge> edges;
    QVector<Cell> cells;
    int toRecycle = nullptr;
    QVector<> beachsectionJunkyard;
    QVector<> circleEventJunkyard;
    QVector<> vertexJunkyard;
    QVector<> edgeJunkyard;
    QVector<> cellJunkyard;
    QSharedPointer<RBTree> beachline;

    void reset()
    {
        if (!this->beachline) {
            this->beachline = std::make_shared<this->RBTree>();
        }
        // Move leftover beachsections to the beachsection junkyard.
        if (this->beachline.root) {
            QSharedPointer<var> beachsection = this->beachline->getFirst(this->beachline.root);
            while (beachsection) {
                this->beachsectionJunkyard.push(beachsection); // mark for reuse
                beachsection = beachsection->rbNext;
            }
        }
        this->beachline->root = nullptr;
        if (!this->circleEvents) {
            this->circleEvents = std::make_shared<this->RBTree>();
        }
        this->circleEvents->root = this->firstCircleEvent = nullptr;
        this->vertices = [];
        this->edges = [];
        this->cells = [];
    }

    //    Voronoi::prototype->sqrt = std::sqrt;
    //    Voronoi::prototype->abs = std::abs;
    //    Voronoi::prototype->ε = Voronoi->ε = 1e-9;
    //    Voronoi::prototype->invε = Voronoi->invε = 1.0 / Voronoi::ε;
    int equalWithEpsilon(a, b) //Voronoi::prototype->equalWithEpsilon = function(a, b)
    {
        return this->abs(a - b) < 1e-9;
    }
    int greaterThanWithEpsilon(a, b) //Voronoi::prototype->greaterThanWithEpsilon = function(a, b)
    {
        return a - b > 1e-9;
    }
    int greaterThanOrEqualWithEpsilon(a, b) //Voronoi::prototype->greaterThanOrEqualWithEpsilon = function(a, b)
    {
        return b - a < 1e-9;
    }
    int lessThanWithEpsilon(a, b) //Voronoi::prototype->lessThanWithEpsilon = function(a, b)
    {
        return b - a > 1e-9;
    }
    int lessThanOrEqualWithEpsilon(a, b) //Voronoi::prototype->lessThanOrEqualWithEpsilon = function(a, b)
    {
        return a - b < 1e-9;
    }

    // ---------------------------------------------------------------------------
    // Diagram methods

    void Diagram(const Vertex& site) //Voronoi::prototype->Diagram = function(site)
    {
        this->site = site;
    }

    // ---------------------------------------------------------------------------
    // Cell methods

    Cell createCell(const Vertex& site) //Voronoi::prototype->createCell = function(site)
    {
        QSharedPointer<var> cell = this->cellJunkyard->pop();
        if (cell) {
            return cell->init(site);
        }
        return std::make_shared<Cell>(site);
    }

    //xBreak :

    // ---------------------------------------------------------------------------
    // Edge methods
    //

    int createHalfedge(edge, lSite, rSite) //Voronoi::prototype->createHalfedge = function(edge, lSite, rSite)
    {
        return std::make_shared<this->Halfedge>(edge, lSite, rSite);
    }

    // this create and add a vertex to the internal collection

    int createVertex(x, y) //Voronoi::prototype->createVertex = function(x, y)
    {
        QSharedPointer;
        QSharedPointer<var> v = this->vertexJunkyard->pop();
        if (!v) {
            v = std::make_shared<this->Vertex>(x, y);
        } else {
            v->x = x;
            v->y = y;
        }
        this->vertices.push(v);
        return v;
    }

    // this create and add an edge to internal collection, and also create
    // two halfedges which are added to each site's counterclockwise array
    // of halfedges.

    int createEdge(lSite, rSite, va, vb) //Voronoi::prototype->createEdge = function(lSite, rSite, va, vb)
    {
        QSharedPointer<var> edge = this->edgeJunkyard->pop();
        if (!edge) {
            edge = std::make_shared<this->Edge>(lSite, rSite);
        } else {
            edge->lSite = lSite;
            edge->rSite = rSite;
            edge->va = edge->vb = nullptr;
        }

        this->edges->push(edge);
        if (va) {
            this->setEdgeStartpoint(edge, lSite, rSite, va);
        }
        if (vb) {
            this->setEdgeEndpoint(edge, lSite, rSite, vb);
        }
        this->cells[lSite->voronoiId].halfedges.push(this->createHalfedge(edge, lSite, rSite));
        this->cells[rSite->voronoiId].halfedges.push(this->createHalfedge(edge, rSite, lSite));
        return edge;
    }

    int createBorderEdge(lSite, va, vb) //Voronoi::prototype->createBorderEdge = function(lSite, va, vb)
    {
        QSharedPointer<var> edge = this->edgeJunkyard->pop();
        if (!edge) {
            edge = std::make_shared<this->Edge>(lSite, nullptr);
        } else {
            edge->lSite = lSite;
            edge->rSite.smartpointerreset();
        }
        edge->va = va;
        edge->vb = vb;
        this->edges->push(edge);
        return edge;
    }

    int setEdgeStartpoint(edge, lSite, rSite, vertex) //Voronoi::prototype->setEdgeStartpoint = function(edge, lSite, rSite, vertex)
    {
        if (!edge->va && !edge->vb) {
            edge->va = vertex;
            edge->lSite = lSite;
            edge->rSite = rSite;
        } else if (edge->lSite == = rSite) {
            edge->vb = vertex;
        } else {
            edge->va = vertex;
        }
    }

    int setEdgeEndpoint(edge, lSite, rSite, vertex) //Voronoi::prototype->setEdgeEndpoint = function(edge, lSite, rSite, vertex)
    {
        this->setEdgeStartpoint(edge, rSite, lSite, vertex);
    }

    // ---------------------------------------------------------------------------
    // Beachline methods

    // rhill 2011-06-07: For some reasons, performance suffers significantly
    // when instanciating a literal object instead of an empty ctor
    // Voronoi::prototype->Beachsection = function() {}
    class Beachsection {
    public:
        Beachsection() {}
        Vertex site;
    };

    // rhill 2011-06-02: A lot of Beachsection instanciations
    // occur during the computation of the Voronoi diagram,
    // somewhere between the number of sites and twice the
    // number of sites, while the number of Beachsections on the
    // beachline at any given time is comparatively low. For this
    // reason, we reuse already created Beachsections, in order
    // to avoid new memory allocation. This resulted in a measurable
    // performance gain.

    QSharedPointer<Beachsection> createBeachsection(Vertex site) //Voronoi::prototype->createBeachsection = function(site)
    {
        QSharedPointer<Beachsection> beachsection = beachsectionJunkyard.takeFirst();
        if (!beachsection) {
            beachsection = QSharedPointer::create<Beachsection>();
        }
        beachsection->site = site;
        return beachsection;
    }

    // calculate the left break point of a particular beach section,
    // given a particular sweep line
    int leftBreakPoint(arc, directrix) //Voronoi::prototype->leftBreakPoint = function(arc, directrix)
    {
        // http://en.wikipedia.org/wiki/Parabola
        // http://en.wikipedia.org/wiki/Quadratic_equation
        // h1 = x1,
        // k1 = (y1+directrix)/2,
        // h2 = x2,
        // k2 = (y2+directrix)/2,
        // p1 = k1-directrix,
        // a1 = 1/(4*p1),
        // b1 = -h1/(2*p1),
        // c1 = h1*h1/(4*p1)+k1,
        // p2 = k2-directrix,
        // a2 = 1/(4*p2),
        // b2 = -h2/(2*p2),
        // c2 = h2*h2/(4*p2)+k2,
        // x = (-(b2-b1) + Math.sqrt((b2-b1)*(b2-b1) - 4*(a2-a1)*(c2-c1))) / (2*(a2-a1))
        // When x1 become the x-origin:
        // h1 = 0,
        // k1 = (y1+directrix)/2,
        // h2 = x2-x1,
        // k2 = (y2+directrix)/2,
        // p1 = k1-directrix,
        // a1 = 1/(4*p1),
        // b1 = 0,
        // c1 = k1,
        // p2 = k2-directrix,
        // a2 = 1/(4*p2),
        // b2 = -h2/(2*p2),
        // c2 = h2*h2/(4*p2)+k2,
        // x = (-b2 + Math.sqrt(b2*b2 - 4*(a2-a1)*(c2-k1))) / (2*(a2-a1)) + x1

        // change code below at your own risk: care has been taken to
        // reduce errors due to computers' finite arithmetic precision.
        // Maybe can still be improved, will see if any more of this
        // kind of errors pop up again.
        QSharedPointer<var> site = arc::site, rfocx = site->x, rfocy = site->y, pby2 = rfocy - directrix;
        // parabola in degenerate case where focus is on directrix
        if (!pby2) {
            return rfocx;
        }
        QSharedPointer<var> lArc = arc::rbPrevious;
        if (!lArc) {
            return -Infinity;
        }
        site = lArc->site;
        QSharedPointer<var> lfocx = site->x, lfocy = site->y, plby2 = lfocy - directrix;
        // parabola in degenerate case where focus is on directrix
        if (!plby2) {
            return lfocx;
        }
        QSharedPointer<var> hl = lfocx - rfocx, aby2 = 1 / pby2 - 1 / plby2, b = hl / plby2;
        if (aby2) {
            return (-b + this->sqrt(b * b - 2 * aby2 * (hl * hl / (-2 * plby2) - lfocy + plby2 / 2 + rfocy - pby2 / 2))) / aby2 + rfocx;
        }
        // both parabolas have same distance to directrix, thus break point is midway
        return (rfocx + lfocx) / 2;
    }

    // calculate the right break point of a particular beach section,
    // given a particular directrix
    int rightBreakPoint(arc, directrix) //Voronoi::prototype->rightBreakPoint = function(arc, directrix)
    {
        QSharedPointer<var> rArc = arc::rbNext;
        if (rArc) {
            return this->leftBreakPoint(rArc, directrix);
        }
        QSharedPointer<var> site = arc::site;
        return site->y == = directrix ? site->x : Infinity;
    }

    int detachBeachsection(beachsection) //Voronoi::prototype->detachBeachsection = function(beachsection)
    {
        this->detachCircleEvent(beachsection); // detach potentially attached circle event
        this->beachline.rbRemoveNode(beachsection); // remove from RB-tree
        this->beachsectionJunkyard.push(beachsection); // mark for reuse
    }

    int removeBeachsection(beachsection) //Voronoi::prototype->removeBeachsection = function(beachsection)
    {
        QSharedPointer<var> circle = beachsection->circleEvent, x = circle->x, y = circle->ycenter, vertex = this->createVertex(x, y), previous = beachsection->rbPrevious, next = beachsection->rbNext, disappearingTransitions = [beachsection], abs_fn = std::abs;

        // remove collapsed beachsection from beachline
        this->detachBeachsection(beachsection);

        // there could be more than one empty arc at the deletion point, this
        // happens when more than two edges are linked by the same vertex,
        // so we will collect all those edges by looking up both sides of
        // the deletion point.
        // by the way, there is *always* a predecessor/successor to any collapsed
        // beach section, it's just impossible to have a collapsing first/last
        // beach sections on the beachline, since they obviously are unconstrained
        // on their left/right side.

        // look left
        QSharedPointer<var> lArc = previous;
        while (lArc->circleEvent && abs_fn(x - lArc->circleEvent.x) < 1e-9 && abs_fn(y - lArc->circleEvent.ycenter) < 1e-9) {
            previous = lArc->rbPrevious;
            disappearingTransitions->unshift(lArc);
            this->detachBeachsection(lArc); // mark for reuse
            lArc = previous;
        }
        // even though it is not disappearing, I will also add the beach section
        // immediately to the left of the left-most collapsed beach section, for
        // convenience, since we need to refer to it later as this beach section
        // is the 'left' site of an edge for which a start point is set.
        disappearingTransitions->unshift(lArc);
        this->detachCircleEvent(lArc);

        // look right
        QSharedPointer<var> rArc = next;
        while (rArc->circleEvent && abs_fn(x - rArc->circleEvent.x) < 1e-9 && abs_fn(y - rArc->circleEvent.ycenter) < 1e-9) {
            next = rArc->rbNext;
            disappearingTransitions->push(rArc);
            this->detachBeachsection(rArc); // mark for reuse
            rArc = next;
        }
        // we also have to add the beach section immediately to the right of the
        // right-most collapsed beach section, since there is also a disappearing
        // transition representing an edge's start point on its left.
        disappearingTransitions->push(rArc);
        this->detachCircleEvent(rArc);

        // walk through all the disappearing transitions between beach sections and
        // set the start point of their (implied) edge.
        QSharedPointer<var> nArcs = disappearingTransitions->length, iArc;
        for (iArc = 1; iArc < nArcs; iArc++) {
            rArc = disappearingTransitions[iArc];
            lArc = disappearingTransitions[iArc - 1];
            this->setEdgeStartpoint(rArc->edge, lArc->site, rArc->site, vertex);
        }

        // create a new edge as we have now a new transition between
        // two beach sections which were previously not adjacent.
        // since this edge appears as a new vertex is defined, the vertex
        // actually define an end point of the edge (relative to the site
        // on the left)
        lArc = disappearingTransitions[0];
        rArc = disappearingTransitions[nArcs - 1];
        rArc->edge = this->createEdge(lArc->site, rArc->site, undefined, vertex);

        // create circle events if any for beach sections left in the beachline
        // adjacent to collapsed sections
        this->attachCircleEvent(lArc);
        this->attachCircleEvent(rArc);
    }

    int addBeachsection(site) //Voronoi::prototype->addBeachsection = function(site)
    {
        QSharedPointer<var> x = site->x, directrix = site->y;

        // find the left and right beach sections which will surround the newly
        // created beach section.
        // rhill 2011-06-01: This loop is one of the most often executed,
        // hence we expand in-place the comparison-against-epsilon calls.
        QSharedPointer<var> lArc, rArc, dxl, dxr, node = this->beachline.root;

        while (node) {
            dxl = this->leftBreakPoint(node, directrix) - x;
            // x lessThanWithEpsilon xl => falls somewhere before the left edge of the beachsection
            if (dxl > 1e-9) {
                // this case should never happen
                // if (!node.rbLeft) {
                //    rArc = node.rbLeft;
                //    break;
                //    }
                node = node->rbLeft;
            } else {
                dxr = x - this->rightBreakPoint(node, directrix);
                // x greaterThanWithEpsilon xr => falls somewhere after the right edge of the beachsection
                if (dxr > 1e-9) {
                    if (!node->rbRight) {
                        lArc = node;
                        break;
                    }
                    node = node->rbRight;
                } else {
                    // x equalWithEpsilon xl => falls exactly on the left edge of the beachsection
                    if (dxl > -1e-9) {
                        lArc = node->rbPrevious;
                        rArc = node;
                    }
                    // x equalWithEpsilon xr => falls exactly on the right edge of the beachsection
                    else if (dxr > -1e-9) {
                        lArc = node;
                        rArc = node->rbNext;
                    }
                    // falls exactly somewhere in the middle of the beachsection
                    else {
                        lArc = rArc = node;
                    }
                    break;
                }
            }
        }
        // at this point, keep in mind that lArc and/or rArc could be
        // undefined or null.

        // create a new beach section object for the site and add it to RB-tree
        QSharedPointer<var> newArc = this->createBeachsection(site);
        this->beachline.rbInsertSuccessor(lArc, newArc);

        // cases:
        //

        // [null,null]
        // least likely case: new beach section is the first beach section on the
        // beachline.
        // This case means:
        //   no new transition appears
        //   no collapsing beach section
        //   new beachsection become root of the RB-tree
        if (!lArc && !rArc) {
            return;
        }

        // [lArc,rArc] where lArc == rArc
        // most likely case: new beach section split an existing beach
        // section.
        // This case means:
        //   one new transition appears
        //   the left and right beach section might be collapsing as a result
        //   two new nodes added to the RB-tree
        if (lArc == = rArc) {
            // invalidate circle event of split beach section
            this->detachCircleEvent(lArc);

            // split the beach section into two separate beach sections
            rArc = this->createBeachsection(lArc->site);
            this->beachline.rbInsertSuccessor(newArc, rArc);

            // since we have a new transition between two beach sections,
            // a new edge is born
            newArc->edge = rArc->edge = this->createEdge(lArc->site, newArc->site);

            // check whether the left and right beach sections are collapsing
            // and if so create circle events, to be notified when the point of
            // collapse is reached.
            this->attachCircleEvent(lArc);
            this->attachCircleEvent(rArc);
            return;
        }

        // [lArc,null]
        // even less likely case: new beach section is the *last* beach section
        // on the beachline -- this can happen *only* if *all* the previous beach
        // sections currently on the beachline share the same y value as
        // the new beach section.
        // This case means:
        //   one new transition appears
        //   no collapsing beach section as a result
        //   new beach section become right-most node of the RB-tree
        if (lArc && !rArc) {
            newArc->edge = this->createEdge(lArc->site, newArc->site);
            return;
        }

        // [null,rArc]
        // impossible case: because sites are strictly processed from top to bottom,
        // and left to right, which guarantees that there will always be a beach section
        // on the left -- except of course when there are no beach section at all on
        // the beach line, which case was handled above.
        // rhill 2011-06-02: No point testing in non-debug version
        //if (!lArc && rArc) {
        //    throw "Voronoi.addBeachsection(): What is this I don't even";
        //    }

        // [lArc,rArc] where lArc != rArc
        // somewhat less likely case: new beach section falls *exactly* in between two
        // existing beach sections
        // This case means:
        //   one transition disappears
        //   two new transitions appear
        //   the left and right beach section might be collapsing as a result
        //   only one new node added to the RB-tree
        if (lArc != = rArc) {
            // invalidate circle events of left and right sites
            this->detachCircleEvent(lArc);
            this->detachCircleEvent(rArc);

            // an existing transition disappears, meaning a vertex is defined at
            // the disappearance point.
            // since the disappearance is caused by the new beachsection, the
            // vertex is at the center of the circumscribed circle of the left,
            // new and right beachsections.
            // http://mathforum.org/library/drmath/view/55002.html
            // Except that I bring the origin at A to simplify
            // calculation
            QSharedPointer<var> lSite = lArc->site, ax = lSite->x, ay = lSite->y, bx = site->x - ax, by = site->y - ay, rSite = rArc->site, cx = rSite->x - ax, cy = rSite->y - ay, d = 2 * (bx * cy - by * cx), hb = bx * bx + by * by, hc = cx * cx + cy * cy, vertex = this->createVertex((cy * hb - by * hc) / d + ax, (bx * hc - cx * hb) / d + ay);

            // one transition disappear
            this->setEdgeStartpoint(rArc->edge, lSite, rSite, vertex);

            // two new transitions appear at the new vertex location
            newArc->edge = this->createEdge(lSite, site, undefined, vertex);
            rArc->edge = this->createEdge(site, rSite, undefined, vertex);

            // check whether the left and right beach sections are collapsing
            // and if so create circle events, to handle the point of collapse.
            this->attachCircleEvent(lArc);
            this->attachCircleEvent(rArc);
            return;
        }
    }

    // ---------------------------------------------------------------------------
    // Circle event methods

    // rhill 2011-06-07: For some reasons, performance suffers significantly
    // when instanciating a literal object instead of an empty ctor
    Voronoi::prototype->CircleEvent = function()
    {
        // rhill 2013-10-12: it helps to state exactly what we are at ctor time.
        this->arc = nullptr;
        this->rbLeft = nullptr;
        this->rbNext = nullptr;
        this->rbParent = nullptr;
        this->rbPrevious = nullptr;
        this->rbRed = false;
        this->rbRight = nullptr;
        this->site.reset();
        this->x = this->y = this->ycenter = 0;
    }

    int attachCircleEvent(arc) //Voronoi::prototype->attachCircleEvent = function(arc)
    {
        QSharedPointer<var> lArc = arc::rbPrevious, rArc = arc::rbNext;
        if (!lArc || !rArc) {
            return;
        } // does that ever happen?
        QSharedPointer<var> lSite = lArc->site, cSite = arc::site, rSite = rArc->site;

        // If site of left beachsection is same as site of
        // right beachsection, there can't be convergence
        if (lSite == = rSite) {
            return;
        }

        // Find the circumscribed circle for the three sites associated
        // with the beachsection triplet.
        // rhill 2011-05-26: It is more efficient to calculate in-place
        // rather than getting the resulting circumscribed circle from an
        // object returned by calling Voronoi.circumcircle()
        // http://mathforum.org/library/drmath/view/55002.html
        // Except that I bring the origin at cSite to simplify calculations.
        // The bottom-most part of the circumcircle is our Fortune 'circle
        // event', and its center is a vertex potentially part of the final
        // Voronoi diagram.
        QSharedPointer<var> bx = cSite->x, by = cSite->y, ax = lSite->x - bx, ay = lSite->y - by, cx = rSite->x - bx, cy = rSite->y - by;

        // If points l->c->r are clockwise, then center beach section does not
        // collapse, hence it can't end up as a vertex (we reuse 'd' here, which
        // sign is reverse of the orientation, hence we reverse the test.
        // http://en.wikipedia.org/wiki/Curve_orientation#Orientation_of_a_simple_polygon
        // rhill 2011-05-21: Nasty finite precision error which caused circumcircle() to
        // return infinites: 1e-12 seems to fix the problem.
        QSharedPointer<var> d = 2 * (ax * cy - ay * cx);
        if (d >= -2e-12) {
            return;
        }

        QSharedPointer<var> ha = ax * ax + ay * ay, hc = cx * cx + cy * cy, x = (cy * ha - ay * hc) / d, y = (ax * hc - cx * ha) / d, ycenter = y + by;

        // Important: ybottom should always be under or at sweep, so no need
        // to waste CPU cycles by checking

        // recycle circle event object if possible
        QSharedPointer<var> circleEvent = this->circleEventJunkyard->pop();
        if (!circleEvent) {
            circleEvent = std::make_shared<this->CircleEvent>();
        }
        circleEvent->arc = arc;
        circleEvent->site = cSite;
        circleEvent->x = x + bx;
        circleEvent->y = ycenter + this->sqrt(x * x + y * y); // y bottom
        circleEvent->ycenter = ycenter;
        arc->circleEvent = circleEvent;

        // find insertion point in RB-tree: circle events are ordered from
        // smallest to largest
        QSharedPointer<var> predecessor = nullptr, node = this->circleEvents.root;
        while (node) {
            if (circleEvent->y < node->y || (circleEvent->y == = node->y && circleEvent->x <= node->x)) {
                if (node->rbLeft) {
                    node = node->rbLeft;
                } else {
                    predecessor = node->rbPrevious;
                    break;
                }
            } else {
                if (node->rbRight) {
                    node = node->rbRight;
                } else {
                    predecessor = node;
                    break;
                }
            }
        }
        this->circleEvents.rbInsertSuccessor(predecessor, circleEvent);
        if (!predecessor) {
            this->firstCircleEvent = circleEvent;
        }
    }

    int detachCircleEvent(arc) //Voronoi::prototype->detachCircleEvent = function(arc)
    {
        QSharedPointer<var> circleEvent = arc::circleEvent;
        if (circleEvent) {
            if (!circleEvent->rbPrevious) {
                this->firstCircleEvent = circleEvent->rbNext;
            }
            this->circleEvents.rbRemoveNode(circleEvent); // remove from RB-tree
            this->circleEventJunkyard.push(circleEvent);
            arc::circleEvent::smartpointerreset();
        }
    }

    // ---------------------------------------------------------------------------
    // Diagram completion methods

    // connect dangling edges (not if a cursory test tells us
    // it is not going to be visible.
    // return value:
    //   false: the dangling endpoint couldn't be connected
    //   true: the dangling endpoint could be connected
    int connectEdge(edge, bbox) //Voronoi::prototype->connectEdge = function(edge, bbox)
    {
        // skip if end point already connected
        QSharedPointer<var> vb = edge->vb;
        if (!!vb) {
            return true;
        }

        // make local copy for performance purpose
        QSharedPointer<var> va = edge->va, xl = bbox::xl, xr = bbox::xr, yt = bbox::yt, yb = bbox::yb, lSite = edge->lSite, rSite = edge->rSite, lx = lSite->x, ly = lSite->y, rx = rSite->x, ry = rSite->y, fx = (lx + rx) / 2, fy = (ly + ry) / 2, fm, fb;

        // if we reach here, this means cells which use this edge will need
        // to be closed, whether because the edge was removed, or because it
        // was connected to the bounding box.
        this->cells[lSite->voronoiId]->closeMe = true;
        this->cells[rSite->voronoiId]->closeMe = true;

        // get the line equation of the bisector if line is not vertical
        if (ry != = ly) {
            fm = (lx - rx) / (ry - ly);
            fb = fy - fm * fx;
        }

        // remember, direction of line (relative to left site):
        // upward: left.x < right.x
        // downward: left.x > right.x
        // horizontal: left.x == right.x
        // upward: left.x < right.x
        // rightward: left.y < right.y
        // leftward: left.y > right.y
        // vertical: left.y == right.y

        // depending on the direction, find the best side of the
        // bounding box to use to determine a reasonable start point

        // rhill 2013-12-02:
        // While at it, since we have the values which define the line,
        // clip the end of va if it is outside the bbox.
        // https://github.com/gorhill/Javascript-Voronoi/issues/15
        // TODO: Do all the clipping here rather than rely on Liang-Barsky
        // which does not do well sometimes due to loss of arithmetic
        // precision. The code here doesn't degrade if one of the vertex is
        // at a huge distance.

        // special case: vertical line
        if (fm == = undefined) {
            // doesn't intersect with viewport
            if (fx < xl || fx >= xr) {
                return false;
            }
            // downward
            if (lx > rx) {
                if (!va || va->y < yt) {
                    va = this->createVertex(fx, yt);
                } else if (va->y >= yb) {
                    return false;
                }
                vb = this->createVertex(fx, yb);
            }
            // upward
            else {
                if (!va || va->y > yb) {
                    va = this->createVertex(fx, yb);
                } else if (va->y < yt) {
                    return false;
                }
                vb = this->createVertex(fx, yt);
            }
        }
        // closer to vertical than horizontal, connect start point to the
        // top or bottom side of the bounding box
        else if (fm < -1 || fm > 1) {
            // downward
            if (lx > rx) {
                if (!va || va->y < yt) {
                    va = this->createVertex((yt - fb) / fm, yt);
                } else if (va->y >= yb) {
                    return false;
                }
                vb = this->createVertex((yb - fb) / fm, yb);
            }
            // upward
            else {
                if (!va || va->y > yb) {
                    va = this->createVertex((yb - fb) / fm, yb);
                } else if (va->y < yt) {
                    return false;
                }
                vb = this->createVertex((yt - fb) / fm, yt);
            }
        }
        // closer to horizontal than vertical, connect start point to the
        // left or right side of the bounding box
        else {
            // rightward
            if (ly < ry) {
                if (!va || va->x < xl) {
                    va = this->createVertex(xl, fm * xl + fb);
                } else if (va->x >= xr) {
                    return false;
                }
                vb = this->createVertex(xr, fm * xr + fb);
            }
            // leftward
            else {
                if (!va || va->x > xr) {
                    va = this->createVertex(xr, fm * xr + fb);
                } else if (va->x < xl) {
                    return false;
                }
                vb = this->createVertex(xl, fm * xl + fb);
            }
        }
        edge->va = va;
        edge->vb = vb;

        return true;
    }

    // line-clipping code taken from:
    //   Liang-Barsky function by Daniel White
    //   http://www.skytopia.com/project/articles/compsci/clipping.html
    // Thanks!
    // A bit modified to minimize code paths
    int clipEdge(edge, bbox) //Voronoi::prototype->clipEdge = function(edge, bbox)
    {
        QSharedPointer<var> ax = edge->va.x, ay = edge->va.y, bx = edge->vb.x, by = edge->vb.y, t0 = 0, t1 = 1, dx = bx - ax, dy = by - ay;
        // left
        QSharedPointer<var> q = ax - bbox::xl;
        if (dx == = 0 && q < 0) {
            return false;
        }
        QSharedPointer<var> r = -q / dx;
        if (dx < 0) {
            if (r < t0) {
                return false;
            }
            if (r < t1) {
                t1 = r;
            }
        } else if (dx > 0) {
            if (r > t1) {
                return false;
            }
            if (r > t0) {
                t0 = r;
            }
        }
        // right
        q = bbox::xr - ax;
        if (dx == = 0 && q < 0) {
            return false;
        }
        r = q / dx;
        if (dx < 0) {
            if (r > t1) {
                return false;
            }
            if (r > t0) {
                t0 = r;
            }
        } else if (dx > 0) {
            if (r < t0) {
                return false;
            }
            if (r < t1) {
                t1 = r;
            }
        }
        // top
        q = ay - bbox::yt;
        if (dy == = 0 && q < 0) {
            return false;
        }
        r = -q / dy;
        if (dy < 0) {
            if (r < t0) {
                return false;
            }
            if (r < t1) {
                t1 = r;
            }
        } else if (dy > 0) {
            if (r > t1) {
                return false;
            }
            if (r > t0) {
                t0 = r;
            }
        }
        // bottom
        q = bbox::yb - ay;
        if (dy == = 0 && q < 0) {
            return false;
        }
        r = q / dy;
        if (dy < 0) {
            if (r > t1) {
                return false;
            }
            if (r > t0) {
                t0 = r;
            }
        } else if (dy > 0) {
            if (r < t0) {
                return false;
            }
            if (r < t1) {
                t1 = r;
            }
        }

        // if we reach this point, Voronoi edge is within bbox

        // if t0 > 0, va needs to change
        // rhill 2011-06-03: we need to create a new vertex rather
        // than modifying the existing one, since the existing
        // one is likely shared with at least another edge
        if (t0 > 0) {
            edge->va = this->createVertex(ax + t0 * dx, ay + t0 * dy);
        }

        // if t1 < 1, vb needs to change
        // rhill 2011-06-03: we need to create a new vertex rather
        // than modifying the existing one, since the existing
        // one is likely shared with at least another edge
        if (t1 < 1) {
            edge->vb = this->createVertex(ax + t1 * dx, ay + t1 * dy);
        }

        // va and/or vb were clipped, thus we will need to close
        // cells which use this edge.
        if (t0 > 0 || t1 < 1) {
            this->cells[edge->lSite.voronoiId]->closeMe = true;
            this->cells[edge->rSite.voronoiId]->closeMe = true;
        }

        return true;
    }

    // Connect/cut edges at bounding box
    int clipEdges(bbox) //Voronoi::prototype->clipEdges = function(bbox)
    {
        // connect all dangling edges to bounding box
        // or get rid of them if it can't be done
        QSharedPointer<var> edges = this->edges, iEdge = edges->length, edge, abs_fn = std::abs;

        // iterate backward so we can splice safely
        while (iEdge--) {
            edge = edges[iEdge];
            // edge is removed if:
            //   it is wholly outside the bounding box
            //   it is looking more like a point than a line
            if (!this->connectEdge(edge, bbox) || !this->clipEdge(edge, bbox) || (abs_fn(edge->va.x - edge->vb.x) < 1e-9 && abs_fn(edge->va.y - edge->vb.y) < 1e-9)) {
                edge->va = edge->vb = nullptr;
                edges->splice(iEdge, 1);
            }
        }
    }

    // Close the cells.
    // The cells are bound by the supplied bounding box.
    // Each cell refers to its associated site, and a list
    // of halfedges ordered counterclockwise.
    int closeCells(bbox) //Voronoi::prototype->closeCells = function(bbox)
    {
        QSharedPointer<var> xl = bbox::xl, xr = bbox::xr, yt = bbox::yt, yb = bbox::yb, cells = this->cells, iCell = cells->length, cell, iLeft, halfedges, nHalfedges, edge, va, vb, vz, lastBorderSegment, abs_fn = std::abs;

        while (iCell--) {
            cell = cells[iCell];
            // prune, order halfedges counterclockwise, then add missing ones
            // required to close cells
            if (!cell->prepareHalfedges()) {
                continue;
            }
            if (!cell->closeMe) {
                continue;
            }
            // find first 'unclosed' point.
            // an 'unclosed' point will be the end point of a halfedge which
            // does not match the start point of the following halfedge
            halfedges = cell->halfedges;
            nHalfedges = halfedges->length;
            // special case: only one site, in which case, the viewport is the cell
            // ...

            // all other cases
            iLeft = 0;
            while (iLeft < nHalfedges) {
                va = halfedges[iLeft].getEndpoint();
                vz = halfedges[(iLeft + 1) % nHalfedges].getStartpoint();
                // if end point is not equal to start point, we need to add the missing
                // halfedge(s) up to vz
                if (abs_fn(va->x - vz->x) >= 1e-9 || abs_fn(va->y - vz->y) >= 1e-9) {

                    // rhill 2013-12-02:
                    // "Holes" in the halfedges are not necessarily always adjacent.
                    // https://github.com/gorhill/Javascript-Voronoi/issues/16

                    // find entry point:
                    switch (true) {

                    // walk downward along left side
                    case this->equalWithEpsilon(va->x, xl) && this->lessThanWithEpsilon(va->y, yb):
                        lastBorderSegment = this->equalWithEpsilon(vz->x, xl);
                        vb = this->createVertex(xl, lastBorderSegment ? vz->y : yb);
                        edge = this->createBorderEdge(cell->site, va, vb);
                        iLeft++;
                        halfedges->splice(iLeft, 0, this->createHalfedge(edge, cell->site, nullptr));
                        nHalfedges++;
                        if (lastBorderSegment) {
                            break;
                        }
                        va = vb;
                        // fall through

                    // walk rightward along bottom side
                    case this->equalWithEpsilon(va->y, yb) && this->lessThanWithEpsilon(va->x, xr):
                        lastBorderSegment = this->equalWithEpsilon(vz->y, yb);
                        vb = this->createVertex(lastBorderSegment ? vz->x : xr, yb);
                        edge = this->createBorderEdge(cell->site, va, vb);
                        iLeft++;
                        halfedges->splice(iLeft, 0, this->createHalfedge(edge, cell->site, nullptr));
                        nHalfedges++;
                        if (lastBorderSegment) {
                            break;
                        }
                        va = vb;
                        // fall through

                    // walk upward along right side
                    case this->equalWithEpsilon(va->x, xr) && this->greaterThanWithEpsilon(va->y, yt):
                        lastBorderSegment = this->equalWithEpsilon(vz->x, xr);
                        vb = this->createVertex(xr, lastBorderSegment ? vz->y : yt);
                        edge = this->createBorderEdge(cell->site, va, vb);
                        iLeft++;
                        halfedges->splice(iLeft, 0, this->createHalfedge(edge, cell->site, nullptr));
                        nHalfedges++;
                        if (lastBorderSegment) {
                            break;
                        }
                        va = vb;
                        // fall through

                    // walk leftward along top side
                    case this->equalWithEpsilon(va->y, yt) && this->greaterThanWithEpsilon(va->x, xl):
                        lastBorderSegment = this->equalWithEpsilon(vz->y, yt);
                        vb = this->createVertex(lastBorderSegment ? vz->x : xl, yt);
                        edge = this->createBorderEdge(cell->site, va, vb);
                        iLeft++;
                        halfedges->splice(iLeft, 0, this->createHalfedge(edge, cell->site, nullptr));
                        nHalfedges++;
                        if (lastBorderSegment) {
                            break;
                        }
                        va = vb;
                        // fall through

                        // walk downward along left side
                        lastBorderSegment = this->equalWithEpsilon(vz->x, xl);
                        vb = this->createVertex(xl, lastBorderSegment ? vz->y : yb);
                        edge = this->createBorderEdge(cell->site, va, vb);
                        iLeft++;
                        halfedges->splice(iLeft, 0, this->createHalfedge(edge, cell->site, nullptr));
                        nHalfedges++;
                        if (lastBorderSegment) {
                            break;
                        }
                        va = vb;
                        // fall through

                        // walk rightward along bottom side
                        lastBorderSegment = this->equalWithEpsilon(vz->y, yb);
                        vb = this->createVertex(lastBorderSegment ? vz->x : xr, yb);
                        edge = this->createBorderEdge(cell->site, va, vb);
                        iLeft++;
                        halfedges->splice(iLeft, 0, this->createHalfedge(edge, cell->site, nullptr));
                        nHalfedges++;
                        if (lastBorderSegment) {
                            break;
                        }
                        va = vb;
                        // fall through

                        // walk upward along right side
                        lastBorderSegment = this->equalWithEpsilon(vz->x, xr);
                        vb = this->createVertex(xr, lastBorderSegment ? vz->y : yt);
                        edge = this->createBorderEdge(cell->site, va, vb);
                        iLeft++;
                        halfedges->splice(iLeft, 0, this->createHalfedge(edge, cell->site, nullptr));
                        nHalfedges++;
                        if (lastBorderSegment) {
                            break;
                        }
                        // fall through

                    default:
                        throw "Voronoi.closeCells() > this makes no sense!";
                    }
                }
                iLeft++;
            }
            cell->closeMe = false;
        }
    }

    // ---------------------------------------------------------------------------
    // Debugging helper
    /*
Voronoi.prototype.dumpBeachline = function(y) {
    console.log('Voronoi.dumpBeachline(%f) > Beachsections, from left to right:', y);
    if ( !this.beachline ) {
        console.log('  None');
        }
    else {
        var bs = this.beachline.getFirst(this.beachline.root);
        while ( bs ) {
            console.log('  site %d: xl: %f, xr: %f', bs.site.voronoiId, this.leftBreakPoint(bs, y), this.rightBreakPoint(bs, y));
            bs = bs.rbNext;
            }
        }
    }
*/

    // ---------------------------------------------------------------------------
    // Helper: Quantize sites

    // rhill 2013-10-12:
    // This is to solve https://github.com/gorhill/Javascript-Voronoi/issues/15
    // Since not all users will end up using the kind of coord values which would
    // cause the issue to arise, I chose to let the user decide whether or not
    // he should sanitize his coord values through this helper. This way, for
    // those users who uses coord values which are known to be fine, no overhead is
    // added.

    int quantizeSites(sites) //Voronoi::prototype->quantizeSites = function(sites)
    {
        QSharedPointer<var> ε = this->ε, n = sites->length, site;
        while (n--) {
            site = sites[n];
            site->x = std::floor(site->x / ε) * ε;
            site->y = std::floor(site->y / ε) * ε;
        }
    }

    // ---------------------------------------------------------------------------
    // Helper: Recycle diagram: all vertex, edge and cell objects are
    // "surrendered" to the Voronoi object for reuse.
    // TODO: rhill-voronoi-core v2: more performance to be gained
    // when I change the semantic of what is returned.

    int recycle(diagram) //Voronoi::prototype->recycle = function(diagram)
    {
        if (diagram) {
            if (std::dynamic_pointer_cast<shared_from_this()::Diagram>(diagram) != nullptr) {
                this->toRecycle = diagram;
            } else {
                throw 'Voronoi::recycleDiagram() > Need a Diagram object.';
            }
        }
    }
    int compute(QVector<Vertex> sites, QRectF bbox) //Voronoi::prototype->compute = function(sites, bbox)
    {
        var startTime = Date();
        this->reset();
        if (this->toRecycle) {
            this->vertexJunkyard = this->vertexJunkyard.concat(this->toRecycle.vertices);
            this->edgeJunkyard = this->edgeJunkyard.concat(this->toRecycle.edges);
            this->cellJunkyard = this->cellJunkyard.concat(this->toRecycle.cells);
            this->toRecycle = nullptr;
        }
        QVector<Vertex> siteEvents(sites);
        sites.clear(); //sites::slice(0);

        std::sort(siteEvents.begin(), siteEvents.end(), [](const Vertex& a, const Vertex& b) {
            const double r = b.y - a.y;
            if (r)
                return r;
            return b.x - a.x;
        });

        //        siteEvents::sort(function(const Vertex &a,const Vertex &b) {
        //            var r = b.y - a.y;
        //            if (r) {
        //                return r;
        //            }
        //            return b->x - a->x;
        //        });
        Vertex site = siteEvents.takeLast();
        int siteid = 0, xsitex, xsitey;
        QVector<Cell> cells = this->cells;
        CircleEvent circle;
        for (;;) {
            circle = this->firstCircleEvent;
            if (site && (!circle || site->y < circle->y || (site->y == = circle->y && site->x < circle->x))) {
                if (site->x != = xsitex || site->y != = xsitey) {
                    cells[siteid] = this->createCell(site);
                    site->voronoiId = siteid++;
                    this->addBeachsection(site);
                    xsitey = site->y;
                    xsitex = site->x;
                }
                site = siteEvents->pop();
            } else if (circle) {
                this->removeBeachsection(circle->arc);
            } else {
                break;
            }
        }
        this->clipEdges(bbox);
        this->closeCells(bbox);
        var stopTime = Date();
        var diagram = std::make_shared<this->Diagram>();
        diagram->cells = this->cells;
        diagram->edges = this->edges;
        diagram->vertices = this->vertices;
        diagram->execTime = stopTime::getTime() - startTime::getTime();
        this->reset();
        return diagram;
    }
}

#endif // VORONOI_H
