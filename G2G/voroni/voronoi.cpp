#include "voronoi.h"

// ---------------------------------------------------------------------------
// Top-level Fortune loop
// rhill 2011-05-19:
//   Voronoi sites are kept client-side now, to allow
//   user to freely modify content. At compute time,
//   *references* to sites are copied locally.
// to measure execution time
// init internal state
// any diagram data available for recycling?
// I do that here so that this is included in execution time
// Initialize site event queue
// process queue
// main loop
// we need to figure whether we handle a site or circle event
// for this we find out if there is a site event and it is
// 'earlier' than the circle event
// add beach section
// only if site is not a duplicate
// first create cell for new site
// then create a beachsection for that site
// remember last site coords to detect duplicate
// remove beach section
// all done, quit
// wrapping-up:
//   connect dangling edges to bounding box
//   cut edges as per bounding box
//   discard edges completely outside bounding box
//   discard edges which are point-like
//   add missing edges in order to close opened cells
// to measure execution time
// prepare return values
// clean up
/// <summary>
///*************************************************************************** </summary>

Voronoi::Voronoi()
{
    vertices = nullptr;
    edges.reset();
    cells.reset();
    toRecycle = nullptr;
    beachsectionJunkyard.clear();
    circleEventJunkyard.clear();
    vertexJunkyard.clear();
    edgeJunkyard.clear();
    cellJunkyard.clear();
}
