/****************************************************************************
**
** Copyright (C) 2016 The Qt Company Ltd.
** Contact: https://www.qt.io/licensing/
**
** This file is part of the examples of the Qt Toolkit.
**
** $QT_BEGIN_LICENSE:BSD$
** Commercial License Usage
** Licensees holding valid commercial Qt licenses may use this file in
** accordance with the commercial license agreement provided with the
** Software or, alternatively, in accordance with the terms contained in
** a written agreement between you and The Qt Company. For licensing terms
** and conditions see https://www.qt.io/terms-conditions. For further
** information use the contact form at https://www.qt.io/contact-us.
**
** BSD License Usage
** Alternatively, you may use this file under the terms of the BSD license
** as follows:
**
** "Redistribution and use in source and binary forms, with or without
** modification, are permitted provided that the following conditions are
** met:
**   * Redistributions of source code must retain the above copyright
**     notice, this list of conditions and the following disclaimer.
**   * Redistributions in binary form must reproduce the above copyright
**     notice, this list of conditions and the following disclaimer in
**     the documentation and/or other materials provided with the
**     distribution.
**   * Neither the name of The Qt Company Ltd nor the names of its
**     contributors may be used to endorse or promote products derived
**     from this software without specific prior written permission.
**
**
** THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
** "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
** LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
** A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
** OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
** SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
** LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
** DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
** THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
** (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
** OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE."
**
** $QT_END_LICENSE$
**
****************************************************************************/

#include <QtWidgets>

#include "treeitem.h"
#include "treemodel.h"

TreeModel::TreeModel(const QStringList& headers, const QString& data, QObject* parent)
    : QAbstractItemModel(parent)
{
    QVector<QVariant> rootData;
    for (const QString& header : headers)
        rootData << header;

    rootItem = new TreeItem(Tool());
    setupModelData(data.split(QString("\r\n")), rootItem);
}

TreeModel::~TreeModel()
{
    delete rootItem;
}

bool TreeModel::insertRows(int position, int rows, const QModelIndex& parent)
{
    qDebug() << "insertRows" << parent.data().value<Tool>().name;
    TreeItem* parentItem = getItem(parent);
    bool success;

    beginInsertRows(parent, position, position + rows - 1);
    success = parentItem->insertChildren(position, rows, rootItem->columnCount());
    endInsertRows();

    return success;
}

bool TreeModel::removeRows(int position, int rows, const QModelIndex& parent)
{
    qDebug() << "removeRows";
    TreeItem* parentItem = getItem(parent);
    bool success = true;

    beginRemoveRows(parent, position, position + rows - 1);
    success = parentItem->removeChildren(position, rows);
    endRemoveRows();

    return success;
}

bool TreeModel::setData(const QModelIndex& index, const QVariant& value, int role)
{
    TreeItem* item;
    bool result = false;
    switch (role) {
    case Qt::EditRole:
        switch (index.column()) {
        case 0:
            item = getItem(index);
            item->getItemData().name = value.toString();
            result = true;
            break;
        case 1:
            item = getItem(index);
            item->getItemData().note = value.toString();
            result = true;
            break;
        default:
            result = true;
            break;
        }
        break;
    case Qt::DisplayRole:
        item = getItem(index);
        item->setData(value.value<Tool>());
        result = true;
    default:
        break;
    }

    if (result)
        emit dataChanged(index, index);

    return result;
}

int TreeModel::columnCount(const QModelIndex& parent) const
{
    return rootItem->columnCount();
}

int TreeModel::rowCount(const QModelIndex& parent) const
{
    TreeItem* parentItem = getItem(parent);

    return parentItem->childCount();
}

QModelIndex TreeModel::index(int row, int column, const QModelIndex& parent) const
{
    if (parent.isValid() && parent.column() != 0)
        return QModelIndex();

    TreeItem* parentItem = getItem(parent);

    TreeItem* childItem = parentItem->child(row);
    if (childItem)
        return createIndex(row, column, childItem);
    else
        return QModelIndex();
}

QModelIndex TreeModel::parent(const QModelIndex& index) const
{
    if (!index.isValid())
        return QModelIndex();
    TreeItem* childItem = static_cast<TreeItem*>(index.internalPointer());
    TreeItem* parentItem = childItem->parent();
    if (parentItem == rootItem)
        return QModelIndex();
    return createIndex(parentItem->childNumber(), 0, parentItem);
}

Qt::DropActions TreeModel::supportedDragActions() const { return Qt::MoveAction; }

Qt::DropActions TreeModel::supportedDropActions() const { return /*Qt::CopyAction |*/ Qt::MoveAction; }

Qt::ItemFlags TreeModel::flags(const QModelIndex& index) const
{
    if (!index.isValid())
        return 0;
    TreeItem* item = getItem(index);
    if (item->getItemData().data.toolType == Group)
        return Qt::ItemIsSelectable | Qt::ItemIsEditable | Qt::ItemIsDragEnabled | Qt::ItemIsDropEnabled | Qt::ItemIsEnabled;
    else
        return Qt::ItemIsSelectable | Qt::ItemIsEditable | Qt::ItemIsDragEnabled | Qt::ItemIsEnabled; //QAbstractItemModel::flags(index);
}

QVariant TreeModel::data(const QModelIndex& index, int role) const
{
    TreeItem* item;
    if (!index.isValid())
        return QVariant();

    switch (role) {
    case Qt::DecorationRole:
        return QVariant();
        if (getItem(index)->childCount() && !index.column())
            return QIcon::fromTheme("package-available");
        break;
    case Qt::UserRole:
        item = getItem(index);
        return item->data(role);
        break;
    case Qt::DisplayRole:
    case Qt::EditRole:
        item = getItem(index);
        return item->data(index.column());
    default:
        return QVariant();
        break;
    }
    return QVariant();
}

QVariant TreeModel::headerData(int section, Qt::Orientation orientation, int role) const
{
    if (orientation == Qt::Horizontal && role == Qt::DisplayRole)
        return rootItem->data(section);

    return QVariant();
}

void TreeModel::setupModelData(const QList<QString>& lines, TreeItem* parent)
{
    QList<TreeItem*> parents;
    QList<int> indentations;
    parents << parent;
    indentations << 0;
    int number = 0;
    while (number < lines.count()) {
        int indentIndex = 0;
        while (indentIndex < lines[number].length()) {
            if (lines[number].at(indentIndex) != '\t')
                break;
            ++indentIndex;
        }
        QString lineData = lines[number].mid(indentIndex).trimmed();
        if (!lineData.isEmpty()) {
            //Прочтите данные столбца из остальной части строки.
            QStringList columnStrings = lineData.split("\t", QString::SkipEmptyParts);
            if (indentIndex > indentations.last()) {
                // Последний ребенок текущего родителя теперь является новым родителем, если у текущего родителя нет детей.
                if (parents.last()->childCount() > 0) {
                    parents << parents.last()->child(parents.last()->childCount() - 1);
                    indentations << indentIndex;
                }
            }
            else {
                while (indentIndex < indentations.last() && parents.count() > 0) {
                    parents.pop_back();
                    indentations.pop_back();
                }
            }

            // Добавить новый элемент в список дочерних элементов текущего родителя.
            TreeItem* parent = parents.last();
            parent->insertChildren(parent->childCount(), 1, rootItem->columnCount());
            parent->child(parent->childCount() - 1)->setData(Tool(columnStrings));
        }
        ++number;
    }
}

TreeItem* TreeModel::getItem(const QModelIndex& index) const
{
    if (index.isValid()) {
        TreeItem* item = static_cast<TreeItem*>(index.internalPointer());
        if (item)
            return item;
    }
    return rootItem;
}

TreeItem* TreeModel::getRootItem() const
{
    return rootItem;
}
