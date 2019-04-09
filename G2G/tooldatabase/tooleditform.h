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

public:
    explicit ToolEditForm(QWidget* parent = nullptr);
    ~ToolEditForm();

    void setItem(ToolItem* item);
    void setTool(const Tool& tool);
    Tool tool() const;
    void setDialog() /*const*/;

signals:
    void itemChanged(ToolItem* item);

private slots:

    void on_cbxFeedSpeeds_currentIndexChanged(int index);
    void on_cbxToolType_currentIndexChanged(int index);

    void on_leName_textChanged(const QString& arg1);
    void on_teNote_textChanged();

    void calculate(int index, double value);
    void valueChanged(double value);

public slots:
    void on_pbApply_clicked();

private:
    Ui::ToolEditForm* ui;

    ToolItem* m_item = nullptr;
    Tool m_tool;
    double m_feed = 1.0;
    bool m_dialog = true;

    void updateName();
    void setRed();
    void setRedReset();
    void flicker(QDoubleSpinBox* dsbx);
    void setVisibleWidgets(bool visible);
};

#endif // TOOLEDITFORM_H
