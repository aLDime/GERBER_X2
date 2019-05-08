#include "toolpathutil.h"

#include "filetree/filemodel.h"
#include <QEvent>
#include <QFile>
#include <QJsonArray>
#include <QJsonDocument>
#include <QTimer>
#include <qprogressdialog.h>

ToolPathUtil::ToolPathUtil(const QString& name, QWidget* parent)
    : QWidget(parent)
    , m_name(name + ".dat")
{
    readTools({ &tool, &tool2 });
}

ToolPathUtil::~ToolPathUtil()
{
    writeTools({ &tool, &tool2 });
}

void ToolPathUtil::readTools(const QVector<Tool*>& tool) const
{
    QFile file(m_name);

    if (!file.open(QIODevice::ReadOnly)) {
        qWarning("Couldn't open tools file.");
        return;
    }

    QJsonDocument loadDoc(QJsonDocument::fromBinaryData(file.readAll()));
    QJsonObject json = loadDoc.object();
    QJsonArray toolArray = json["tools"].toArray();
    for (int treeIndex = 0; treeIndex < toolArray.size(); ++treeIndex) {
        QJsonObject toolObject = toolArray[treeIndex].toObject();
        tool[treeIndex]->read(toolObject);
    }
}

void ToolPathUtil::writeTools(const QVector<Tool*>& tool) const
{
    QFile file(m_name);
    if (!file.open(QIODevice::WriteOnly)) {
        qWarning("Couldn't open tools file.");
        return;
    }

    QJsonArray toolArray;
    for (int treeIndex = 0; treeIndex < tool.size(); ++treeIndex) {
        QJsonObject toolObject;
        tool[treeIndex]->write(toolObject);
        toolArray.append(toolObject);
    }
    QJsonObject json;
    json["tools"] = toolArray;
    QJsonDocument saveDoc(json);
    file.write(saveDoc.toBinaryData());
}

ToolPathCreator* ToolPathUtil::toolPathCreator(const Paths& value, const bool convent, SideOfMilling side)
{
    thread.wait();
    m_tps = new ToolPathCreator(value, convent, side);
    m_tps->moveToThread(&thread);
    //    connect(m_tps, &ToolPathCreator::progress, this, &ToolPathUtil::progress, Qt::QueuedConnection);
    connect(m_tps, &ToolPathCreator::fileReady, this, &ToolPathUtil::setFile, Qt::QueuedConnection);
    connect(&thread, &QThread::finished, m_tps, &QObject::deleteLater);
    //    connect(&thread, &QThread::finished, timer, &QObject::deleteLater);
    thread.start(QThread::HighestPriority);
    return m_tps;
}


void ToolPathUtil::progress(int max, int value)
{
    //qDebug() << "progress" << max << value;

    if (max && !value) {
        qDebug() << "create";
        if (pd)
            delete pd;
        pd = new QProgressDialog(this);
        pd->setLabelText(m_fileName);
        pd->setMaximum(max);
        pd->setModal(true);
        pd->setWindowFlag(Qt::WindowCloseButtonHint, false);
        pd->show();
        connect(pd, &QProgressDialog::canceled, this, &ToolPathUtil::cancel);
        m_timerId = startTimer(50);
    } else if (pd && !max && !value) {
        qDebug() << "close";
        pd->hide();
        pd->deleteLater();
        pd = nullptr;
    } else if (pd) {
        pd->setMaximum(max);
        pd->setValue(value);
    }
}

void ToolPathUtil::cancel()
{
    thread.requestInterruption();
    thread.quit();
    thread.wait();
    if (pd) {
        pd->deleteLater();
        pd = nullptr;
    }
    if (m_timerId) {
        killTimer(m_timerId);
        m_timerId = 0;
    }
    qDebug("canceled");
}

void ToolPathUtil::setFile(GCodeFile* file)
{
    if (pd)
        pd->canceled();
    thread.quit();
    thread.wait();
    if (file == nullptr) {
        QMessageBox::information(this, tr("Warning"), tr("The tool does not fit in the Working items!"));
        return;
    }
    file->setFileName(m_fileName);
    file->setSide(boardSide);
    FileModel::addFile(file);
}

void ToolPathUtil::timerEvent(QTimerEvent* event)
{
    if (event->timerId() == m_timerId && pd && m_tps) {
        pd->setMaximum(m_tps->progressMax());
        pd->setValue(m_tps->progressValue());
        if (!m_tps->progressMax() && !m_tps->progressValue())
            pd->canceled();
    }
}
