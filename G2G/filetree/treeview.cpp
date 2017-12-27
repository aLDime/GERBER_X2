#include "treeview.h"
#include "abstractitem.h"

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
#include <QMenu>
#include <drillforapertureform.h>
#include <mainwindow.h>

TreeView::TreeView(QWidget* parent)
    : QTreeView(parent)
{
    setDragDropMode(QAbstractItemView::InternalMove);
    setDefaultDropAction(Qt::MoveAction);
    setAlternatingRowColors(true);
    // setAnimated(true);

    m_model = new Model(this);
    setModel(m_model);
    connect(selectionModel(), &QItemSelectionModel::selectionChanged, this, &TreeView::updateActions);
    connect(&iconTimer, &QTimer::timeout, this, &TreeView::updateIcons);
    updateActions();
}

TreeView::~TreeView()
{
}

void TreeView::addFile(GerberFile* gerberFile)
{
    m_model->addFile(gerberFile);
    iconTimer.start(200);
    updateActions();

    //    //associateFileTypes('.' + gerberFile->fileName.split('.').last());
    //    files[gerberFile->fileName] = pair(gerberFile, nullptr);
    //    switch (drawingType) {
    //    case RAW:
    //        drawingType = REDRAW;
    //        drawingRaw();
    //        break;
    //    case UNITED:
    //        drawingType = REDRAW;
    //        drawUnited();
    //        break;
    //    case REDRAW:
    //        break;
    //    }
    //    //    mainWindow->treeWidget->addGerberFile(gerberFile->fileName);
    //    mainWindow->graphicsView->ZoomFit();
}

void TreeView::removeAllFiles()
{
}

QString TreeView::files()
{
    QString f;
    QModelIndex index = m_model->index(0, 0, QModelIndex());
    int rowCount = static_cast<AbstractItem*>(index.internalPointer())->rowCount(QModelIndex());
    for (int row = 0; row < rowCount; ++row) {
        f += m_model->index(row, 0, index).data(Qt::EditRole).toString() + "|";
    }
    return f;
}

//void TreeView::newGroup()
//{
//    QModelIndex index = selectionModel()->currentIndex();

//    if (index.data(Qt::UserRole).value<ToolType>() != Group)
//        index = index.parent();

//    if (!m_model->insertRows(0, 1, index))
//        return;

//    QModelIndex child = m_model->index(0, 0, index);
//    m_model->setData(child, "New Group", Qt::EditRole);

//    selectionModel()->setCurrentIndex(m_model->index(0, 0, index), QItemSelectionModel::ClearAndSelect);
//    updateActions();
//}

//void TreeView::newTool()
//{
//    QModelIndex index = selectionModel()->currentIndex();

//    qDebug() << index.data().value<Tool>().data.toolType;

//    if (index.data(Qt::UserRole).value<ToolType>() == Group) {
//        if (!m_model->insertRows(0, 1, index))
//            return;
//    }
//    else {
//        index = index.parent();
//        if (!m_model->insertRows(0, 1, index))
//            return;
//    }

//    Tool tool;
//    tool.data.toolType = EndMill;
//    QModelIndex child = m_model->index(0, 0, index);
//    static_cast<Item*>(child.internalPointer())->setTool(tool);
//    m_model->setData(child, "New Tool", Qt::EditRole);
//    updateActions();
//}

//void TreeView::deleteItem()
//{
//    if (QMessageBox::question(this, "!!!", "Are you sure you want to delete the item and all content?",
//            QMessageBox::Yes, QMessageBox::No)
//        == QMessageBox::No)
//        return;
//    QModelIndex index = selectionModel()->currentIndex();
//    if (m_model->removeRows(index.row(), 1, index.parent()))
//        updateActions();
//}

//void TreeView::copyTool()
//{
//    QModelIndex index = selectionModel()->currentIndex();

//    if (!m_model->insertRows(index.row() + 1, 1, index.parent()))
//        return;

//    Item* itemSrc = static_cast<Item*>(index.internalPointer());
//    Item* itemDst = static_cast<Item*>(indexBelow(index).internalPointer());

//    if (selectionModel()->currentIndex().isValid() && itemSrc && itemDst)
//        itemDst->setTool(itemSrc->getTool());

//    updateActions();
//}

void TreeView::updateActions()
{
    //    QModelIndex index = selectionModel()->currentIndex();
    //    Item* item = static_cast<Item*>(index.internalPointer());

    //    m_buttons[Delete]->setEnabled(!selectionModel()->selection().isEmpty());
    //    m_buttons[New]->setEnabled(index.isValid());
    //    if (item) {
    //        m_buttons[Copy]->setEnabled(item->getTool().data.toolType != Group);
    //        emit toolSelected(item->getTool());
    //    }
    //    else
    //        m_buttons[Copy]->setEnabled(false);

    expandAll();
    for (int column = 0; column < m_model->columnCount(QModelIndex()); ++column) {
        resizeColumnToContents(column);
    }
}

void TreeView::updateIcons()
{
    qDebug() << "updateIcons";
    QModelIndex index = m_model->index(0, 0, QModelIndex());
    int rowCount = static_cast<AbstractItem*>(index.internalPointer())->rowCount(QModelIndex());
    for (int r = 0; r < rowCount; ++r) {
        update(m_model->index(r, 0, index));
    }
    //m_model->dataChanged(m_model->index(0, 0, index), m_model->index(rowCount - 1, 0, index) /*, QVector<int>(rowCount, Qt::DecorationRole)*/);
    iconTimer.stop();
}

//void TreeView::setTool(const Tool& value)
//{
//    QModelIndex index = selectionModel()->currentIndex();
//    Item* item = static_cast<Item*>(index.internalPointer());
//    if (selectionModel()->currentIndex().isValid() && item)
//        item->setTool(value);
//    for (QModelIndex index : selectionModel()->selection().indexes()) {
//        update(index);
//    }
//}

void TreeView::showEvent(QShowEvent* /*event*/)
{
    //setIconSize(QSize(20, 20));
    int w = indentation();
    int h = rowHeight(m_model->index(1, 0, QModelIndex()));

    QImage i(w, h, QImage::Format_ARGB32);

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

void TreeView::contextMenuEvent(QContextMenuEvent* event)
{
    qDebug() << m_model->data(QModelIndex());
    qDebug() << static_cast<AbstractItem*>(m_model->index(0, 0, QModelIndex()).internalPointer())->rowCount(QModelIndex());
    qDebug() << event << indexAt(event->pos()).data();
    QModelIndex index = indexAt(event->pos());
    if (m_model->index(0, 0, QModelIndex()) == index.parent()) {
        QMenu menu(this);
        menu.addAction(QIcon::fromTheme("document-close"), tr("&Close"), [=] {
            m_model->removeRow(index.row(), index.parent());
            iconTimer.start(200);
            //            QTimer::singleShot(100, Qt::CoarseTimer, [&] {
            //                QModelIndex index = m_model->index(0, 0, QModelIndex());
            //                int rowCount = static_cast<AbstractItem*>(index.internalPointer())->rowCount(QModelIndex());
            //                for (int row = 0; row < rowCount; ++row) {
            //                    update(m_model->index(row, 0, index));
            //                }
            //            });
        });
        menu.addAction(QIcon::fromTheme("crosshairs"), tr("&Drill For Aperture"), [=] {
            DrillForApertureForm dfa(/*item->text(0)*/ "", this);
            dfa.exec();
        });
        menu.exec(mapToGlobal(event->pos()));
    }
}
