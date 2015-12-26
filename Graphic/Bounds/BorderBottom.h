#ifndef LIBENG_BORDERBOTTOM_H_
#define LIBENG_BORDERBOTTOM_H_
#if defined(__cplusplus)

#include <libeng/Graphic/Bounds/Bounds.h>
#include <libeng/Log/Log.h>

namespace eng {

//////
class BorderBottom : public Bounds {

public:
    BorderBottom(float bottom) : Bounds(), mBottom(bottom) {

        LOGV(LIBENG_LOG_BOUNDS, 0, LOG_FORMAT(" - b:%f"), __PRETTY_FUNCTION__, __LINE__, bottom);
    }
    virtual ~BorderBottom() { LOGV(LIBENG_LOG_BOUNDS, 0, LOG_FORMAT(), __PRETTY_FUNCTION__, __LINE__); }

    float mBottom;

    //////
    template<class T> inline bool overlap(const T* over) const { assert(NULL); return false; }
    template<class T> inline float overlapDist(const T* over) const { assert(NULL); return NO_BOUNDS_DIST; }
    template<class T> inline float overlapDistance(const T* over) const { assert(NULL); return NO_BOUNDS_DIST; }
    // WARNING: Only use template specializations! Defined in 'Overlap.h' header file
    // -> No way to put the definition of an inline member function into its implementation file (forward declaration needs)

};

class BorderTop;
class Rectangle2D;
class Circle2D;
class Polygone2D;

//
template<> inline bool BorderBottom::overlap<BorderTop>(const BorderTop* over) const;
template<> inline bool BorderBottom::overlap<Rectangle2D>(const Rectangle2D* over) const;
template<> inline bool BorderBottom::overlap<Circle2D>(const Circle2D* over) const;
template<> inline bool BorderBottom::overlap<Polygone2D>(const Polygone2D* over) const;

//
template<> inline float BorderBottom::overlapDist<BorderTop>(const BorderTop* over) const;
template<> inline float BorderBottom::overlapDist<Rectangle2D>(const Rectangle2D* over) const;
template<> inline float BorderBottom::overlapDist<Circle2D>(const Circle2D* over) const;
template<> inline float BorderBottom::overlapDist<Polygone2D>(const Polygone2D* over) const;

//
template<> inline float BorderBottom::overlapDistance<BorderTop>(const BorderTop* over) const;
template<> inline float BorderBottom::overlapDistance<Rectangle2D>(const Rectangle2D* over) const;
template<> inline float BorderBottom::overlapDistance<Circle2D>(const Circle2D* over) const;
template<> inline float BorderBottom::overlapDistance<Polygone2D>(const Polygone2D* over) const;

} // namespace

// Cast(s)
#define bBotVia(b)  static_cast<const eng::BorderBottom*>(b)

#endif // __cplusplus
#endif // LIBENG_BORDERBOTTOM_H_
