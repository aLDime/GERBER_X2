//#ifndef GERBERFILEHOLDER_H
//#define GERBERFILEHOLDER_H

//#include "gerber/gerberparser.h"

//#include <QObject>

//class GerberItemGroup;
//class MainWindow;
//class QTimer;

//class GerberFileHolder : public QObject {
//    Q_OBJECT

//public:
//    explicit GerberFileHolder(QObject* parent = nullptr);

//    static void closeFile(const QString& fileName);
//    static void closeAllFiles();
//    static void setVisible(const QString& fileName, bool visible);
//    static GerberFile* getFile(const QString& fileName);
//    static bool contains(const QString& fileName);

//    static void drawingRaw();
//    static void drawUnited();

//    static int getDrawingType();

//    enum DRAWING_TYPE {
//        RAW,
//        UNITED,
//        REDRAW
//    };

//signals:

//public slots:
//    void handleFile(GerberFile* gerberFile);

//private:
//    static QMap<QString, QPair<GerberFile*, GerberItemGroup*> > files;

//    QPair<GerberFile*, GerberItemGroup*> pair(GerberFile* file, GerberItemGroup* itemGroup)
//    {
//        return qMakePair<GerberFile*, GerberItemGroup*>(file, itemGroup);
//    }

//    static MainWindow* mainWindow;
//    static DRAWING_TYPE drawingType;
//    static QTimer* repaint;
//};

//#endif // GERBERFILEHOLDER_H
