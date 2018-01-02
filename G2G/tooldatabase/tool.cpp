#include "tool.h"
#include <QtMath>

Tool::Tool()
    : name("Name")
    , note("Note")
{
}

Tool::Tool(const QString& name, const QString& note, const QByteArray& Data)
    : name(name)
    , note(note)
    , data(*reinterpret_cast<D*>(QByteArray::fromHex(Data).data()))
{
}

Tool::Tool(const QList<QString>& Data)
    : name(Data[0])
    , note(Data[1])
    , data(*reinterpret_cast<D*>(QByteArray::fromHex(Data[2].toLocal8Bit()).data()))
{
}

Tool::~Tool()
{
}

void Tool::fromHex(const QByteArray& Data)
{
    data = *reinterpret_cast<D*>(QByteArray::fromHex(Data).data());
}

QByteArray Tool::toHex() const
{
    return QByteArray(reinterpret_cast<const char*>(&data), sizeof(D)).toHex();
}

double Tool::diameter(double depth)const
{
    double d = data.params[Diameter];
    if (depth > 0.0 && data.params[SideAngle] > 0.0) {
        double a = qDegreesToRadians(90 - data.params[SideAngle] / 2);
        d = depth * cos(a) / sin(a);
        d = d * 2 + data.params[Diameter];
    }
    else
        d = data.params[Diameter];
    return d;
}
