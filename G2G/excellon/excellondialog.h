#ifndef EXCELLONDIALOG_H
#define EXCELLONDIALOG_H

#include "exvars.h"
#include <QDialog>

namespace Ui {
class ExcellonDialog;
}

namespace Excellon {
class DrillFile;
}

class ExcellonDialog : public QDialog {
    Q_OBJECT

public:
    explicit ExcellonDialog(Excellon::DrillFile* file);
    ~ExcellonDialog();

private slots:
    void on_pbStep_clicked();

private:
    Ui::ExcellonDialog* ui;
    Excellon::DrillFile* m_file;
    const Excellon::Format m_format;
    Excellon::Format m_tmpFormat;
    int m_step = 3;

    void updateFormat();

protected:
    void closeEvent(QCloseEvent* event) override;
};

#endif // EXCELLONDIALOG_H
