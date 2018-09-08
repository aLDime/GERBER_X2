#ifndef DRILLFORM_H
#define DRILLFORM_H

#include <QGraphicsItem>
#include <QWidget>
#include <aperture.h>

#include <gcode/gcode.h>

namespace Ui {
class DrillForm;
}

class QStandardItemModel;

class DrillForm : public QWidget {
    Q_OBJECT

public:
    explicit DrillForm(QWidget* parent = nullptr);
    ~DrillForm();
    static DrillForm* self;

    void setApertures(const QMap<int, G::Aperture*>& value);
    void updateFiles();

private slots:
    void on_cbxFile_currentIndexChanged(int index);
    void on_doubleClicked(const QModelIndex& current);
    void on_currentChanged(const QModelIndex& current, const QModelIndex& previous);

    void on_pbClose_clicked();

    void on_pbCreate_clicked();

private:
    QStandardItemModel* model;
    Ui::DrillForm* ui;
    QMap<int, G::Aperture*> apertures;

    QMap<int, QVector<QGraphicsPathItem*>> gia;
    QMap<int, QVector<DrillItem*>> gid;

    //    QVector<QGraphicsPathItem*> gia;
    //    QVector<QGraphicsEllipseItem*> gid;
    void clear();
};

#endif // DRILLFORM_H
