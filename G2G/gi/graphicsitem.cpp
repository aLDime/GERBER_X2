#include "graphicsitem.h"
//#include "gerber.h"
//#include "point.h"
//#include <QApplication>
//#include <QDebug>
//#include <QGraphicsSceneMouseEvent>
//#include <QPainter>
//#include <QStyleOptionGraphicsItem>
//#include <file.h>
//#include <mainwindow.h>
//#include <myscene.h>
//using namespace ClipperLib;

GraphicsItem::GraphicsItem()
    : m_pen(Qt::white)
    , m_brush(Qt::white)
{
    m_pen.setColor(Qt::white);
    m_brush.setColor(Qt::white);
}

QBrush GraphicsItem::brush() const { return m_brush; }

QPen GraphicsItem::pen() const { return m_pen; }

void GraphicsItem::setBrush(const QBrush& brush) { m_brush = brush; }

void GraphicsItem::setPen(const QPen& pen) { m_pen = pen; }
