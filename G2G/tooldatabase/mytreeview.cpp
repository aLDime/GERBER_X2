#include "edittool.h"
#include "mytreeview.h"
#include "treeitem.h"

#include <QDragEnterEvent>
#include <QDragMoveEvent>
#include <QDragLeaveEvent>
#include <QDropEvent>
#include <QAbstractItemView>
#include <QApplication>
#include <QMessageBox>
#include <QSettings>
#include <QFileDialog>
#include <QHeaderView>
#include <QDebug>

MyTreeView::MyTreeView(QVector<QPushButton*> buttons, QWidget* parent)
    : QTreeView(parent)
    , m_buttons(buttons)
{
    setDragDropMode(QAbstractItemView::InternalMove);
    setDefaultDropAction(Qt::MoveAction);
    setAlternatingRowColors(true);
    setAnimated(true);

    connect(m_buttons[Copy], &QPushButton::clicked, this, &MyTreeView::copyTool);
    connect(m_buttons[Delete], &QPushButton::clicked, this, &MyTreeView::deleteItem);
    connect(m_buttons[New], &QPushButton::clicked, this, &MyTreeView::newTool);
    connect(m_buttons[NewGroup], &QPushButton::clicked, this, &MyTreeView::newGroup);

    QFile file2("default.txt");
    if (!file2.open(QIODevice::ReadOnly)) {
        QMessageBox::information(this, tr("Unable to open file"), file2.errorString());
        return;
    }
    m_model = new ToolModel();
    setModel(m_model);
    connect(selectionModel(), &QItemSelectionModel::selectionChanged, this, &MyTreeView::updateActions);

    QFile file(":/qtreeviewstylesheet/QTreeView.qss");
    file.open(QFile::ReadOnly);
    setStyleSheet(file.readAll());

    updateActions();
}

MyTreeView::~MyTreeView()
{
}

void MyTreeView::newGroup()
{
    QModelIndex index = selectionModel()->currentIndex();

    if (index.data(Qt::UserRole).value<ToolType>() != Group)
        index = index.parent();

    if (!m_model->insertRow(0, index))
        return;

    Tool t;
    t.name = "New Group";
    QModelIndex child = m_model->index(0, 0, index);
    m_model->setData(child, QVariant::fromValue(t), Qt::DisplayRole);

    selectionModel()->setCurrentIndex(m_model->index(0, 0, index), QItemSelectionModel::ClearAndSelect);
    updateActions();
}

void MyTreeView::newTool()
{
    QModelIndex index = selectionModel()->currentIndex();

    qDebug() << index.data().value<Tool>().data.toolType;

    if (index.data(Qt::UserRole).value<ToolType>() == Group) {
        if (!m_model->insertRow(0, index))
            return;
    }
    else {
        index = index.parent();
        if (!m_model->insertRow(0, index))
            return;
    }

    Tool t;
    t.data.toolType = EndMill;
    t.name = "New Tool";
    QModelIndex child = m_model->index(0, 0, index);
    m_model->setData(child, QVariant::fromValue(t), Qt::DisplayRole);

    updateActions();
}

void MyTreeView::deleteItem()
{
    // if (QMessageBox::question(this, "!!!", "Are you sure you want to delete the item and all content?",
    // QMessageBox::Yes, QMessageBox::No)
    // == QMessageBox::No)
    // return;
    QModelIndex index = selectionModel()->currentIndex();
    if (m_model->removeRow(index.row(), index.parent()))
        updateActions();
}

void MyTreeView::copyTool()
{
    QModelIndex index = selectionModel()->currentIndex();

    if (!m_model->insertRow(index.row() + 1, index.parent()))
        return;

    for (int column = 0; column < m_model->columnCount(index.parent()); ++column) {
        QModelIndex child = m_model->index(index.row() + 1, column, index.parent());
        QModelIndex copyIndex = m_model->index(index.row(), column, index.parent());
        //m_model->setData(child, copyIndex.data());
        //  m_model->itemFromIndex(child)->setFlags(Qt::ItemIsSelectable /*| Qt::ItemIsEditable*/ | Qt::ItemIsDragEnabled | Qt::ItemIsEnabled);
    }

    updateActions();
}

void MyTreeView::updateActions()
{
    bool hasSelection = !selectionModel()->selection().isEmpty();
    m_buttons[Delete]->setEnabled(hasSelection);
    bool hasCurrent = selectionModel()->currentIndex().isValid();
    m_buttons[New]->setEnabled(hasCurrent);
    m_buttons[Copy]->setEnabled(!(selectionModel()->currentIndex().flags() & Qt::ItemIsDropEnabled) && selectionModel()->currentIndex().column() > -1);

    expandAll();
    for (int column = 0; column < m_model->columnCount(QModelIndex()); ++column)
        resizeColumnToContents(column);
}
