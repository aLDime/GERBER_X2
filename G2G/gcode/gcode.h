#ifndef GCODE_H
#define GCODE_H

#include <gerber/gerber.h>
#include <QGraphicsItemGroup>
#include <QObject>
#include <tooldatabase/tool.h>

class GCode {
public:
    GCode();
};

class GCodeProfile : public QGraphicsItemGroup {
public:
    GCodeProfile(const Paths& paths, const Tool& tool, double m_depth);
    //    QRectF boundingRect() const override { return rect; }
    //    QPainterPath shape() const override { return m_shape; }
    //    void paint(QPainter* painter, const QStyleOptionGraphicsItem* option, QWidget* widget) override
    //    {
    //        Q_UNUSED(widget);
    //        //    painter->setCompositionMode(QPainter::CompositionMode_Xor);
    //        painter->setBrush(Qt::NoBrush);
    //        QColor c(255, 255, 255, 100);
    //        if (option->state & QStyle::State_MouseOver) {
    //            c.setAlpha(150);
    //        }
    //        if (option->state & QStyle::State_Selected) {
    //            c.setAlpha(200);
    //        }
    //        painter->setPen(QPen(c, tool.data.Params[Diameter], Qt::SolidLine, Qt::RoundCap, Qt::RoundJoin));
    //        painter->drawPath(m_drawShape);
    //        if (cutDepth > 0.0 && tool.data.Params[SideAngle] > 0.0) {
    //            painter->setPen(QPen(c, d, Qt::SolidLine, Qt::RoundCap, Qt::RoundJoin));
    //            painter->drawPath(m_drawShape);
    //        }
    //        painter->setPen(QPen(Qt::white, 0.0));
    //        painter->drawPath(m_drawShape);
    //    }
    //    int type() const { return QGraphicsItem::UserType + 3; }
    Paths getPaths() const;
    void save(const QString& name);
private:
    QPainterPath m_shape;
    QPainterPath m_drawShape;
    Paths paths;
    QRectF rect;
    Tool tool;
    double m_depth;
    double d;
};

#endif // GCODE_H
