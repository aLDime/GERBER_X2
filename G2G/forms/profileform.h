#ifndef PROFILEFORM_H
#define PROFILEFORM_H

#include "toolpathutil.h"
#include <QWidget>

namespace Ui {
class ProfileForm;
}

class ProfileForm : public QWidget, public ToolPathUtil {
    Q_OBJECT

public:
    explicit ProfileForm(QWidget* parent = nullptr);
    ~ProfileForm();

private slots:
    void on_pbSelect_clicked();
    void on_pbEdit_clicked();
    void on_pbCreate_clicked();
    void on_pbClose_clicked();

private:
    Ui::ProfileForm* ui;

    int direction = 0;
    int side = 0;
};

#endif // PROFILEFORM_H
