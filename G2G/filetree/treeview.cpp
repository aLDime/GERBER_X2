#include "treeview.h"
#include "abstractnode.h"
#include "project.h"
#include "forms/drillform.h"
#include "gerbernode.h"
#include "layerdelegate.h"
#include <QContextMenuEvent>
#include <QFileDialog>
#include <QGraphicsScene>
#include <QHeaderView>
#include <QMenu>
#include <QPainter>
#include <excellondialog.h>

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
    connect(m_model, &FileModel::select, [=](const QModelIndex& index) {
        selectionModel()->select(index, QItemSelectionModel::Rows | QItemSelectionModel::ClearAndSelect);
    });
    connect(selectionModel(), &QItemSelectionModel::selectionChanged, this, &TreeView::onSelectionChanged);
    connect(selectionModel(), &QItemSelectionModel::selectionChanged, this, &TreeView::updateTree);
    connect(this, &TreeView::doubleClicked, this, &TreeView::on_doubleClicked);

    //QTimer::singleShot(100, Qt::CoarseTimer, [=] {
    int w = indentation();
    int h = rowHeight(m_model->index(1, 0, QModelIndex()));
    QImage image(w, h, QImage::Format_ARGB32);
    qDebug() << w << h << image;
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
    //});

    header()->setSectionResizeMode(QHeaderView::ResizeToContents);
    header()->setSectionResizeMode(0, QHeaderView::Stretch);
    header()->setStretchLastSection(false);

    setItemDelegateForColumn(1, new LayerDelegate(this));
    setItemDelegateForColumn(2, new RadioDelegate(this));
    setIconSize(QSize(22, 22));
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

void TreeView::onSelectionChanged(const QItemSelection& selected, const QItemSelection& deselected)
{
    if (!selected.indexes().isEmpty()) {
        QModelIndex& index = selected.indexes().first();
        const int row = index.parent().row();
        if (row == NodeGerberFiles || row == NodeDrillFiles || row == NodeToolPath) {
            if (index.isValid()) {
                const int id = index.data(Qt::UserRole).toInt();
                AbstractFile* file = Project::file(id);
                file->itemGroup()->setZValue(id);
            }
        }
    }
    if (!deselected.indexes().isEmpty()) {
        QModelIndex& index = deselected.indexes().first();
        const int row = index.parent().row();
        if (row == NodeGerberFiles || row == NodeDrillFiles || row == NodeToolPath) {
            if (index.isValid()) {
                const int id = index.data(Qt::UserRole).toInt();
                AbstractFile* file = Project::file(id);
                file->itemGroup()->setZValue(-id);
            }
        }
    }
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
        DrillForm::self->on_pbClose_clicked();
}

void TreeView::saveGcodeFile()
{
    GCodeFile* file = Project::file<GCodeFile>(m_menuIndex.data(Qt::UserRole).toInt());

    QString name(QFileDialog::getSaveFileName(this, tr("Save GCode file"),
        GCodeFile::getLastDir().append(m_menuIndex.data().toString()) + QStringList({ "(Top)", "(Bot)" })[file->side()],
        tr("GCode (*.tap)")));

    if (name.isEmpty())
        return;

    file->save(name);
}

void TreeView::showExcellonDialog()
{
    if (DrillForm::self)
        DrillForm::self->on_pbClose_clicked();
    m_exFormatDialog = new ExcellonDialog(Project::file<Excellon::File>(m_menuIndex.data(Qt::UserRole).toInt()));
    connect(m_exFormatDialog, &ExcellonDialog::destroyed, [&] { m_exFormatDialog = nullptr; });
    m_exFormatDialog->show();
}

void TreeView::contextMenuEvent(QContextMenuEvent* event)
{
    QMenu menu(this);
    m_menuIndex = indexAt(event->pos());
    QAction* a = nullptr;

    switch (m_menuIndex.parent().row()) {
    case NodeGerberFiles: {
        menu.addAction(Icon(CloseIcon), tr("&Close"), this, &TreeView::closeFile);
        menu.addAction(Icon(HideOtherIcon), tr("&Hide other"), this, &TreeView::hideOther);
        Gerber::File* file = Project::file<Gerber::File>(m_menuIndex.data(Qt::UserRole).toInt());
        a = menu.addAction(/*QIcon::fromTheme("layer-visible-off"),*/ tr("&Raw Lines"), [=](bool checked) {
            if (file)
                file->setItemType(static_cast<Gerber::File::ItemsType>(checked));
        });
        a->setCheckable(true);
        a->setChecked(file->itemsType());
    } break;
    case NodeDrillFiles:
        a = menu.addAction(Icon(CloseIcon), tr("&Close"), this, &TreeView::closeFile);
        menu.addAction(Icon(HideOtherIcon), tr("&Hide other"), this, &TreeView::hideOther);
        if (!m_exFormatDialog)
            menu.addAction(Icon(SettingsIcon), tr("&Edit Format"), this, &TreeView::showExcellonDialog);
        break;
    case NodeToolPath:
        a = menu.addAction(Icon(DeleteIcon), tr("&Delete Toolpath"), this, &TreeView::closeFile);
        menu.addAction(Icon(HideOtherIcon), tr("&Hide other"), this, &TreeView::hideOther);
        menu.addAction(Icon(SaveIcon), tr("&Save Toolpath"), this, &TreeView::saveGcodeFile);
        break;
    default:
        break;
    }

    if (m_menuIndex.parent().row() == -1 && m_menuIndex.row() == NodeToolPath && static_cast<AbstractNode*>(m_menuIndex.internalPointer())->childCount()) {
        a = menu.addAction(Icon(DeleteIcon), tr("&Delete All Toolpaths"), [=] {
            if (QMessageBox::question(this, "", tr("Really?"), QMessageBox::Yes, QMessageBox::No) == QMessageBox::Yes)
                m_model->removeRows(0, static_cast<AbstractNode*>(m_menuIndex.internalPointer())->childCount(), m_menuIndex);
        });
    }

    if (a) {
        m_menuIndex = indexAt(event->pos());
        menu.exec(mapToGlobal(event->pos()));
    }
}
