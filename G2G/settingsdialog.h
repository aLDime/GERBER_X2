#ifndef SETTINGSDIALOG_H
#define SETTINGSDIALOG_H

#include "ui_settingsdialog.h"

enum class Colors : int {
    Background,
    Shtift,
    CutArea,
    Grid1,
    Grid5,
    Grid10,
    Hole,
    Home,
    ToolPath,
    Zero,
    G0,
    Count
};

class SettingsDialog : public QDialog, private Ui::SettingsDialog {
    Q_OBJECT

public:
    explicit SettingsDialog(QWidget* parent = nullptr);
    ~SettingsDialog() override;
    void readSettings();
    void writeSettings();
    /////////////////////
    static QColor& color(Colors id) { return m_color[static_cast<int>(id)]; }

public slots:
    void reject() override;
    void accept() override;


private:
    static QColor m_color[(size_t)Colors::Count];
};

#endif // SETTINGSDIALOG_H
