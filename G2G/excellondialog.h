#ifndef EXCELLONDIALOG_H
#define EXCELLONDIALOG_H

#include "gcode/drl.h"
#include <QDialog>

namespace Ui {
class ExcellonDialog;
}
class DrillFile;
class ExcellonDialog : public QDialog {
    Q_OBJECT

public:
    explicit ExcellonDialog(DrillFile* file);
    ~ExcellonDialog();

private slots:
    void on_pbStep_clicked();

private:
    Ui::ExcellonDialog* ui;
    DrillFile* m_file;
    const Format m_format;
    Format m_tmpFormat;
    int m_step = 3;
    //    ExcellonDialog*& m_widget;
    // QWidget interface
protected:
    void closeEvent(QCloseEvent* event) override;
};

#endif // EXCELLONDIALOG_H
