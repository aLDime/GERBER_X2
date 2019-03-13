#include "treeview.h"
#include "excellondialog.h"
#include "forms/drillform.h"
#include "gerbernode.h"
#include "layerdelegate.h"
#include "staticholders/fileholder.h"
#include <QContextMenuEvent>
#include <QFileDialog>
#include <QGraphicsScene>
#include <QHeaderView>
#include <QMenu>
#include <QPainter>
#include <QSettings>

TreeView::TreeView(QWidget* parent)
    : QTreeView(parent)
    , m_model(new FileModel(this))
{
    setModel(m_model);
    setAlternatingRowColors(true);
    setAnimated(true);
    connect(GerberNode::repaintTimer(), &QTimer::timeout, this, &TreeView::updateIcons);
    connect(m_model, &FileModel::rowsInserted, this, &TreeView::updateTree);
    connect(m_model, &FileModel::rowsRemoved, this, &TreeView::updateTree);
    connect(m_model, &FileModel::updateActions, this, &TreeView::updateTree);
    //    connect(selectionModel(), &QItemSelectionModel::selectionChanged, this, &TreeView::on_selectionChanged);
    connect(selectionModel(), &QItemSelectionModel::selectionChanged, this, &TreeView::updateTree);
    connect(this, &TreeView::doubleClicked, this, &TreeView::on_doubleClicked);

    int w = indentation();
    int h = rowHeight(m_model->index(1, 0, QModelIndex()));

    QImage image(w, h, QImage::Format_ARGB32);

    QPainter painter(&image);
    painter.setPen(QColor(128, 128, 128));

    image.fill(Qt::transparent);
    painter.drawLine(h / 2, 0, h / 2, w);
    image.save("vline.png", "PNG");

    painter.drawLine(h / 2, w / 2, h, w / 2);
    image.save("branch-more.png", "PNG");

    image.fill(Qt::transparent);
    painter.drawLine(h / 2, 0, h / 2, w / 2);
    painter.drawLine(h / 2, w / 2, h, w / 2);
    image.save("branch-end.png", "PNG");

    QFile file(":/qtreeviewstylesheet/QTreeView.qss");
    file.open(QFile::ReadOnly);
    setStyleSheet(file.readAll());

    header()->setSectionResizeMode(QHeaderView::Stretch);
    header()->setSectionResizeMode(1, QHeaderView::Fixed);
    setColumnWidth(1, QFontMetrics(font()).size(Qt::TextSingleLine, "Bottom").width() * 2);
    header()->setStretchLastSection(false);

    setItemDelegateForColumn(1, new LayerDelegate(this));
}

TreeView::~TreeView()
{
}

void TreeView::updateTree()
{
    expandAll();
}

void TreeView::updateIcons()
{
    QModelIndex index = m_model->index(0, 0, QModelIndex());
    int rowCount = static_cast<AbstractNode*>(index.internalPointer())->childCount();
    for (int r = 0; r < rowCount; ++r)
        update(m_model->index(r, 0, index));
}

void TreeView::on_doubleClicked(const QModelIndex& index)
{
    if (!index.column()) {
        m_menuIndex = index;
        if (index.parent() == m_model->index(NodeGerberFiles, 0, QModelIndex()))
            hideOther();
        if (index.parent() == m_model->index(NodeDrillFiles, 0, QModelIndex()))
            hideOther();
        if (index.parent() == m_model->index(NodeToolPath, 0, QModelIndex()))
            hideOther();
    }
}

void TreeView::on_selectionChanged(const QItemSelection& /*selected*/, const QItemSelection& /*deselected*/)
{
    //    if (!selected.indexes().isEmpty() && selected.indexes().first().parent().row() == NODE_GERBER_FILES) {
    //        QModelIndex& index = selected.indexes().first();
    //        if (index.isValid()) {
    //            G::File* file = static_cast<G::File*>(index.data(Qt::UserRole).value<void*>());
    //            int id = FileHolder::gerberFiles().key(file);
    //            file->itemGroup()->setZValue(id);
    //        }
    //    }
    //    if (!deselected.indexes().isEmpty() && deselected.indexes().first().parent().row() == NODE_GERBER_FILES) {
    //        QModelIndex& index = deselected.indexes().first();
    //        if (index.isValid()) {
    //            G::File* file = static_cast<G::File*>(index.data(Qt::UserRole).value<void*>());
    //            int id = FileHolder::gerberFiles().key(file);
    //            file->itemGroup()->setZValue(-id);
    //        }
    //    }
}

void TreeView::hideOther()
{
    const int rowCount = static_cast<AbstractNode*>(m_menuIndex.parent().internalPointer())->childCount();
    for (int row = 0; row < rowCount; ++row) {
        QModelIndex index2 = m_menuIndex.sibling(row, 0);
        AbstractNode* item = static_cast<AbstractNode*>(index2.internalPointer());
        if (row == m_menuIndex.row())
            item->setData(index2, Qt::Checked, Qt::CheckStateRole);
        else
            item->setData(index2, Qt::Unchecked, Qt::CheckStateRole);
    }
    m_model->dataChanged(m_menuIndex.sibling(0, 0), m_menuIndex.sibling(rowCount, 0));
}

void TreeView::closeFile()
{
    m_model->removeRow(m_menuIndex.row(), m_menuIndex.parent());
    if (DrillForm::self)
        //&& (m_menuIndex.parent().row() == NodeGerberFiles || m_menuIndex.parent().row() == NodeDrillFiles))
        DrillForm::self->updateFiles();
}

void TreeView::saveGcodeFile()
{
    QSettings settings;
    QString name(QFileDialog::getSaveFileName(this, tr("Save GCode file"),
        QString(settings.value("LastGCodeDir").toString()).append(m_menuIndex.data().toString()),
        tr("GCode (*.tap)")));

    if (name.isEmpty())
        return;

    settings.setValue("LastGCodeDir", name.left(name.lastIndexOf('/') + 1));
    GCodeFile* gcp = reinterpret_cast<GCodeFile*>(m_menuIndex.data(Qt::UserRole).toULongLong());
    gcp->save(name);
}

void TreeView::showExcellonDialog()
{
    d = new ExcellonDialog(FileHolder::file<DrillFile>(m_menuIndex.data(Qt::UserRole).toInt()));
    connect(d, &ExcellonDialog::destroyed, [&] { d = nullptr; });
    d->show();
}

void TreeView::contextMenuEvent(QContextMenuEvent* event)
{
    QMenu menu(this);
    m_menuIndex = indexAt(event->pos());
    QAction* a = nullptr;
    switch (m_menuIndex.parent().row()) {
    case NodeGerberFiles: {
        menu.addAction(QIcon::fromTheme("document-close"), tr("&Close"), this, &TreeView::closeFile);
        menu.addAction(QIcon::fromTheme("hint"), tr("&Hide other"), this, &TreeView::hideOther);
        G::File* file = FileHolder::file<G::File>(m_menuIndex.data(Qt::UserRole).toInt());
        a = menu.addAction(/*QIcon::fromTheme("layer-visible-off"),*/ tr("&Raw Lines"), [=](bool checked) {
            if (file)
                file->setItemType(static_cast<G::File::ItemsType>(checked));
        });
        a->setCheckable(true);
        a->setChecked(file->itemsType());
    } break;
    case NodeDrillFiles:
        a = menu.addAction(QIcon::fromTheme("document-close"), tr("&Close"), this, &TreeView::closeFile);
        menu.addAction(QIcon::fromTheme("hint"), tr("&Hide other"), this, &TreeView::hideOther);
        menu.addAction(QIcon::fromTheme("configure-shortcuts"), tr("&Edit Format"), this, &TreeView::showExcellonDialog)->setEnabled(!d);
        break;
    case NodeToolPath:
        a = menu.addAction(QIcon::fromTheme("edit-delete"), tr("&Delete Toolpath"), this, &TreeView::closeFile);
        menu.addAction(QIcon::fromTheme("hint"), tr("&Hide other"), this, &TreeView::hideOther);
        menu.addAction(QIcon::fromTheme("document-save"), tr("&Save Toolpath"), this, &TreeView::saveGcodeFile);
        break;
    default:
        break;
    }
    if (a) {
        m_menuIndex = indexAt(event->pos());
        menu.exec(mapToGlobal(event->pos()));
    }
}
