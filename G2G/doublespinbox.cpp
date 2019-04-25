#include "doublespinbox.h"

#include <QDebug>
#include <QEvent>
#include <QKeyEvent>
#include <QLineEdit>

DoubleSpinBox::DoubleSpinBox(QWidget* parent)
    : QDoubleSpinBox(parent)
{
    lineEdit()->installEventFilter(this);
    setToolTipDuration(0);
}

void DoubleSpinBox::setRange(double min, double max)
{
    QDoubleSpinBox::setRange(min, max);
    setToolTip(QString(tr("Range from %1 to %2.")).arg(minimum()).arg(maximum()));
}

void DoubleSpinBox::setMaximum(double max)
{
    QDoubleSpinBox::setMaximum(max);
    setToolTip(QString(tr("Range from %1 to %2.")).arg(minimum()).arg(maximum()));
}

void DoubleSpinBox::setMinimum(double min)
{
    QDoubleSpinBox::setMinimum(min);
    setToolTip(QString(tr("Range from %1 to %2.")).arg(minimum()).arg(maximum()));
}

void DoubleSpinBox::keyPressEvent(QKeyEvent* event)
{
    if (event->key() == Qt::Key_Backspace) {
        QString text(lineEdit()->text());
        int start = lineEdit()->selectionStart();
        text.remove(--start, 1);
        lineEdit()->setText(text);
        lineEdit()->setSelection(start, 100);
        return;
    }
    if (event->text() == '.')
        QDoubleSpinBox::keyPressEvent(new QKeyEvent(event->type(), Qt::Key_Comma, event->modifiers(), ","));
    else
        QDoubleSpinBox::keyPressEvent(event);
    lineEdit()->setSelection(lineEdit()->cursorPosition(), 100);
}

bool DoubleSpinBox::eventFilter(QObject* watched, QEvent* event)
{
    if (event->type() == QEvent::MouseButtonRelease)
        lineEdit()->selectAll();
    return QDoubleSpinBox::eventFilter(watched, event);
}
