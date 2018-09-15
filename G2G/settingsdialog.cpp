#include "settingsdialog.h"

#include <QtWidgets>

SettingsDialog::SettingsDialog(QWidget* parent)
    : QDialog(parent)
{
    setupUi(this);

    for (QGroupBox* box : this->findChildren<QGroupBox*>()) {
        listCategories->addItem(box->title());
        listCategories->item(listCategories->count() - 1)->setData(Qt::UserRole, box->objectName());
    }

    listCategories->item(0)->setSelected(true);
    connect(scrollSettings->verticalScrollBar(), &QScrollBar::valueChanged, this, &SettingsDialog::onScrollBarValueChanged);
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

void SettingsDialog::onListCategoriesCurrentRowChanged(int currentRow)
{
    // Scroll to selected groupbox
    QGroupBox* box = this->findChild<QGroupBox*>(listCategories->item(currentRow)->data(Qt::UserRole).toString());
    if (box) {
        scrollSettings->ensureWidgetVisible(box);
    }
}

void SettingsDialog::readSettings()
{
    QSettings settings(QCoreApplication::organizationName(), QCoreApplication::applicationName());
    settings.beginGroup("Viewer");
    chbOpenGl->setChecked(settings.value("OpenGl").toBool());
    chbAntialiasing->setChecked(settings.value("Antialiasing").toBool());
    settings.endGroup();
}

void SettingsDialog::writeSettings()
{
    QSettings settings(QCoreApplication::organizationName(), QCoreApplication::applicationName());
    settings.beginGroup("Viewer");
    settings.setValue("OpenGl", chbOpenGl->isChecked());
    settings.setValue("Antialiasing", chbAntialiasing->isChecked());
    settings.endGroup();
}

void SettingsDialog::showEvent(QShowEvent* event)
{
    Q_UNUSED(event)
    //    scrollSettings->updateMinimumWidth();
}

void SettingsDialog::reject()
{
    //qDebug() << "reject";
    QDialog::reject();
}

void SettingsDialog::accept()
{
    //qDebug() << "accept";
    writeSettings();
    QDialog::accept();
}
