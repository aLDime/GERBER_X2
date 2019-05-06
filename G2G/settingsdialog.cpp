#include "settingsdialog.h"
#include "colorselector.h"

#include <QGLWidget>
#include <QtWidgets>
#include <graphicsview.h>

const int gridColor = 100;

const QColor defaultColor[Colors::Count]{
    QColor(), //Background
    QColor(255, 255, 0, 120), //Shtift
    QColor(Qt::gray), //CutArea
    QColor(gridColor, gridColor, gridColor, 50), //Grid1
    QColor(gridColor, gridColor, gridColor, 100), //Grid5
    QColor(gridColor, gridColor, gridColor, 200), //Grid10
    QColor(), //Hole
    QColor(0, 255, 0, 120), //Home
    QColor(Qt::black), //ToolPath
    QColor(255, 0, 0, 120), //Zero
    QColor(Qt::red) //G0
};

const QString colorName[Colors::Count]{
    "Background",
    "Shtift",
    "CutArea",
    "Grid1",
    "Grid5",
    "Grid10",
    "Hole",
    "Home",
    "ToolPath",
    "Zero",
    "G0",
};

QColor SettingsDialog::m_color[Colors::Count]{
    QColor(), //Background
    QColor(255, 255, 0, 120), //Shtift
    QColor(Qt::gray), //CutArea
    QColor(gridColor, gridColor, gridColor, 50), //Grid1
    QColor(gridColor, gridColor, gridColor, 100), //Grid5
    QColor(gridColor, gridColor, gridColor, 200), //Grid10
    QColor(), //Hole
    QColor(0, 255, 0, 120), //Home
    QColor(Qt::black), //ToolPath
    QColor(255, 0, 0, 120), //Zero
    QColor(Qt::red) //G0
};

SettingsDialog::SettingsDialog(QWidget* parent)
    : QDialog(parent)
{
    setupUi(this);

    for (QGroupBox* box : this->findChildren<QGroupBox*>()) {
        listCategories->addItem(box->title());
        listCategories->item(listCategories->count() - 1)->setData(Qt::UserRole, box->objectName());
    }

    for (int i = 0; i < static_cast<int>(Colors::Count); ++i) {
        formLayout->setWidget(i, QFormLayout::FieldRole, new ColorSelector(m_color[i], defaultColor[i], gbxColor));
        formLayout->setWidget(i, QFormLayout::LabelRole, new QLabel(colorName[i] + ":", gbxColor));
    }

    listCategories->item(0)->setSelected(true);
    //connect(scrollSettings->verticalScrollBar(), &QScrollBar::valueChanged, this, &SettingsDialog::onScrollBarValueChanged);
    connect(listCategories, &QListWidget::currentRowChanged, this, &SettingsDialog::onListCategoriesCurrentRowChanged);
    readSettings();
}

SettingsDialog::~SettingsDialog()
{
}

void SettingsDialog::onScrollBarValueChanged(int value)
{
    Q_UNUSED(value)

    // Search for first full visible groupbox
    for (int i = 0; i < listCategories->count(); i++) {
        QGroupBox* box = this->findChild<QGroupBox*>(listCategories->item(i)->data(Qt::UserRole).toString());
        if (box) {
            if (!box->visibleRegion().isEmpty() && box->visibleRegion().boundingRect().y() == 0) {
                // Select corresponding item in categories list
                listCategories->setCurrentRow(i);
                return;
            }
        }
    }
}

void SettingsDialog::onListCategoriesCurrentRowChanged(int /*currentRow*/)
{
    // Scroll to selected groupbox
    //    QGroupBox* box = this->findChild<QGroupBox*>(listCategories->item(currentRow)->data(Qt::UserRole).toString());
    //    if (box) {
    //        scrollSettings->ensureWidgetVisible(box);
    //    }
}

//QRectF SettingsDialog::worckRect()
//{
//    return m_worckRect;
//}

//void SettingsDialog::setWorckRect(const QRectF& value)
//{
//    m_worckRect = value;
//}

void SettingsDialog::readSettings()
{
    QSettings settings;
    settings.beginGroup("Viewer");
    chbOpenGl->setChecked(settings.value("OpenGl").toBool());
    chbAntialiasing->setChecked(settings.value("Antialiasing").toBool());
    settings.endGroup();
    settings.beginGroup("Color");
    for (int i = 0; i < static_cast<int>(Colors::Count); ++i) {
        m_color[i].setNamedColor(settings.value(QString("%1").arg(i), m_color[i].name(QColor::HexArgb)).toString());
    }
    settings.endGroup();
}

void SettingsDialog::writeSettings()
{
    QSettings settings;
    settings.beginGroup("Viewer");
    if (settings.value("OpenGl").toBool() != chbOpenGl->isChecked()) {
        GraphicsView::self->setViewport(chbOpenGl->isChecked()
                ? new QGLWidget(QGLFormat(QGL::SampleBuffers | QGL::AlphaChannel | QGL::Rgba))
                : new QWidget);
//        chbOpenGl->isChecked()
//            ? GraphicsView::self->setViewportUpdateMode(QGraphicsView::FullViewportUpdate)
//            : GraphicsView::self->setViewportUpdateMode(QGraphicsView::SmartViewportUpdate);
        GraphicsView::self->viewport()->setObjectName("viewport");
        GraphicsView::self->setRenderHint(QPainter::Antialiasing, chbAntialiasing->isChecked());
        settings.setValue("OpenGl", chbOpenGl->isChecked());
    }
    if (settings.value("Antialiasing").toBool() != chbAntialiasing->isChecked()) {
        GraphicsView::self->setRenderHint(QPainter::Antialiasing, chbAntialiasing->isChecked());
        settings.setValue("Antialiasing", chbAntialiasing->isChecked());
    }
    settings.endGroup();

    settings.beginGroup("Color");
    for (int i = 0; i < static_cast<int>(Colors::Count); ++i) {
        settings.setValue(QString("%1").arg(i), m_color[i].name(QColor::HexArgb));
    }
    settings.endGroup();
}

void SettingsDialog::showEvent(QShowEvent* event)
{
    Q_UNUSED(event)
    //    scrollSettings->updateMinimumWidth();
}

void SettingsDialog::reject()
{
    readSettings();
    QDialog::reject();
}

void SettingsDialog::accept()
{
    //qDebug() << "accept";
    writeSettings();
    QDialog::accept();
}
