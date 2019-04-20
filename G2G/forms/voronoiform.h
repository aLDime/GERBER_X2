#ifndef VORONOIFORM_H
#define VORONOIFORM_H

#include "toolpathutil.h"
#include <QWidget>

namespace Ui {
class VoronoiForm;
}

class VoronoiForm : public QWidget, public ToolPathUtil {
    Q_OBJECT

public:
    explicit VoronoiForm(QWidget* parent = nullptr);
    ~VoronoiForm();

private slots:
    void on_pbSelect_clicked();
    void on_pbEdit_clicked();
    void on_pbCreate_clicked();
    void on_pbClose_clicked();

private:
    Ui::VoronoiForm* ui;

    double m_size = 0.0;
    double m_lenght = 0.0;

    // ToolPathUtil interface
protected:
    void create() override;
    void updateName() override;
};

#endif // VORONOIFORM_H
