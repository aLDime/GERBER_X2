#ifndef FILE_H
#define FILE_H

#include "abstractitem.h"

#include <gerber/graphicsitem.h>
#include <gerber/parser.h>
#include <QObject>

class FileItem : public QObject, public AbstractItem {
    Q_OBJECT

public:
    FileItem(Gerber::File* gerberFile);
    ~FileItem();

    // AbstractItem interface
public:
    bool setData(const QModelIndex& index, const QVariant& value, int role) override;
    int columnCount(/*const QModelIndex& parent*/) const override;
    int rowCount(/*const QModelIndex& parent*/) const override;
    Qt::ItemFlags flags(const QModelIndex& index) const override;
    QVariant data(const QModelIndex& index, int role) const override;

private:
    Gerber::File* gerberFile;

    Qt::CheckState checkState = Qt::Checked;
    static QTimer repaintTimer;
    void repaint();

    //public:
    //    explicit GerberFileHolder(QObject* parent = nullptr);
    //    static void closeFile(const QString& fileName);
    //    static void closeAllFiles();
    //    static void setVisible(const QString& fileName, bool visible);
    //    static GERBER_FILE* getFile(const QString& fileName);
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
    //    void handleResults(GERBER_FILE* gerberFile);
    //private:
    //    static QMap<QString, QPair<GERBER_FILE*, GerberItemGroup*> > files;
    //    QPair<GERBER_FILE*, GerberItemGroup*> pair(GERBER_FILE* file, GerberItemGroup* itemGroup)
    //    {
    //        return qMakePair<GERBER_FILE*, GerberItemGroup*>(file, itemGroup);
    //    }
    //    static MainWindow* mainWindow;
    //    static DRAWING_TYPE drawingType;
    //    static QTimer* repaint;
};

//QMap<QString, QPair<GERBER_FILE*, GerberItemGroup*> > GerberFileHolder::files;
//GerberFileHolder::DRAWING_TYPE GerberFileHolder::drawingType = GerberFileHolder::UNITED;
//MainWindow* GerberFileHolder::mainWindow = nullptr;
//QTimer* GerberFileHolder::repaint = nullptr;
//GerberFileHolder::GerberFileHolder(QObject* parent)
//    : QObject(parent)
//{
//    if (parent != nullptr) {
//        mainWindow = static_cast<MainWindow*>(parent);
//        repaint = new QTimer(parent);
//        connect(repaint, &QTimer::timeout, [=]() {
//            int c = 0, cc;
//            QMapIterator<QString, QPair<GERBER_FILE*, GerberItemGroup*> > i(files);
//            while (i.hasNext()) {
//                i.next();
//                cc = (files.size() > 1) ? (240.0 / (files.size() - 1)) * c++ : 0;
//                QColor color(QColor::fromHsv(cc, 255 - cc * 0.2, 255, 150));
//                i.value().second->setBrush(color);
//                //                mainWindow->treeWidget->setGerberFileColor(i.key(), color);
//            }
//            mainWindow->graphicsView->scene()->update();
//        });
//    }
//}
//void GerberFileHolder::handleResults(GERBER_FILE* gerberFile)
//{
//    //associateFileTypes('.' + gerberFile->fileName.split('.').last());
//    files[gerberFile->fileName] = pair(gerberFile, nullptr);
//    switch (drawingType) {
//    case RAW:
//        drawingType = REDRAW;
//        drawingRaw();
//        break;
//    case UNITED:
//        drawingType = REDRAW;
//        drawUnited();
//        break;
//    case REDRAW:
//        break;
//    }
//    //    mainWindow->treeWidget->addGerberFile(gerberFile->fileName);
//    mainWindow->graphicsView->ZoomFit();
//}
//void GerberFileHolder::closeFile(const QString& fileName)
//{
//    QGraphicsScene* scene = files[fileName].second->first()->scene();
//    delete files[fileName].first;
//    delete files[fileName].second;
//    files.remove(fileName);
//    if (scene != nullptr)
//        scene->update();
//    mainWindow->getScene()->setSceneRect(mainWindow->getScene()->itemsBoundingRect());
//    if (drawingType == UNITED) {
//        repaint->start(100);
//    }
//}
//int GerberFileHolder::getDrawingType()
//{
//    return drawingType;
//}
//void GerberFileHolder::closeAllFiles()
//{
//    QMapIterator<QString, QPair<GERBER_FILE*, GerberItemGroup*> > i(files);
//    while (i.hasNext()) {
//        i.next();
//        delete i.value().first;
//        delete i.value().second;
//    }
//    files.clear();
//}
//void GerberFileHolder::setVisible(const QString& fileName, bool visible)
//{
//    files[fileName].second->setVisible(visible);
//}
//GERBER_FILE* GerberFileHolder::getFile(const QString& fileName)
//{
//    return files[fileName].first;
//}
//bool GerberFileHolder::contains(const QString& fileName)
//{
//    return files.contains(fileName);
//}
//void GerberFileHolder::drawingRaw()
//{
//    if (drawingType == RAW)
//        return;
//    bool visible = true;
//    QMapIterator<QString, QPair<GERBER_FILE*, GerberItemGroup*> > i(files);
//    auto createGroup = [&i](QGraphicsScene* scene) {
//        *const_cast<GerberItemGroup**>(&i.value().second) = new GerberItemGroup;
//        for (GERBER_ITEM& item : *i.value().first) {
//            if ((item.state.type == APERTURE && item.state.imgPolarity == NEGATIVE) || item.state.type != APERTURE)
//                i.value().second->append(new GerberRawItem(item));
//        }
//        for (GERBER_ITEM& item : *i.value().first) {
//            if (item.state.type == APERTURE && item.state.imgPolarity == POSITIVE)
//                i.value().second->append(new GerberRawItem(item));
//        }
//        i.value().second->addToTheScene(scene);
//    };
//    while (i.hasNext()) {
//        i.next();
//        if (i.value().second != nullptr) {
//            visible = i.value().second->isVisible();
//            if (i.value().second->first()->type() != GerberRawItemType) {
//                delete i.value().second;
//                createGroup(mainWindow->getScene());
//            }
//        }
//        else {
//            createGroup(mainWindow->getScene());
//        }
//        i.value().second->setVisible(visible);
//    }
//    drawingType = RAW;
//}
//void GerberFileHolder::drawUnited()
//{
//    if (drawingType == UNITED)
//        return;
//    bool visible = true;
//    QMapIterator<QString, QPair<GERBER_FILE*, GerberItemGroup*> > i(files);
//    auto createGroup = [&i](MyGraphicsScene* scene) {
//        *const_cast<GerberItemGroup**>(&i.value().second) = new GerberItemGroup;
//        ToolPathCreator tpc;
//        tpc.Merge(i.value().first);
//        int counter = 0;
//        for (Paths& vpaths : tpc.GetGroupedPaths(COPPER)) {
//            i.value().second->append(new GerberWorkItem(vpaths));
//            i.value().second->last()->setToolTip(QString("COPPER %1").arg(++counter));
//        }
//        //tpc.Clear();
//        i.value().second->addToTheScene(scene);
//    };
//    while (i.hasNext()) {
//        i.next();
//        if (i.value().second != nullptr) {
//            visible = i.value().second->isVisible();
//            if (i.value().second->first()->type() != GerberWorkItemType) {
//                delete i.value().second;
//                createGroup(mainWindow->getScene());
//            }
//        }
//        else {
//            createGroup(mainWindow->getScene());
//        }
//        i.value().second->setVisible(visible);
//        repaint->start(300);
//    }
//    drawingType = UNITED;
//}

#endif // FILE_H
