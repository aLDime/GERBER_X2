#ifndef TERMALFORM_H
#define TERMALFORM_H

#include "toolpathutil.h"

#include <QItemSelection>

namespace Ui {
class TermalForm;
}

class QCheckBox;
class QGridLayout;
class TermalModel;
class TermalPreviewItem;

class TermalForm : public ToolPathUtil {
    Q_OBJECT

public:
    explicit TermalForm(QWidget* parent = nullptr);
    ~TermalForm();

    void updateFiles();

signals:
    void createTermal(Gerber::File* file, const Tool& tool, double depth);

private slots:
    void on_pbSelect_clicked();
    void on_pbEdit_clicked();
    void on_pbCreate_clicked();
    void on_pbClose_clicked();
    void on_leName_textChanged(const QString& arg1);

    void on_cbxFileCurrentIndexChanged(int index);

private:
    Ui::TermalForm* ui;

    void setApertures(const QMap<int, QSharedPointer<Gerber::AbstractAperture>>* value);
    QVector<QSharedPointer<TermalPreviewItem>> m_sourcePreview;
    QMap<int, QSharedPointer<Gerber::AbstractAperture>> m_apertures;
    TermalModel* model;
    void on_currentChanged(const QModelIndex& current, const QModelIndex& previous);
    void on_selectionChanged(const QItemSelection &selected, const QItemSelection &deselected);

    QCheckBox* cbx;
    QGridLayout* lay;

    // ToolPathUtil interface
protected:
    void create() override;
    void updateName() override;
};

#endif // TERMALFORM_H
