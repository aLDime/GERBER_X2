#ifndef DRILLFORM_H
#define DRILLFORM_H

#include <QAbstractTableModel>
#include <QGraphicsItem>
#include <QIcon>
#include <QItemSelectionModel>
#include <QWidget>
#include <aperture.h>
#include <gcode/toolpathcreator.h>

#include <gcode/gcode.h>
#include <gi/drillitem.h>

namespace Ui {
class DrillForm;
}

class DrillModel;
class PreviewItem;

class DrillForm : public QWidget {
    Q_OBJECT

public:
    explicit DrillForm(QWidget* parent = nullptr);
    ~DrillForm();
    static DrillForm* self;

    void setApertures(const QMap<int, QSharedPointer<Gerber::AbstractAperture>>* value);
    void setHoles(const QMap<int, double>& value);
    void updateFiles();

public slots:
    void on_pbClose_clicked();

private slots:
    void on_pbCreate_clicked();

private:
    enum WorckType {
        drilling,
        profile,
        pocket,
    };
    WorckType worckType = drilling;

    SideOfMilling side = Inner;

    void on_cbxFileCurrentIndexChanged(int index);
    void on_clicked(const QModelIndex& index);
    void on_doubleClicked(const QModelIndex& current);
    void on_currentChanged(const QModelIndex& current, const QModelIndex& previous);
    void on_customContextMenuRequested(const QPoint& pos);

    void createHoles(int toolId, double diameter);
    void pickUpTool(int apertureId, double diameter, bool isSlot = false);

    DrillModel* model;
    Ui::DrillForm* ui;

    int m_type;
    QMap<int, QSharedPointer<Gerber::AbstractAperture>> m_apertures;
    QMap<int, double> m_tools;
    QMap<int, QVector<QSharedPointer<PreviewItem>>> m_sourcePreview;

    void clear();
};

#endif // DRILLFORM_H
