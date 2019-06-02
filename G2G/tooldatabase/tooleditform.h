#ifndef TOOLEDITFORM_H
#define TOOLEDITFORM_H

#include "tool.h"

#include <QWidget>

namespace Ui {
class ToolEditForm;
}

class ToolItem;
class QDoubleSpinBox;

class ToolEditForm : public QWidget {
    Q_OBJECT
    friend class ToolEditDialog;

public:
    explicit ToolEditForm(QWidget* parent = nullptr);
    ~ToolEditForm();

    void setItem(ToolItem* item);
    void setTool(const Tool& tool);
    void setDialog() /*const*/;

signals:
    void itemChanged(ToolItem* item);
    void toolChanged(const Tool& item);

public slots:
    void on_pbApply_clicked();

private:
    Ui::ToolEditForm* ui;

    void calculate(int index, double value);
    void valueChanged(double value);

    ToolItem* m_item = nullptr;
    Tool m_tool;
    double m_feed = 1.0;
    bool m_dialog = true;

    void updateName();
    void setRed();
    void setRedReset();
    void flicker(QDoubleSpinBox* dsbx);
    void setVisibleWidgets(bool visible);
    QVector<QObject*> set;
};

#endif // TOOLEDITFORM_H
