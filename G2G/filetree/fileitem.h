#ifndef FILE_H
#define FILE_H

#include "abstractitem.h"
#include <QObject>
#include <file.h>

class FileItem : public QObject, public AbstractItem {
    Q_OBJECT

public:
    FileItem(G::File* gerberFile);
    ~FileItem();
    // AbstractItem interface
public:
    bool setData(const QModelIndex& index, const QVariant& value, int role) override;
    int columnCount(/*const QModelIndex& parent*/) const override;
    int childCount(/*const QModelIndex& parent*/) const override;
    Qt::ItemFlags flags(const QModelIndex& index) const override;
    QVariant data(const QModelIndex& index, int role) const override;
    static QTimer repaintTimer;

private:
    G::File* gFile;

    Qt::CheckState checkState = Qt::Checked;
    void repaint();
};

#endif // FILE_H
