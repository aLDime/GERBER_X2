#ifndef DRL_H
#define DRL_H

#include <QMap>
#include <QObject>
#include <abstractfile.h>
#include <myclipper.h>

using namespace ClipperLib;

enum UNIT_MODE {
    INCHES,
    MILLIMETERS,
};

enum M_CODE {
    M00,
    M30 = 30, // end of file
    M48 = 48, // begin of file // HEADER
    M71 = 71, // mm
    M72 = 72, // in
};

enum G_CODE {
    G00,
    //self.statements.append(RouteModeStmt())
    //self.state = 'ROUT'
    G05 = 5,
    //self.statements.append(DrillModeStmt())
    //self.state = 'DRILL'
    G90 = 90,
    //self.statements.append(AbsoluteModeStmt())
    //self.notation = 'absolute'
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

class Drill;
class ItemGroup;

class Hole {
public:
    Hole(
        const State& state,
        Drill* gFile)
        : state(state)
        , gFile(gFile)
    {
    }
    State state;
    Drill* gFile = nullptr;
};

class Drill : public AbstractFile, public QList<Hole> {
public:
    Drill() {}
    ~Drill() {}
    //    QSharedPointer<ItemGroup> itemGroup;
    QMap<int, double> m_toolDiameter;
    //    QString fileName;
    //Paths paths;
    FileType type() const override { return DrillFile; }

protected:
    Paths merge() const override
    {
        for (GraphicsItem* item : *m_itemGroup.data())
            m_mergedPaths.append(item->paths());
        return m_mergedPaths;
    }
};
class DrillParser : public QObject {
    Q_OBJECT
public:
    explicit DrillParser(QObject* parent = nullptr);
    Drill* parseFile(const QString& fileName);

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
    Drill* m_file = nullptr;
};

#endif // DRL_H
