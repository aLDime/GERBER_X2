#ifndef DRILLFORM_H
#define DRILLFORM_H

#include <QGraphicsItem>
#include <QWidget>
#include <aperture.h>

namespace Ui {
class DrillForm;
}

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

private:
    Ui::DrillForm* ui;
    QMap<int, G::Aperture*> apertures;
    QVector<QGraphicsPathItem*> gia;

    void clear();
};

#endif // DRILLFORM_H
