#include "toolpathnameform.h"

#include <QGridLayout>
#include <QLabel>
#include <QLineEdit>
#include <QApplication>

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

    label_10 = new QLabel(this);
    label_10->setObjectName(QStringLiteral("label_10"));

    label_11 = new QLabel(this);
    label_11->setObjectName(QStringLiteral("label_11"));
    label_11->setAlignment(Qt::AlignRight | Qt::AlignVCenter);

    QFrame* line_3 = new QFrame(this);
    line_3->setObjectName(QStringLiteral("line_3"));
    line_3->setFrameShape(QFrame::HLine);
    line_3->setFrameShadow(QFrame::Sunken);

    label_8 = new QLabel(this);
    label_8->setObjectName(QStringLiteral("label_8"));

    label_7 = new QLabel(this);
    label_7->setObjectName(QStringLiteral("label_7"));
    label_7->setAlignment(Qt::AlignRight | Qt::AlignVCenter);

    QGridLayout* gridLayout = new QGridLayout(this);
    gridLayout->setObjectName(QStringLiteral("gridLayout"));
    gridLayout->setMargin(0);
    gridLayout->addWidget(leName, 3, 1, 1, 1);
    gridLayout->addWidget(label_9, 1, 0, 1, 1);
    gridLayout->addWidget(label_10, 1, 1, 1, 1);
    gridLayout->addWidget(label_11, 3, 0, 1, 1);
    gridLayout->addWidget(line_3, 2, 0, 1, 2);
    gridLayout->addWidget(label_8, 0, 1, 1, 1);
    gridLayout->addWidget(label_7, 0, 0, 1, 1);

    retranslateUi(this);
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
    label_7->setText(QApplication::translate("ProfileToolpathForm", "Safe Z:", Q_NULLPTR));
    label_8->setText(QApplication::translate("ProfileToolpathForm", "TextLabel", Q_NULLPTR));
    label_9->setText(QApplication::translate("ProfileToolpathForm", "Home Pos:", Q_NULLPTR));
    label_10->setText(QApplication::translate("ProfileToolpathForm", "TextLabel", Q_NULLPTR));
    label_11->setText(QApplication::translate("ProfileToolpathForm", "Name:", Q_NULLPTR));
}

QString ToolpathNameForm::text() const
{
    return leName->text();
}

void ToolpathNameForm::setText(const QString& value)
{
    leName->setText(value);
}
