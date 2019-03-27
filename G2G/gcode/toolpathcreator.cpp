#include "toolpathcreator.h"
#include "forms/materialsetupform.h"
#include <QCoreApplication>
#include <QDebug>
#include <QElapsedTimer>
#include <QFile>
#include <QSettings>
#include <QStack>
#include <algorithm>
#include <aperture.h>
#include <gi/bridgeitem.h>
#include <scene.h>

void fixBegin(Path& path)
{
    IntPoint p = path.first();
    int s = 0;
    for (int i = 1, end = path.size(); i < end; ++i) {
        if (p.Y >= path[i].Y) {
            p.Y = path[i].Y;
            s = i;
            if (p.X > path[i].X) {
                p.X = path[i].X;
                s = i;
            }
        }
    }
    if (s)
        std::rotate(path.begin(), path.begin() + s, path.end());
}

Paths sortByStratDistance(Paths src)
{
    Paths dst;
    dst.reserve(src.size());
    IntPoint p1(MaterialSetup::homePos.x() * uScale, MaterialSetup::homePos.y() * uScale);
    while (src.size()) {
        int s = 0;
        IntPoint p2;
        double l1 = Length(p1, p2);
        for (int i = 0; i < src.size(); ++i) {
            p2 = src[i].first();
            double l2 = Length(p1, p2);
            if (l1 > l2) {
                l1 = l2;
                s = i;
            }
        }
        dst.append(src.takeAt(s));
        p1 = dst.last().last();
    }
    return dst;
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

ToolPathCreator::ToolPathCreator(const Paths& value, const bool convent)
    : m_workingPaths(value)
    , m_convent(convent)
{
}

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

        for (Paths paths : m_groupedPaths) {
            ClipperOffset offset;
            offset.AddPaths(paths, jtRound, etClosedPolygon);
            offset.Execute(paths, -m_dOffset);
            //            Paths tmpPaths;

            fillPaths.append(paths);

            {

                Paths tmpPaths;
                int k = 1;
                QStack<Paths> stack;
                stack.push(paths);
                do {
                    offset.Clear();
                    offset.AddPaths(paths, jtMiter, etClosedPolygon);
                    offset.Execute(tmpPaths, -m_stepOver * k++);
                    if (stack.top().size() == tmpPaths.size()) {
                        for (int i = 0; i < tmpPaths.size(); ++i) {
                            stack.top()[i].append(tmpPaths[i]);
                        }
                    } else if (!tmpPaths.isEmpty()) {
                        stack.push(tmpPaths);
                    }
                } while (tmpPaths.size());
                while (!stack.isEmpty()) {
                    m_returnPaths.append(stack.pop());
                }
            }

            //            if (steps) {
            //                int counter = steps;
            //                if (counter > 1) {
            //                    do {
            //                        if (counter == 1)
            //                            fillPaths.append(paths);
            //                        tmpPaths.append(paths);
            //                        ClipperOffset offset;
            //                        offset.AddPaths(paths, jtMiter, etClosedPolygon);
            //                        offset.Execute(paths, -stepOver);
            //                    } while (paths.size() && --counter);
            //                } else {
            //                    tmpPaths.append(paths);
            //                    fillPaths.append(paths);
            //                }
            //            } else {

            {
                //            Pathss pathss;
                //            pathss.append(paths);
                //            //            QVector<bool> flags;
                //            //for (Path& path : pathss.last()) {
                //            //                flags.append(Orientation(path));
                //            //path.append(path.first());
                //            //                if (!m_convent)
                //            //                    ReversePath(path);
                //            //}
                //            qDebug("DoOffset");
                //            DoOffset(paths, pathss /*, flags*/);
                //                for (const Paths& paths : pathss) {
                //                    m_returnPaths.append(paths);
                //                }
            }

            //            do {
            //                ClipperOffset offset;
            //                offset.AddPaths(paths, jtMiter, etClosedPolygon);
            //                offset.Execute(paths, -stepOver);
            //                if (tmpPaths.size() == paths.size()) {
            //                    for (int i = 0; i < tmpPaths.size(); ++i) {
            //                        Path path(paths[i]);
            //                        (m_convent ^ Orientation(path)) ? ReversePath(path) : fixBegin(path);
            //                        path.append(path.first());
            //                        tmpPaths[i].append(path);
            //                    }
            //                } else {
            //                    m_returnPaths.append(tmpPaths);
            //                    tmpPaths = paths;
            //                    for (int i = 0; i < tmpPaths.size(); ++i) {
            //                        (m_convent ^ Orientation(tmpPaths[i])) ? ReversePath(tmpPaths[i]) : fixBegin(tmpPaths[i]);
            //                        tmpPaths[i].append(tmpPaths[i].first());
            //                    }
            //                }
            //            } while (paths.size());

            //            m_returnPaths.append(tmpPaths);
        }

        if (m_returnPaths.size() == 0)
            return nullptr;

        //        for (Path& path : m_returnPaths) {
        //            (m_convent ^ Orientation(path)) ? ReversePath(path) : fixBegin(path);
        //            //if (path.first() != path.last())
        //            path.append(path.first());
        //        }

        // merge result toolPaths
        //        for (int i = 0; i < m_returnPaths.size(); ++i) {
        //            for (int j = 0; j < m_returnPaths.size(); ++j) {
        //                if (i == j)
        //                    continue;
        //                if (Length(m_returnPaths[i].last(), m_returnPaths[j].last()) < m_toolDiameter) {
        //                    m_returnPaths[i].append(m_returnPaths[j]);
        //                    m_returnPaths.remove(j);
        //                    i = 0;
        //                    break;
        //                }
        //                if (0) {
        //                    IntPoint p1(m_returnPaths[i][0]), p2(m_returnPaths[i][1]), p(m_returnPaths[j][0]);
        //                    const double A = p1.Y - p2.Y;
        //                    const double B = p1.X - p2.X;
        //                    const double C = p1.Y * p2.X - p2.Y * p1.X;
        //                    const double H = (A * p.X + B * p.Y + C) / sqrt(A * A + B * B);
        //                    if (abs(H) < m_toolDiameter) {
        //                        m_returnPaths[i].append(m_returnPaths[j]);
        //                        m_returnPaths.remove(j);
        //                        i = 0;
        //                        break;
        //                    }
        //                }
        //            }
        //        }
        //        std::reverse(m_returnPaths.begin(), m_returnPaths.end());
    } else {
        if (side) {
            groupedPaths(CutoffPaths, m_toolDiameter + 5);
        } else
            groupedPaths(CopperPaths);

        ClipperOffset offset(uScale, uScale / 1000);
        Clipper clipper;
        Paths tmpPaths;
        Pathss sortedPathss;

        for (Paths paths : m_groupedPaths) {
            offset.Clear();
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

            clipper.Clear();
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
            for (Paths paths : sortedPathss) {
                m_returnPaths.append(paths);
            }
            tmpPaths.clear();
            sortedPathss.clear();
        }

        if (m_returnPaths.size() == 0)
            return nullptr;

        //        if (m_convent) {
        for (Path& path : m_returnPaths) {
            if (m_convent ^ !Orientation(path))
                ReversePath(path);
            else
                fixBegin(path);
            if (path.first() != path.last())
                path.append(path.first());
        }
        //    }
        //    else
        //    {
        //        for (Path& path : m_returnPaths) {
        //            if (Orientation(path))
        //                ReversePath(path);
        //            else
        //                fixBegin(path);
        //        }
        //    }
        //        for (Path& path : m_returnPaths)
        //            if (path.first() != path.last())
        //                path.append(path.first());

        std::reverse(m_returnPaths.begin(), m_returnPaths.end());
    }

    return new GCodeFile(sortByStratDistance(m_returnPaths), tool, depth, Pocket, fillPaths);
}

QPair<GCodeFile*, GCodeFile*> ToolPathCreator::createPocket2(const QPair<Tool, Tool>& /*tool*/, double /*depth*/, bool /*side*/, int /*steps*/)
{
    QPair<GCodeFile*, GCodeFile*> gcf;
    return gcf;
}

GCodeFile* ToolPathCreator::createProfile(const Tool& tool, double depth, const SideOfMilling side)
{

    m_toolDiameter = tool.getDiameter(depth);

    if (side == On) {
        // execute offset
        m_returnPaths = m_workingPaths;
        if (!m_convent)
            ReversePaths(m_returnPaths);
    } else {
        double dOffset;
        // calc offset
        if (side == Outer)
            dOffset = +m_toolDiameter * uScale * 0.5;
        else
            dOffset = -m_toolDiameter * uScale * 0.5;

        // execute offset
        ClipperOffset offset;
        for (Paths& paths : groupedPaths(CopperPaths))
            offset.AddPaths(paths, jtRound, etClosedPolygon);

        offset.Execute(m_returnPaths, dOffset);

        if (m_returnPaths.size() == 0)
            return nullptr;

        // fix direction
        if (side == Outer && m_convent)
            ReversePaths(m_returnPaths);
        else if (side == Inner && !m_convent)
            ReversePaths(m_returnPaths);
    }

    for (Path& path : m_returnPaths)
        fixBegin(path);

    for (Path& path : m_returnPaths)
        if (path.first() != path.last())
            path.append(path.first());

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
                if (!Orientation(m_returnPaths[index]))
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

void ToolPathCreator::DoOffset(const Paths& paths, Pathss& pathss)
{
    if (paths.isEmpty())
        return;
    static bool fl = false;
    Paths wPaths;

    QVector<bool> flags;
    for (const Path& path : paths)
        flags.append(Orientation(path));
    qDebug() << flags;

    if (flags.contains(false)) {
        qDebug() << "NNN";
        ClipperOffset offset;
        offset.AddPaths(paths, fl ? jtMiter : jtRound, etClosedPolygon);
        offset.Execute(wPaths, -m_stepOver);
        fl = true;
        if (wPaths.isEmpty())
            return;
        Paths tmp(wPaths);
        for (Path& path : tmp) {
            //path.append(path.first());
            //            if (!m_convent)
            //                ReversePath(path);
        }
        if (paths.size() == tmp.size()) {
            bool fl = true;
            for (int i = 0; i < tmp.size() && fl; ++i) {
                if (flags[i])
                    fl = PointInPolygon(tmp[i][0], pathss.last()[i]) > 0;
                else
                    fl = PointInPolygon(pathss.last()[i][0], tmp[i]) > 0;
            }
            if (fl) {
                qDebug() << "append N";
                for (int i = 0; i < tmp.size(); ++i)
                    pathss.last()[i].append(tmp[i]);
                DoOffset(wPaths, pathss);
            } else {
                qDebug() << "insert N1";
                pathss.append(tmp);
                DoOffset(wPaths, pathss);
            }
        } else {
            qDebug() << "insert N2";
            pathss.append(tmp);
            DoOffset(wPaths, pathss);
        }
    } else {
        for (const Path& path : paths) {
            qDebug() << "111";
            ClipperOffset offset;
            offset.AddPath(path, fl ? jtMiter : jtRound, etClosedPolygon);
            offset.Execute(wPaths, -m_stepOver);
            fl = true;
            if (wPaths.isEmpty())
                return;
            Paths tmp(wPaths);
            for (Path& path : tmp) {
                flags.append(Orientation(path));
                //path.append(path.first());
                //                if (!m_convent)
                //                    ReversePath(path);
            }
            if (wPaths.size() == 1) {
                qDebug() << "append";
                if (PointInPolygon(tmp[0][0], pathss.last()[0]))
                    pathss.last()[0].append(tmp[0]);
                else
                    pathss.append({ tmp[0] });
                DoOffset(wPaths, pathss);
            } else {
                for (int i = 0; i < tmp.size(); ++i) {
                    qDebug() << "insert";
                    pathss.append({ tmp[i] });
                    DoOffset({ wPaths[i] }, pathss);
                }
            }
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
