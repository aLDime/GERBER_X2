#include "exfile.h"
#include <gi/drillitem.h>

namespace Excellon {

File::File()
    : m_format(this)
{
}

File::~File()
{
    saveFormat();
    qDebug("~File()");
}

Format File::format() const
{
    return m_format;
}

void File::setFormat(const Format& value)
{
    //    m_format = value;
    //    m_format.file = this;
    m_format.zeroMode = value.zeroMode;
    m_format.unitMode = value.unitMode;
    m_format.decimal = value.decimal;
    m_format.integer = value.integer;
    m_format.offsetPos = value.offsetPos;
    for (Hole& hole : *this) {
        hole.state.updatePos();
        hole.item->updateHole();
    }
}

void File::setFormatForFile(const Format& /*value*/)
{
    //    QList<QString> lines;
    //    QFile file(fileName());
    //    if (!file.open(QIODevice::ReadOnly | QIODevice::Text))
    //        return;
    //    QTextStream in(&file);
    //    while (!in.atEnd()) {
    //        lines.append(in.readLine());
    //    }
}

template <typename T>
void write(QFile& file, T t)
{
    file.write(reinterpret_cast<const char*>(&t), sizeof(T));
}
template <typename T>
void read(QFile& file, T& t)
{
    file.read(reinterpret_cast<char*>(&t), sizeof(T));
}

void File::saveFormat()
{
    QFile file(m_fileName + ".fmt");
    if (file.open(QFile::WriteOnly)) {
        qDebug("saveFormat()");
        write(file, m_format.zeroMode);
        write(file, m_format.unitMode);
        write(file, m_format.decimal);
        write(file, m_format.integer);
        write(file, m_format.offsetPos.x());
        write(file, m_format.offsetPos.y());
    }
}

void File::restoreFormat()
{
    QFile file(m_fileName + ".fmt");
    if (file.exists() && file.open(QFile::ReadOnly)) {
        double x, y;
        read(file, m_format.zeroMode);
        read(file, m_format.unitMode);
        read(file, m_format.decimal);
        read(file, m_format.integer);
        read(file, x);
        read(file, y);
        m_format.offsetPos = QPointF(x, y);
        setFormat(m_format);
    }
}

double File::tool(int t) const
{
    double tool = 0.0;
    if (m_tools.contains(t)) {
        tool = m_tools[t];
        if (m_format.unitMode == Inches)
            tool *= 25.4;
    }
    return tool;
}

QMap<int, double> File::tools() const
{
    QMap<int, double> tools(m_tools);
    QMap<int, double>::iterator toolIt;
    if (m_format.unitMode == Inches)
        for (toolIt = tools.begin(); toolIt != tools.end(); ++toolIt)
            toolIt.value() *= 25.4;
    return tools;
}

Paths Excellon::File::merge() const
{
    for (GraphicsItem* item : *m_itemGroup.data())
        m_mergedPaths.append(item->paths());
    return m_mergedPaths;
}
} //  namespace Excellon


void Excellon::File::save() const
{
}

void Excellon::File::open() const
{
}