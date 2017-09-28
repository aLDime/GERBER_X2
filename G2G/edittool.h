#ifndef EDITTOOL_H
#define EDITTOOL_H

#include <QWidget>

class QComboBox;
class QDoubleSpinBox;
class QGroupBox;
class QLabel;
class QLineEdit;
class QPlainTextEdit;
class QPushButton;
class QSpinBox;

namespace Ui {
class EditTool;
}

typedef struct {
public:
    double Params[10];
    //    double ClearencePassStepover;
    //    double ClearencePassStepoverPercent;
    //    double Diameter;
    //    double FeedRate;
    //    double FlatDiameter;
    //    double PassDepth;
    //    double PlungeRate;
    //    double SideAngle;
    //    double Stepover;
    //    double StepoverPercent;
    int FeedSpeeds;
    int SpindleSpeed;
    int ToolType;
    QString Name;
    QString Note;
    //    TOOL() {}
} TOOL;

class EditTool : public QWidget {
    Q_OBJECT

public:
    explicit EditTool(QWidget* parent = 0);
    ~EditTool();

    TOOL getTool();
    void setTool(const TOOL& value);
    void createNew();

private slots:
    void on_cbxToolType_activated(int index);

private:
    TOOL tool;

    enum ToolType {
        EndMill,
        Engraving,
        Drill
    };

    enum FeedpSpeeds {
        mm_sec,
        mm_min,
        m_min
    };

    enum {
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

    void setupUi(QWidget* EditTool); // setupUi
    void retranslateUi(QWidget* EditTool); // retranslateUi
    void updateName();

    int lastIndex = 0;
    bool isNew = true;
};

#endif // EDITTOOL_H
