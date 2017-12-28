#ifndef MYTREEVIEW_H
#define MYTREEVIEW_H

#include "Model.h"

#include <QItemSelection>
#include <QItemSelectionModel>
#include <QStandardItemModel>
#include <QTreeView>
#include <QPushButton>

class TreeView : public QTreeView {
    Q_OBJECT
public:
    explicit TreeView(QWidget* parent = nullptr);
    ~TreeView();
    void addFile(Gerber::File* gerberFile);
    void removeAllFiles();
    QString files();

signals:

private:
    void updateActions();
    void updateIcons();
    Model* m_model;
    QTimer iconTimer;

    // QWidget interface
protected:
    void showEvent(QShowEvent* event) override;

    // QWidget interface
protected:
    void contextMenuEvent(QContextMenuEvent* event) override;
};

#endif // MYTREEVIEW_H
