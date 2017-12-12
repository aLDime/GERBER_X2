#include "tooledit.h"
#include "tooltreeview.h"
#include "tooldatabase.h"
//#include "treemodel.h"

#include <QBoxLayout>
#include <QDebug>
#include <QDialogButtonBox>
#include <QEvent>
#include <QFile>
#include <QListWidget>
#include <QMessageBox>
#include <QPushButton>
#include <QStandardItemModel>

ToolDatabase::ToolDatabase(QWidget* parent)
    : QDialog(parent)
{
    setupUi(this);
    resize(1000, 600);
    connect(tree, &ToolTreeView::toolSelected, tool, &ToolEdit::setTool);
    connect(tool, &ToolEdit::toolEdited, tree, &ToolTreeView::setTool);
}

ToolDatabase::~ToolDatabase()
{
}

void ToolDatabase::setupUi(QDialog* ToolDatabase)
{
    if (ToolDatabase->objectName().isEmpty())
        ToolDatabase->setObjectName(QStringLiteral("ToolDatabase"));

    ToolDatabase->resize(533, 360);
    QSizePolicy sizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
    sizePolicy.setHorizontalStretch(0);
    sizePolicy.setVerticalStretch(0);
    sizePolicy.setHeightForWidth(ToolDatabase->sizePolicy().hasHeightForWidth());
    ToolDatabase->setSizePolicy(sizePolicy);

    bpCopy = new QPushButton(ToolDatabase);
    bpCopy->setObjectName(QStringLiteral("bpCopy"));

    pbDelete = new QPushButton(ToolDatabase);
    pbDelete->setObjectName(QStringLiteral("pbDelete"));

    pbNew = new QPushButton(ToolDatabase);
    pbNew->setObjectName(QStringLiteral("pbNew"));

    pbNewGroup = new QPushButton(ToolDatabase);
    pbNewGroup->setObjectName(QStringLiteral("pbNewGroup"));

    QHBoxLayout* horizontalLayout_1 = new QHBoxLayout();
    horizontalLayout_1->setObjectName(QStringLiteral("horizontalLayout_2"));

    tree = new ToolTreeView({ bpCopy, pbDelete, pbNew, pbNewGroup }, ToolDatabase);
    tree->setObjectName(QStringLiteral("tree"));
    tree->setDragDropMode(QAbstractItemView::DragDrop);

    tool = new ToolEdit(ToolDatabase);
    tool->setObjectName(QStringLiteral("widget"));

    horizontalLayout_1->addWidget(tree);
    horizontalLayout_1->addWidget(tool);

    buttonBox = new QDialogButtonBox(ToolDatabase);
    buttonBox->setObjectName(QStringLiteral("buttonBox"));
    buttonBox->setOrientation(Qt::Horizontal);
    buttonBox->setStandardButtons(QDialogButtonBox::Cancel | QDialogButtonBox::Ok);

    QHBoxLayout* horizontalLayout_2 = new QHBoxLayout();
    horizontalLayout_2->setObjectName(QStringLiteral("horizontalLayout"));
    horizontalLayout_2->addWidget(pbNew);
    horizontalLayout_2->addWidget(bpCopy);
    horizontalLayout_2->addWidget(pbDelete);
    horizontalLayout_2->addWidget(pbNewGroup);
    horizontalLayout_2->addItem(new QSpacerItem(40, 20, QSizePolicy::Expanding, QSizePolicy::Minimum));
    horizontalLayout_2->addWidget(buttonBox);

    QVBoxLayout* verticalLayout = new QVBoxLayout(ToolDatabase);
    verticalLayout->setObjectName(QStringLiteral("verticalLayout"));
    verticalLayout->addLayout(horizontalLayout_1);
    verticalLayout->addLayout(horizontalLayout_2);

    retranslateUi(ToolDatabase);
    QObject::connect(buttonBox, &QDialogButtonBox::accepted, ToolDatabase, &QDialog::accept);
    QObject::connect(buttonBox, &QDialogButtonBox::rejected, ToolDatabase, &QDialog::reject);
    QMetaObject::connectSlotsByName(ToolDatabase);
}

void ToolDatabase::retranslateUi(QDialog* ToolDatabase)
{
    ToolDatabase->setWindowTitle(tr("Dialog"));
    pbNew->setText(tr("New..."));
    bpCopy->setText(tr("Copy..."));
    pbDelete->setText(tr("Delete..."));
    pbNewGroup->setText(tr("New Group"));
}
