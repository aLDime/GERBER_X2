#include "mytreeview.h"

#include <QDragEnterEvent>
#include <QDragMoveEvent>
#include <QDragLeaveEvent>
#include <QDropEvent>
#include <QAbstractItemView>
#include <QApplication>
#include <QMessageBox>
#include <QSettings>
#include <QFileDialog>

#include <QDebug>

MyTreeView::MyTreeView(QVector<QPushButton*> buttons, QWidget* parent)
    : QTreeView(parent)
    , m_buttons(buttons)
{
    // setLineWidth(4);
    //    setDragDropOverwriteMode(false);
    //    setDragDropMode(InternalMove);
    //    setSelectionMode(SingleSelection);

    setDragDropMode(QAbstractItemView::InternalMove);
    setDefaultDropAction(Qt::MoveAction);
    setAlternatingRowColors(true);
    setAnimated(true);

    // connect(this, &MyTreeView::clicked, [&](const QModelIndex& index) {
    // bool hasData = index.data().toString().contains("No data");
    // m_buttons[Copy]->setEnabled(hasData);
    // });

    connect(m_buttons[Copy], &QPushButton::clicked, this, &MyTreeView::copyTool);
    connect(m_buttons[Delete], &QPushButton::clicked, this, &MyTreeView::deleteItem);
    connect(m_buttons[New], &QPushButton::clicked, this, &MyTreeView::newTool);
    connect(m_buttons[NewGroup], &QPushButton::clicked, this, &MyTreeView::newGroup);

    m_model = new QStandardItemModel();
    m_model->setHorizontalHeaderLabels({ "Name", "Note" });
    setModel(m_model);
    connect(selectionModel(), &QItemSelectionModel::selectionChanged, this, &MyTreeView::updateActions);

    QFile file(":/qtreeviewstylesheet/QTreeView.qss");
    file.open(QFile::ReadOnly);
    setStyleSheet(file.readAll());

    importTools();
    updateActions();
}

MyTreeView::~MyTreeView()
{
    exportTools();
}

void MyTreeView::newGroup()
{
    QModelIndex index = selectionModel()->currentIndex();

    if (!(index.flags() & Qt::ItemIsDropEnabled))
        index = index.parent();

    if (m_model->columnCount(index) == 0)
        if (!m_model->insertColumn(0, index) || !m_model->insertColumn(1, index))
            return;

    if (!m_model->insertRow(0, index))
        return;

    ++ig;

    for (int column = 0; column < m_model->columnCount(index); ++column) {
        QModelIndex child = m_model->index(0, column, index);
        m_model->setData(child, QVariant(QString("[New Group %1 %2]").arg(ig).arg(column)));
        // m_model->setData(child, GroupItem, TypeRole);
    }

    selectionModel()->setCurrentIndex(m_model->index(0, 0, index), QItemSelectionModel::ClearAndSelect);
    updateActions();
}

void MyTreeView::newTool()
{
    QModelIndex index = selectionModel()->currentIndex();

    if (m_model->columnCount(index) == 0)
        if (!m_model->insertColumn(0, index) || !m_model->insertColumn(1, index))
            return;

    if (index.flags() & Qt::ItemIsDropEnabled) {
        if (!m_model->insertRow(0, index))
            return;
    }
    else {
        index = index.parent();
        if (!m_model->insertRow(0, index))
            return;
    }

    ++id;

    for (int column = 0; column < m_model->columnCount(index); ++column) {
        QModelIndex child = m_model->index(0, column, index);
        m_model->setData(child, QVariant(QString("[No data %1 %2]").arg(id).arg(column)));
        // m_model->setData(child, ToolItem, TypeRole);
        m_model->itemFromIndex(child)->setFlags(Qt::ItemIsSelectable /*| Qt::ItemIsEditable*/ | Qt::ItemIsDragEnabled | Qt::ItemIsEnabled);
    }
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
        m_model->setData(child, copyIndex.data());
        m_model->itemFromIndex(child)->setFlags(Qt::ItemIsSelectable /*| Qt::ItemIsEditable*/ | Qt::ItemIsDragEnabled | Qt::ItemIsEnabled);
    }

    updateActions();
}

void MyTreeView::exportTools()
{

    //    QFile file("Tools2.ini");
    //    if (!file.open(QIODevice::WriteOnly)) {
    //        QMessageBox::information(this, tr("Unable to open file"), file.errorString());
    //        return;
    //    }
    //    // Need to find a way to clear all present data in file, if the file already exists.
    //    QDataStream out(&file);
    //    out.setVersion(QDataStream::Qt_4_3);

    //    for (int row = 0; row < m_model->rowCount(); ++row) {
    //        qDebug() << m_model->index(row, 0).data() << m_model->index(row, 1).data();
    //    }

    //    QSettings settings("Tools.ini", QSettings::IniFormat);
    //    settings.beginGroup("Tools");
    //    QModelIndex index = m_model->invisibleRootItem()->index();
    //    for (int row = 0; row < m_model->rowCount(); ++row) {
    //        qDebug() << row;
    //    }

    //    settings.setValue("Name", size());
    //    settings.setValue("Note", pos());
    //    settings.setValue("Tool", pos());

    //    settings.endGroup();
}

void MyTreeView::importTools()
{
    QFile file("default.txt");
    if (!file.open(QIODevice::ReadOnly)) {
        QMessageBox::information(this, tr("Unable to open file"), file.errorString());
        return;
    }
    const QList<QString> lines = QString(file.readAll()).split('\n');
    file.close();

    auto prepareRow = [&](const QString& first, const QString& second) -> QList<QStandardItem*> {
        //QList<QStandardItem*> rowItems({ new QStandardItem(first), new QStandardItem(second) });
        return { new QStandardItem(first), new QStandardItem(second) };
    };

    QStandardItem* item = m_model->invisibleRootItem();

    //    QList<QStandardItem*> preparedRow = prepareRow("first", "second");
    //    QStandardItem* item = m_model->invisibleRootItem();
    //    // adding a row to the invisible root item produces a root element
    //    item->appendRow(preparedRow);
    //    QList<QStandardItem*> secondRow = prepareRow("111", "222");
    //    // adding a row to an item starts a subtree
    //    preparedRow.first()->appendRow(secondRow);

    int number = 0;
    int tabNew = 0;
    int tabOld = 0;
    int tab = 0;
    //    QModelIndex index = selectionModel()->currentIndex();

    //    auto AddRow = [&](const QList<QString>& data) {
    //        if (m_model->columnCount(index) == 0)
    //            if (!m_model->insertColumn(0, index) || !m_model->insertColumn(1, index))
    //                return;

    //        if (!m_model->insertRow(0, index))
    //            return;

    //        for (int column = 0; column < m_model->columnCount(index) && column < data.size(); ++column) {
    //            QModelIndex child = m_model->index(0, column, index);
    //            m_model->setData(child, data[column]);
    //        }
    //    };

    while (number < lines.count()) {
        QList<QString> dList = lines[number].split('\t');
        tabNew = 0;
        if (dList.size() < 2)
            break;

        while (dList.size() && dList.first().isEmpty()) {
            dList.removeFirst();
            ++tabNew;
        }

        dList.last() = dList.last() + " : " + QString().setNum(tab);

        if (tabNew > tabOld) {
            item = item->child(item->rowCount() - 1, 0);
            item->appendRow(prepareRow(dList[0], dList[1]));
        }
        else if (tabNew == tabOld) {
            item->appendRow(prepareRow(dList[0], dList[1]));
        }
        else if (tabNew < tabOld) {
            while (tabNew < tabOld--) {
                item = item->parent();
            }
            if (item == nullptr)
                item = m_model->invisibleRootItem();
            item->appendRow(prepareRow(dList[0], dList[1]));
        }
        //        qDebug() << index;

        //        qDebug() << (tabNew - tabOld);

        //        if (tabNew > tabOld) {
        //            index = m_model->index(0, 0, index);
        //            AddRow(dList);
        //        }
        //        else if (tabNew == tabOld) {
        //            if (index.parent().isValid()) {
        //                index = m_model->index(m_model->itemFromIndex(index.parent())->rowCount(), 0, index.parent());
        //            }
        //            else {
        //                //                index = m_model->index(m_model->invisibleRootItem()->rowCount(), 0, index);
        //            }
        //            AddRow(dList);
        //        }
        //        else if (tabNew < tabOld) {
        //            while (tabNew < tabOld--) {
        //                index = index.parent();
        //            }
        //            AddRow(dList);
        //        }
        //        qDebug() << index;
        ++number;
        tab = tabNew - tabOld;
        tabOld = tabNew;
    }
    for (int column = 0; column < m_model->columnCount(); ++column)
        resizeColumnToContents(column);
    expandAll();
}

void MyTreeView::updateActions()
{
    bool hasSelection = !selectionModel()->selection().isEmpty();
    m_buttons[Delete]->setEnabled(hasSelection);
    bool hasCurrent = selectionModel()->currentIndex().isValid();
    m_buttons[New]->setEnabled(hasCurrent);
    m_buttons[Copy]->setEnabled(!(selectionModel()->currentIndex().flags() & Qt::ItemIsDropEnabled) && selectionModel()->currentIndex().column() > -1);
    for (int column = 0; column < m_model->columnCount(); ++column)
        resizeColumnToContents(column);
    expandAll();
}
