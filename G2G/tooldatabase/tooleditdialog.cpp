#include "tooleditdialog.h"

#include <QMessageBox>

ToolEditDialog::ToolEditDialog(QWidget* parent)
    : QDialog(parent)
{
    setupUi(this);
    toolEdit->setDialog();
    connect(buttonBox, &QDialogButtonBox::accepted, [=] {
        Tool t(toolEdit->tool());
        if (t.isValid())
            accept();
        else
            t.errorMessageBox(this);
    });
}

ToolEditDialog::~ToolEditDialog()
{
}
