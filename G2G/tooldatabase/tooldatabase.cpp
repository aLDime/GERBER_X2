#include "tooldatabase.h"
#include "tooleditform.h"
#include "toolitem.h"
#include "tooltreeview.h"
#include "ui_tooldatabase.h"
#include <QKeyEvent>
#include <QMessageBox>

ToolDatabase::ToolDatabase(QWidget* parent, QVector<Tool::Type> types)
    : QDialog(parent)
    , ui(new Ui::ToolDatabase)
    , m_types(types)
{

    ui->setupUi(this);
    ui->treeView->setButtons({ ui->pbCopy, ui->pbDelete, ui->pbNew, ui->pbNewGroup });

    ui->buttonBox->button(QDialogButtonBox::Ok)->setEnabled(m_types.isEmpty());

    connect(ui->buttonBox, &QDialogButtonBox::accepted, this, &ToolDatabase::accept);
    connect(ui->buttonBox, &QDialogButtonBox::rejected, this, &ToolDatabase::reject);

    connect(ui->toolEdit, &ToolEditForm::itemChanged, [=](ToolItem* item) {
        if (item->isTool())
            setTool(item->tool());
        ui->treeView->updateItem();
    });

    connect(ui->treeView, &ToolTreeView::itemSelected, [=](ToolItem* item) {
        if (item->isTool())
            setTool(item->tool());
        ui->buttonBox->button(QDialogButtonBox::Ok)->setEnabled((item->isTool() && m_types.contains(item->tool().type)) || m_types.isEmpty());
        ui->toolEdit->setItem(item);
    });

    connect(ui->treeView, &ToolTreeView::doubleClicked, [=](const QModelIndex& index) {
        ToolItem* item = static_cast<ToolItem*>(index.internalPointer());

        if ((item->isTool() && m_types.contains(item->tool().type))) {
            if (item->tool().isValid()) {
                setTool(item->tool());
                accept();
            } else {
                QMessageBox ::information(this, "Invalid tool", item->tool().errorStr());
            }
        }
    });
}

ToolDatabase::~ToolDatabase()
{
    delete ui;
}

Tool ToolDatabase::tool() const
{
    return m_tool;
}

void ToolDatabase::setTool(const Tool& tool)
{
    m_tool = tool;
}

void ToolDatabase::keyPressEvent(QKeyEvent* evt)
{
    if (evt->key() == Qt::Key_Enter || evt->key() == Qt::Key_Return) {
        ui->toolEdit->on_pbApply_clicked();
        return;
    }
    QDialog::keyPressEvent(evt);
}
