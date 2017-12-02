#include "edittool.h"
#include "mytreeview.h"
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
    //    QStringList headers;
    //    headers << tr("Title") << tr("Note");
    //    QFile file("default.txt");
    //    file.open(QIODevice::ReadOnly);
    //    TreeModel* model = new TreeModel(headers, file.readAll());
    //    //  QStandardItemModel* model = new QStandardItemModel();
    //    file.close();
    //    tree->setModel(model);
    //    for (int column = 0; column < model->columnCount(); ++column)
    //        tree->resizeColumnToContents(column);
    //    connect(tree->selectionModel(), &QItemSelectionModel::selectionChanged, this, &ToolDatabase::updateActions);
    //    connect(pbNew, &QPushButton::clicked, this, &ToolDatabase::insertRow);
    //    connect(bpCopy, &QPushButton::clicked, this, &ToolDatabase::insertRow);
    //    connect(pbDelete, &QPushButton::clicked, this, &ToolDatabase::removeRow);
    //    connect(pbNewGroup, &QPushButton::clicked, this, &ToolDatabase::insertChild);
    //    updateActions();
    // QStandardItemModel* model = new QStandardItemModel(this);
    // QFile file("file.txt");
    // if (!file.open(QIODevice::ReadOnly | QIODevice::Text))
    // return;
    // QTextStream in(&file);
    // while (!in.atEnd()) {
    // qDebug() << in.readLine();
    // }
    //tree->installEventFilter(this);
    /*----------------------------------------------------------------------------*/
    /* FOLDER TREE */
    /*----------------------------------------------------------------------------*/
    //    QFile f(":/qtreeviewstylesheet/QTreeView.qss");
    //    f.open(QIODevice::ReadOnly);
    //    tree->setStyleSheet(f.readAll());
    //    f.close();

    resize(1000, 600);
}

ToolDatabase::~ToolDatabase()
{
    QFile file("default2.txt");
    if (!file.open(QIODevice::WriteOnly | QIODevice::Text))
        return;
    QTextStream out(&file);
    out << "The magic number is: " << 49 << "\n";
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

    tree = new MyTreeView({ bpCopy, pbDelete, pbNew, pbNewGroup }, ToolDatabase);
    tree->setObjectName(QStringLiteral("tree"));
    tree->setDragDropMode(QAbstractItemView::DragDrop);

    widget = new EditTool(ToolDatabase);
    widget->setObjectName(QStringLiteral("widget"));

    horizontalLayout_1->addWidget(tree);
    horizontalLayout_1->addWidget(widget);

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
