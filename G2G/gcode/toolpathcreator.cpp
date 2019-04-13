#include "toolpathcreator.h"
#include "forms/materialsetupform.h"
#include "toolpathcreator.h"
#include <QCoreApplication>
#include <QDebug>
#include <QElapsedTimer>
#include <QFile>
#include <QSettings>
#include <QStack>
#include <algorithm>
#include <aperture.h>
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
    IntPoint startPt(toIntPoint(MaterialSetup::homePos + MaterialSetup::zeroPos));
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
    IntPoint startPt(toIntPoint(MaterialSetup::homePos + MaterialSetup::zeroPos));
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
ToolPathCreator::ToolPathCreator(const Paths& value, const bool convent)
    : m_workingPaths(value)
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
GCodeFile* ToolPathCreator::createPocket(const Tool& tool, const double depth, const bool side, const int steps, const bool ex)
{
    m_toolDiameter = tool.getDiameter(depth) * uScale;
    m_dOffset = m_toolDiameter / 2;
    m_stepOver = tool.stepover * uScale;

    Paths fillPaths;

    if (ex) {

        if (side) {
            groupedPaths(CutoffPaths, m_toolDiameter + 5);
        } else
            groupedPaths(CopperPaths);

        if (side)
            if (m_groupedPaths.size() > 1 && m_groupedPaths.first().size() == 2)
                m_groupedPaths.removeFirst();

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
    } else {
        if (side) {
            groupedPaths(CutoffPaths, m_toolDiameter + 5);
        } else
            groupedPaths(CopperPaths);

        if (side)
            if (m_groupedPaths.size() > 1 && m_groupedPaths.first().size() == 2)
                m_groupedPaths.removeFirst();

        for (Paths paths : m_groupedPaths) {
            ClipperOffset offset(uScale, uScale / 1000);
            offset.AddPaths(paths, /*jtMiter*/ jtRound, etClosedPolygon);
            offset.Execute(paths, -m_dOffset);
            Paths tmpPaths;
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

            Pathss sortedPathss;
            Clipper clipper;
            clipper.AddPaths(tmpPaths, ptSubject, true);
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
            grouping(polyTree.GetFirst(), &sortedPathss, CopperPaths);
            for (Paths paths : sortedPathss)
                m_returnPaths.append(paths);
        }

        if (m_returnPaths.size() == 0)
            return nullptr;

        for (Path& path : m_returnPaths) {
            if (m_convent ^ !Orientation(path))
                ReversePath(path);
            else
                fixBegin(path);
            if (path.first() != path.last())
                path.append(path.first());
        }
        sortByStratDistance(m_returnPaths);
    }
    return new GCodeFile(m_returnPaths, tool, depth, Pocket, fillPaths);
}

QPair<GCodeFile*, GCodeFile*> ToolPathCreator::createPocket2(const QPair<Tool, Tool>& /*tool*/, double /*depth*/, bool /*side*/, int /*steps*/)
{
    QPair<GCodeFile*, GCodeFile*> gcf;
    return gcf;
}
////////////////////////////////////////////////////////////////
/// \brief ToolPathCreator::createProfile
/// \param tool
/// \param depth
/// \param side
/// \return
///
GCodeFile* ToolPathCreator::createProfile(const Tool& tool, double depth, const SideOfMilling side)
{
    m_toolDiameter = tool.getDiameter(depth);
    // execute offset
    if (side == On) {
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
        const double dOffset = (side == Outer) ? +m_toolDiameter * uScale * 0.5 : -m_toolDiameter * uScale * 0.5;

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
        if (side == Outer && !m_convent)
            ReversePaths(m_returnPaths);
        else if (side == Inner && m_convent)
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
                    clipper.Execute(ctDifference, polytree, pftNonZero);
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
                            i = 0;
                            break;
                        }
                        if (tmpPaths[i].first() == tmpPaths[j].last()) {
                            tmpPaths[j].append(tmpPaths[i]);
                            tmpPaths.remove(i);
                            i = 0;
                            break;
                        }
                    }
                }
                if (Orientation(m_returnPaths[index]))
                    ReversePaths(tmpPaths);
                --size;
                m_returnPaths.remove(index--);
                m_returnPaths.append(tmpPaths);
            }
        }
    }

    return new GCodeFile(sortByStratDistance(m_returnPaths), tool, depth, Profile);
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
