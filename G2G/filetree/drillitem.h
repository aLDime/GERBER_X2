#ifndef DrillItem__H
#define DrillItem__H

#include "abstractitem.h"
#include <QObject>
#include <gcode/drl.h>

class DrillItem_ : public QObject, public AbstractItem {
    Q_OBJECT

public:
    DrillItem_(DrlFile* file);
    ~DrillItem_();
    // AbstractItem interface
public:
    bool setData(const QModelIndex& index, const QVariant& value, int role) override;
    int columnCount() const override;
    int childCount() const override;
    Qt::ItemFlags flags(const QModelIndex& index) const override;
    QVariant data(const QModelIndex& index, int role) const override;
    static QMap<int, DrlFile*> files;

private:
    Qt::CheckState checkState = Qt::Checked;
    const int m_id;
};

#endif // DrillItem__H
