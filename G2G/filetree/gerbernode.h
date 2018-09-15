#ifndef FILE_H
#define FILE_H

#include "abstractnode.h"
#include <QObject>
#include <file.h>

class GerberNode : public QObject, public AbstractNode {
    Q_OBJECT

public:
    GerberNode(G::File* file);
    ~GerberNode();
    // AbstractItem interface
public:
    bool setData(const QModelIndex& index, const QVariant& value, int role) override;
    int columnCount() const override;
    int childCount() const override;
    Qt::ItemFlags flags(const QModelIndex& index) const override;
    QVariant data(const QModelIndex& index, int role) const override;
    static QTimer* repaintTimer();

private:
    Qt::CheckState checkState = Qt::Checked;
    const int m_id;
    static QTimer m_repaintTimer;
    void repaint();
};

#endif // FILE_H
