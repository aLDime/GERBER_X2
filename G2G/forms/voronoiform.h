#ifndef VORONOIFORM_H
#define VORONOIFORM_H

#include "formsutil.h"

namespace Ui {
class VoronoiForm;
}

class VoronoiForm : public FormsUtil {
    Q_OBJECT

public:
    explicit VoronoiForm(QWidget* parent = nullptr);
    ~VoronoiForm();

signals:
    GCodeFile* createVoronoi(const Tool& tool, double depth, const double k);

private slots:
    void on_pbSelect_clicked();
    void on_pbEdit_clicked();
    void on_pbCreate_clicked();
    void on_pbClose_clicked();
    void on_leName_textChanged(const QString& arg1);

private:
    Ui::VoronoiForm* ui;

    double m_size = 0.0;
    double m_lenght = 0.0;

    // FormsUtil interface
protected:
    void create() override;
    void updateName() override;
  public:  virtual void editFile(GCodeFile* file) override;
};

#endif // VORONOIFORM_H
