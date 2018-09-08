#ifndef FILE_H
#define FILE_H

#include "abstractitem.h"
#include <QObject>
#include <file.h>

class GerberItem : public QObject, public AbstractItem {
    Q_OBJECT

public:
    GerberItem(G::File* file);
    ~GerberItem();
    // AbstractItem interface
public:
    bool setData(const QModelIndex& index, const QVariant& value, int role) override;
    int columnCount() const override;
    int childCount() const override;
    Qt::ItemFlags flags(const QModelIndex& index) const override;
    QVariant data(const QModelIndex& index, int role) const override;
    static QMap<int, G::File*> gFiles;
    static QTimer* repaintTimer();

private:
    Qt::CheckState checkState = Qt::Checked;
    const int m_id;
    static QTimer m_repaintTimer;
    void repaint();
};

#endif // FILE_H
