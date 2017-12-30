#ifndef TOOLDATABASE_H
#define TOOLDATABASE_H

#include "tool.h"

#include <QAbstractItemModel>
#include <QDialog>

class ToolEdit;
class QDialogButtonBox;
class ToolTreeView;
class QPushButton;

namespace Ui {
class ToolDatabase;
}

class ToolDatabase : public QDialog {
    Q_OBJECT

public:
    explicit ToolDatabase(QWidget* parent = 0, const QVector<int> tools = QVector<int>({}));
    ~ToolDatabase();

    Tool getTool() const;
    void setTool(const Tool& value);
    static QVector<int> getTools();

private:
    //    Ui::ToolDatabase *ui;
    ToolTreeView* tree;
    ToolEdit* toolEdit;
    QPushButton* pbNew;
    QPushButton* bpCopy;
    QPushButton* pbDelete;
    QPushButton* pbNewGroup;
    QDialogButtonBox* buttonBox;
    Tool tool;
    static QVector<int> tools;

    void setupUi(QDialog* ToolDatabase); // setupUi
    void retranslateUi(QDialog* ToolDatabase); // retranslateUi
};

#endif // TOOLDATABASE_H
