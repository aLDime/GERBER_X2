#ifndef MYTREEVIEW_H
#define MYTREEVIEW_H

#include "toolmodel.h"

#include <QItemSelection>
#include <QItemSelectionModel>
#include <QStandardItemModel>
#include <QTreeView>
#include <QPushButton>

class ToolTreeView : public QTreeView {
    Q_OBJECT
public:
    explicit ToolTreeView(QVector<QPushButton*> buttons, QWidget* parent = nullptr);
    ~ToolTreeView();
    void setTool(const Tool& value);

signals:
    void toolSelected(const Tool& tool);

public slots:

private:
    void newGroup();
    void newTool();
    void deleteItem();
    void copyTool();

    void updateActions();
    ToolModel* m_model;
    int ig = 0;
    int id = 0;
    enum {
        Copy,
        Delete,
        New,
        NewGroup,
    };
    QVector<QPushButton*> m_buttons;

    // QWidget interface
protected:
    void showEvent(QShowEvent *event) override;
};

#endif // MYTREEVIEW_H
