#ifndef LIBENG_BORDERTOP_H_
#define LIBENG_BORDERTOP_H_
#if defined(__cplusplus)

#include <libeng/Graphic/Bounds/Bounds.h>
#include <libeng/Log/Log.h>

namespace eng {

//////
class BorderTop : public Bounds {

public:
    BorderTop(float top) : Bounds(), mTop(top) {

        LOGV(LIBENG_LOG_BOUNDS, 0, LOG_FORMAT(" - t:%f"), __PRETTY_FUNCTION__, __LINE__, top);
    }
    virtual ~BorderTop() { LOGV(LIBENG_LOG_BOUNDS, 0, LOG_FORMAT(), __PRETTY_FUNCTION__, __LINE__); }

    float mTop;

    //////
    template<class T> inline bool overlap(const T* over) const { assert(NULL); return false; }
    template<class T> inline float overlapDist(const T* over) const { assert(NULL); return NO_BOUNDS_DIST; }
    template<class T> inline float overlapDistance(const T* over) const { assert(NULL); return NO_BOUNDS_DIST; }
    // WARNING: Only use template specializations! Defined in 'Overlap.h' header file
    // -> No way to put the definition of an inline member function into its implementation file (forward declaration needs)

};

class BorderBottom;
class Rectangle2D;
class Circle2D;
class Polygone2D;

//
template<> inline bool BorderTop::overlap<BorderBottom>(const BorderBottom* over) const;
template<> inline bool BorderTop::overlap<Rectangle2D>(const Rectangle2D* over) const;
template<> inline bool BorderTop::overlap<Circle2D>(const Circle2D* over) const;
template<> inline bool BorderTop::overlap<Polygone2D>(const Polygone2D* over) const;

//
template<> inline float BorderTop::overlapDist<BorderBottom>(const BorderBottom* over) const;
template<> inline float BorderTop::overlapDist<Rectangle2D>(const Rectangle2D* over) const;
template<> inline float BorderTop::overlapDist<Circle2D>(const Circle2D* over) const;
template<> inline float BorderTop::overlapDist<Polygone2D>(const Polygone2D* over) const;

//
template<> inline float BorderTop::overlapDistance<BorderBottom>(const BorderBottom* over) const;
template<> inline float BorderTop::overlapDistance<Rectangle2D>(const Rectangle2D* over) const;
template<> inline float BorderTop::overlapDistance<Circle2D>(const Circle2D* over) const;
template<> inline float BorderTop::overlapDistance<Polygone2D>(const Polygone2D* over) const;

} // namespace

// Cast(s)
#define bTopVia(b)  static_cast<const eng::BorderTop*>(b)

#endif // __cplusplus
#endif // LIBENG_BORDERTOP_H_
