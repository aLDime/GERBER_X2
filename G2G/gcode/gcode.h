#ifndef GCODE_H
#define GCODE_H

#include <QGraphicsItemGroup>
#include <QObject>
#include <graphicsitem.h>
#include <myclipper.h>
#include <tooldatabase/tool.h>

enum GCodeType {
    PROFILE,
    POCKET,
    DRILLING,
    MATERIAL_SETUP_FORM
};

class GCode : public ItemGroup {
public:
    GCode(const Paths& paths, const Tool& tool, double m_depth, GCodeType type);

    Paths getPaths() const;
    void save(const QString& name = QString());
    void saveDrill();
    void saveProfilePocket();

    QString name() const;
    void setName(const QString& name);
    const GCodeType type;

private:
    const Paths paths;
    const Tool tool;
    QString m_name;
    double m_depth;

    inline QString g0() { return "G0"; }
    inline QString g1() { return "G1"; }
    inline QString x(double val) { return "X" + format(val); }
    inline QString y(double val) { return "Y" + format(val); }
    inline QString z(double val) { return "Z" + format(val); }
    inline QString feed(double val) { return "F" + format(val); }
    inline QString s(int val) { return "S" + QString::number(val); }
    inline QString format(double val) { return QString::number(val, 'f', 3); }

    void startPath(const QPointF& point);
    void endPath();
    void statFile();
    void endFile();

    QList<QString> sl;
};
////////////////////////////////////////////////////
/// \brief The GItem class
///
class PathItem : public GraphicsItem {
public:
    PathItem(const Path& m_path);
    //~WorkItem() override {}

    QRectF boundingRect() const override;
    QPainterPath shape() const override;
    void paint(QPainter* painter, const QStyleOptionGraphicsItem* option, QWidget* widget) override;
    int type() const override;
    double w = 0.0;

private:
    QPainterPath m_shape;
    Path m_path;
    QRectF rect;
};
////////////////////////////////////////////////////
/// \brief The DrillItem class
///
class DrillItem : public GraphicsItem {
public:
    DrillItem(double diameter);
    //~WorkItem() override {}

    QRectF boundingRect() const override;
    QPainterPath shape() const override;
    void paint(QPainter* painter, const QStyleOptionGraphicsItem* option, QWidget* widget) override;
    int type() const override;
    double w = 0.0;

    double diameter() const;
    void setDiameter(double diameter);

private:
    QPainterPath m_shape;
    double m_diameter;
    QRectF rect;
};
#endif // GCODE_H
