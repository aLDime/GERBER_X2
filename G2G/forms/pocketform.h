#ifndef POCKETFORM_H
#define POCKETFORM_H

#include "toolpathutil.h"
#include <QWidget>

namespace Ui {
class PocketForm;
}

class PocketForm : public QWidget, public ToolPathUtil {
    Q_OBJECT

public:
    explicit PocketForm(QWidget* parent = nullptr);
    ~PocketForm();

private slots:
    void on_pbSelect_clicked();
    void on_pbEdit_clicked();

    void on_pbCreate_clicked();
    void on_pbClose_clicked();

    void on_pbSelect_2_clicked();
    void on_pbEdit_2_clicked();

private:
    Ui::PocketForm* ui;

    int direction = 0;
    int type = 0;

    // ToolPathUtil interface
public:
    void create() override;
};

#endif // POCKETFORM_H
