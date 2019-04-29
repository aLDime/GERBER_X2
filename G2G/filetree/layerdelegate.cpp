#include "layerdelegate.h"

LayerDelegate::LayerDelegate(QObject* parent)
    : QItemDelegate(parent)
{
}

LayerDelegate::~LayerDelegate() {}

QWidget* LayerDelegate::createEditor(QWidget* parent, const QStyleOptionViewItem& option, const QModelIndex& index) const
{
    QComboBox* comboBox = new QComboBox(parent);
    comboBox->addItems({ tr("Top"), tr("Bottom") });
    //        if (index.column() == 1)
    //            comboBox->addItems(IconPreviewArea::iconModeNames());
    //        else if (index.column() == 2)
    //            comboBox->addItems(IconPreviewArea::iconStateNames());
    //connect(comboBox, QOverload<int>::of(&QComboBox::activated), this, &LayerDelegate::emitCommitData);
    return comboBox;
    return QItemDelegate::createEditor(parent, option, index);
}

void LayerDelegate::setEditorData(QWidget* editor, const QModelIndex& index) const
{
    QComboBox* comboBox = qobject_cast<QComboBox*>(editor);
    if (!comboBox)
        return;
    //        int pos = comboBox->findText(index.model()->data(index).toString(),
    //            Qt::MatchExactly);
    comboBox->setCurrentIndex(index.model()->data(index).toInt());
}

void LayerDelegate::setModelData(QWidget* editor, QAbstractItemModel* model, const QModelIndex& index) const
{
    QComboBox* comboBox = qobject_cast<QComboBox*>(editor);
    if (!comboBox)
        return;
    model->setData(index, bool(comboBox->currentIndex()));
}

void LayerDelegate::emitCommitData()
{
    emit commitData(qobject_cast<QWidget*>(sender()));
}
