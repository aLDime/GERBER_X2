#ifndef TOOLPATHCREATOR_H
#define TOOLPATHCREATOR_H

#include "clipper/myclipper.h"
#include "gerber/gerber.h"
#include <QObject>
#include "gerber/gerberparser.h"

using namespace ClipperLib;

enum MILLING {
    OUTSIDE_MILLING,
    INSIDE_MILLING,
    ON_MILLING,
};

enum GROUP {
    COPPER,
    CUTOFF,
};

class ToolPathCreator : public QObject {
    Q_OBJECT
public:
    explicit ToolPathCreator(QObject* parent = 0);
    void Clear();

    Paths& Merge(GerberFile* gerberFile);
    Pathss& ToolPathPocket(MILLING milling, double toolDiameter);
    Paths& ToolPathProfile(MILLING milling, double toolDiameter);
    constexpr Paths& GetMergedPaths();

    ToolPathCreator& addPaths(const Paths& value);
    ToolPathCreator& setPaths(const Paths& value);

    //    Paths& ToolPath2()
    //    {
    //        //        Path poligon1;
    //        //        Path poligon2;
    //        //        Path poligon3;
    //        //        QGraphicsPathItem* PathItem;
    //        //        QPainterPath Path;
    //        //        // Clipper clipper;
    //        //        poligon1.push_back(IntPoint(1.0, 1.0));
    //        //        poligon1.push_back(IntPoint(10.0, 1.0));
    //        //        poligon1.push_back(IntPoint(6.0, 6.0));
    //        //        poligon1.push_back(IntPoint(10.0, 10.0));
    //        //        poligon1.push_back(IntPoint(1.0, 10.0));
    //        //        poligon1.push_back(IntPoint(1.0, 1.0));
    //        //        Path.addPolygon(poligon1);
    //        //        PathItem = new QGraphicsPathItem(Path);
    //        //        Path = QPainterPath();
    //        //        PathItem->setPen(QPen(QColor(255, 0, 0, 200), 0.1, Qt::SolidLine, Qt::RoundCap, Qt::RoundJoin));
    //        //        graphicsView->scene()->addItem(PathItem);
    //        //        //    poligon2.push_back(IntPoint(0.5, 0.0));
    //        //        //    poligon2.push_back(IntPoint(10.5, 11.0));
    //        //        //    poligon2.push_back(IntPoint(0.0, -1.0));
    //        //        for (int i = 0; i < 12; i += 2) {
    //        //            poligon2.push_back(IntPoint(0.5 + i, 0.0));
    //        //            poligon2.push_back(IntPoint(0.5 + i, 11.0));
    //        //            poligon2.push_back(IntPoint(1.5 + i, 11.0));
    //        //            poligon2.push_back(IntPoint(1.5 + i, 0.0));
    //        //        }
    //        //        // poligon2.last().ry() -= 1.0;
    //        //        Path.addPolygon(poligon2);
    //        //        PathItem = new QGraphicsPathItem(Path);
    //        //        Path = QPainterPath();
    //        //        PathItem->setPen(QPen(QColor(0, 255, 0, 200), 0.1, Qt::SolidLine, Qt::RoundCap, Qt::RoundJoin));
    //        //        graphicsView->scene()->addItem(PathItem);
    //        //        Paths  solution;
    //        //        Clipper clipper;
    //        //        clipper.AddPath(poligon1, ptClip, true);
    //        //        clipper.AddPath(poligon2, ptSubject, 0);
    //        //        //clipper.PreserveCollinear(true);
    //        //        clipper.Execute(ctIntersection, solution, pftPositive, pftPositive);
    //        //        for (Path poly, solution) {
    //        //            poligon3.push_back(poly);
    //        //            //        PolygonItem = new QGraphicsPolygonItem(poly);
    //        //            //        PolygonItem->setPen(QPen(QColor(0, 0, 255, 200), 0.1, Qt::SolidLine, Qt::RoundCap, Qt::RoundJoin));
    //        //            //        graphicsView->scene()->addItem(PolygonItem);
    //        //        }
    //        //        Path.addPolygon(poligon3);
    //        //        PathItem = new QGraphicsPathItem(Path);
    //        //        Path = QPainterPath();
    //        //        PathItem->setPen(QPen(QColor(0, 0, 255, 200), 0.1, Qt::SolidLine, Qt::RoundCap, Qt::RoundJoin));
    //        //        graphicsView->scene()->addItem(PathItem);
    //        //        QTimer::singleShot(100, Qt::CoarseTimer, graphicsView, &MyGraphicsView::Rescale);
    //    }

    Pathss& GetGroupedPaths(GROUP group, bool fl = false);

signals:

public slots:

private:
    Paths mergedPaths;
    Paths tmpPaths;
    Pathss groupedPaths;
    //    static GERBER_FILE file;
    void grouping(PolyNode* n, Pathss* p, GROUP group);
    //    static Clipper clipper;
    //    static ClipperOffset offset;
};

#endif // TOOLPATHCREATOR_H
