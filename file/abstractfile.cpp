#include "abstractfile.h"

//Paths AbstractFile::merge()
//{
//    Paths paths;
//    Paths tmpPaths;
//    //    AbstractFile = gerberAbstractFile;
//    int i = 0, exp = -1;
//    Clipper clipper(ioStrictlySimple);
//    while (i < size()) {
//        clipper.Clear();
//        clipper.AddPaths(paths, ptSubject, true);
//        exp = at(i).state.imgPolarity;
//        do {
//            tmpPaths = at(i++).paths;
//            SimplifyPolygons(tmpPaths, pftNonZero);
//            clipper.AddPaths(tmpPaths, ptClip, true);
//        } while (i < size() && exp == at(i).state.imgPolarity);
//        if (at(i - 1).state.imgPolarity == G::Positive) {
//            clipper.Execute(ctUnion, paths, pftPositive);
//        } else {
//            clipper.Execute(ctDifference, paths, pftNonZero);
//        }
//    }
//    m_mergedPaths = paths;
//    return m_mergedPaths;
//}


