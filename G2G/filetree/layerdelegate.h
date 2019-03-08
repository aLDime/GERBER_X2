#ifndef LAYERDELEGATE_H
#define LAYERDELEGATE_H

#include <QComboBox>
#include <QItemDelegate>

class LayerDelegate : public QItemDelegate {
    Q_OBJECT
public:
    LayerDelegate(QObject* parent = nullptr);
    virtual ~LayerDelegate();

    // QAbstractItemDelegate interface
public:
    QWidget* createEditor(QWidget* parent, const QStyleOptionViewItem& option, const QModelIndex& index) const override;
    void setEditorData(QWidget* editor, const QModelIndex& index) const override;
    void setModelData(QWidget* editor, QAbstractItemModel* model, const QModelIndex& index) const override;
    void emitCommitData();
};

#endif // LAYERDELEGATE_H
