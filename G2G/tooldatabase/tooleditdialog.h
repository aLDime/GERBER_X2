#ifndef TOOLEDITDIALOG_H
#define TOOLEDITDIALOG_H

#include "ui_tooleditdialog.h"
#include <QDialog>

class ToolEditDialog : public QDialog, public Ui::ToolEditDialog {
    Q_OBJECT

public:
    explicit ToolEditDialog(QWidget* parent = nullptr);
    ~ToolEditDialog();
};

#endif // TOOLEDITDIALOG_H
