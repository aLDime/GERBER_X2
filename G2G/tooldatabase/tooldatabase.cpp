#include "edittool.h"
#include "tooldatabase.h"
#include "treemodel.h"

#include <QDialogButtonBox>
#include <QListWidget>
#include <QPushButton>
#include <QBoxLayout>
#include <QTreeView>
#include <QFile>
#include <QEvent>
#include <QDebug>
#include <QMessageBox>
#include <QStandardItemModel>
//#include "ui_tooldatabase.h"

ToolDatabase::ToolDatabase(QWidget* parent)
    : QDialog(parent)
//, ui(new Ui::ToolDatabase)
{
    /*ui->*/ setupUi(this);

    QStringList headers;
    headers << tr("Title") << tr("Note");
    QFile file("C:/Qt/Examples/Qt-5.9.1/widgets/itemviews/editabletreemodel/default.txt");
    file.open(QIODevice::ReadOnly);
    TreeModel* model = new TreeModel(headers, file.readAll());
    file.close();
    tree->setModel(model);
    for (int column = 0; column < model->columnCount(); ++column)
        tree->resizeColumnToContents(column);

    connect(tree->selectionModel(), &QItemSelectionModel::selectionChanged, this, &ToolDatabase::updateActions);

    connect(pbNew, &QPushButton::clicked, this, &ToolDatabase::insertRow);
    connect(bpCopy, &QPushButton::clicked, this, &ToolDatabase::insertRow);
    connect(pbDelete, &QPushButton::clicked, this, &ToolDatabase::removeRow);
    connect(pbNewGroup, &QPushButton::clicked, this, &ToolDatabase::insertChild);
    updateActions();

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

    // "QTreeView::branch:closed:has-children:!has-siblings, QTreeView::branch:closed:has-children:has-siblings{ image: url(UI:treeview_arrow_right.png); }"
    // "QTreeView::branch:open:has-children:!has-siblings, QTreeView::branch:open:has-children:has-siblings{ image: url(UI:treeview_arrow_down.png); }"

    QFile f(":/qdarkstyle/QTreeView.qss");
    f.open(QIODevice::ReadOnly);
    tree->setStyleSheet(f.readAll());
    f.close();
    resize(1000, 1);
}

ToolDatabase::~ToolDatabase()
{
    // QFile file("file.txt");
    // if (!file.open(QIODevice::WriteOnly | QIODevice::Text))
    // return;
    // QTextStream out(&file);
    // out << "The magic number is: " << 49 << "\n";
    // delete ui;
}

void ToolDatabase::setupUi(QDialog* ToolDatabase)
{
    if (ToolDatabase->objectName().isEmpty())
        ToolDatabase->setObjectName(QStringLiteral("ToolDatabase"));

    // ToolDatabase->resize(533, 360);
    // QSizePolicy sizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
    // sizePolicy.setHorizontalStretch(0);
    // sizePolicy.setVerticalStretch(0);
    // sizePolicy.setHeightForWidth(ToolDatabase->sizePolicy().hasHeightForWidth());
    // ToolDatabase->setSizePolicy(sizePolicy);

    tree = new QTreeView(ToolDatabase);
    tree->setObjectName(QStringLiteral("tree"));
    tree->setDragDropMode(QAbstractItemView::DragDrop);

    widget = new EditTool(ToolDatabase);
    widget->setObjectName(QStringLiteral("widget"));

    QHBoxLayout* horizontalLayout_1 = new QHBoxLayout();
    horizontalLayout_1->setObjectName(QStringLiteral("horizontalLayout_2"));

    horizontalLayout_1->addWidget(tree);
    horizontalLayout_1->addWidget(widget);

    pbNew = new QPushButton(ToolDatabase);
    pbNew->setObjectName(QStringLiteral("pbNew"));

    bpCopy = new QPushButton(ToolDatabase);
    bpCopy->setObjectName(QStringLiteral("bpCopy"));

    pbDelete = new QPushButton(ToolDatabase);
    pbDelete->setObjectName(QStringLiteral("pbDelete"));

    pbNewGroup = new QPushButton(ToolDatabase);
    pbNewGroup->setObjectName(QStringLiteral("pbNewGroup"));

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

void ToolDatabase::insertChild()
{
    qDebug() << "insertChild";
    QModelIndex index = tree->selectionModel()->currentIndex();
    QAbstractItemModel* model = tree->model();

    if (model->columnCount(index) == 0) {
        if (!model->insertColumn(0, index))
            return;
    }

    if (!model->insertRow(0, index))
        return;

    for (int column = 0; column < model->columnCount(index); ++column) {
        QModelIndex child = model->index(0, column, index);
        model->setData(child, QVariant("[New Group]"), Qt::EditRole);
        if (!model->headerData(column, Qt::Horizontal).isValid())
            model->setHeaderData(column, Qt::Horizontal, QVariant("[No header]"), Qt::EditRole);
    }

    tree->selectionModel()->setCurrentIndex(model->index(0, 0, index), QItemSelectionModel::ClearAndSelect);
    updateActions();
}

bool ToolDatabase::insertColumn()
{
    qDebug() << "insertColumn";
    QAbstractItemModel* model = tree->model();
    int column = tree->selectionModel()->currentIndex().column();

    // Insert a column in the parent item.
    bool changed = model->insertColumn(column + 1);
    if (changed)
        model->setHeaderData(column + 1, Qt::Horizontal, QVariant("[No header]"), Qt::EditRole);

    updateActions();

    return changed;
}

void ToolDatabase::insertRow()
{
    qDebug() << "insertRow";
    QModelIndex index = tree->selectionModel()->currentIndex();
    QAbstractItemModel* model = tree->model();

    if (!model->insertRow(index.row() + 1, index.parent()))
        return;

    updateActions();

    for (int column = 0; column < model->columnCount(index.parent()); ++column) {
        QModelIndex child = model->index(index.row() + 1, column, index.parent());
        model->setData(child, QVariant("[No data]"), Qt::EditRole);
    }
}

bool ToolDatabase::removeColumn()
{
    qDebug() << "removeColumn";
    QAbstractItemModel* model = tree->model();
    int column = tree->selectionModel()->currentIndex().column();

    // Insert columns in each child of the parent item.
    bool changed = model->removeColumn(column);

    if (changed)
        updateActions();

    return changed;
}

void ToolDatabase::removeRow()
{
    qDebug() << "removeRow";
    if (QMessageBox::question(this, "!!!", "Are you sure you want to delete the item and all content?", QMessageBox::Yes, QMessageBox::No) == QMessageBox::No) {
        return;
    }
    QModelIndex index = tree->selectionModel()->currentIndex();
    QAbstractItemModel* model = tree->model();
    if (model->removeRow(index.row(), index.parent()))
        updateActions();
}

void ToolDatabase::updateActions()
{
    qDebug() << "updateActions";
    bool hasSelection = !tree->selectionModel()->selection().isEmpty();
    // removeRowAction->setEnabled(hasSelection);
    // removeColumnAction->setEnabled(hasSelection);
    pbDelete->setEnabled(hasSelection);

    bool hasCurrent = tree->selectionModel()->currentIndex().isValid();
    // insertRowAction->setEnabled(hasCurrent);
    // insertColumnAction->setEnabled(hasCurrent);
    pbNew->setEnabled(hasCurrent);

    if (hasCurrent) {
        tree->closePersistentEditor(tree->selectionModel()->currentIndex());

        // int row = tree->selectionModel()->currentIndex().row();
        // int column = tree->selectionModel()->currentIndex().column();
        // if (tree->selectionModel()->currentIndex().parent().isValid())
        // statusBar()->showMessage(tr("Position: (%1,%2)").arg(row).arg(column));
        // else
        // statusBar()->showMessage(tr("Position: (%1,%2) in top level").arg(row).arg(column));
    }
}
