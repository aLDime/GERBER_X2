#ifndef TOOLEDIT_H
#define TOOLEDIT_H

#include "tool.h"
#include <QDialog>

namespace Ui {
class ToolDatabase;
}
class ToolItem;

class ToolDatabase : public QDialog {
    Q_OBJECT

public:
    explicit ToolDatabase(QWidget* parent = nullptr, QVector<Tool::Type> types = QVector<Tool::Type>{ Tool::Drill, Tool::EndMill, Tool::Engraving });
    ~ToolDatabase();

public:
    Tool tool() const;

private:
    Ui::ToolDatabase* ui;
    Tool m_tool;
    ToolItem* m_item;
    const QVector<Tool::Type> m_types;

protected:
    void keyPressEvent(QKeyEvent* evt) override;
};

#endif // TOOLEDIT_H
