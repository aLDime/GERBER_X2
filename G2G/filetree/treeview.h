#ifndef MYTREEVIEW_H
#define MYTREEVIEW_H

#include "filemodel.h"

#include <QItemSelection>
#include <QItemSelectionModel>
#include <QPushButton>
#include <QStandardItemModel>
#include <QTimer>
#include <QTreeView>

class TreeView : public QTreeView {
    Q_OBJECT
public:
    explicit TreeView(QWidget* parent = nullptr);
    ~TreeView();

private:
    void updateTree();
    void updateIcons();
    FileModel* m_model;

    void on_doubleClicked(const QModelIndex& index);
    void on_selectionChanged(const QItemSelection& selected, const QItemSelection& deselected);

    void hideOther(const QModelIndex& index);

protected:
    void contextMenuEvent(QContextMenuEvent* event) override;

    // QAbstractItemView interface
protected slots:
    void selectionChanged(const QItemSelection& selected, const QItemSelection& deselected) override;
};

#endif // MYTREEVIEW_H
