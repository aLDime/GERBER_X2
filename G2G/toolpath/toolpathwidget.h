#ifndef TOOLPATHWIDGET_H
#define TOOLPATHWIDGET_H

#include <QWidget>

#include <tooldatabase/tool.h>

namespace Ui {
class ToolPathWidget;
}

class QCheckBox;
class QDoubleSpinBox;
class QFormLayout;
class QFrame;
class QGridLayout;
class QGroupBox;
class QLabel;
class QLineEdit;
class QPushButton;
class QVBoxLayout;
class Widget;

enum {
    PROFILE_TOOLPATH_FORM,
    POCKET_TOOLPATH_FORM,
    DRILLING_TOOLPATH_FORM,
    MATERIAL_SETUP_FORM
};

class ToolPathWidget : public QWidget {
    Q_OBJECT
    friend class Widget;

public:
    explicit ToolPathWidget(int m_type, QWidget* parent = nullptr);
    ~ToolPathWidget();

    int type() const;

private:
    void setVisibleTool2(bool visible);

signals:

public slots:

private:
    int m_type;

    QCheckBox* cbxTool2;
    QDoubleSpinBox* dsbxDepth;
    QFormLayout* formLayout;
    QFrame* line_3;
    QFrame* line_4;
    QFrame* lineT1;
    QFrame* lineT2;
    QGridLayout* gridLayout;
    QGroupBox* groupBox;
    QGroupBox* groupBox_2;
    QGroupBox* groupBox_3;
    QLabel* label;
    QLabel* label_1;
    QLabel* label_2;
    QLabel* label_3;
    QLabel* label_4;
    QLabel* label_5;
    QLabel* labelT1;
    QLabel* labelT2;
    QLabel* lblHome;
    QLabel* lblPanelName;
    QLabel* lblSafeZ;
    QLabel* lblToolName1;
    QLabel* lblToolName2;
    QLabel* lblVectors;
    QLabel* lblZero;
    QLineEdit* leNameToolPath;
    QPushButton* pbCalculate;
    QPushButton* pbClose;
    QPushButton* pbEditTool1;
    QPushButton* pbEditTool2;
    QPushButton* pbSelectTool1;
    QPushButton* pbSelectTool2;
    QVBoxLayout* verticalLayout;
    QVBoxLayout* verticalLayout_2;
    Widget* tpcWidget;

    void setupUi(QWidget* Form); // setupUi

    void retranslateUi(QWidget* Form); // retranslateUi
};

#endif // TOOLPATHWIDGET_H
