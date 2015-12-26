#ifndef LIBENG_BORDERLEFT_H_
#define LIBENG_BORDERLEFT_H_
#if defined(__cplusplus)

#include <libeng/Graphic/Bounds/Bounds.h>
#include <libeng/Log/Log.h>

namespace eng {

//////
class BorderLeft : public Bounds {

public:
    BorderLeft(float left) : Bounds(), mLeft(left) {

        LOGV(LIBENG_LOG_BOUNDS, 0, LOG_FORMAT(" - l:%f"), __PRETTY_FUNCTION__, __LINE__, left);
    }
    virtual ~BorderLeft() { LOGV(LIBENG_LOG_BOUNDS, 0, LOG_FORMAT(), __PRETTY_FUNCTION__, __LINE__); }

    float mLeft;

    //////
    template<class T> inline bool overlap(const T* over) const { assert(NULL); return false; }
    template<class T> inline float overlapDist(const T* over) const { assert(NULL); return NO_BOUNDS_DIST; }
    template<class T> inline float overlapDistance(const T* over) const { assert(NULL); return NO_BOUNDS_DIST; }
    // WARNING: Only use template specializations! Defined in 'Overlap.h' header file
    // -> No way to put the definition of an inline member function into its implementation file (forward declaration needs)

};

class BorderRight;
class Rectangle2D;
class Circle2D;
class Polygone2D;

//
template<> inline bool BorderLeft::overlap<BorderRight>(const BorderRight* over) const;
template<> inline bool BorderLeft::overlap<Rectangle2D>(const Rectangle2D* over) const;
template<> inline bool BorderLeft::overlap<Circle2D>(const Circle2D* over) const;
template<> inline bool BorderLeft::overlap<Polygone2D>(const Polygone2D* over) const;

//
template<> inline float BorderLeft::overlapDist<BorderRight>(const BorderRight* over) const;
template<> inline float BorderLeft::overlapDist<Rectangle2D>(const Rectangle2D* over) const;
template<> inline float BorderLeft::overlapDist<Circle2D>(const Circle2D* over) const;
template<> inline float BorderLeft::overlapDist<Polygone2D>(const Polygone2D* over) const;

//
template<> inline float BorderLeft::overlapDistance<BorderRight>(const BorderRight* over) const;
template<> inline float BorderLeft::overlapDistance<Rectangle2D>(const Rectangle2D* over) const;
template<> inline float BorderLeft::overlapDistance<Circle2D>(const Circle2D* over) const;
template<> inline float BorderLeft::overlapDistance<Polygone2D>(const Polygone2D* over) const;

} // namespace

// Cast(s)
#define bLeftVia(b) static_cast<const eng::BorderLeft*>(b)

#endif // __cplusplus
#endif // LIBENG_BORDERLEFT_H_
