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
    void on_currentChanged(const QModelIndex& current, const QModelIndex& previous);
    void on_pbClose_clicked();
    void on_pbCreate_clicked();

private:
    void createHoles(int apertureId, double diameter);
    void removeHoles(int apertureId);
    void pickUpTool(int apertureId, double diameter);

    DrillModel* model;
    Ui::DrillForm* ui;

    bool m_isAperture = false;
    QMap<int, QSharedPointer<G::AbstractAperture>> m_apertures;
    QMap<int, double> m_tools;
    QMap<int, QVector<QGraphicsPathItem*>> m_giaperture;
    QMap<int, QVector<DrillItem*>> m_gid;

    void clear();
};

class DrillModel : public QAbstractTableModel {
    typedef struct Row {
        Row(const QString& name, const QIcon& icon, int id)
            : name{ name, "" }
            , icon{ icon, QIcon() }
            , id{ id, -1 }
        {
        }
        QString name[2];
        QIcon icon[2];
        int id[2];
    } Row;
    QList<Row> m_data;
    bool m_isAperture;

public:
    DrillModel(bool isAperture, QObject* parent = nullptr);
    void appendRow(const QString& name, const QIcon& icon, int id);
    void setToolId(int row, int id);
    int toolId(int row);
    void setApertureId(int row, int id);
    int apertureId(int row);

    // QAbstractItemModel interface
public:
    int rowCount(const QModelIndex& parent = QModelIndex()) const override;
    int columnCount(const QModelIndex& parent = QModelIndex()) const override;
    QVariant data(const QModelIndex& index, int role) const override;
    QVariant headerData(int section, Qt::Orientation orientation, int role) const override;
    Qt::ItemFlags flags(const QModelIndex& index) const override;
};
#endif // DRILLFORM_H
