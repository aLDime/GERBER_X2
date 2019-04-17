#ifndef DRILLFORM_H
#define DRILLFORM_H

#include <QHeaderView>
#include <QWidget>

#include <gcode/gcode.h>
#include <gcode/toolpathcreator.h>

class MyHeader : public QHeaderView {
    Q_OBJECT
    bool isShecked = false;

public:
    MyHeader(Qt::Orientation orientation, QWidget* parent = nullptr);
    // QWidget interface
signals:
    void updateCreateButton();

protected:
    void mouseReleaseEvent(QMouseEvent* event) override;
    void paintSection(QPainter* painter, const QRect& rect, int logicalIndex) const override;
};

namespace Ui {
class DrillForm;
}
namespace Gerber {
class AbstractAperture;
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
    GCodeType m_worckType = Drilling;
    SideOfMilling m_side = Inner;

    void on_cbxFileCurrentIndexChanged(int index);
    void on_clicked(const QModelIndex& index);
    void on_doubleClicked(const QModelIndex& current);
    void on_currentChanged(const QModelIndex& current, const QModelIndex& previous);
    void on_customContextMenuRequested(const QPoint& pos);

    void createHoles(int toolId, int toolIdSelected);
    void pickUpTool(int apertureId, double diameter, bool isSlot = false);
    void updateCreateButton();

    DrillModel* model;
    Ui::DrillForm* ui;

    int m_type;
    QMap<int, QSharedPointer<Gerber::AbstractAperture>> m_apertures;
    QMap<int, double> m_tools;
    QMap<int, QVector<QSharedPointer<PreviewItem>>> m_sourcePreview;

    void clear();
};

#endif // DRILLFORM_H
