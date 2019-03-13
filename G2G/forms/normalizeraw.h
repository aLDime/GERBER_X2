#ifndef NORMALIZERAW_H
#define NORMALIZERAW_H

#include <myclipper.h>

class NormalizeRaw {
public:
    NormalizeRaw(const Paths& paths);

    Paths paths();

private:
    Paths m_paths;
};

#endif // NORMALIZERAW_H
