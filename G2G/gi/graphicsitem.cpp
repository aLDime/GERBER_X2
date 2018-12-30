#include "graphicsitem.h"
#include "itemgroup.h"

GraphicsItem::GraphicsItem()
    : m_pen(Qt::white)
    , m_brush(Qt::white)
{
    //    setAcceptDrops(false);
    //    setAcceptedMouseButtons(false);
    //    setAcceptHoverEvents(false);
    //    setAcceptTouchEvents(false);
}

GraphicsItem::~GraphicsItem()
{
    if (m_ig)
        m_ig->takeAt(m_ig->indexOf(this));
}

QBrush GraphicsItem::brush() const { return m_brush; }

QPen GraphicsItem::pen() const { return m_pen; }

void GraphicsItem::setBrush(const QBrush& brush) { m_brush = brush; }

void GraphicsItem::setPen(const QPen& pen) { m_pen = pen; }

void GraphicsItem::setItemGroup(ItemGroup* itemGroup) { m_ig = itemGroup; }

ItemGroup* GraphicsItem::parentItemGroup() const { return m_ig; }

QPointF GraphicsItem::center() const { return m_rect.center() + pos(); }
