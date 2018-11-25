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

//class QStandardItemModel;
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
    void on_currentChanged(const QModelIndex& current, const QModelIndex& previous);
    void on_pbClose_clicked();
    void on_pbCreate_clicked();

private:
    DrillModel /*QStandardItemModel*/* model;
    Ui::DrillForm* ui;

    bool m_isAperture = false;
    QMap<int, QSharedPointer<G::AbstractAperture>> apertures;
    QMap<int, double> tools;
    QMap<int, QVector<QGraphicsPathItem*>> gia;
    QMap<int, QVector<DrillItem*>> gid;

    void clear();
};

class DrillModel : public QAbstractTableModel {
public:
    DrillModel(QObject* parent = nullptr)
        : QAbstractTableModel(parent)
    {
    }

    void appendRow(const QString& name, QIcon& icon, int id);

    // QAbstractItemModel interface
public:
    int rowCount(const QModelIndex& parent) const override;
    int columnCount(const QModelIndex& parent) const override;
    QVariant data(const QModelIndex& index, int role) const override;
    QVariant headerData(int section, Qt::Orientation orientation, int role) const override;

private:
    typedef struct {
        QString name[2];
        QIcon icon[2];
        int id[2];
    } row;
    QList<row> m_data;
};
#endif // DRILLFORM_H
