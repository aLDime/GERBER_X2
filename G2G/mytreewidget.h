#ifndef MYTREEWIDGET_H
#define MYTREEWIDGET_H

#include <QtWidgets>
#include <QTreeWidget>

class MyTreeWidget : public QTreeWidget {
    Q_OBJECT
public:
    explicit MyTreeWidget(QWidget* parent = 0);
    void addGerberFile(const QString& name);
    void setGerberFileColor(const QString& name, QColor &c);
    void closeAllFiles();
    void addMilling(const QString& name, QGraphicsItemGroup* group);
    QTreeWidgetItem* getCurrent_Item() const;
    QTreeWidgetItem* getCurrent_GerberItem() const;

signals:

private:
    int treeCount(QTreeWidget*, QTreeWidgetItem*); //подсчёт количества элементов в QTreeWidget
    void DeleteItem(QTreeWidgetItem* currentItem); //удаление элемента из QTreeWidget
    QTreeWidgetItem* InsertItem(QTreeWidgetItem*, QString); //добавление элемента в QTreeWidget
    void showAll(void); //вывод информации о QTreeWidget
    void gerbeFilerMenu(QTreeWidgetItem* item, QPoint pos);

private:
    QTreeWidgetItem* current_Item; //текущий элемент, запоминается при клике в QTreeWidget
    int currentColumn; //номер столбца, на самом деле будет = 0, т.к. у нас 1 столбец

    QVBoxLayout* verticalLayout;
    QSpacerItem* verticalSpacer;

    QTreeWidgetItem* filesItem;
    QTreeWidgetItem* gerberItem;
    QTreeWidgetItem* drillItem;
    QTreeWidgetItem* trajectoriesItem;
    QTreeWidgetItem* drillersItem;
    QTreeWidgetItem* millingItem;
    QTreeWidgetItem* pinsItem;

    // QWidget interface
protected:
    virtual void mouseReleaseEvent(QMouseEvent* event) override;
};

#endif // MYTREEWIDGET_H
