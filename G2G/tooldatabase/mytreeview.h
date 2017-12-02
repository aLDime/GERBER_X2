#ifndef MYTREEVIEW_H
#define MYTREEVIEW_H

#include <QItemSelection>
#include <QItemSelectionModel>
#include <QStandardItemModel>
#include <QTreeView>
#include <QPushButton>

enum ItemType {
    GroupItem,
    ToolItem,
    TypeRole = Qt::UserRole + 1,
};

class MyTreeView : public QTreeView {
    Q_OBJECT
public:
    explicit MyTreeView(QVector<QPushButton*> buttons, QWidget* parent = nullptr);
    ~MyTreeView();
signals:

public slots:

private:
    void newGroup();
    void newTool();
    void deleteItem();
    void copyTool();

    void exportTools();

    void importTools();

    void updateActions();
    QStandardItemModel* m_model;
    int ig = 0;
    int id = 0;
    enum {
        Copy,
        Delete,
        New,
        NewGroup,
    };
    QVector<QPushButton*> m_buttons;
};

#endif // MYTREEVIEW_H
