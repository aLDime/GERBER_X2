#ifndef DRL_H
#define DRL_H

#include <QMap>
#include <QObject>
#include <abstractfile.h>
#include <myclipper.h>

using namespace ClipperLib;

enum UnitMode {
    Inches,
    Millimeters,
};

enum ZeroMode {
    LeadingZeros,
    TrailingZeros,
};

enum MCode {
    M00,
    M30 = 30, // end of file
    M48 = 48, // Beginning of a Part Program Header
    M71 = 71, // mm
    M72 = 72, // in
    M95 = 95, // End of a Part Program Header or %
};

enum GCode {
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
        format.unitMode = Millimeters;
        format.decimal = 3;
        format.integer = 3;
        gCode = G00;
        mCode = M00;
        tCode = 0;
        currentToolDiameter = 0.0;
        pos = QPointF();
    }
    struct Format {
        ZeroMode zeroMode = LeadingZeros;
        UnitMode unitMode = Millimeters;
        int decimal = 3;
        int integer = 3;
    } format;
    GCode gCode = G00;
    MCode mCode = M00;
    int tCode = 0;
    double currentToolDiameter = 0.0;
    QPointF pos;
};

class DrillFile;
class ItemGroup;

class Hole {
public:
    Hole(
        const State& state,
        DrillFile* gFile )
        : state(state)
        , gFile(gFile)
    {
    }
    State state;
    DrillFile* gFile = nullptr;
};

class DrillFile : public AbstractFile, public QList<Hole> {
public:
    DrillFile() {}
    ~DrillFile() {}
    //    QSharedPointer<ItemGroup> itemGroup;
    QMap<int, double> m_toolDiameter;
    //    QString fileName;
    //Paths paths;
    FileType type() const override { return FileType::Drill; }

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
    DrillFile* parseFile(const QString& fileName);

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
    DrillFile* m_file = nullptr;
};

#endif // DRL_H
