#ifndef TOOLDATABASE_H
#define TOOLDATABASE_H

#include <QAbstractItemModel>
#include <QDialog>

class EditTool;
class QDialogButtonBox;
class MyTreeView;
class QPushButton;

namespace Ui {
class ToolDatabase;
}

class ToolDatabase : public QDialog {
    Q_OBJECT

public:
    explicit ToolDatabase(QWidget* parent = 0);
    ~ToolDatabase();

private:
    //    Ui::ToolDatabase *ui;

    MyTreeView* tree;
    EditTool* widget;
    QPushButton* pbNew;
    QPushButton* bpCopy;
    QPushButton* pbDelete;
    QPushButton* pbNewGroup;
    QDialogButtonBox* buttonBox;

    void setupUi(QDialog* ToolDatabase); // setupUi
    void retranslateUi(QDialog* ToolDatabase); // retranslateUi
};

#endif // TOOLDATABASE_H
