#ifndef DRILLFORM_H
#define DRILLFORM_H

#include <QAbstractTableModel>
#include <QGraphicsItem>
#include <QIcon>
#include <QWidget>
#include <aperture.h>

#include <gcode/gcode.h>

#include <gi/drillitem.h>

namespace Ui {
class DrillForm;
}

class DrillModel;

class DrillForm : public QWidget {
    Q_OBJECT

public:
    explicit DrillForm(QWidget* parent = nullptr);
    ~DrillForm();
    static DrillForm* self;

    void setApertures(const QMap<int, QSharedPointer<G::AbstractAperture>>& value);
    void setHoles(const QMap<int, double>& value);
    void updateFiles();

private slots:
    void on_cbxFile_currentIndexChanged(int index);
    void on_doubleClicked(const QModelIndex& current);
    void on_clicked(const QModelIndex& index);
    void on_currentChanged(const QModelIndex& current, const QModelIndex& previous);
    void on_pbClose_clicked();
    void on_pbCreate_clicked();

private:
    void createHoles(int toolId, double diameter);
    void removeHoles(int apertureId);
    void pickUpTool(int apertureId, double diameter, bool isSlot = false);

    DrillModel* model;
    Ui::DrillForm* ui;

    int m_type;
    QMap<int, QSharedPointer<G::AbstractAperture>> m_apertures;
    QMap<int, double> m_tools;
    QMap<int, QVector<QGraphicsPathItem*>> m_sourcePreview;
    QMap<int, QVector<DrillItem*>> m_giDrill;

    QMap<int, QPolygonF> m_drills;
    QMap<int, QVector<QPolygonF>> m_slots;

    void clear();
};

#endif // DRILLFORM_H
