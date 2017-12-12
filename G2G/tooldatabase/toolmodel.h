#ifndef MYMODEL_H
#define MYMODEL_H

#include "toolitem.h"

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
    static inline QString myModelMimeType() { return QStringLiteral("application/ToolItem"); }

private:
    void exportTools()
    {
        QList<QString> lines;
        ToolItem* item;
        QList<ToolItem*> stack;
        QList<int> row;

        stack.push_back(rootItem);
        row.append(0);

        while (stack.size()) {
            if (stack.last()->childCount() && row.last()) {
                stack.pop_back();
                row.pop_back();
                if (!stack.size())
                    break;
                ++row.last();
            }
            else if (stack.last() == rootItem && stack.last()->childCount() == 0) {
                break;
            }
            while (stack.last()->childCount() > row.last()) {
                item = stack.last()->child(row.last());
                QString str(row.size() - 1, '\t');
                str += (item->getTool().name.isEmpty() ? " " : item->getTool().name) + "\t";
                str += (item->getTool().note.isEmpty() ? " " : item->getTool().note) + "\t";
                str += item->getTool().toHex();
                lines << str;
                if (item->childCount()) {
                    stack.push_back(item);
                    row.push_back(0);
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
        for (QString line : lines) {
            out << line << endl;
        }
        file.close();
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
        in.setCodec("UTF-8");
        while (!in.atEnd()) {
            lines << in.readLine();
        }

        file.close();
        QList<ToolItem*> parentsStack;
        QList<int> nestingStack;
        parentsStack << rootItem;
        nestingStack << 0;
        int number = 0;
        while (number < lines.count()) {
            int nesting = 0;
            while (nesting < lines[number].length()) {
                if (lines[number].at(nesting) != '\t')
                    break;
                ++nesting;
            }

            // Read the column data from the rest of the line.
            QStringList toolData = lines[number].split("\t", QString::SkipEmptyParts);
            if (!toolData.isEmpty()) {
                if (nesting > nestingStack.last()) {
                    // The last child of the current parent is now the new parent unless the current parent has no children.
                    if (parentsStack.last()->childCount() > 0) {
                        parentsStack.push_back(parentsStack.last()->child(parentsStack.last()->childCount() - 1));
                        nestingStack.push_back(nesting);
                    }
                }
                else {
                    while (nesting < nestingStack.last() && parentsStack.count() > 0) {
                        parentsStack.pop_back();
                        nestingStack.pop_back();
                    }
                }
                if (toolData.count() < 3) {
                    QMessageBox::information(0, "", tr("Tool Database is corupted!"));
                    return;
                }

                // Append a new item to the current parent's list of children.
                ToolItem* parent = parentsStack.last();
                parent->insertChild(parent->childCount(), new ToolItem(Tool(toolData)));
            }

            ++number;
        }
    }
};

#endif // MYMODEL_H
