#ifndef MATERIALSETUPFORM_H
#define MATERIALSETUPFORM_H

#include <QWidget>

namespace Ui {
class MaterialSetupForm;
}

class MaterialSetup : public QWidget {
    Q_OBJECT

public:
    explicit MaterialSetup(QWidget* prnt = nullptr);
    ~MaterialSetup();

    void setHomePos(QPointF pos);
    void setZeroPos(QPointF pos);

    static QPointF homePos;
    static QPointF zeroPos;
    static double z;
    static double thickness;
    static double clearence;
    static double plunge;
    static MaterialSetup* self;

private:
    Ui::MaterialSetupForm* ui;
};

#endif // MATERIALSETUPFORM_H
