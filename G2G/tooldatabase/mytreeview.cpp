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
    m_model = new TreeModel({ "Name", "Note", "", "" }, file2.readAll());
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

void MyTreeView::exportTools()
{
    QString lines;
    TreeItem* item = m_model->getRootItem();
    QList<TreeItem*> stack;
    QList<int> row;

    stack.append(item);
    row.append(0);

    while (stack.size()) {
        if (stack.last()->childCount() && row.last()) {
            stack.pop_back();
            row.pop_back();
            if (!stack.size())
                break;
            ++row.last();
        }
        while (stack.last()->childCount() > row.last()) {
            item = stack.last()->getChildItems()[row.last()];
            QString str(row.size() - 1, '\t');
            str += item->getItemData().name + "\t";
            str += item->getItemData().note + "\t";
            str += item->getItemData().toHex() + "\r\n";
            lines += str;
            if (item->childCount()) {
                stack.append(item);
                row.append(0);
                break;
            }
            ++row.last();
        }
    }

    QFile file("default.txt");
    if (!file.open(QIODevice::WriteOnly | QIODevice::Text))
        return;

    QTextStream out(&file);
    out.setCodec("UTF-8");
    out << lines;
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

    //    QStandardItem* item = m_model->invisibleRootItem();

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

        //        QList<QString> dList = lines[number].split('\t');
        //        tabNew = 0;
        //        if (dList.size() < 2)
        //            break;

        //        while (dList.size() && dList.first().isEmpty()) {
        //            dList.removeFirst();
        //            ++tabNew;
        //        }

        //        dList.last() = dList.last() + " : " + QString().setNum(tab);

        //        if (tabNew > tabOld) {
        //            item = item->child(item->rowCount() - 1, 0);
        //            item->appendRow(prepareRow(dList[0], dList[1]));
        //        }
        //        else if (tabNew == tabOld) {
        //            item->appendRow(prepareRow(dList[0], dList[1]));
        //        }
        //        else if (tabNew < tabOld) {
        //            while (tabNew < tabOld--) {
        //                item = item->parent();
        //            }
        //            if (item == nullptr)
        //                item = m_model->invisibleRootItem();
        //            item->appendRow(prepareRow(dList[0], dList[1]));
        //        }
        ///////////////////////
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
