#ifndef EDITTOOL_H
#define EDITTOOL_H

#include "tool.h"

#include <QWidget>
#include <QString>
#include <QByteArray>
#include <QByteArray>

class QComboBox;
class QDoubleSpinBox;
class QGroupBox;
class QLabel;
class QLineEdit;
class QPlainTextEdit;
class QPushButton;
class QSpinBox;

enum ToolInfo {
    ClearencePassStepover,
    ClearencePassStepoverPercent,
    Diameter,
    FeedRate,
    FlatDiameter,
    PassDepth,
    PlungeRate,
    SideAngle,
    Stepover,
    StepoverPercent
};

namespace Ui {
class EditTool;
}

class ToolEdit : public QWidget {
    Q_OBJECT

public:
    explicit ToolEdit(QWidget* parent = 0);
    ~ToolEdit();

    void apply();
    void setTool(const Tool& value);

signals:
    void toolEdited(const Tool& tool);

private slots:
    void on_cbxToolType_currentIndexChanged(int index);

private:
    Tool tool;

    enum FeedpSpeeds {
        mm_sec,
        mm_min,
        m_min
    };

    QComboBox* cbxFeedSpeeds;
    QComboBox* cbxToolType;
    QComboBox* cbxUnits;
    QVector<QDoubleSpinBox*> dsbList;
    QGroupBox* groupBox;
    QGroupBox* grBox_2;
    QGroupBox* grBox_3;
    QGroupBox* grBox_4;
    QLabel* lblPixmap;
    QLabel* label_10;
    QLabel* label_11;
    QLabel* label_12;
    QLabel* label_13;
    QLabel* label_14;
    QLabel* label_2;
    QLabel* label_3;
    QLabel* label_4;
    QLabel* label_5;
    QLabel* label_6;
    QLabel* label_7;
    QLabel* label_8;
    QLabel* label_9;
    QLineEdit* leName;
    QPlainTextEdit* pteNote;
    QPushButton* bpApply;
    QSpinBox* sbSpindleSpeed;

    void setupUi(QWidget* ToolEdit); // setupUi
    void retranslateUi(QWidget* ToolEdit); // retranslateUi
    void updateName();

    int lastType = 0;
    bool isNew = true;

    // QWidget interface
protected:
    void showEvent(QShowEvent *event) override;
};

#endif // EDITTOOL_H
