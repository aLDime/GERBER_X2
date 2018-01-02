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

#include <gcode/gcode.h>

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
    connect(m_model, &Model::updateActions, this, &TreeView::updateActions);
    connect(&iconTimer, &QTimer::timeout, this, &TreeView::updateIcons);
    updateActions();
}

TreeView::~TreeView()
{
}

void TreeView::addFile(G::File* gerberFile)
{
    m_model->addGerberFile(gerberFile);
    iconTimer.start(200);
    updateActions();
}

void TreeView::removeAllFiles()
{
}

QString TreeView::files()
{
    QString f;
    QModelIndex index = m_model->index(0, 0, QModelIndex());
    int rowCount = static_cast<AbstractItem*>(index.internalPointer())->rowCount();
    for (int row = 0; row < rowCount; ++row) {
        f += m_model->index(row, 0, index).data(Qt::EditRole).toString() + "|";
    }
    return f;
}

void TreeView::updateActions()
{

    expandAll();
    for (int column = 0; column < m_model->columnCount(QModelIndex()); ++column) {
        resizeColumnToContents(column);
    }
}

void TreeView::updateIcons()
{
    qDebug() << "updateIcons";
    QModelIndex index = m_model->index(0, 0, QModelIndex());
    int rowCount = static_cast<AbstractItem*>(index.internalPointer())->rowCount();
    for (int r = 0; r < rowCount; ++r) {
        update(m_model->index(r, 0, index));
    }
    iconTimer.stop();
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
    if (m_model->index(NODE_FILES, 0, QModelIndex()) == index.parent()) {
        QMenu menu(this);
        menu.addAction(QIcon::fromTheme("document-close"), tr("&Close"), [&] {
            m_model->removeRow(index.row(), index.parent());
            iconTimer.start(200);
        });
        menu.addAction(QIcon::fromTheme("crosshairs"), tr("&Assignment of drills for holes"), [&] {
            DrillForApertureForm dfa(reinterpret_cast<G::File*>(index.data(Qt::UserRole).toULongLong()), this);
            dfa.exec();
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
            //            if (name.lastIndexOf(".tap") < (name.length() - 4))
            //                name.append(".tap");

            qDebug() << name << name.left(name.lastIndexOf('/') + 1);

            settings.setValue("LastGCodeDir", name.left(name.lastIndexOf('/') + 1));
            GCodeProfile* gcp = reinterpret_cast<GCodeProfile*>(index.data(Qt::UserRole).toULongLong());
            gcp->save(name);
            //            QFileDialog fileDialog(this, tr("Export PDF"));
            //            fileDialog.setAcceptMode(QFileDialog::AcceptSave);
            //            fileDialog.setMimeTypeFilters(QStringList("application/ngc"));
            //            fileDialog.setDefaultSuffix("pdf");
            //            if (fileDialog.exec() != QDialog::Accepted)
            //                return;
        });
        menu.exec(mapToGlobal(event->pos()));
    }
}
