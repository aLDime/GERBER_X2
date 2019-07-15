#ifndef PATHITEM_H
#define PATHITEM_H

#include "graphicsitem.h"
namespace GCode {
class File;
}
class PathItem : public GraphicsItem {
public:
    PathItem(const Paths& paths, GCode::File* file = nullptr);
    PathItem(const Path& path, GCode::File* file = nullptr);
    ~PathItem();
    QRectF boundingRect() const override;
    void paint(QPainter* painter, const QStyleOptionGraphicsItem* option, QWidget* widget) override;
    int type() const override;
    Paths paths() const override;

private:
    GCode::File* m_file;
};

#endif // PATHITEM_H
