#include "gerberitem.h"

#include <QPainter>
#include <QStyleOptionGraphicsItem>
#include <mygraphicsview.h>

GerberItem::GerberItem(const Paths& paths, G::File* file)
    : m_file(file)
{
    m_paths = paths;
    for (Path& path : this->m_paths) {
        if (path.first() != path.last())
            path.append(path.first());
        m_shape.addPolygon(PathToQPolygon(path));
    }
    m_rect = m_shape.boundingRect();
    setAcceptHoverEvents(true);
    setFlag(ItemIsSelectable, true);
}

GerberItem::~GerberItem()
{
}

QRectF GerberItem::boundingRect() const { return m_rect; }

QPainterPath GerberItem::shape() const { return m_shape; }

void GerberItem::paint(QPainter* painter, const QStyleOptionGraphicsItem* option, QWidget* /*widget*/)
{
    QColor cb(m_brush.color());
    QBrush b(cb);
    QColor cp(cb);
    QPen pen(Qt::NoPen);

    if (option->state & QStyle::State_Selected) {
        cb.setAlpha(255);
        b.setColor(cb);
        cp.setAlpha(255);
        pen = QPen(cp, 0.0);
    }
    if (option->state & QStyle::State_MouseOver) {
        cb = cb.darker();
        b.setColor(cb);
        //        b.setStyle(Qt::Dense4Pattern);
        //        b.setMatrix(matrix().scale(1/MyGraphicsView::self->matrix().m11(), 1/MyGraphicsView::self->matrix().m11()));
        cp.setAlpha(255);
        pen = QPen(cp, 0.0);
    }

    painter->setBrush(b);
    if (m_file)
        painter->setPen(pen);
    else {
        painter->setPen(m_pen);
        qDebug() << m_pen.widthF();
    }
    painter->drawPath(m_shape);
}

int GerberItem::type() const { return GERBER_ITEM; }

Paths GerberItem::paths() const { return m_paths; }

const G::File* GerberItem::file() const { return m_file; }
