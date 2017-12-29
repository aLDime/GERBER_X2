#ifndef DRILLFORAPERTUREFORM_H
#define DRILLFORAPERTUREFORM_H

#include <QDialog>

#include <gerber/aperture.h>

class QTableView;
class MyGraphicsView;
class QDialogButtonBox;

namespace Ui {
class DrillForApertureForm;
}

class DrillForApertureForm : public QDialog {
public:
    DrillForApertureForm(const QString& fileName, QWidget* parent = 0);

    //    QVBoxLayout* verticalLayout;
    //    QHBoxLayout* horizontalLayout;
    QTableView* tableView;
    MyGraphicsView* graphicsView;
    QDialogButtonBox* buttonBox;
    QString fileName;
    QMap<int, G::Aperture*> apertures;
    void setupUi(QDialog* Dialog); // setupUi
    void retranslateUi(QDialog* Dialog); // retranslateUi

    // QWidget interface
protected:
    virtual void resizeEvent(QResizeEvent* event) override;
};

#endif // DRILLFORAPERTUREFORM_H
