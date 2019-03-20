#include "tooleditdialog.h"

ToolEditDialog::ToolEditDialog(QWidget* parent)
    : QDialog(parent)
{
    setupUi(this);
    toolEdit->setDialog();
}

ToolEditDialog::~ToolEditDialog()
{
}
