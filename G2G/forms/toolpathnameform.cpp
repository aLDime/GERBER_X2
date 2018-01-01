#include "toolpathnameform.h"

#include <QGridLayout>
#include <QLabel>
#include <QLineEdit>
#include <QApplication>
#include <QDoubleSpinBox>
#include <qsettings.h>

namespace Ui {
class ToolpathNameForm;
}

ToolpathNameForm::ToolpathNameForm(QWidget* parent)
    : QWidget(parent)
{

    leName = new QLineEdit(this);
    leName->setObjectName(QStringLiteral("leName"));

    label_9 = new QLabel(this);
    label_9->setObjectName(QStringLiteral("label_9"));
    label_9->setAlignment(Qt::AlignRight | Qt::AlignVCenter);

    //    leHomePos = new QLineEdit(this);
    //    leHomePos->setObjectName(QStringLiteral("label_10"));
    //    leHomePos->setInputMask("\\X:900000 \Y:900000 \Z:900000");

    label_11 = new QLabel(this);
    label_11->setObjectName(QStringLiteral("label_11"));
    label_11->setAlignment(Qt::AlignRight | Qt::AlignVCenter);

    QFrame* line_3 = new QFrame(this);
    line_3->setObjectName(QStringLiteral("line_3"));
    line_3->setFrameShape(QFrame::HLine);
    line_3->setFrameShadow(QFrame::Sunken);

    dsbSafeZ = new QSpinBox(this);
    dsbSafeZ->setObjectName(QStringLiteral("dsbSafeZ"));
    dsbSafeZ->setSuffix(" mm");
    dsbSafeZ->setButtonSymbols(QAbstractSpinBox::NoButtons);

    dsbHomeZ = new QSpinBox(this);
    dsbHomeZ->setObjectName(QStringLiteral("dsbHomeZ"));
    dsbHomeZ->setPrefix("Z: ");
    dsbHomeZ->setSuffix(" mm");
    dsbHomeZ->setButtonSymbols(QAbstractSpinBox::NoButtons);

    label_7 = new QLabel(this);
    label_7->setObjectName(QStringLiteral("label_7"));
    label_7->setAlignment(Qt::AlignRight | Qt::AlignVCenter);

    QGridLayout* gridLayout = new QGridLayout(this);
    gridLayout->setObjectName(QStringLiteral("gridLayout"));
    gridLayout->setMargin(0);
    gridLayout->addWidget(leName, 3, 1, 1, 1);
    gridLayout->addWidget(label_9, 1, 0, 1, 1);
    //    gridLayout->addWidget(leHomePos, 1, 1, 1, 1);
    gridLayout->addWidget(dsbHomeZ, 1, 1, 1, 1);
    gridLayout->addWidget(label_11, 3, 0, 1, 1);
    gridLayout->addWidget(line_3, 2, 0, 1, 2);
    gridLayout->addWidget(dsbSafeZ, 0, 1, 1, 1);
    gridLayout->addWidget(label_7, 0, 0, 1, 1);

    retranslateUi(this);

    QSettings settings;
    settings.beginGroup("ToolpathNameForm");
    dsbHomeZ->setValue(settings.value("HomeZ", 10.0).toFloat());
    dsbSafeZ->setValue(settings.value("SafeZ", 10.0).toFloat());
    settings.endGroup();
}

ToolpathNameForm::~ToolpathNameForm()
{
    QSettings settings;
    settings.beginGroup("ToolpathNameForm");
    settings.setValue("HomeZ", dsbHomeZ->value());
    settings.setValue("SafeZ", dsbSafeZ->value());
    settings.endGroup();
}

void ToolpathNameForm::changeEvent(QEvent* e)
{
    QWidget::changeEvent(e);
    switch (e->type()) {
    case QEvent::LanguageChange:
        retranslateUi(this);
        break;
    default:
        break;
    }
}

void ToolpathNameForm::retranslateUi(QWidget* Form)
{
    Q_UNUSED(Form)
    label_7->setText(tr("Safe Z:"));
    //    label_8->setText(tr("TextLabel"));
    label_9->setText(tr("Home Pos:"));
    //    label_10->setText(tr("TextLabel"));
    label_11->setText(tr("Name:"));
}

QString ToolpathNameForm::text() const
{
    return leName->text();
}

void ToolpathNameForm::setText(const QString& value)
{
    leName->setText(value);
}

void ToolpathNameForm::getTool(Tool& tool, const QString& name) const
{
    QSettings settings;
    settings.beginGroup(name);
    QByteArray data(settings.value("tool").toByteArray());
    if (data.size()) {
        tool.fromHex(data.split('|')[0]);
        tool.name = data.split('|')[1];
        tool.note = data.split('|')[2];
    }
    settings.endGroup();
}

void ToolpathNameForm::setTool(const Tool& tool, const QString& name)
{
    QSettings settings;
    settings.beginGroup(name);
    settings.setValue("tool", tool.toHex().append('|').append(tool.name).append('|').append(tool.note));
    settings.endGroup();
}
