#ifndef WORKITEM_H
#define WORKITEM_H

#include "graphicsitem.h"

namespace Gerber {
class File;
}

class GerberItem : public GraphicsItem {
public:
    GerberItem(const Paths& m_paths, Gerber::File* file);
    ~GerberItem();
    QRectF boundingRect() const override;
    QPainterPath shape() const override;
    void paint(QPainter* painter, const QStyleOptionGraphicsItem* option, QWidget* widget) override;
    int type() const override;

    Paths paths() const override;
    const Gerber::File* file() const;

private:
    const Gerber::File* m_file;
};
#endif // WORKITEM_H
