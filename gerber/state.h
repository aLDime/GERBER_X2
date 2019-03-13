#ifndef STATE_H
#define STATE_H

#include "../clipper/myclipper.h"
#include "format.h"
#include "gerber.h"
#include <QObject>

using namespace ClipperLib;
namespace G {

class State {
public:
    void reset(Format* f)
    {
        m_format = f;
        m_dCode = D02;
        m_gCode = G01;
        m_imgPolarity = Positive;
        m_interpolation = Linear;
        m_type = Aperture;
        m_quadrant = Undef;
        m_region = Off;
        m_aperture = 0;
        m_lineNum = 0;
        m_curPos = IntPoint();
        m_mirroring = NoMirroring;
        m_scaling = 1.0;
        m_rotating = 0.0;
    }

    inline Format* format() const { return m_format; }
    inline void setFormat(Format* format) { m_format = format; }

    inline DCode dCode() const { return m_dCode; }
    inline void setDCode(const DCode& dCode) { m_dCode = dCode; }

    inline GCode gCode() const { return m_gCode; }
    inline void setGCode(const GCode& gCode) { m_gCode = gCode; }

    inline ImagePolarity imgPolarity() const { return m_imgPolarity; }
    inline void setImgPolarity(const ImagePolarity& imgPolarity) { m_imgPolarity = imgPolarity; }

    inline InterpolationMode interpolation() const { return m_interpolation; }
    inline void setInterpolation(const InterpolationMode& interpolation) { m_interpolation = interpolation; }

    inline PrimitiveType type() const { return m_type; }
    inline void setType(const PrimitiveType& type) { m_type = type; }

    inline QuadrantMode quadrant() const { return m_quadrant; }
    inline void setQuadrant(const QuadrantMode& quadrant) { m_quadrant = quadrant; }

    inline RegionMode region() const { return m_region; }
    inline void setRegion(const RegionMode& region) { m_region = region; }

    inline int aperture() const { return m_aperture; }
    inline void setAperture(int aperture) { m_aperture = aperture; }

    inline IntPoint& curPos() { return m_curPos; }
    inline IntPoint curPos() const { return m_curPos; }
    inline void setCurPos(const IntPoint& curPos) { m_curPos = curPos; }

    inline Mirroring mirroring() const { return m_mirroring; }
    inline void setMirroring(const Mirroring& mirroring) { m_mirroring = mirroring; }

    inline double scaling() const { return m_scaling; }
    inline void setScaling(double scaling) { m_scaling = scaling; }

    inline double rotating() const { return m_rotating; }
    inline void setRotating(double rotating) { m_rotating = rotating; }

private:
    Format* m_format = nullptr;
    DCode m_dCode = D02;
    GCode m_gCode = G01;
    ImagePolarity m_imgPolarity = Positive;
    InterpolationMode m_interpolation = Linear;
    PrimitiveType m_type = Aperture;
    QuadrantMode m_quadrant = Undef;
    RegionMode m_region = Off;
    int m_aperture = 0;
    int m_lineNum = 0;
    IntPoint m_curPos;
    Mirroring m_mirroring;
    double m_scaling = 1.0;
    double m_rotating = 0.0;
};
}

#endif // STATE_H
