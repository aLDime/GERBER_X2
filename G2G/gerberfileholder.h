#ifndef GERBERFILEHOLDER_H
#define GERBERFILEHOLDER_H

#include "gerber/gerberparser.h"

#include <QObject>

class GerberItemGroup;
class MainWindow;
class QTimer;

class GerberFileHolder : public QObject {
    Q_OBJECT
public:
    explicit GerberFileHolder(QObject* parent = nullptr);

    static void closeFile(const QString& fileName);
    static void closeAllFiles();
    static void setVisible(const QString& fileName, bool visible);
    static GERBER_FILE* getFile(const QString& fileName);
    static bool contains(const QString& fileName);

    static void drawingRaw();
    static void drawUnited();

    static int getDrawingType();

    enum DRAWING_TYPE {
        RAW,
        UNITED,
        REDRAW
    };

signals:

public slots:
    void handleResults(GERBER_FILE* gerberFile);

private:
    static QMap<QString, QPair<GERBER_FILE*, GerberItemGroup*> > files;

    QPair<GERBER_FILE*, GerberItemGroup*> pair(GERBER_FILE* file, GerberItemGroup* itemGroup)
    {
        return qMakePair<GERBER_FILE*, GerberItemGroup*>(file, itemGroup);
    }

    static MainWindow* mainWindow;
    static DRAWING_TYPE drawingType;
    static QTimer* repaint;
};

#endif // GERBERFILEHOLDER_H
