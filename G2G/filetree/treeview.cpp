#include "treeview.h"
#include "abstractitem.h"
#include "gerberitem.h"

#include <QAbstractItemView>
#include <QApplication>
#include <QDebug>
#include <QDragEnterEvent>
#include <QDragLeaveEvent>
#include <QDragMoveEvent>
#include <QDropEvent>
#include <QFileDialog>
#include <QHeaderView>
#include <QMenu>
#include <QMessageBox>
#include <QSettings>
#include <drillforapertureform.h>
#include <mainwindow.h>

#include <gcode/gcode.h>

#include "forms/drillform.h"

TreeView::TreeView(QWidget* parent)
    : QTreeView(parent)
    , m_model(new FileModel(this))
{
    setModel(m_model);
    setAlternatingRowColors(true);
    setAnimated(true);
    connect(GerberItem::repaintTimer(), &QTimer::timeout, this, &TreeView::updateIcons);
    connect(m_model, &FileModel::rowsInserted, this, &TreeView::updateTree);
    connect(m_model, &FileModel::rowsRemoved, this, &TreeView::updateTree);
    connect(m_model, &FileModel::updateActions, this, &TreeView::updateTree);
    connect(selectionModel(), &QItemSelectionModel::selectionChanged, this, &TreeView::on_selectionChanged);
    connect(selectionModel(), &QItemSelectionModel::selectionChanged, this, &TreeView::updateTree);
    connect(this, &TreeView::doubleClicked, this, &TreeView::on_doubleClicked);
}

TreeView::~TreeView()
{
}

void TreeView::addFile(G::File* gerberFile)
{
    m_model->addGerberFile(gerberFile);
    updateTree();
}

QString TreeView::files()
{
    QString f;
    QModelIndex index = m_model->index(0, 0, QModelIndex());
    int rowCount = static_cast<AbstractItem*>(index.internalPointer())->childCount();
    for (int row = 0; row < rowCount; ++row) {
        f += m_model->index(row, 0, index).data().toString() + "|";
    }
    return f;
}

void TreeView::updateTree()
{
    expandAll();
    for (int column = 0; column < m_model->columnCount(QModelIndex()); ++column)
        resizeColumnToContents(column);
}

void TreeView::updateIcons()
{
    QModelIndex index = m_model->index(0, 0, QModelIndex());
    int rowCount = static_cast<AbstractItem*>(index.internalPointer())->childCount();
    for (int r = 0; r < rowCount; ++r)
        update(m_model->index(r, 0, index));
    if (DrillForm::self)
        DrillForm::self->updateFiles();
}

void TreeView::on_doubleClicked(const QModelIndex& index)
{
    if (index.parent() == m_model->index(NODE_FILES, 0, QModelIndex())) {
        hideOther(index);
    }
}

void TreeView::on_selectionChanged(const QItemSelection& selected, const QItemSelection& deselected)
{

    if (!selected.indexes().isEmpty() && selected.indexes().first().parent().row() == NODE_FILES) {
        QModelIndex& index = selected.indexes().first();
        if (index.isValid()) {
            G::File* file = static_cast<G::File*>(index.data(Qt::UserRole).value<void*>());
            int id = GerberItem::gFiles.key(file);
            file->itemGroup->setZValue(id);
        }
    }
    if (!deselected.indexes().isEmpty() && deselected.indexes().first().parent().row() == NODE_FILES) {
        QModelIndex& index = deselected.indexes().first();
        if (index.isValid()) {
            G::File* file = static_cast<G::File*>(index.data(Qt::UserRole).value<void*>());
            int id = GerberItem::gFiles.key(file);
            file->itemGroup->setZValue(-id);
        }
    }
}

void TreeView::hideOther(const QModelIndex& index)
{
    const int rowCount = static_cast<AbstractItem*>(index.parent().internalPointer())->childCount();
    for (int row = 0; row < rowCount; ++row) {

        QModelIndex index2 = index.sibling(row, 0);

        AbstractItem* item = static_cast<AbstractItem*>(index2.internalPointer());
        if (row == index.row())
            item->setData(index2, Qt::Checked, Qt::CheckStateRole);
        else
            item->setData(index2, Qt::Unchecked, Qt::CheckStateRole);
    }
    m_model->dataChanged(index.sibling(0, 0), index.sibling(rowCount, 0));
}

void TreeView::showEvent(QShowEvent* /*event*/)
{
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
    QModelIndex index = indexAt(event->pos());
    if (index.parent().row() == NODE_FILES) {
        QMenu menu(this);

        menu.addAction(QIcon::fromTheme("document-close"), tr("&Close"), [&] {
            m_model->removeRow(index.row(), index.parent());
        });

        menu.addAction(QIcon::fromTheme("hint"), tr("&Hide other"), [&] {
            hideOther(index);
        });

        menu.exec(mapToGlobal(event->pos()));
    }
    if (m_model->index(NODE_MILLING, 0, QModelIndex()) == index.parent()) {
        QMenu menu(this);

        menu.addAction(QIcon::fromTheme("edit-delete"), tr("&Delete Toolpath"), [&] {
            m_model->removeRow(index.row(), index.parent());
        });

        menu.addAction(QIcon::fromTheme("document-save"), tr("&Save Toolpath"), [&] {
            QSettings settings;

            QString name(QFileDialog::getSaveFileName(this, tr("Save GCode file"),
                QString(settings.value("LastGCodeDir").toString()).append(index.data().toString()),
                tr("GCode (*.tap)")));

            if (name.isEmpty())
                return;

            settings.setValue("LastGCodeDir", name.left(name.lastIndexOf('/') + 1));
            GCode* gcp = reinterpret_cast<GCode*>(index.data(Qt::UserRole).toULongLong());
            gcp->save(name);
        });

        menu.exec(mapToGlobal(event->pos()));
    }
}

//void TreeView::selectionChanged(const QItemSelection& selected, const QItemSelection& deselected)
//{

//    QModelIndexList sIndexes(selected.indexes());
//    QModelIndexList dIndexes(deselected.indexes());

//    if (!sIndexes.isEmpty() && sIndexes.first().parent().row() == NODE_FILES) {
//        GerberItem* item = static_cast<GerberItem*>(sIndexes.first().internalPointer());
//        if (item) {
//            int id = item->id();
//            qDebug() << id;
//        }
//        //GerberItem::gFiles[id]->itemGroup->setZValue(id);
//    }
//    if (!dIndexes.isEmpty() && dIndexes.first().parent().row() == NODE_FILES) {
//        int id = static_cast<GerberItem*>(dIndexes.first().internalPointer())->id();
//        //qDebug() << id;
//        //GerberItem::gFiles[id]->itemGroup->setZValue(-id);
//    }
//    QTreeView::selectionChanged(selected, deselected);
//}
