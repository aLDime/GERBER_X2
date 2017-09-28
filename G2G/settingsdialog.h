#ifndef SETTINGSDIALOG_H
#define SETTINGSDIALOG_H

#include "ui_settingsdialog.h"

class SettingsDialog : public QDialog, private Ui::SettingsDialog {
    Q_OBJECT

public:
    explicit SettingsDialog(QWidget* parent = 0);
    ~SettingsDialog();

protected:
    void showEvent(QShowEvent* event) override;

private:
    void onScrollBarValueChanged(int value);
    void onListCategoriesCurrentRowChanged(int currentRow);
    void readSettings();
    void writeSettings();

public slots:
    void reject() override;
    void accept() override;
};

#endif // SETTINGSDIALOG_H
