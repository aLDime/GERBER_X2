#include "tooltreeview.h"
#include "toolitem.h"

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

ToolTreeView::ToolTreeView(QVector<QPushButton*> buttons, QWidget* parent)
    : QTreeView(parent)
    , m_buttons(buttons)
{
    setDragDropMode(QAbstractItemView::InternalMove);
    setDefaultDropAction(Qt::MoveAction);
    setAlternatingRowColors(true);
    setAnimated(true);

    connect(m_buttons[Copy], &QPushButton::clicked, this, &ToolTreeView::copyTool);
    connect(m_buttons[Delete], &QPushButton::clicked, this, &ToolTreeView::deleteItem);
    connect(m_buttons[New], &QPushButton::clicked, this, &ToolTreeView::newTool);
    connect(m_buttons[NewGroup], &QPushButton::clicked, this, &ToolTreeView::newGroup);

    m_model = new ToolModel(this);
    setModel(m_model);
    connect(selectionModel(), &QItemSelectionModel::selectionChanged, this, &ToolTreeView::updateActions);

    updateActions();
    setMinimumWidth(500);
}

ToolTreeView::~ToolTreeView()
{
}

void ToolTreeView::newGroup()
{
    QModelIndex index = selectionModel()->currentIndex();

    if (index.data(Qt::UserRole).value<ToolType>() != Group)
        index = index.parent();

    if (!m_model->insertRows(0, 1, index))
        return;

    QModelIndex child = m_model->index(0, 0, index);
    m_model->setData(child, "New Group", Qt::EditRole);

    selectionModel()->setCurrentIndex(m_model->index(0, 0, index), QItemSelectionModel::ClearAndSelect);
    updateActions();
}

void ToolTreeView::newTool()
{
    QModelIndex index = selectionModel()->currentIndex();

    qDebug() << index.data().value<Tool>().data.toolType;

    if (index.data(Qt::UserRole).value<ToolType>() == Group) {
        if (!m_model->insertRows(0, 1, index))
            return;
    }
    else {
        index = index.parent();
        if (!m_model->insertRows(0, 1, index))
            return;
    }

    Tool tool;
    tool.data.toolType = EndMill;
    QModelIndex child = m_model->index(0, 0, index);
    static_cast<ToolItem*>(child.internalPointer())->setTool(tool);
    m_model->setData(child, "New Tool", Qt::EditRole);
    updateActions();
}

void ToolTreeView::deleteItem()
{
    if (QMessageBox::question(this, "!!!", "Are you sure you want to delete the item and all content?",
            QMessageBox::Yes, QMessageBox::No)
        == QMessageBox::No)
        return;
    QModelIndex index = selectionModel()->currentIndex();
    if (m_model->removeRows(index.row(), 1, index.parent()))
        updateActions();
}

void ToolTreeView::copyTool()
{
    QModelIndex index = selectionModel()->currentIndex();

    if (!m_model->insertRows(index.row() + 1, 1, index.parent()))
        return;

    ToolItem* itemSrc = static_cast<ToolItem*>(index.internalPointer());
    ToolItem* itemDst = static_cast<ToolItem*>(indexBelow(index).internalPointer());

    if (selectionModel()->currentIndex().isValid() && itemSrc && itemDst)
        itemDst->setTool(itemSrc->getTool());

    updateActions();
}

void ToolTreeView::updateActions()
{
    QModelIndex index = selectionModel()->currentIndex();
    ToolItem* item = static_cast<ToolItem*>(index.internalPointer());

    m_buttons[Delete]->setEnabled(!selectionModel()->selection().isEmpty());
    m_buttons[New]->setEnabled(index.isValid());
    if (item) {
        m_buttons[Copy]->setEnabled(item->getTool().data.toolType != Group);
        emit toolSelected(item->getTool());
    }
    else
        m_buttons[Copy]->setEnabled(false);

    expandAll();
    for (int column = 0; column < m_model->columnCount(QModelIndex()); ++column) {
        resizeColumnToContents(column);
    }
}

void ToolTreeView::setTool(const Tool& value)
{
    QModelIndex index = selectionModel()->currentIndex();
    ToolItem* item = static_cast<ToolItem*>(index.internalPointer());
    if (selectionModel()->currentIndex().isValid() && item)
        item->setTool(value);
    for (QModelIndex index : selectionModel()->selection().indexes()) {
        update(index);
    }
}

void ToolTreeView::showEvent(QShowEvent* /*event*/)
{
    //setIconSize(QSize(20, 20));
    int w = indentation();
    int h = rowHeight(m_model->index(1, 0, QModelIndex()));

    QImage i(w, h, QImage::Format_ARGB32);
    qDebug() << w << h << i;
    i.fill(Qt::transparent);
    for (int y = 0; y < h; ++y)
        i.setPixelColor(w / 2, y, QColor(128, 128, 128));
    i.save("vline.png", "PNG");
    for (int x = w / 2; x < w; ++x)
        i.setPixelColor(x, h / 2, QColor(128, 128, 128));
    i.save("branch-more.png", "PNG");
    i.fill(Qt::transparent);
    for (int y = 0; y < h / 2; ++y)
        i.setPixelColor(w / 2, y, QColor(128, 128, 128));
    for (int x = w / 2; x < w; ++x)
        i.setPixelColor(x, h / 2, QColor(128, 128, 128));
    i.save("branch-end.png", "PNG");

    QFile file(":/qtreeviewstylesheet/QTreeView.qss");
    file.open(QFile::ReadOnly);
    setStyleSheet(file.readAll());
}
