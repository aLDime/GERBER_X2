#ifndef EDITTOOL_H
#define EDITTOOL_H

#include "tool.h"

#include <QWidget>
#include <QString>
#include <QByteArray>
#include <QByteArray>
#include <QDialog>
#include <qmap.h>

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

class ToolEdit : public QWidget {
    Q_OBJECT

public:
    explicit ToolEdit(QWidget* parent = 0);
    ~ToolEdit();

    void apply();
    void setTool(const Tool& value);

    QPushButton* pbApply;
    QComboBox* cbxToolType;
    Tool getTool() const;

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

    QComboBox* cbxUnits;
    QMap<int, QDoubleSpinBox*> dsbList;
    QGroupBox* groupBox;
    QGroupBox* grBox_2;
    QGroupBox* grBox_3;
    QGroupBox* grBox_4;
    QLabel* lblPixmap;
    //    QLabel* label_10;
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

    QSpinBox* sbSpindleSpeed;

    void setupUi(QWidget* ToolEdit); // setupUi
    void retranslateUi(QWidget* ToolEdit); // retranslateUi
    void updateName();

    int lastType = 0;
    bool isNew = true;

    // QWidget interface
protected:
    void showEvent(QShowEvent* event) override;
};

class EditToolDialog : public QDialog {
    Q_OBJECT
public:
    EditToolDialog(QWidget* parent, const Tool& tool);
    virtual ~EditToolDialog();

    ToolEdit* toolEdit;
    void setupUi(QDialog* Dialog); // setupUi
    void retranslateUi(QDialog* Dialog); // retranslateUi
    Tool getTool() const;
};

#endif // EDITTOOL_H
