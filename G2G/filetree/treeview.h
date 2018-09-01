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
    void addFile(G::File* gerberFile);
    QString files();

private:
    void updateTree();
    void updateIcons();
    FileModel* m_model;

protected:
    void showEvent(QShowEvent* event) override;
    void contextMenuEvent(QContextMenuEvent* event) override;
};

#endif // MYTREEVIEW_H
