#include "toolpathcreator.h"
#include "forms/materialsetupform.h"
#include "toolpathcreator.h"
#include "voroni/jc_voronoi.h"
#include <QCoreApplication>
#include <QDebug>
#include <QElapsedTimer>
#include <QFile>
#include <QSettings>
#include <QStack>
#include <algorithm>
#include <filetree/filemodel.h>
#include <gbraperture.h>
#include <gi/bridgeitem.h>
#include <limits>
#include <scene.h>

void fixBegin(Path& path)
{
    IntPoint pt(path.first());
    int rotate = 0;
    for (int i = 1, end = path.size(); i < end; ++i) {
        if (pt.Y >= path[i].Y) {
            pt.Y = path[i].Y;
            rotate = i;
            if (pt.X > path[i].X) {
                pt.X = path[i].X;
                rotate = i;
            }
        }
    }
    if (rotate)
        std::rotate(path.begin(), path.begin() + rotate, path.end());
}

Paths& sortByStratDistance(Paths& src)
{
    IntPoint startPt(toIntPoint(MaterialSetup::homePoint->pos() + MaterialSetup::zeroPoint->pos()));
    for (int firstIdx = 0; firstIdx < src.size(); ++firstIdx) {
        int swapIdx = firstIdx;
        double destLen = std::numeric_limits<double>::max();
        for (int secondIdx = firstIdx; secondIdx < src.size(); ++secondIdx) {
            const double length = Length(startPt, src[secondIdx].first());
            if (destLen > length) {
                destLen = length;
                swapIdx = secondIdx;
            }
        }
        startPt = src[swapIdx].last();
        if (swapIdx != firstIdx)
            std::swap(src[firstIdx], src[swapIdx]);
    }
    return src;
}

Pathss& sortByStratDistance2(Pathss& src)
{
    IntPoint startPt(toIntPoint(MaterialSetup::homePoint->pos() + MaterialSetup::zeroPoint->pos()));
    for (int firstIdx = 0; firstIdx < src.size(); ++firstIdx) {
        int swapIdx = firstIdx;
        double destLen = std::numeric_limits<double>::max();
        for (int secondIdx = firstIdx; secondIdx < src.size(); ++secondIdx) {
            const double length = Length(startPt, src[secondIdx].first().first());
            if (destLen > length) {
                destLen = length;
                swapIdx = secondIdx;
            }
        }
        startPt = src[swapIdx].last().last();
        if (swapIdx != firstIdx)
            std::swap(src[firstIdx], src[swapIdx]);
    }
    return src;
}

bool PointOnPolygon(const QLineF& l2, const Path& path, IntPoint* ret = nullptr)
{
    int cnt = path.size();
    if (cnt < 3)
        return false;
    IntPoint pt1 = path[0];
    QPointF p;
    for (int i = 1; i <= cnt; ++i) {
        IntPoint pt2(i == cnt ? path[0] : path[i]);
        QLineF l1(toQPointF(pt1), toQPointF(pt2));
        if (QLineF::BoundedIntersection == l1.intersect(l2, &p)) {
            if (ret)
                *ret = toIntPoint(p);
            return true;
        }
        pt1 = pt2;
    }
    return false;
}
////////////////////////////////////////////////////////////////
/// \brief ToolPathCreator::ToolPathCreator
/// \param value
/// \param convent
///
ToolPathCreator::ToolPathCreator(const Paths& value, const bool convent, SideOfMilling side)
    : m_side(side)
    , m_workingPaths(value)
    , m_convent(convent)
{
}
////////////////////////////////////////////////////////////////
/// \brief ToolPathCreator::createPocket
/// \param tool
/// \param depth
/// \param side
/// \param steps
/// \param ex
/// \return
///
GCodeFile* ToolPathCreator::createPocket(const Tool& tool, const double depth, const int steps)
{
    if (m_side == On)
        return nullptr;

    m_toolDiameter = tool.getDiameter(depth) * uScale;
    m_dOffset = m_toolDiameter / 2;
    m_stepOver = tool.stepover * uScale;

    Paths fillPaths;

    //    if (ex) {
    switch (m_side) {
    case Outer:
        groupedPaths(CutoffPaths, m_toolDiameter + 5);
        if (m_groupedPaths.size() > 1 && m_groupedPaths.first().size() == 2)
            m_groupedPaths.removeFirst();
        break;
    case Inner:
        groupedPaths(CopperPaths);
        break;
    }

    for (Paths paths : m_groupedPaths) {
        Paths tmpPaths;
        ClipperOffset offset(uScale, uScale / 1000);
        offset.AddPaths(paths, /*jtMiter*/ jtRound, etClosedPolygon);
        offset.Execute(paths, -m_dOffset);
        fillPaths.append(paths);
        if (steps) {
            int counter = steps;
            if (counter > 1) {
                do {
                    if (counter == 1)
                        fillPaths.append(paths);
                    tmpPaths.append(paths);
                    offset.Clear();
                    offset.AddPaths(paths, jtMiter, etClosedPolygon);
                    offset.Execute(paths, -m_stepOver);
                } while (paths.size() && --counter);
            } else {
                tmpPaths.append(paths);
                fillPaths.append(paths);
            }
        } else {
            do {
                tmpPaths.append(paths);
                offset.Clear();
                offset.AddPaths(paths, jtMiter, etClosedPolygon);
                offset.Execute(paths, -m_stepOver);
            } while (paths.size());
        }
        m_returnPaths.append(tmpPaths);
    }

    if (m_returnPaths.size() == 0)
        return nullptr;

    Clipper clipper;
    clipper.AddPaths(m_returnPaths, ptSubject, true);
    IntRect r(clipper.GetBounds());
    int k = tool.diameter * uScale;
    Path outer = {
        IntPoint(r.left - k, r.bottom + k),
        IntPoint(r.right + k, r.bottom + k),
        IntPoint(r.right + k, r.top - k),
        IntPoint(r.left - k, r.top - k)
    };
    PolyTree polyTree;
    clipper.AddPath(outer, ptSubject, true);
    clipper.Execute(ctUnion, polyTree, pftEvenOdd);
    m_returnPaths.clear();
    grouping2(polyTree.GetFirst(), &m_returnPaths);
    ReversePaths(m_returnPaths);
    sortByStratDistance(m_returnPaths);

    return new GCodeFile(m_returnPaths, tool, depth, Pocket, fillPaths);
}

QPair<GCodeFile*, GCodeFile*> ToolPathCreator::createPocket2(const QPair<Tool, Tool>& tool, double depth)
{

    QPair<GCodeFile*, GCodeFile*> files{ nullptr, nullptr };
    if (m_side == On)
        return files;

    Paths fillPaths;

    {
        m_toolDiameter = tool.first.getDiameter(depth) * uScale;
        m_dOffset = m_toolDiameter / 2;
        m_stepOver = tool.first.stepover * uScale;

        switch (m_side) {
        case Outer:
            groupedPaths(CutoffPaths, m_toolDiameter + 5);
            if (m_groupedPaths.size() > 1 && m_groupedPaths.first().size() == 2)
                m_groupedPaths.removeFirst();
            break;
        case Inner:
            groupedPaths(CopperPaths);
            break;
        }

        for (Paths paths : m_groupedPaths) {
            Paths tmpPaths;
            ClipperOffset offset(uScale, uScale / 1000);
            offset.AddPaths(paths, /*jtMiter*/ jtRound, etClosedPolygon);
            offset.Execute(paths, -m_dOffset);
            fillPaths.append(paths);

            do {
                tmpPaths.append(paths);
                offset.Clear();
                offset.AddPaths(paths, jtMiter, etClosedPolygon);
                offset.Execute(paths, -m_stepOver);
            } while (paths.size());

            m_returnPaths.append(tmpPaths);
        }

        if (m_returnPaths.size() == 0)
            return files;

        Clipper clipper;
        clipper.AddPaths(m_returnPaths, ptSubject, true);
        IntRect r(clipper.GetBounds());
        int k = tool.first.getDiameter(depth) * uScale;
        Path outer = {
            IntPoint(r.left - k, r.bottom + k),
            IntPoint(r.right + k, r.bottom + k),
            IntPoint(r.right + k, r.top - k),
            IntPoint(r.left - k, r.top - k)
        };
        PolyTree polyTree;
        clipper.AddPath(outer, ptSubject, true);
        clipper.Execute(ctUnion, polyTree, pftEvenOdd);
        m_returnPaths.clear();
        grouping2(polyTree.GetFirst(), &m_returnPaths);
        ReversePaths(m_returnPaths);
        sortByStratDistance(m_returnPaths);
        // files.first = new GCodeFile(m_returnPaths, tool.first, depth, Pocket, fillPaths);
        m_returnPaths.clear();
    }
    {
        m_toolDiameter = tool.second.getDiameter(depth) * uScale;
        {
            ClipperOffset offset(uScale, uScale / 1000);
            offset.AddPaths(fillPaths, /*jtMiter*/ jtRound, etClosedPolygon);
            offset.Execute(fillPaths, m_dOffset - m_toolDiameter / 2);
            m_dOffset = m_toolDiameter / 2;
            //offset.Clear();
            //offset.AddPaths(m_workingPaths, /*jtMiter*/ jtRound, etClosedPolygon);
            //offset.Execute(m_workingPaths, m_dOffset);
        }

        m_stepOver = tool.second.stepover * uScale;

        {
            Clipper clipper;
            clipper.AddPaths(m_workingPaths, ptSubject, true);
            ReversePaths(fillPaths);
            clipper.AddPaths(fillPaths, ptClip, true);
            clipper.Execute(ctXor, m_workingPaths, pftNegative);
        }
        //        groupedPaths(CopperPaths);
        m_workingPaths.removeFirst();
        //        if (m_workingPaths.size())
        //            files.second = new GCodeFile(m_workingPaths, tool.second, depth, Pocket, {});

        //        ReversePaths(fillPaths);
        //        m_workingPaths.append(fillPaths);
        //        fillPaths.clear();
        //        switch (m_side) {
        //        case Outer:
        //            groupedPaths(CopperPaths);
        //            break;
        //        case Inner:
        //            groupedPaths(CutoffPaths, m_toolDiameter + 5);
        //            if (m_groupedPaths.size() > 1 && m_groupedPaths.first().size() == 2)
        //                m_groupedPaths.removeFirst();
        //            break;
        //        }

        {
            Paths paths(m_workingPaths);
            Paths tmpPaths(m_workingPaths);
            ClipperOffset offset(uScale, uScale / 1000);
            offset.AddPaths(paths, /*jtMiter*/ jtRound, etClosedPolygon);
            offset.Execute(paths, m_dOffset);
            fillPaths.append(paths);
            int k1 = 100;
            do {
                tmpPaths.append(paths);
                offset.Clear();
                offset.AddPaths(paths, jtMiter, etClosedPolygon);
                offset.Execute(paths, -m_stepOver);
            } while (paths.size() && --k1);
            m_returnPaths.append(tmpPaths);
        }

        //        groupedPaths(CopperPaths);
        //        for (Paths paths : m_groupedPaths) {
        //            Paths tmpPaths;
        //            ClipperOffset offset(uScale, uScale / 1000);
        //            offset.AddPaths(paths, /*jtMiter*/ jtRound, etClosedPolygon);
        //            offset.Execute(paths, -m_dOffset);
        //            fillPaths.append(paths);
        //            int k = 100;
        //            do {
        //                tmpPaths.append(paths);
        //                offset.Clear();
        //                offset.AddPaths(paths, jtMiter, etClosedPolygon);
        //                offset.Execute(paths, -m_stepOver);
        //            } while (paths.size() && --k);
        //            m_returnPaths.append(tmpPaths);
        //        }

        if (m_returnPaths.size() == 0)
            return files;

        Clipper clipper;
        clipper.AddPaths(m_returnPaths, ptSubject, true);
        IntRect r(clipper.GetBounds());
        int k = tool.second.getDiameter(depth) * uScale;
        Path outer = {
            IntPoint(r.left - k, r.bottom + k),
            IntPoint(r.right + k, r.bottom + k),
            IntPoint(r.right + k, r.top - k),
            IntPoint(r.left - k, r.top - k)
        };
        PolyTree polyTree;
        clipper.AddPath(outer, ptSubject, true);
        clipper.Execute(ctUnion, polyTree, pftEvenOdd);
        m_returnPaths.clear();
        grouping2(polyTree.GetFirst(), &m_returnPaths);
        ReversePaths(m_returnPaths);
        sortByStratDistance(m_returnPaths);
        files.second = new GCodeFile(m_returnPaths, tool.second, depth, Pocket, {} /*fillPaths*/);
    }
    return files;
}
////////////////////////////////////////////////////////////////
/// \brief ToolPathCreator::createProfile
/// \param tool
/// \param depth
/// \param side
/// \return
///
GCodeFile* ToolPathCreator::createProfile(const Tool& tool, double depth)
{
    m_toolDiameter = tool.getDiameter(depth);
    // execute offset
    if (m_side == On) {
        m_returnPaths = m_workingPaths;

        for (Path& path : m_returnPaths)
            path.append(path.first());

        // fix direction
        if (m_convent)
            ReversePaths(m_returnPaths);

        if (m_workingRawPaths.size())
            m_returnPaths.append(m_workingRawPaths);

    } else {
        // calc offset
        const double dOffset = (m_side == Outer) ? +m_toolDiameter * uScale * 0.5 : -m_toolDiameter * uScale * 0.5;

        // execute offset
        if (!m_workingPaths.isEmpty()) {
            ClipperOffset offset;
            for (Paths& paths : groupedPaths(CopperPaths))
                offset.AddPaths(paths, jtRound, etClosedPolygon);
            offset.Execute(m_returnPaths, dOffset);
        }
        if (!m_workingRawPaths.isEmpty()) {
            ClipperOffset offset;
            offset.AddPaths(m_workingRawPaths, jtRound, etOpenRound);
            offset.Execute(m_workingRawPaths, dOffset);
        }

        if (!m_workingRawPaths.isEmpty())
            m_returnPaths.append(m_workingRawPaths);

        // fix direction
        if (m_side == Outer && !m_convent)
            ReversePaths(m_returnPaths);
        else if (m_side == Inner && m_convent)
            ReversePaths(m_returnPaths);

        for (Path& path : m_returnPaths)
            path.append(path.first());

        if (m_returnPaths.size() == 0)
            return nullptr;
    }

    // find Bridges
    QVector<BridgeItem*> bridgeItems;
    for (QGraphicsItem* item : Scene::self->items()) {
        if (item->type() == BridgeType)
            bridgeItems.append(static_cast<BridgeItem*>(item));
    }
    // create Bridges
    if (bridgeItems.size()) {
        for (int index = 0, size = m_returnPaths.size(); index < size; ++index) {
            Path& path = m_returnPaths[index];
            QList<QPair<BridgeItem*, IntPoint>> biStack;
            for (BridgeItem* bi : bridgeItems) {
                IntPoint pt;
                if (PointOnPolygon(bi->getPath(), path, &pt))
                    biStack.append({ bi, pt });
            }
            if (!biStack.isEmpty()) {
                Paths tmpPaths;
                // create frame
                {
                    ClipperOffset offset;
                    offset.AddPath(path, jtMiter, etClosedLine);
                    offset.Execute(tmpPaths, +m_toolDiameter * uScale * 0.1);

                    Clipper clipper;
                    clipper.AddPaths(tmpPaths, ptSubject, true);
                    for (const QPair<BridgeItem*, IntPoint>& bip : biStack) {
                        clipper.AddPath(CirclePath((bip.first->lenght() + m_toolDiameter) * uScale, bip.second), ptClip, true);
                    }
                    clipper.Execute(ctIntersection, tmpPaths, pftPositive);
                }
                // cut toolPath
                {
                    Clipper clipper;
                    clipper.AddPath(path, ptSubject, false);
                    clipper.AddPaths(tmpPaths, ptClip, true);
                    PolyTree polytree;
                    clipper.Execute(ctDifference, polytree, pftPositive);
                    PolyTreeToPaths(polytree, tmpPaths);
                }
                // merge result toolPaths
                for (int i = 0; i < tmpPaths.size(); ++i) {
                    for (int j = 0; j < tmpPaths.size(); ++j) {
                        if (i == j)
                            continue;
                        if (tmpPaths[i].last() == tmpPaths[j].first()) {
                            tmpPaths[i].append(tmpPaths[j]);
                            tmpPaths.remove(j);
                            i = -1;
                            break;
                        }
                        if (tmpPaths[i].first() == tmpPaths[j].last()) {
                            tmpPaths[j].append(tmpPaths[i]);
                            tmpPaths.remove(i);
                            i = -1;
                            break;
                        }
                        if (tmpPaths[i].last() == tmpPaths[j].last()) {
                            ReversePath(tmpPaths[j]);
                            tmpPaths[i].append(tmpPaths[j]);
                            tmpPaths.remove(j);
                            i = -1;
                            break;
                        }
                    }
                }

                --size;
                m_returnPaths.remove(index--);
                m_returnPaths.append(tmpPaths);
            }
        }
    }

    return new GCodeFile(sortByStratDistance(m_returnPaths), tool, depth, Profile);
}

int indexOf(const Paths& c, const Path& t)
{
    int i = 0;
    for (const Path& path : c) {
        if (path.first() == t.first() && path.last() == t.last())
            return i;
        if (path.first() == t.last() && path.last() == t.first())
            return i;
        ++i;
    }
    return -1;
}

GCodeFile* ToolPathCreator::createVoronoi(const Tool& tool, double depth, const double k)
{
    QVector<jcv_point> points;
    points.reserve(1000000);
    //    const double k = 0.1;
    int id = 0;

    CleanPolygons(m_workingPaths, k * 0.1 * uScale);

    groupedPaths(CopperPaths);
    for (const Paths& paths : m_groupedPaths) {
        for (const Path& path : paths) {
            IntPoint tmp(path.first());
            for (const IntPoint& point : path) {
                QLineF line(toQPointF(tmp), toQPointF(point));
                if (line.length() > k) {
                    for (int i = 1, total = line.length() / k; i < total; ++i) {
                        line.setLength(i * k);
                        IntPoint point(toIntPoint(line.p2()));
                        points.append({ static_cast<jcv_real>(point.X), static_cast<jcv_real>(point.Y), id });
                    }
                }
                points.append({ static_cast<jcv_real>(point.X), static_cast<jcv_real>(point.Y), id });
                tmp = point;
            }
            QLineF line(toQPointF(tmp), toQPointF(path.first()));
            if (line.length() > k) {
                for (int i = 1, total = line.length() / k; i < total; ++i) {
                    line.setLength(i * k);
                    IntPoint point(toIntPoint(line.p2()));
                    points.append({ static_cast<jcv_real>(point.X), static_cast<jcv_real>(point.Y), id });
                }
            }
        }
        ++id;
    }

    {

        Clipper clipper;
        for (const Paths& paths : m_groupedPaths) {
            clipper.AddPaths(paths, ptClip, true);
        }
        IntRect r(clipper.GetBounds());
        jcv_rect bounding_box = {
            { static_cast<jcv_real>(r.left - uScale), static_cast<jcv_real>(r.top - uScale) },
            { static_cast<jcv_real>(r.right + uScale), static_cast<jcv_real>(r.bottom + uScale) }
        };

        Pathss edges;
        edges.resize(id);
        {
            jcv_diagram diagram;
            jcv_diagram_generate(points.size(), points.data(), &bounding_box, &diagram);
            const jcv_site* sites = jcv_diagram_get_sites(&diagram);
            for (int i = 0; i < diagram.numsites; i++) {
                jcv_graphedge* graph_edge = sites[i].edges;
                while (graph_edge) {
                    Path path{ { static_cast<cInt>(graph_edge->pos[0].x), static_cast<cInt>(graph_edge->pos[0].y) }, { static_cast<cInt>(graph_edge->pos[1].x), static_cast<cInt>(graph_edge->pos[1].y) } };
                    int index = indexOf(edges[sites[i].p.id], path);
                    if (index == -1)
                        edges[sites[i].p.id].append(path);
                    else
                        edges[sites[i].p.id].remove(index);
                    graph_edge = graph_edge->next;
                }
            }
            jcv_diagram_free(&diagram);
        }

        for (Paths& edge : edges) {
            for (Path& path : edge) {
                int index = indexOf(m_returnPaths, path);
                if (index == -1)
                    m_returnPaths.append(path);
            }
        }
    }

    // merge result toolPaths
    for (int k = 0; k < 10; ++k) { // overhead for better merge result
        for (int i = 0; i < m_returnPaths.size(); ++i) {
            qDebug() << "i" << i << m_returnPaths.size();
            for (int j = 0; j < m_returnPaths.size(); ++j) {
                if (i == j)
                    continue;
                if (m_returnPaths[i].last() == m_returnPaths[j].first()) {
                    m_returnPaths[i].append(m_returnPaths[j]);
                    m_returnPaths.remove(j--);
                    continue;
                }
                if (m_returnPaths[i].last() == m_returnPaths[j].last()) {
                    ReversePath(m_returnPaths[j]);
                    m_returnPaths[i].append(m_returnPaths[j]);
                    m_returnPaths.remove(j--);
                    continue;
                }
                if (m_returnPaths[i].first() == m_returnPaths[j].last()) {
                    m_returnPaths[j].append(m_returnPaths[i]);
                    m_returnPaths.remove(i--);
                    break;
                }
            }
        }
    }

    return new GCodeFile(sortByStratDistance(m_returnPaths), tool, depth, Voronoi);
}

Pathss& ToolPathCreator::groupedPaths(Grouping group, cInt k, bool fl)
{
    PolyTree polyTree;
    Clipper clipper;
    clipper.AddPaths(m_workingPaths, ptSubject, true);
    IntRect r(clipper.GetBounds());
    //int k = /*uScale*/ 1;
    Path outer = {
        IntPoint(r.left - k, r.bottom + k),
        IntPoint(r.right + k, r.bottom + k),
        IntPoint(r.right + k, r.top - k),
        IntPoint(r.left - k, r.top - k)
    };
    if (fl)
        ReversePath(outer);
    clipper.AddPath(outer, ptSubject, true);
    clipper.Execute(ctUnion, polyTree, pftNonZero);
    grouping(polyTree.GetFirst(), &m_groupedPaths, group);
    return m_groupedPaths;
}
////////////////////////////////////////////////////////////////
/// \brief ToolPathCreator::addRawPaths
/// \param paths
///
void ToolPathCreator::addRawPaths(Paths rawPaths)
{
    if (m_side == On) {
        m_workingRawPaths.append(rawPaths);
        return;
    }

    qDebug() << rawPaths.size();

    const double glueLen = 0.1 * uScale;
    Paths paths;

    do {
        Path path = rawPaths.takeFirst();
        for (int i = 0; i < rawPaths.size();) {
            const IntPoint& pt1 = path.last();
            const IntPoint& pt2 = rawPaths[i].first();
            const IntPoint& pt3 = rawPaths[i].last();
            if (Length(pt1, pt2) < glueLen) {
                path.append(rawPaths.takeAt(i));
                i = 0;
            } else if (Length(pt1, pt3) < glueLen) {
                ReversePath(rawPaths[i]);
                path.append(rawPaths.takeAt(i));
                i = 0;
            } else
                ++i;
        }
        paths.append(path);
    } while (rawPaths.size());

    Clipper clipper;

    for (Path path : paths) {
        if (Length(path.first(), path.last()) < glueLen) //path.first() == path.last())
            clipper.AddPath(path, ptSubject, true);
        else
            m_workingRawPaths.append(path);
    }

    IntRect r(clipper.GetBounds());
    int k = uScale * 10;
    Path outer = {
        IntPoint(r.left - k, r.bottom + k),
        IntPoint(r.right + k, r.bottom + k),
        IntPoint(r.right + k, r.top - k),
        IntPoint(r.left - k, r.top - k)
    };

    clipper.AddPath(outer, ptClip, true);
    clipper.Execute(ctXor, paths, pftEvenOdd);
    paths.takeFirst();
    qDebug() << paths.size();
    m_workingPaths.append(paths);
}

void ToolPathCreator::addPaths(const Paths& paths)
{
    m_workingPaths.append(paths);
}

void ToolPathCreator::grouping(PolyNode* node, Pathss* pathss, Grouping group)
{
    Path path;
    Paths paths;
    switch (group) {
    case CutoffPaths:
        if (!node->IsHole()) {
            path = node->Contour;
            paths.push_back(path);
            for (int i = 0, end = node->ChildCount(); i < end; ++i) {
                path = node->Childs[i]->Contour;
                paths.push_back(path);
            }
            pathss->push_back(paths);
        }

        for (int i = 0, end = node->ChildCount(); i < end; ++i)
            grouping(node->Childs[i], pathss, group);
        break;
    case CopperPaths:
        if (node->IsHole()) {
            path = node->Contour;
            paths.push_back(path);
            for (int i = 0, end = node->ChildCount(); i < end; ++i) {
                path = node->Childs[i]->Contour;
                paths.push_back(path);
            }
            pathss->push_back(paths);
        }
        for (int i = 0, end = node->ChildCount(); i < end; ++i)
            grouping(node->Childs[i], pathss, group);
        break;
    }
}

Path& ToolPathCreator::fixPath(PolyNode* node)
{
    if (m_convent ^ !node->IsHole())
        ReversePath(node->Contour);
    else {
        //std::rotate(path.begin(), path.begin() + 1, path.end());
        fixBegin(node->Contour);
    }
    node->Contour.append(node->Contour.first());
    return node->Contour;
}

void ToolPathCreator::grouping2(PolyNode* node, Paths* paths, bool fl)
{
    static bool newPath = false;
    //    static Path* lastPaph = nullptr;
    if (fl) {
        Path path(fixPath(node));

        if (paths->isEmpty() || newPath) {
            paths->append(path);
        } else {
            if (Length(paths->last().last(), path.last()) < m_toolDiameter * 1.5)
                paths->last().append(path);
            else
                paths->append(path);
        }

        //lastPaph = &node->Contour;

        if (node->ChildCount() == 1) {
            newPath = false;
            grouping2(node->Childs[0], paths, true);
        } else {
            for (int i = 0, end = node->ChildCount(); i < end; ++i) {
                newPath = true;
                grouping2(node->Childs[i], paths, true);
            }
        }
    } else {
        // Start from non hole paths
        for (int i = 0, end = node->ChildCount(); i < end; ++i) {
            qDebug() << "end1" << end << i;
            newPath = true;
            grouping2(node->Childs[i], paths, true);
        }
    }
}

//void ToolPathCreator::DoOffset(const Paths& paths, Pathss& pathss, QVector<bool> flags)
//{
//    if (paths.isEmpty())
//        return;
//    static bool fl = false;
//    qDebug() << flags;
//    QVector<bool> flags2;
//    Paths wPaths;
//    if (flags.contains(false)) {
//        qDebug() << "NNN";
//        ClipperOffset offset;
//        offset.AddPaths(paths, fl ? jtMiter : jtRound, etClosedPolygon);
//        offset.Execute(wPaths, -m_stepOver);
//        fl = true;
//        if (wPaths.isEmpty())
//            return;
//        Paths tmp(wPaths);
//        for (Path& path : tmp) {
//            flags2.append(Orientation(path));
//            path.append(path.first());
//            if (!m_convent)
//                ReversePath(path);
//        }

//        if (flags == flags2) {
//            bool fl = true;
//            for (int i = 0; i < tmp.size() && fl; ++i) {
//                if (!flags[i])
//                    fl = PointInPolygon(tmp[i][0], pathss.last()[i]);
//                else
//                    fl = PointInPolygon(pathss.last()[i][0], tmp[i]);
//            }
//            if (fl) {
//                qDebug() << "append";
//                for (int i = 0; i < tmp.size(); ++i)
//                    pathss.last()[i].append(tmp[i]);
//                DoOffset(wPaths, pathss, flags2);
//                flags = flags2;
//            } else {
//                qDebug() << "insert";
//                pathss.append(tmp);
//                DoOffset(wPaths, pathss, flags2);
//                flags = flags2;
//            }
//        } else {
//            qDebug() << "insert";
//            pathss.append(tmp);
//            DoOffset(wPaths, pathss, flags2);
//            flags = flags2;
//        }
//    } else {
//        for (const Path& path : paths) {
//            qDebug() << "111";
//            ClipperOffset offset;
//            offset.AddPath(path, fl ? jtMiter : jtRound, etClosedPolygon);
//            offset.Execute(wPaths, -m_stepOver);
//            fl = true;
//            if (wPaths.isEmpty())
//                return;
//            Paths tmp(wPaths);
//            for (Path& path : tmp) {
//                flags2.append(Orientation(path));
//                path.append(path.first());
//                if (!m_convent)
//                    ReversePath(path);
//            }
//            if (wPaths.size() == 1) {
//                qDebug() << "append";
//                if (PointInPolygon(tmp[0][0], pathss.last()[0]))
//                    pathss.last()[0].append(tmp[0]);
//                else
//                    pathss.append({ tmp[0] });
//                DoOffset(wPaths, pathss, flags2);
//                flags = flags2;
//            } else {
//                for (int i = 0; i < tmp.size(); ++i) {
//                    qDebug() << "insert";
//                    pathss.append({ tmp[i] });
//                    DoOffset({ wPaths[i] }, pathss, flags2);
//                    flags = flags2;
//                }
//            }
//        }
//    }
//}
