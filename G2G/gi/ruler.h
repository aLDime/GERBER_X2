#ifndef RULER_H
#define RULER_H

#include <QFont>
#include <QGraphicsItem>

class Ruler : public QGraphicsItem {
public:
    Ruler(const QPointF& point);
    ~Ruler();

    // QGraphicsItem interface
    virtual QRectF boundingRect() const override;
    virtual void paint(QPainter* painter, const QStyleOptionGraphicsItem* option, QWidget* widget) override;
    virtual int type() const override;

    void setPoint1(const QPointF& point1);
    void setPoint2(const QPointF& point2);
    static Ruler* self;

private:
    QPointF m_pt1;
    QPointF m_pt2;
    QString m_text;
    QRectF m_textRect;
    QFont m_font;
};

#endif // RULER_H
