#ifndef MYMODEL_H
#define MYMODEL_H

#include "treeitem.h"

#include <QAbstractItemModel>
#include <QDebug>
#include <QFile>
#include <QMessageBox>

class ToolModel : public QAbstractItemModel {
    Q_OBJECT
    ToolItem* rootItem;

public:
    explicit ToolModel(QObject* parent = nullptr);
    ~ToolModel();

    // QAbstractItemModel interface

    bool insertRows(int row, int count, const QModelIndex& parent) override;
    bool removeRows(int row, int count, const QModelIndex& parent) override;
    int columnCount(const QModelIndex& parent) const override;
    int rowCount(const QModelIndex& parent) const override;

    QModelIndex index(int row, int column, const QModelIndex& parent) const override;
    QModelIndex parent(const QModelIndex& child) const override;

    Qt::ItemFlags flags(const QModelIndex& index) const override;

    bool setData(const QModelIndex& index, const QVariant& value, int role = Qt::EditRole) override;
    QVariant data(const QModelIndex& index, int role = Qt::DisplayRole) const override;

    QVariant headerData(int section, Qt::Orientation orientation, int role) const override;
    QStringList mimeTypes() const override;
    QMimeData* mimeData(const QModelIndexList& indexes) const override;
    bool dropMimeData(const QMimeData* data, Qt::DropAction action, int row, int column, const QModelIndex& parent) override;
    Qt::DropActions supportedDragActions() const override { return Qt::MoveAction | Qt::TargetMoveAction; }
    Qt::DropActions supportedDropActions() const override { return Qt::MoveAction | Qt::TargetMoveAction; }
    static inline QString myModelMimeType() { return QStringLiteral("application/x-qstandarditemmodeldatalist"); }

private:
    void exportTools()
    {
        //    QString lines;
        //    TreeItem* item = m_model->getRootItem();
        //    QList<TreeItem*> stack;
        //    QList<int> row;

        //    stack.append(item);
        //    row.append(0);

        //    while (stack.size()) {
        //        if (stack.last()->childCount() && row.last()) {
        //            stack.pop_back();
        //            row.pop_back();
        //            if (!stack.size())
        //                break;
        //            ++row.last();
        //        }
        //        while (stack.last()->childCount() > row.last()) {
        //            item = stack.last()->getChildItems()[row.last()];
        //            QString str(row.size() - 1, '\t');
        //            str += item->getItemData().name + "\t";
        //            str += item->getItemData().note + "\t";
        //            str += item->getItemData().toHex() + "\r\n";
        //            lines += str;
        //            if (item->childCount()) {
        //                stack.append(item);
        //                row.append(0);
        //                break;
        //            }
        //            ++row.last();
        //        }
        //    }

        //    QFile file("default.txt");
        //    if (!file.open(QIODevice::WriteOnly | QIODevice::Text))
        //        return;

        //    QTextStream out(&file);
        //    out.setCodec("UTF-8");
        //    out << lines;
    }

    void importTools()
    {
        QFile file("default.txt");
        if (!file.open(QIODevice::ReadOnly)) {
            QMessageBox::information(0, tr("Unable to open file"), file.errorString());
            return;
        }
        QList<QString> lines;
        QTextStream in(&file);
        while (!in.atEnd()) {
            lines << in.readLine();
        }
        file.close();
        QList<ToolItem*> parents;
        QList<int> indentations;
        parents << rootItem;
        indentations << 0;
        int number = 0;
        while (number < lines.count()) {
            int position = 0;
            while (position < lines[number].length()) {
                if (lines[number].at(position) != '\t')
                    break;
                ++position;
            }

            QString lineData = lines[number].mid(position).trimmed();

            if (!lineData.isEmpty()) {
                // Read the column data from the rest of the line.
                QStringList columnStrings = lineData.split("\t", QString::SkipEmptyParts);
                QVector<QVariant> columnData;
                for (int column = 0; column < columnStrings.count(); ++column)
                    columnData << columnStrings[column];
                if (position > indentations.last()) {
                    // The last child of the current parent is now the new parent
                    // unless the current parent has no children.
                    if (parents.last()->childCount() > 0) {
                        parents << parents.last()->child(parents.last()->childCount() - 1);
                        indentations << position;
                    }
                }
                else {
                    while (position < indentations.last() && parents.count() > 0) {
                        parents.pop_back();
                        indentations.pop_back();
                    }
                }

                // Append a new item to the current parent's list of children.
                ToolItem* parent = parents.last();
                parent->insertChild(parent->childCount(), new ToolItem(
                                                              Tool(columnStrings.value(0),
                                                                  columnStrings.value(1),
                                                                  columnStrings.value(2).toLocal8Bit())));
            }

            ++number;
        }
    }
};

#endif // MYMODEL_H
