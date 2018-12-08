#include "aboutform.h"
#include "ui_aboutform.h"

AboutForm::AboutForm(QWidget* parent)
    : QDialog(parent)
    , ui(new Ui::AboutForm)
{
    ui->setupUi(this);
#ifdef MINGW_DDK_H
    QString str(QString(__TIMESTAMP__).append("<br/>MINGW: ") + QString::number(__MINGW32_MAJOR_VERSION) + "." + QString::number(__MINGW32_MINOR_VERSION));
#else
    QString str(QString(__TIMESTAMP__).append("<br/>MSC_VER: ") + QString::number(_MSC_VER));
#endif
    ui->lblAbout->setText(ui->lblAbout->text().arg(/*qApp->applicationVersion()*/ str));
    connect(ui->cmdOk_2, &QPushButton::clicked, this, &AboutForm::accept);
}

AboutForm::~AboutForm()
{
    delete ui;
}
