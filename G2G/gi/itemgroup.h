#ifndef ITEMGROUP_H
#define ITEMGROUP_H

#include "drillitem.h"
#include "gerberitem.h"
#include "graphicsitem.h"
#include "pathitem.h"
#include "rawitem.h"

class ItemGroup : public QList<GraphicsItem*> {
public:
    ~ItemGroup();
    void append(GraphicsItem* value);
    void setVisible(const bool visible);
    bool isVisible() { return m_visible; }
    void addToTheScene();
    QBrush brush() const { return m_brush; }
    QPen pen() const { return m_pen; }
    void setBrush(const QBrush& brush);
    void setPen(const QPen& pen);
    void setZValue(qreal z);

private:
    bool m_visible = true;
    QPen m_pen;
    QBrush m_brush;
};

#endif // ITEMGROUP_H
