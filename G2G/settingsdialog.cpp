#include "settingsdialog.h"
#include "colorselector.h"

#include <QApplication>
#include <QGLWidget>
#include <QtWidgets>
#include <graphicsview.h>

const int gridColor = 100;

const QColor defaultColor[(size_t)Colors::Count]{
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

const QString colorName[(size_t)Colors::Count]{
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

SettingsDialog::SettingsDialog(QWidget* parent)
    : QDialog(parent)
{
    setupUi(this);

    for (int i = 0; i < static_cast<int>(Colors::Count); ++i) {
        formLayout->setWidget(i, QFormLayout::FieldRole, new ColorSelector(m_color[i], defaultColor[i], gbxColor));
        formLayout->setWidget(i, QFormLayout::LabelRole, new QLabel(colorName[i] + ":", gbxColor));
    }
    connect(cbxFontSize, &QComboBox::currentTextChanged, [=](const QString& fontSize) {
        QFont f;
        f.setPointSize(fontSize.toInt());
        qApp->setFont(f);
        qApp->setStyleSheet(QString(qApp->styleSheet()).replace(QRegExp("font-size:\\s*\\d+"), "font-size: " + fontSize));
    });

    //    sbxMinCircleSegments;
    //    dsbxMinCircleSegmentLenght;
    readSettings();
}

SettingsDialog::~SettingsDialog()
{
}

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

    settings.beginGroup("Circle");
    sbxMinCircleSegments->setValue(settings.value("MinCircleSegments", 36).toInt());
    dsbxMinCircleSegmentLenght->setValue(settings.value("MinCircleSegmentLenght", 0.5).toDouble());
    chbxCleanPolygons->setChecked(settings.value("MinCircleSegmentLenght", true).toBool());
    m_minCircleSegments = sbxMinCircleSegments->value();
    m_minCircleSegmentLenght = dsbxMinCircleSegmentLenght->value();
    m_cleanPolygons = chbxCleanPolygons->isChecked();
    settings.endGroup();

    settings.beginGroup("Application");
    const QString fontSize(settings.value("FontSize", "8").toString());
    qApp->setStyleSheet(/*a.styleSheet() +*/ "QWidget {font-size: " + fontSize + "pt}");
    cbxFontSize->setCurrentText(fontSize);
    settings.endGroup();

    settings.beginGroup("SettingsDialog");
    if (isVisible())
        settings.setValue("geometry", saveGeometry());
    restoreGeometry(settings.value("geometry", QByteArray()).toByteArray());
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

    settings.beginGroup("Circle");
    settings.setValue("MinCircleSegments", (m_minCircleSegments = sbxMinCircleSegments->value()));
    settings.setValue("MinCircleSegmentLenght", (m_minCircleSegmentLenght = dsbxMinCircleSegmentLenght->value()));
    settings.setValue("CleanPolygons", (m_cleanPolygons = chbxCleanPolygons->isChecked()));

    settings.endGroup();

    settings.beginGroup("Application");
    settings.setValue("FontSize", cbxFontSize->currentText());
    settings.endGroup();

    settings.beginGroup("SettingsDialog");
    settings.setValue("geometry", saveGeometry());
    settings.endGroup();
}

void SettingsDialog::reject()
{
    readSettings();
    QDialog::reject();
}

void SettingsDialog::accept()
{
    writeSettings();
    QDialog::accept();
}
