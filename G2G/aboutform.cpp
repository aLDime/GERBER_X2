#include "aboutform.h"
#include "ui_aboutform.h"

AboutForm::AboutForm(QWidget* parent)
    : QDialog(parent)
    , ui(new Ui::AboutForm)
{
    ui->setupUi(this);
    QString str(QString(__TIMESTAMP__).append("<br/>MSC_VER: ") + QString::number(_MSC_VER));
    ui->lblAbout->setText(ui->lblAbout->text().arg(/*qApp->applicationVersion()*/str));
    connect(ui->cmdOk_2, &QPushButton::clicked, this, &AboutForm::accept);
}

AboutForm::~AboutForm()
{
    delete ui;
}
