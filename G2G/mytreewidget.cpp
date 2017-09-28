#include "mytreewidget.h"

#include "drillforapertureform.h"
#include "gerberfileholder.h"

Q_DECLARE_METATYPE(QGraphicsItemGroup*)

QMap<QString, QTreeWidgetItem*> map;

MyTreeWidget::MyTreeWidget(QWidget* parent)
    : QTreeWidget(parent)
{
    //Теперь напишем код файла widget.cpp. В конструктор добавим инициализацию наших данных:
    //ниже наша часть конструктора
    setColumnCount(1);
    QStringList headers;
    headers << tr("Объекты");
    setHeaderLabels(headers);
    current_Item = nullptr;
    currentColumn = 0;

    filesItem = new QTreeWidgetItem(this, 0);
    filesItem->setText(/*currentColumn*/ 0, tr("Файлы"));
    //    filesItem->setIcon(0, QIcon(":/svg/factory.svg"));

    gerberItem = InsertItem(filesItem, tr("GERBER X2"));
    //gerberItem->setIcon(0, QIcon(":/svg/bank note.svg"));
    drillItem = InsertItem(filesItem, tr("DRILL"));
    //drillItem->setIcon(0, QIcon(":/svg/share.svg"));

    millingItem = new QTreeWidgetItem(this, 0);
    millingItem->setText(/*currentColumn*/ 0, tr("Фрезеровки"));
    //millingItem->setIcon(0, QIcon(":/svg//cmd.svg"));

    drillersItem = new QTreeWidgetItem(this, 0);
    drillersItem->setText(/*currentColumn*/ 0, tr("Сверловки"));
    //drillersItem->setIcon(0, QIcon(":/svg/share.svg"));

    trajectoriesItem = new QTreeWidgetItem(this, 0);
    trajectoriesItem->setText(/*currentColumn*/ 0, tr("Траектории"));
    //trajectoriesItem->setIcon(0, QIcon(":/svg/cmd.svg"));

    pinsItem = new QTreeWidgetItem(this, 0);
    pinsItem->setText(/*currentColumn*/ 0, tr("Штифты"));
    //pinsItem->setIcon(0, QIcon(":/svg/pin point.svg"));

    pinsItem->setExpanded(true);

    verticalLayout = new QVBoxLayout(this);
    verticalLayout->setObjectName(QStringLiteral("verticalLayout"));
    verticalSpacer = new QSpacerItem(20, 450, QSizePolicy::Minimum, QSizePolicy::Expanding);

    verticalLayout->addItem(verticalSpacer);

    setWindowTitle(QApplication::translate("this", "this", Q_NULLPTR));
    connect(this, &MyTreeWidget::itemClicked, [=](QTreeWidgetItem* item, int column) {
        current_Item = item;
        ///*currentColumn*/ 0 = column;
    });

    connect(this, &MyTreeWidget::itemChanged,
        [&](QTreeWidgetItem* item, int column) {
            if (item->parent() == gerberItem && !item->text(0).isEmpty())
                GerberFileHolder::setVisible(item->text(0), item->checkState(column) == Qt::Checked);
            if (item->parent() == millingItem && !item->text(0).isEmpty() && item->data(0, Qt::UserRole).value<QGraphicsItemGroup*>() != nullptr)
                item->data(0, Qt::UserRole).value<QGraphicsItemGroup*>()->setVisible(item->checkState(column) == Qt::Checked);
        });

    header()->setContextMenuPolicy(Qt::CustomContextMenu);
    connect(header(), &QHeaderView::customContextMenuRequested, [=](const QPoint& pos) { qDebug() << pos; });
}

void MyTreeWidget::addGerberFile(const QString& name)
{
    if (gerberItem->isExpanded() == false)
        gerberItem->setExpanded(true);
    QTreeWidgetItem* newItem = new QTreeWidgetItem(gerberItem, this->currentItem());
    newItem->setCheckState(/*currentColumn*/ 0, Qt::Checked);
    newItem->setText(/*currentColumn*/ 0, name);
    newItem->setExpanded(true);
    //newItem->setData(/*currentColumn*/ 0, Qt::UserRole, qVariantFromValue(group));
    showAll();
    map[name] = newItem;
}

void MyTreeWidget::setGerberFileColor(const QString& name, QColor& c)
{
    if (!map.contains(name))
        return;
    QPixmap pixmap(16, 16);
    c.setAlpha(255);
    pixmap.fill(c);
    map[name]->setData(/*currentColumn*/ 0, Qt::DecorationRole, qVariantFromValue(QIcon(pixmap)));
}

void MyTreeWidget::closeAllFiles()
{
    for (QTreeWidgetItem* item : gerberItem->takeChildren()) {
        DeleteItem(item);
    }
    for (QTreeWidgetItem* item : millingItem->takeChildren()) {
        DeleteItem(item);
    }
    map.clear();
}

void MyTreeWidget::addMilling(const QString& name, QGraphicsItemGroup* group)
{
    if (millingItem->isExpanded() == false)
        millingItem->setExpanded(true);
    QTreeWidgetItem* newItem = new QTreeWidgetItem(millingItem, this->currentItem());
    newItem->setCheckState(/*currentColumn*/ 0, Qt::Checked);
    newItem->setText(/*currentColumn*/ 0, name);
    newItem->setExpanded(true);
    newItem->setData(/*currentColumn*/ 0, Qt::UserRole, qVariantFromValue(group));
    showAll();
}

int MyTreeWidget::treeCount(QTreeWidget* tree, QTreeWidgetItem* parent = 0)
{
    /*
 не учтёт свёрнутые ветви; потому что правильно было бы делать через модель
*/
    tree->expandAll(); //а это "костыль"
    int count = 0;
    if (parent == 0) {
        int topCount = tree->topLevelItemCount();
        for (int i = 0; i < topCount; i++) {
            QTreeWidgetItem* item = tree->topLevelItem(i);
            if (item->isExpanded()) {
                count += treeCount(tree, item);
            }
        }
        count += topCount;
    }
    else {
        int childCount = parent->childCount();
        for (int i = 0; i < childCount; i++) {
            QTreeWidgetItem* item = parent->child(i);
            if (item->isExpanded()) {
                count += treeCount(tree, item);
            }
        }
        count += childCount;
    }
    return count;
}

void MyTreeWidget::DeleteItem(QTreeWidgetItem* currentItem)
{
    QTreeWidgetItem* parent = currentItem->parent();
    int index;
    if (parent) {
        index = parent->indexOfChild(this->currentItem());
        delete parent->takeChild(index);
    }
    else {
        index = this->indexOfTopLevelItem(this->currentItem());
        delete this->takeTopLevelItem(index);
    }
}

QTreeWidgetItem* MyTreeWidget::InsertItem(QTreeWidgetItem* parent, QString text)
{
    if (parent->isExpanded() == false)
        parent->setExpanded(true);
    QTreeWidgetItem* newItem = new QTreeWidgetItem(parent, this->currentItem());
    newItem->setText(/*currentColumn*/ 0, text);
    newItem->setExpanded(true);
    return newItem;
}

void MyTreeWidget::showAll(void)
{
    int cnt = treeCount(this, 0);
    QString str(tr("Всего: ") + QString("%1").arg(cnt));
    setWindowTitle(str);
}

void MyTreeWidget::gerbeFilerMenu(QTreeWidgetItem* item, QPoint pos)
{
    QMenu menu(this);
    menu.addAction(QIcon::fromTheme("document-close"), tr("&Close"), [=]() {
        //item->data(0, Qt::UserRole).value<QGraphicsItemGroup*>();
        GerberFileHolder::closeFile(item->text(0));
        map.remove(item->text(0));
        DeleteItem(item);

    });
    menu.addAction(QIcon::fromTheme("crosshairs"), tr("&Drill For Aperture"), [=]() {
        //item->data(0, Qt::UserRole).value<QGraphicsItemGroup*>();
        DrillForApertureForm dfa(item->text(0), this);
        dfa.exec();
    });
    menu.exec(pos);
}

QTreeWidgetItem* MyTreeWidget::getCurrent_Item() const
{
    return current_Item;
}

QTreeWidgetItem* MyTreeWidget::getCurrent_GerberItem() const
{
    if (current_Item != nullptr && current_Item->parent() == gerberItem)
        return current_Item;
    return nullptr;
}

void MyTreeWidget::mouseReleaseEvent(QMouseEvent* event)
{
    auto gerbeFilerMenu = [=](QTreeWidgetItem* item, QPoint pos) {
        QMenu menu(this);
        menu.addAction(QIcon::fromTheme("document-close"), tr("&Close"), [=]() {
            //item->data(0, Qt::UserRole).value<QGraphicsItemGroup*>();
            GerberFileHolder::closeFile(item->text(0));
            DeleteItem(item);
        });
        menu.addAction(QIcon::fromTheme("crosshairs"), tr("&Drill For Aperture"), [=]() {
            //item->data(0, Qt::UserRole).value<QGraphicsItemGroup*>();
            DrillForApertureForm dfa(item->text(0), this);
            dfa.exec();
        });
        menu.exec(pos);
    };

    auto millingItemFilerMenu = [=](QTreeWidgetItem* item, QPoint pos) {
        QMenu menu(this);
        menu.addAction(QIcon::fromTheme("document-close"), tr("&delete"), [=]() {
            QGraphicsScene* scene = item->data(0, Qt::UserRole).value<QGraphicsItemGroup*>()->scene();
            scene->removeItem(item->data(0, Qt::UserRole).value<QGraphicsItemGroup*>());
            for (QGraphicsItem* i : item->data(0, Qt::UserRole).value<QGraphicsItemGroup*>()->childItems()) {
                delete i;
            }
            delete item->data(0, Qt::UserRole).value<QGraphicsItemGroup*>();
            item->data(0, Qt::UserRole).value<QGraphicsItemGroup*>();
            DeleteItem(item);
        });
        menu.exec(pos);
    };

    QTreeWidgetItem* item = itemAt(event->pos());
    if (item != nullptr && event->button() == Qt::RightButton) {
        if (item->parent() == gerberItem) {
            gerbeFilerMenu(item, event->globalPos());
            return;
        }
        else if (item->parent() == millingItem) {
            millingItemFilerMenu(item, event->globalPos());
            return;
        }
        else if (item == gerberItem) {
            QMenu menu(this);
            menu.addAction(QIcon(), tr("Drawing raw"), [=]() {
                GerberFileHolder::drawingRaw();

            });
            menu.addAction(QIcon(), tr("Draw united"), [=]() {
                GerberFileHolder::drawUnited();
            });
            menu.exec(event->globalPos());
            return;
        }
    }
    QTreeWidget::mouseReleaseEvent(event);
}
