#ifndef DRL_H
#define DRL_H

#include <QMap>
#include <QObject>
#include <myclipper.h>

using namespace ClipperLib;

enum UNIT_MODE {
    INCHES,
    MILLIMETERS,
};

enum M_CODE {
    M00,
    M30 = 30, // end of file
    M48 = 48, // begin of file
    M71 = 71, // mm
    M72 = 72, // in
};

enum G_CODE {
    G00,
    G05 = 5,
    G90 = 90,
};

struct State {
    void reset()
    {
        format.unitMode = MILLIMETERS;
        format.decimal = 3;
        format.integer = 3;
        gCode = G00;
        mCode = M00;
        tCode = 0;
        currentToolDiameter = 0.0;
        pos = QPointF();
    }
    struct Format {
        UNIT_MODE unitMode = MILLIMETERS;
        int decimal = 3;
        int integer = 3;
    } format;
    G_CODE gCode = G00;
    M_CODE mCode = M00;
    int tCode = 0;
    double currentToolDiameter = 0.0;
    QPointF pos;
};

class DrlFile;
class ItemGroup;

class Hole {
public:
    Hole(
        const State& state,
        DrlFile* gFile)
        : state(state)
        , gFile(gFile)
    {
    }
    State state;
    DrlFile* gFile = nullptr;
};

class DrlFile : public QList<Hole> {
public:
    DrlFile() {}
    ~DrlFile()
    {
        if (itemGroup)
            delete itemGroup;
    }
    ItemGroup* itemGroup = nullptr;
    QMap<int, double> m_toolDiameter;
    QString fileName;
    //Paths paths;
};
class Drl : public QObject {
    Q_OBJECT
public:
    explicit Drl(QObject* parent = nullptr);
    DrlFile* parseFile(const QString& fileName);

signals:

public slots:

private:
    bool parseComment(const QString& line);

    bool parseGCode(const QString& line);
    bool parseMCode(const QString& line);
    bool parseTCode(const QString& line);

    bool parsePos(const QString& line);

    bool parseFormat(const QString& line);

    ////////////
    /// \brief The Format class
    ///

    State m_state;
    DrlFile* m_file = nullptr;
};

#endif // DRL_H
