#ifndef THERMALFORM_H
#define THERMALFORM_H

#include "toolpathutil.h"

#include <QItemSelection>

namespace Ui {
class ThermalForm;
}

class QCheckBox;
class QGridLayout;
class ThermalModel;
class ThermalPreviewItem;

class ThermalForm : public ToolPathUtil {
    Q_OBJECT

public:
    explicit ThermalForm(QWidget* parent = nullptr);
    ~ThermalForm();

    void updateFiles();

signals:
    void createThermal(Gerber::File* file, const Tool& tool, double depth);

private slots:
    void on_pbSelect_clicked();
    void on_pbEdit_clicked();
    void on_pbCreate_clicked();
    void on_pbClose_clicked();
    void on_leName_textChanged(const QString& arg1);

    void on_cbxFileCurrentIndexChanged(int index);

    void on_dsbxDepth_valueChanged(double arg1);

private:
    Ui::ThermalForm* ui;

    void setApertures(const QMap<int, QSharedPointer<Gerber::AbstractAperture>>* value);
    QVector<QSharedPointer<ThermalPreviewItem>> m_sourcePreview;
    QMap<int, QSharedPointer<Gerber::AbstractAperture>> m_apertures;
    ThermalModel* model;
    void on_selectionChanged(const QItemSelection& selected, const QItemSelection& deselected);

    QCheckBox* cbx;
    QGridLayout* lay;
    double m_depth;
    inline void redraw();

    // ToolPathUtil interface
protected:
    void create() override;
    void updateName() override;
};

#endif // THERMALFORM_H
