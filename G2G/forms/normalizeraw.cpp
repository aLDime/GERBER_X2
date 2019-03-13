#include "normalizeraw.h"

NormalizeRaw::NormalizeRaw(const Paths& paths)
    : m_paths(paths)
{
}

Paths NormalizeRaw::paths() /*const*/
{
    double ks = 0.1 * uScale;
    Paths paths;
    Path path;
    do {
        path = m_paths.takeFirst();
        for (int i = 0; i < m_paths.size(); /*++i*/) {
            const IntPoint& pt1 = path.last();
            const IntPoint& pt2 = m_paths[i].first();
            const IntPoint& pt3 = m_paths[i].last();
            if (Length(pt1, pt2) < ks) {
                qDebug("first");
                while (m_paths[i].size())
                    path.append(m_paths[i].takeFirst());
                m_paths.remove(i);
                i = 0;
                //continue;
            } else if (Length(pt1, pt3) < ks) {
                qDebug("last");
                while (m_paths[i].size())
                    path.append(m_paths[i].takeLast());
                m_paths.remove(i);
                i = 0;
                //continue;
            } else
                ++i;
            qDebug() << "i" << i << "size" << m_paths.size();
        }

        if (Area(path) < 0)
            ReversePath(path);
        paths.append(path);
        qDebug() << "paths.size" << paths.size();
    } while (m_paths.size());

    Clipper clipper;
    clipper.AddPaths(paths, ptSubject, true);
    //clipper.StrictlySimple(true);
    IntRect r(clipper.GetBounds());
    int k = uScale * 1;
    Path outer = {
        IntPoint(r.left - k, r.bottom + k),
        IntPoint(r.right + k, r.bottom + k),
        IntPoint(r.right + k, r.top - k),
        IntPoint(r.left - k, r.top - k)
    };
    ReversePath(outer);
    clipper.AddPath(outer, ptClip, true);
    clipper.Execute(ctXor, paths, pftEvenOdd);
    paths.takeFirst();
    qDebug() << paths.size();
    return paths;
}
