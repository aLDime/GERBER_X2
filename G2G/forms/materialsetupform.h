#ifndef MATERIALSETUPFORM_H
#define MATERIALSETUPFORM_H

#include "point.h"
#include <QWidget>

namespace Ui {
class MaterialSetupForm;
}

class MaterialSetup : public QWidget {
    Q_OBJECT

public:
    explicit MaterialSetup(QWidget* prnt = nullptr);
    ~MaterialSetup();

    void updatePosDsbxs();

    static Point* homePoint;
    static Point* zeroPoint;
    static double safeZ;
    static double thickness;
    static double clearence;
    static double plunge;
    static double glue;
    static MaterialSetup* self;

private:
    Ui::MaterialSetupForm* ui;
};

#endif // MATERIALSETUPFORM_H
