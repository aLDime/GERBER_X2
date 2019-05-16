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
#include <QThread>
#include <algorithm>
#include <filetree/filemodel.h>
#include <gbraperture.h>
#include <gi/bridgeitem.h>
#include <limits>
#include <scene.h>
#include <voronoi.h>

ToolPathCreator* ToolPathCreator::self = nullptr;

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

Paths& sortByStratDistance2(Paths& src)
{
    IntPoint startPt(toIntPoint(MaterialSetup::homePoint->pos() + MaterialSetup::zeroPoint->pos()));
    for (int firstIdx = 0; firstIdx < src.size(); ++firstIdx) {
        int swapIdx = firstIdx;
        double destLen = std::numeric_limits<double>::max();
        bool reverse;
        for (int secondIdx = firstIdx; secondIdx < src.size(); ++secondIdx) {
            const double lenFirst = Length(startPt, src[secondIdx].first());
            const double lenLast = Length(startPt, src[secondIdx].last());
            if (lenFirst < lenLast) {
                if (destLen > lenFirst) {
                    destLen = lenFirst;
                    swapIdx = secondIdx;
                    reverse = false;
                }
            } else {
                if (destLen > lenLast) {
                    destLen = lenLast;
                    swapIdx = secondIdx;
                    reverse = true;
                }
            }
        }
        if (reverse)
            ReversePath(src[swapIdx]);
        startPt = src[swapIdx].last();
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
ToolPathCreator::ToolPathCreator(const Paths& workingPaths, const bool convent, SideOfMilling side)
    : m_side(side)
    , m_workingPaths(workingPaths)
    , m_convent(convent)
{
}

ToolPathCreator::~ToolPathCreator()
{
    self = nullptr;
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
void ToolPathCreator::createPocket(const Tool& tool, const double depth, const int steps)
{
    try {
        self = this;

        if (m_side == On)
            return;

        // emit progress(1, 0); // progress

        m_toolDiameter = tool.getDiameter(depth) * uScale;
        m_dOffset = m_toolDiameter / 2;
        m_stepOver = tool.stepover * uScale;

        Paths fillPaths;

        if (steps) {
            groupedPaths(CopperPaths);
            if (m_side == Inner) {
                m_dOffset *= -1;
                for (Paths paths : m_groupedPathss) {
                    ClipperOffset offset(uScale);
                    offset.AddPaths(paths, jtRound, etClosedPolygon);
                    offset.Execute(paths, m_dOffset);
                    fillPaths.append(paths);

                    Paths tmpPaths;
                    int counter = steps;
                    if (counter > 1) {
                        do {
                            if (counter == 1)
                                fillPaths.append(paths);
                            tmpPaths.append(paths);
                            offset.Clear();
                            offset.AddPaths(paths, jtMiter, etClosedPolygon);
                            offset.Execute(paths, m_dOffset);
                        } while (paths.size() && --counter);
                    } else {
                        tmpPaths.append(paths);
                        fillPaths.append(paths);
                    }
                    m_returnPaths.append(tmpPaths);
                }
            } else {
                ClipperOffset offset(uScale);
                for (Paths paths : m_groupedPathss) {
                    offset.AddPaths(paths, jtRound, etClosedPolygon);
                }
                Paths paths;
                offset.Execute(paths, m_dOffset);
                fillPaths.append(paths);
                int counter = steps;
                if (counter > 1) {
                    do {
                        if (counter == 1)
                            fillPaths.append(paths);
                        m_returnPaths.append(paths);
                        offset.Clear();
                        offset.AddPaths(paths, jtMiter, etClosedPolygon);
                        offset.Execute(paths, m_dOffset);
                    } while (paths.size() && --counter);
                } else {
                    m_returnPaths.append(paths);
                    fillPaths.append(paths);
                }
            }
        } else {
            switch (m_side) {
            case Outer:
                groupedPaths(CutoffPaths, m_toolDiameter + 5);
                if (m_groupedPathss.size() > 1 && m_groupedPathss.first().size() == 2)
                    m_groupedPathss.removeFirst();
                break;
            case Inner:
                groupedPaths(CopperPaths);
                break;
            }
            for (Paths paths : m_groupedPathss) {
                ClipperOffset offset(uScale);
                offset.AddPaths(paths, jtRound, etClosedPolygon);
                offset.Execute(paths, -m_dOffset);
                fillPaths.append(paths);

                Paths tmpPaths;
                do {
                    tmpPaths.append(paths);
                    offset.Clear();
                    offset.AddPaths(paths, jtMiter, etClosedPolygon);
                    offset.Execute(paths, -m_stepOver);
                } while (paths.size());
                m_returnPaths.append(tmpPaths);
            }
        }

        if (m_returnPaths.isEmpty()) {
            emit fileReady(nullptr);
            // emit progress(0, 0); // progress
            return;
        }

        Clipper clipper;
        clipper.AddPaths(m_returnPaths, ptSubject, true);
        IntRect r(clipper.GetBounds());

        //        qDebug() << "GetBounds" << t.elapsed();
        //        t.start();

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

        //        qDebug() << "Execute polyTree" << t.elapsed();
        //        t.start();

        m_returnPaths.clear();
        grouping2(polyTree.GetFirst(), &m_returnPaths);

        //        qDebug() << "grouping2" << t.elapsed();
        //        t.start();

        ReversePaths(m_returnPaths);
        sortByStratDistance(m_returnPaths);

        //        qDebug() << "sortByStratDistance" << t.elapsed();
        //        t.start();

        if (m_returnPaths.isEmpty()) {
            emit fileReady(nullptr);
            // emit progress(0, 0); // progress
        } else {
            m_file = new GCodeFile(m_returnPaths, tool, depth, Pocket, fillPaths);
            emit fileReady(m_file);
            // emit progress(0, 0); // progress
        }
    } catch (...) {
        qDebug() << "catch";
    }
}

void ToolPathCreator::createPocket2(const QPair<Tool, Tool>& tool, double depth)
{
    QPair<GCodeFile*, GCodeFile*> files{ nullptr, nullptr };
    //        if (m_side == On)
    //            return files;

    //        Paths fillPaths;

    //        {
    //            m_toolDiameter = tool.first.getDiameter(depth) * uScale;
    //            m_dOffset = m_toolDiameter / 2;
    //            m_stepOver = tool.first.stepover * uScale;

    //            switch (m_side) {
    //            case Outer:
    //                groupedPaths(CutoffPaths, m_toolDiameter + 5);
    //                if (m_groupedPaths.size() > 1 && m_groupedPaths.first().size() == 2)
    //                    m_groupedPaths.removeFirst();
    //                break;
    //            case Inner:
    //                groupedPaths(CopperPaths);
    //                break;
    //            }

    //            for (Paths paths : m_groupedPaths) {
    //                Paths tmpPaths;
    //                ClipperOffset offset(uScale, uScale / 1000);
    //                offset.AddPaths(paths, /*jtMiter*/ jtRound, etClosedPolygon);
    //                offset.Execute(paths, -m_dOffset);
    //                fillPaths.append(paths);

    //                do {
    //                    tmpPaths.append(paths);
    //                    offset.Clear();
    //                    offset.AddPaths(paths, jtMiter, etClosedPolygon);
    //                    offset.Execute(paths, -m_stepOver);
    //                } while (paths.size());

    //                m_returnPaths.append(tmpPaths);
    //            }

    //            if (m_returnPaths.size() == 0)
    //                return files;

    //            Clipper clipper;
    //            clipper.AddPaths(m_returnPaths, ptSubject, true);
    //            IntRect r(clipper.GetBounds());
    //            int k = tool.first.getDiameter(depth) * uScale;
    //            Path outer = {
    //                IntPoint(r.left - k, r.bottom + k),
    //                IntPoint(r.right + k, r.bottom + k),
    //                IntPoint(r.right + k, r.top - k),
    //                IntPoint(r.left - k, r.top - k)
    //            };
    //            PolyTree polyTree;
    //            clipper.AddPath(outer, ptSubject, true);
    //            clipper.Execute(ctUnion, polyTree, pftEvenOdd);
    //            m_returnPaths.clear();
    //            grouping2(polyTree.GetFirst(), &m_returnPaths);
    //            ReversePaths(m_returnPaths);
    //            sortByStratDistance(m_returnPaths);
    //            // files.first = new GCodeFile(m_returnPaths, tool.first, depth, Pocket, fillPaths);
    //            m_returnPaths.clear();
    //        }
    //        {
    //            m_toolDiameter = tool.second.getDiameter(depth) * uScale;
    //            {
    //                ClipperOffset offset(uScale, uScale / 1000);
    //                offset.AddPaths(fillPaths, /*jtMiter*/ jtRound, etClosedPolygon);
    //                offset.Execute(fillPaths, m_dOffset - m_toolDiameter / 2);
    //                m_dOffset = m_toolDiameter / 2;
    //                //offset.Clear();
    //                //offset.AddPaths(m_workingPaths, /*jtMiter*/ jtRound, etClosedPolygon);
    //                //offset.Execute(m_workingPaths, m_dOffset);
    //            }

    //            m_stepOver = tool.second.stepover * uScale;

    //            {
    //                Clipper clipper;
    //                clipper.AddPaths(m_workingPaths, ptSubject, true);
    //                ReversePaths(fillPaths);
    //                clipper.AddPaths(fillPaths, ptClip, true);
    //                clipper.Execute(ctXor, m_workingPaths, pftNegative);
    //            }
    //            //        groupedPaths(CopperPaths);
    //            m_workingPaths.removeFirst();
    //            //        if (m_workingPaths.size())
    //            //            files.second = new GCodeFile(m_workingPaths, tool.second, depth, Pocket, {});

    //            //        ReversePaths(fillPaths);
    //            //        m_workingPaths.append(fillPaths);
    //            //        fillPaths.clear();
    //            //        switch (m_side) {
    //            //        case Outer:
    //            //            groupedPaths(CopperPaths);
    //            //            break;
    //            //        case Inner:
    //            //            groupedPaths(CutoffPaths, m_toolDiameter + 5);
    //            //            if (m_groupedPaths.size() > 1 && m_groupedPaths.first().size() == 2)
    //            //                m_groupedPaths.removeFirst();
    //            //            break;
    //            //        }

    //            {
    //                Paths paths(m_workingPaths);
    //                Paths tmpPaths(m_workingPaths);
    //                ClipperOffset offset(uScale, uScale / 1000);
    //                offset.AddPaths(paths, /*jtMiter*/ jtRound, etClosedPolygon);
    //                offset.Execute(paths, m_dOffset);
    //                fillPaths.append(paths);
    //                int k1 = 100;
    //                do {
    //                    tmpPaths.append(paths);
    //                    offset.Clear();
    //                    offset.AddPaths(paths, jtMiter, etClosedPolygon);
    //                    offset.Execute(paths, -m_stepOver);
    //                } while (paths.size() && --k1);
    //                m_returnPaths.append(tmpPaths);
    //            }

    //            //        groupedPaths(CopperPaths);
    //            //        for (Paths paths : m_groupedPaths) {
    //            //            Paths tmpPaths;
    //            //            ClipperOffset offset(uScale, uScale / 1000);
    //            //            offset.AddPaths(paths, /*jtMiter*/ jtRound, etClosedPolygon);
    //            //            offset.Execute(paths, -m_dOffset);
    //            //            fillPaths.append(paths);
    //            //            int k = 100;
    //            //            do {
    //            //                tmpPaths.append(paths);
    //            //                offset.Clear();
    //            //                offset.AddPaths(paths, jtMiter, etClosedPolygon);
    //            //                offset.Execute(paths, -m_stepOver);
    //            //            } while (paths.size() && --k);
    //            //            m_returnPaths.append(tmpPaths);
    //            //        }

    //            if (m_returnPaths.size() == 0)
    //                return files;

    //            Clipper clipper;
    //            clipper.AddPaths(m_returnPaths, ptSubject, true);
    //            IntRect r(clipper.GetBounds());
    //            int k = tool.second.getDiameter(depth) * uScale;
    //            Path outer = {
    //                IntPoint(r.left - k, r.bottom + k),
    //                IntPoint(r.right + k, r.bottom + k),
    //                IntPoint(r.right + k, r.top - k),
    //                IntPoint(r.left - k, r.top - k)
    //            };
    //            PolyTree polyTree;
    //            clipper.AddPath(outer, ptSubject, true);
    //            clipper.Execute(ctUnion, polyTree, pftEvenOdd);
    //            m_returnPaths.clear();
    //            grouping2(polyTree.GetFirst(), &m_returnPaths);
    //            ReversePaths(m_returnPaths);
    //            sortByStratDistance(m_returnPaths);
    //            files.second = new GCodeFile(m_returnPaths, tool.second, depth, Pocket, {} /*fillPaths*/);
    //        }
}

////////////////////////////////////////////////////////////////
/// \brief ToolPathCreator::createProfile
/// \param tool
/// \param depth
/// \param side
/// \return
///
void ToolPathCreator::createProfile(const Tool& tool, double depth)
{
    try {
        self = this;

        // emit progress(1, 0); // progress

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
        }

        if (m_returnPaths.isEmpty()) {
            // emit progress(0, 0); // progress
            return;
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
                                tmpPaths[j].removeFirst();
                                tmpPaths[i].append(tmpPaths[j]);
                                tmpPaths.remove(j--);
                                continue;
                            }
                            if (tmpPaths[i].first() == tmpPaths[j].last()) {
                                tmpPaths[i].removeFirst();
                                tmpPaths[j].append(tmpPaths[i]);
                                tmpPaths.remove(i--);
                                break;
                            }
                            if (tmpPaths[i].last() == tmpPaths[j].last()) {
                                ReversePath(tmpPaths[j]);
                                tmpPaths[j].removeFirst();
                                tmpPaths[i].append(tmpPaths[j]);
                                tmpPaths.remove(j--);
                                continue;
                            }
                        }
                    }

                    --size;
                    m_returnPaths.remove(index--);
                    m_returnPaths.append(tmpPaths);
                }
            }
        }

        if (m_returnPaths.isEmpty()) {
            emit fileReady(nullptr);
            // emit progress(0, 0); // progress
        } else {
            m_file = new GCodeFile(sortByStratDistance(m_returnPaths), tool, depth, Profile);
            emit fileReady(m_file);
            // emit progress(0, 0); // progress
        }
    } catch (...) {
        qDebug() << "catch";
    }
}

void ToolPathCreator::createThermal(Gerber::File* file, const Tool& tool, double depth)
{
    try {
        self = this;
        m_toolDiameter = tool.getDiameter(depth);
        const double dOffset = m_toolDiameter * uScale * 0.5;

        // execute offset
        ClipperOffset offset;
        offset.AddPaths(m_workingPaths, jtRound, etClosedPolygon);
        offset.Execute(m_returnPaths, dOffset);

        // fix direction
        if (m_side == Outer && !m_convent)
            ReversePaths(m_returnPaths);
        else if (m_side == Inner && m_convent)
            ReversePaths(m_returnPaths);

        for (Path& path : m_returnPaths)
            path.append(path.first());

        if (m_returnPaths.isEmpty()) {
            emit fileReady(nullptr); // emit progress(0, 0); // progress
            return;
        }

        // create frame
        Paths framePaths;
        {
            ClipperOffset offset;

            offset.AddPaths(m_returnPaths, jtMiter, etClosedLine);

            offset.Execute(framePaths, +m_toolDiameter * uScale * 0.1);
            Clipper clipper;
            clipper.AddPaths(framePaths, ptSubject, true);
            {
                ClipperOffset offset;
                for (const Gerber::GraphicObject& go : *file) {
                    if (go.state.type() == Gerber::Line && go.state.imgPolarity() == Gerber::Positive) {
                        offset.AddPaths(go.paths, jtMiter, etClosedPolygon);
                    }
                }
                offset.Execute(framePaths, dOffset - 0.005 * uScale);
            }
            for (const Paths& paths : m_supportPathss) {
                clipper.AddPaths(paths, ptClip, true);
            }
            clipper.AddPaths(framePaths, ptClip, true);
            clipper.Execute(ctIntersection, framePaths, pftPositive);
        }

        // create thermal

        for (int index = 0, size = m_returnPaths.size(); index < size; ++index) {
            Path& path = m_returnPaths[index];
            //            {
            //                ClipperOffset offset;
            //                offset.AddPaths(paths, jtRound, etClosedPolygon);
            //                offset.Execute(paths, dOffset);
            //                for (Path& path : paths) {
            //                    path.append(path.last());
            //                }
            //            }

            Paths tmpPaths;
            // cut toolPath
            {
                Clipper clipper;
                clipper.AddPath(path, ptSubject, false);
                clipper.AddPaths(framePaths, ptClip, true);
                //                clipper.AddPaths(m_supportPathss[index], ptClip, true);
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
                        tmpPaths[j].removeFirst();
                        tmpPaths[i].append(tmpPaths[j]);
                        tmpPaths.remove(j--);
                        continue;
                    }
                    if (tmpPaths[i].first() == tmpPaths[j].last()) {
                        tmpPaths[i].removeFirst();
                        tmpPaths[j].append(tmpPaths[i]);
                        tmpPaths.remove(i--);
                        break;
                    }
                    if (tmpPaths[i].last() == tmpPaths[j].last()) {
                        ReversePath(tmpPaths[j]);
                        tmpPaths[j].removeFirst();
                        tmpPaths[i].append(tmpPaths[j]);
                        tmpPaths.remove(j--);
                        continue;
                    }
                }
            }

            --size;
            m_returnPaths.remove(index--);
            m_returnPaths.append(tmpPaths);
        }

        if (m_returnPaths.isEmpty()) {
            emit fileReady(nullptr);
            // emit progress(0, 0); // progress
        } else {
            m_file = new GCodeFile(sortByStratDistance(m_returnPaths), tool, depth, Thermal);
            emit fileReady(m_file);
            // emit progress(0, 0); // progress
        }
    } catch (...) {
        qDebug() << "catch";
    }
}

struct Pair {
    IntPoint first;
    IntPoint second;
    int id;
    bool operator==(const Pair& b) const
    {
        return first == b.first && second == b.second;
    }
};
using Pairs = QSet<Pair>;
using Pairss = QVector<Pairs>;
inline uint qHash(const Pair& tag, uint seed = 0)
{
    return qHash(tag.first.X * tag.second.X, seed ^ 0xa317a317) ^ qHash(tag.first.Y * tag.second.Y, seed ^ 0x17a317a3);
}

struct OrdPath {
    IntPoint Pt;
    OrdPath* Next = nullptr;
    OrdPath* Prev = nullptr;
    OrdPath* Last = nullptr;

    inline void append(OrdPath* opt)
    {
        Last->Next = opt;
        Last = opt->Prev->Last;
        opt->Prev = this;
        //        OrdPath* next = this;
        //        do {
        //            if (next->Next)
        //                next = next->Next;
        //            else {
        //                next->Next = opt;
        //                Last = opt->Prev->Last;
        //                opt->Prev = this;
        //                //                OrdPath* next = opt;
        //                //                do {
        //                //                    if (next->Next)
        //                //                        next = next->Next;
        //                //                    else {
        //                //                        Last = next;
        //                //                        return;
        //                //                    }
        //                //                } while (next);
        //                return;
        //            }
        //        } while (1);
    }
};

void ToolPathCreator::createVoronoi(const Tool& tool, double depth, const double k)
{
    try {
        qDebug() << "";
        // emit progress(3, 0); // progress
        QElapsedTimer t;
        QElapsedTimer t2;
        t.start();
        t2.start();

        //            QVector<Vrn::Vertex*> points;
        //            points.reserve(1000000);
        //            int id = 0;
        //            CleanPolygons(m_workingPaths, k * 0.1 * uScale);
        //            groupedPaths(CopperPaths);
        //            for (const Paths& paths : m_groupedPaths) {
        //                for (const Path& path : paths) {
        //                    IntPoint tmp(path.first());
        //                    for (const IntPoint& point : path) {
        //                        QLineF line(toQPointF(tmp), toQPointF(point));
        //                        if (line.length() > k) {
        //                            for (int i = 1, total = line.length() / k; i < total; ++i) {
        //                                line.setLength(i * k);
        //                                IntPoint point(toIntPoint(line.p2()));
        //                                points.append(new Vrn::Vertex(point.X * dScale, point.Y * dScale, id));
        //                            }
        //                        }
        //                        points.append(new Vrn::Vertex(point.X * dScale, point.Y * dScale, id));
        //                        tmp = point;
        //                    }
        //                    QLineF line(toQPointF(tmp), toQPointF(path.first()));
        //                    if (line.length() > k) {
        //                        for (int i = 1, total = line.length() / k; i < total; ++i) {
        //                            line.setLength(i * k);
        //                            IntPoint point(toIntPoint(line.p2()));
        //                            points.append(new Vrn::Vertex(point.X * dScale, point.Y * dScale, id));
        //                        }
        //                    }
        //                }
        //                ++id;
        //            }
        //            // emit progress(3, 1); // progress
        //            {
        //                Clipper clipper;
        //                for (const Paths& paths : m_groupedPaths) {
        //                    clipper.AddPaths(paths, ptClip, true);
        //                }
        //                clipper.AddPaths(m_workingRawPaths, ptClip, true);
        //                IntRect r(clipper.GetBounds());

        //                QRectF bounding_box;
        //                bounding_box.setBottom(r.bottom * dScale + 1);
        //                bounding_box.setLeft(r.left * dScale - 1);
        //                bounding_box.setRight(r.right * dScale + 1);
        //                bounding_box.setTop(r.top * dScale - 1);
        //                qDebug() << "size" << points.size();
        //                Vrn::Voronoi voronoi;
        //                Vrn::Diagram* diagram = voronoi.compute(points, bounding_box);
        //                qDebug() << "Voronoi.compute" << diagram->execTime;

        //                m_returnPaths.reserve(diagram->edges.size());
        //                for (Vrn::Edge* edge : diagram->edges) {
        //                    if (edge->lSite && edge->rSite && edge->lSite->id != edge->rSite->id)
        //                        m_returnPaths.append(Path{ IntPoint(edge->va->x * uScale, edge->va->y * uScale), IntPoint(edge->vb->x * uScale, edge->vb->y * uScale) });
        //                    //                    else if (!edge->lSite)
        //                    //                        m_returnPaths.append(Path{ IntPoint(edge->va->x, edge->va->y), IntPoint(edge->vb->x, edge->vb->y) });
        //                    else if (!edge->rSite)
        //                        m_returnPaths.append(Path{ IntPoint(edge->va->x * uScale, edge->va->y * uScale), IntPoint(edge->vb->x * uScale, edge->vb->y * uScale) });
        //                }
        //                qDebug() << "m_returnPaths" << m_returnPaths.size();
        //                delete diagram;
        //                // emit progress(3, 2); // progress
        //            }

        //            std::sort(m_returnPaths.begin(), m_returnPaths.end(), [](const Path& a, const Path& b) {
        //                cInt r = b.first().Y - a.last().Y;
        //                if (r)
        //                    return r > 0;
        //                return (b.first().X - a.last().X) >= 0;
        //            });

        //            // merge result toolPaths
        //            const int max = m_returnPaths.size();
        //            for (int k = 0; k < 10; ++k) {
        //                for (int i = 0; i < m_returnPaths.size(); ++i) {
        //                    // progress
        //                    if (progressOrCancel(max, max - m_returnPaths.size(), 100))
        //                        return;
        //                    // progress
        //                    for (int j = 0; j < m_returnPaths.size(); ++j) {
        //                        if (i == j)
        //                            continue;
        //                        if (m_returnPaths[i].last() == m_returnPaths[j].first()) {
        //                            //if (abs(m_returnPaths[i].last().X - m_returnPaths[j].first().X) < 0.01 * uScale && abs(m_returnPaths[i].last().Y - m_returnPaths[j].first().Y) < 0.01 * uScale) {
        //                            m_returnPaths[i].append(m_returnPaths[j].mid(1));
        //                            m_returnPaths.remove(j--);
        //                            continue;
        //                        }
        //                        if (m_returnPaths[i].first() == m_returnPaths[j].last()) {
        //                            //if (abs(m_returnPaths[i].first().X - m_returnPaths[j].last().X) < 0.01 * uScale && abs(m_returnPaths[i].first().Y - m_returnPaths[j].last().Y) < 0.01 * uScale) {
        //                            m_returnPaths[j].append(m_returnPaths[i].mid(1));
        //                            m_returnPaths.remove(i--);
        //                            break;
        //                        }
        //                        if (m_returnPaths[i].first() == m_returnPaths[j].first()) {
        //                            //if (abs(m_returnPaths[i].first().X - m_returnPaths[j].first().X) < 0.01 * uScale && abs(m_returnPaths[i].first().Y - m_returnPaths[j].first().Y) < 0.01 * uScale) {
        //                            ReversePath(m_returnPaths[j]);
        //                            m_returnPaths[j].append(m_returnPaths[i].mid(1));
        //                            m_returnPaths.remove(i--);
        //                            break;
        //                        }
        //                    }
        //                }
        //            }
        //            int size = m_returnPaths.size();
        //            for (int i = 0; i < m_returnPaths.size(); ++i) {
        //                if (m_returnPaths[i].size() == 2) {
        //                    m_returnPaths.remove(i--);
        //                    continue;
        //                }
        //            }
        //            qDebug() << "remove" << (size - m_returnPaths.size());
        //            qDebug() << "createVoronoi test" << t.elapsed() << m_returnPaths.size();

        QVector<jcv_point> points;
        points.reserve(100000);
        CleanPolygons(m_workingPaths, k * 0.1 * uScale);
        groupedPaths(CopperPaths);
        int id = 0;

        t.start();

        auto condei = [&](IntPoint tmp, IntPoint point) {
            QLineF line(toQPointF(tmp), toQPointF(point));
            if (line.length() > k) {
                for (int i = 1, total = line.length() / k; i < total; ++i) {
                    line.setLength(i * k);
                    IntPoint point(toIntPoint(line.p2()));
                    points.append({ static_cast<jcv_real>(point.X), static_cast<jcv_real>(point.Y), id });
                }
            }
        };

        for (const Paths& paths : m_groupedPathss) {
            for (const Path& path : paths) {
                IntPoint tmp(path.first());
                for (const IntPoint& point : path) {
                    condei(tmp, point);
                    points.append({ static_cast<jcv_real>(point.X), static_cast<jcv_real>(point.Y), id });
                    tmp = point;
                }
                condei(tmp, path.first());
            }
            ++id;
        }

        qDebug() << "DATA PREPARE" << t.elapsed();
        t.start();

        for (const Path& path : m_workingRawPaths) {
            IntPoint tmp(path.first());
            for (const IntPoint& point : path) {
                condei(tmp, point);
                points.append({ static_cast<jcv_real>(point.X), static_cast<jcv_real>(point.Y), id });
                tmp = point;
            }
            condei(tmp, path.first());
            ++id;
        }

        QList<QSharedPointer<OrdPath>> holder;
        QList<OrdPath*> merge;

        progressOrCancel(3, 1); // progress
        {
            Clipper clipper;
            for (const Paths& paths : m_groupedPathss) {
                clipper.AddPaths(paths, ptClip, true);
            }
            clipper.AddPaths(m_workingRawPaths, ptClip, true);
            IntRect r(clipper.GetBounds());
            jcv_rect bounding_box = {
                { static_cast<jcv_real>(r.left - uScale), static_cast<jcv_real>(r.top - uScale) },
                { static_cast<jcv_real>(r.right + uScale), static_cast<jcv_real>(r.bottom + uScale) }
            };

            Pairss edges;
            edges.resize(id);
            {
                auto toIntPoint = [](const jcv_point& pos) -> const IntPoint {
                    return { static_cast<cInt>(pos.x), static_cast<cInt>(pos.y) };
                };

                jcv_diagram diagram;
                jcv_diagram_generate(points.size(), points.data(), &bounding_box, &diagram);

                const jcv_site* sites = jcv_diagram_get_sites(&diagram);
                for (int i = 0; i < diagram.numsites; i++) {
                    jcv_graphedge* graph_edge = sites[i].edges;
                    while (graph_edge) {
                        const Pair pair1{ toIntPoint(graph_edge->pos[0]), toIntPoint(graph_edge->pos[1]), sites[i].p.id };
                        const Pair pair2{ toIntPoint(graph_edge->pos[1]), toIntPoint(graph_edge->pos[0]), sites[i].p.id };
                        if (edges[sites[i].p.id].contains(pair1))
                            edges[sites[i].p.id].remove(pair1);
                        else if (edges[sites[i].p.id].contains(pair2))
                            edges[sites[i].p.id].remove(pair2);
                        else
                            edges[sites[i].p.id].insert(pair1);
                        graph_edge = graph_edge->next;
                    }
                }
                jcv_diagram_free(&diagram);
            }

            progressOrCancel(3, 2); // progress

            Pairs tmp;
            for (const Pairs& edge : edges) {
                for (const Pair& pair1 : edge) {
                    Pair pair2(pair1);
                    std::swap(pair2.first, pair2.second);
                    if (!tmp.contains(pair1) && !tmp.contains(pair2)) {
                        tmp.insert(pair1);
                        progressOrCancel(edges.size() * edge.size(), tmp.size()); // progress
                    }
                }
            }

            QList<Pair> tmp2(tmp.toList());
            std::sort(tmp2.begin(), tmp2.end(), [](const Pair& a, const Pair& b) { return a.id > b.id; });

            for (const Pair& path : tmp2) {
                OrdPath* pt1 = new OrdPath;
                OrdPath* pt2 = new OrdPath;
                pt1->Pt = path.first;
                pt1->Next = pt2;
                pt1->Last = pt2;
                pt2->Pt = path.second;
                pt2->Prev = pt1;
                holder.append(QSharedPointer<OrdPath>(pt1));
                holder.append(QSharedPointer<OrdPath>(pt2));
                merge.append(pt1);
                progressOrCancel(tmp.size(), merge.size()); // progress
            }

            progressOrCancel(3, 3); // progress
            qDebug() << "GENERATE" << t.elapsed();
            t.start();
        }
        {
            const int max = merge.size();
            for (int k = 0; k < 10; ++k) {
                for (int i = 0; i < merge.size(); ++i) {
                    progressOrCancel(max, max - merge.size()); // progress
                    for (int j = 0; j < merge.size(); ++j) {
                        if (i == j)
                            continue;
                        if (merge[i]->Last->Pt == merge[j]->Pt) {
                            merge[i]->append(merge[j]->Next);
                            merge.removeAt(j--);
                            continue;
                        }
                        if (merge[i]->Pt == merge[j]->Last->Pt) {
                            merge[j]->append(merge[i]->Next);
                            merge.removeAt(i--);
                            break;
                        }
                    }
                }
            }
            m_returnPaths.resize(merge.size());
            for (int i = 0; i < merge.size(); ++i) {
                progressOrCancel(merge.size(), i); // progress
                m_returnPaths[i].append(merge[i]->Pt);
                OrdPath* Next = merge[i]->Next;
                while (Next) {
                    m_returnPaths[i].append(Next->Pt);
                    Next = Next->Next;
                }
            }
            qDebug() << "MERGE_1" << t.elapsed() << "size" << m_returnPaths.size();
            t.start();
        }
        const int max = m_returnPaths.size();
        for (int k = 0; k < 3; ++k) {
            for (int i = 0; i < m_returnPaths.size(); ++i) {
                progressOrCancel(max, max - m_returnPaths.size()); // progress
                for (int j = 0; j < m_returnPaths.size(); ++j) {
                    if (i == j)
                        continue;
                    if (m_returnPaths[i].first() == m_returnPaths[j].first()) {
                        //qDebug("1 f f");
                        ReversePath(m_returnPaths[j]);
                        m_returnPaths[j].append(m_returnPaths[i].mid(1));
                        m_returnPaths.remove(i--);
                        break;
                    }
                    if (m_returnPaths[i].last() == m_returnPaths[j].last()) {
                        //qDebug("2 l l");
                        ReversePath(m_returnPaths[j]);
                        m_returnPaths[i].append(m_returnPaths[j].mid(1));
                        m_returnPaths.remove(j--);
                        break;
                    }
                    if (Length(m_returnPaths[i].last(), m_returnPaths[j].last()) < 0.001 * uScale) {
                        //qDebug("3 l l l");
                        ReversePath(m_returnPaths[j]);
                        m_returnPaths[i].append(m_returnPaths[j].mid(1));
                        m_returnPaths.remove(j--);
                        break;
                    }
                    if (Length(m_returnPaths[i].last(), m_returnPaths[j].first()) < 0.001 * uScale) {
                        //qDebug("4 l f l");
                        m_returnPaths[i].append(m_returnPaths[j].mid(1));
                        m_returnPaths.remove(j--);
                        break;
                    }
                    if (Length(m_returnPaths[i].first(), m_returnPaths[j].last()) < 0.001 * uScale) {
                        //qDebug("5 f l l");
                        m_returnPaths[j].append(m_returnPaths[i].mid(1));
                        m_returnPaths.remove(i--);
                        break;
                    }
                    if (Length(m_returnPaths[i].first(), m_returnPaths[j].first()) < 0.001 * uScale) {
                        //qDebug("6 f f l");
                        ReversePath(m_returnPaths[j]);
                        m_returnPaths[j].append(m_returnPaths[i].mid(1));
                        m_returnPaths.remove(i--);
                        break;
                    }
                }
            }
        }

        qDebug() << "MERGE_2" << t.elapsed() << "size" << m_returnPaths.size();
        t.start();

        for (int i = 0; i < m_returnPaths.size(); ++i) {
            if (m_returnPaths[i].size() < 4 && Length(m_returnPaths[i].first(), m_returnPaths[i].last()) < k * 0.5 * uScale)
                m_returnPaths.remove(i--);
        }

        qDebug() << "CLEAR" << t.elapsed() << "size" << m_returnPaths.size();
        t.start();

        //        m_file = new GCodeFile(sortByStratDistance2(m_returnPaths), tool, depth, Voronoi);
        //        emit fileReady(m_file);

        QVector<QSet<int>> intersect(m_returnPaths.size());
        for (int i = 0; i < m_returnPaths.size(); ++i) {
            Path& path = m_returnPaths[i];
            progressOrCancel(m_returnPaths.size(), i); // progress
            for (int pi = 0; pi < path.size(); ++pi) {
                for (int j = 0; j < m_returnPaths.size(); ++j) {
                    Path& iPath = m_returnPaths[j];
                    if (abs(path[pi].X - iPath.first().X) < (k * 0.001 * uScale)
                        && abs(path[pi].Y - iPath.first().Y) < (k * 0.001 * uScale))
                        intersect[i].insert(pi);
                    if (abs(path[pi].X - iPath.last().X) < (k * 0.001 * uScale)
                        && abs(path[pi].Y - iPath.last().Y) < (k * 0.001 * uScale))
                        intersect[i].insert(pi);
                }
            }
        }

        auto Clean = [k](Path& path, QList<int> skip) {
            std::sort(skip.begin(), skip.end());
            for (int i = 0, o = 0; i < path.size() - 1; ++i) {
                if (i > (skip.first() + o))
                    skip.removeFirst();
                if ((i == (skip.first() + o)) || ((i + 1) == (skip.first() + o)))
                    continue;
                QLineF line(toQPointF(path[i]), toQPointF(path[i + 1]));
                if (line.length() < k) {
                    path[i] = toIntPoint(line.center());
                    path.remove(i + 1);
                    --o;
                    --i;
                }
            }

            for (int i = 1; i < path.size() - 1; ++i) {
                const double a1 = Angle(path[i - 1], path[i]);
                const double a2 = Angle(path[i], path[i + 1]);
                if (abs(a1 - a2) < 0.2) {
                    path.remove(i--);
                }
            }
        };
        double size1 = 0, size2 = 0;
        for (int i = 0; i < m_returnPaths.size(); ++i) {
            size1 += m_returnPaths[i].size();
            Clean(m_returnPaths[i], intersect[i].toList());
            size2 += m_returnPaths[i].size();
        }

        qDebug() << "OPTIMIZE" << t.elapsed() << "before" << size1 << "after" << size2 << "redused size" << (size1 / size2);
        qDebug() << "elapsed" << t2.elapsed();
        m_file = new GCodeFile(sortByStratDistance2(m_returnPaths), tool, depth, Voronoi);
        emit fileReady(m_file);

    } catch (...) {
        qDebug() << "catch";
    }
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
    grouping(polyTree.GetFirst(), &m_groupedPathss, group);
    return m_groupedPathss;
}
////////////////////////////////////////////////////////////////
/// \brief ToolPathCreator::addRawPaths
/// \param paths
///
void ToolPathCreator::addRawPaths(Paths rawPaths)
{
    if (rawPaths.isEmpty())
        return;

    if (m_side == On) {
        m_workingRawPaths.append(rawPaths);
        return;
    }

    qDebug() << rawPaths.size();

    const double glueLen = MaterialSetup::glue * uScale;
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

void ToolPathCreator::addSupportPaths(Pathss supportPaths)
{
    m_supportPathss.append(supportPaths);
}

void ToolPathCreator::addPaths(const Paths& paths)
{
    m_workingPaths.append(paths);
}

GCodeFile* ToolPathCreator::file() const
{
    return m_file;
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
            newPath = true;
            grouping2(node->Childs[i], paths, true);
        }
    }
}

void ToolPathCreator::progressOrCancel(int progressMax, int progressValue)
{
    m_progressValue = progressValue;
    m_progressMax = progressMax;
    if (!(progressValue % 100)) {
        if (QThread::currentThread()->isInterruptionRequested())
            throw true;
    }
}

void ToolPathCreator::progressOrCancel()
{
    if (!ToolPathCreator::self)
        return;
    if (!(++self->m_progressValue % 1000)) {
        if (self->m_progressMax == self->m_progressValue)
            self->m_progressMax += 1000000;
        //        emit self->progress(self->m_progressMax, self->m_progressValue);
        if (QThread::currentThread()->isInterruptionRequested())
            throw true;
    }
}

int ToolPathCreator::progressValue() const
{
    if (QThread::currentThread()->isInterruptionRequested())
        throw true;
    return m_progressValue;
}
int ToolPathCreator::progressMax() const { return m_progressMax; }

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
