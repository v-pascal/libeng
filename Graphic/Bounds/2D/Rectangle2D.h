#ifndef LIBENG_RECTANGLE2D_H_
#define LIBENG_RECTANGLE2D_H_
#if defined(__cplusplus)

#include <libeng/Graphic/Bounds/2D/Bounds2D.h>
#include <libeng/Log/Log.h>
#include <assert.h>

namespace eng {

//////
class Rectangle2D : public Bounds2D {

public:
    Rectangle2D(float left, float right, float top, float bottom) : Bounds2D(), mLeft(left), mRight(right),
        mTop(top), mBottom(bottom) {

        LOGV(LIBENG_LOG_BOUNDS, 0, LOG_FORMAT(" - l:%f; r:%f; t:%f; b:%f"), __PRETTY_FUNCTION__, __LINE__, left, right,
                top, bottom);
        assert(right > left);
        assert(top > bottom);
    }
    virtual ~Rectangle2D() { LOGV(LIBENG_LOG_BOUNDS, 0, LOG_FORMAT(), __PRETTY_FUNCTION__, __LINE__); }

    float mLeft;
    float mRight;
    float mTop;
    float mBottom;

    inline bool inRect2D(float x, float y) const {
        return ((mRight > x) && (mTop > y) && (mLeft < x) && (mBottom < y));
    }

    //
    inline void translate(float x, float y) {

#ifdef DEBUG
        LOGV(LIBENG_LOG_BOUNDS, (*mLog % 100), LOG_FORMAT(" - x:%f; y:%f"), __PRETTY_FUNCTION__, __LINE__, x, y);
#endif
        mLeft += x;
        mRight += x;
        mTop += y;
        mBottom += y;
    }
    inline void rotate(float angle, float tX, float tY) { assert(NULL); } // Use 'Polygone2D' instead
    inline void scale(float x, float y) {

#ifdef DEBUG
        LOGV(LIBENG_LOG_BOUNDS, (*mLog % 100), LOG_FORMAT(" - x:%f; y:%f"), __PRETTY_FUNCTION__, __LINE__, x, y);
        if (((mRight * x) < (mLeft * x)) || ((mTop * y) < (mBottom * y))) {

            LOGW(LOG_FORMAT(" - Scale must be > 0"), __PRETTY_FUNCTION__, __LINE__);
            assert(NULL);
        }
#endif
        mLeft *= x;
        mRight *= x;
        mTop *= y;
        mBottom *= y;
    }

    //////
    template<class T> inline bool overlap(const T* over) const { assert(NULL); return false; }
    template<class T> inline float overlapDist(const T* over) const { assert(NULL); return NO_BOUNDS_DIST; }
    template<class T> inline float overlapDistance(const T* over) const { assert(NULL); return NO_BOUNDS_DIST; }
    // WARNING: Only use template specializations! Defined in 'Overlap2D.h' header file
    // -> No way to put the definition of an inline member function into its implementation file (forward declaration needs)

};

class BorderLeft;
class BorderRight;
class BorderTop;
class BorderBottom;
class Circle2D;
class Polygone2D;
class Triangle2D;

//
template<> inline bool Rectangle2D::overlap<BorderLeft>(const BorderLeft* over) const;
template<> inline bool Rectangle2D::overlap<BorderRight>(const BorderRight* over) const;
template<> inline bool Rectangle2D::overlap<BorderTop>(const BorderTop* over) const;
template<> inline bool Rectangle2D::overlap<BorderBottom>(const BorderBottom* over) const;
template<> inline bool Rectangle2D::overlap<Rectangle2D>(const Rectangle2D* over) const;
template<> inline bool Rectangle2D::overlap<Circle2D>(const Circle2D* over) const;
template<> inline bool Rectangle2D::overlap<Polygone2D>(const Polygone2D* over) const;
template<> inline bool Rectangle2D::overlap<Triangle2D>(const Triangle2D* over) const;

//
template<> inline float Rectangle2D::overlapDist<BorderLeft>(const BorderLeft* over) const;
template<> inline float Rectangle2D::overlapDist<BorderRight>(const BorderRight* over) const;
template<> inline float Rectangle2D::overlapDist<BorderTop>(const BorderTop* over) const;
template<> inline float Rectangle2D::overlapDist<BorderBottom>(const BorderBottom* over) const;
template<> inline float Rectangle2D::overlapDist<Rectangle2D>(const Rectangle2D* over) const;
template<> inline float Rectangle2D::overlapDist<Circle2D>(const Circle2D* over) const;
template<> inline float Rectangle2D::overlapDist<Polygone2D>(const Polygone2D* over) const;
template<> inline float Rectangle2D::overlapDist<Triangle2D>(const Triangle2D* over) const;

//
template<> inline float Rectangle2D::overlapDistance<BorderLeft>(const BorderLeft* over) const;
template<> inline float Rectangle2D::overlapDistance<BorderRight>(const BorderRight* over) const;
template<> inline float Rectangle2D::overlapDistance<BorderTop>(const BorderTop* over) const;
template<> inline float Rectangle2D::overlapDistance<BorderBottom>(const BorderBottom* over) const;
template<> inline float Rectangle2D::overlapDistance<Rectangle2D>(const Rectangle2D* over) const;
template<> inline float Rectangle2D::overlapDistance<Circle2D>(const Circle2D* over) const;
template<> inline float Rectangle2D::overlapDistance<Polygone2D>(const Polygone2D* over) const;
template<> inline float Rectangle2D::overlapDistance<Triangle2D>(const Triangle2D* over) const;

} // namespace

// Cast(s)
#define rect2DVia(b)    static_cast<const eng::Rectangle2D*>(b)

#endif // __cplusplus
#endif // LIBENG_RECTANGLE2D_H_
