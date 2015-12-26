#ifndef LIBENG_TRIANGLE2D_H_
#define LIBENG_TRIANGLE2D_H_
#if defined(__cplusplus)

#include <libeng/Graphic/Bounds/2D/Bounds2D.h>
#include <libeng/Log/Log.h>

namespace eng {

//////
class Triangle2D : public Bounds2D {

public:
    Triangle2D(float x1, float y1, float x2, float y2, float x3, float y3) : Bounds2D(), mX1(x1), mY1(y1), mX2(x2),
        mY2(y2), mX3(x3), mY3(y3) {

        LOGV(LIBENG_LOG_BOUNDS, 0, LOG_FORMAT(" - x1:%f; y1:%f; x2:%f; y2:%f; x3:%f; y3:%f"), __PRETTY_FUNCTION__,
                __LINE__, x1, y1, x2, y2, x3, y3);
    }
    virtual ~Triangle2D() { LOGV(LIBENG_LOG_BOUNDS, 0, LOG_FORMAT(), __PRETTY_FUNCTION__, __LINE__); }

    float mX1;
    float mY1;
    float mX2;
    float mY2;
    float mX3;
    float mY3;

    //
    inline void translate(float x, float y) {

#ifdef DEBUG
        LOGV(LIBENG_LOG_BOUNDS, (*mLog % 100), LOG_FORMAT(" - x:%f; y:%f"), __PRETTY_FUNCTION__, __LINE__, x, y);
#endif
        mX1 += x;
        mX2 += x;
        mX3 += x;

        mY1 += y;
        mY2 += y;
        mY3 += y;
    }
    inline void rotate(float angle, float tX, float tY) {

#ifdef DEBUG
        LOGV(LIBENG_LOG_BOUNDS, (*mLog % 100), LOG_FORMAT(" - a:%f; x:%f; y:%f"), __PRETTY_FUNCTION__, __LINE__, angle, tX, tY);
#endif
        float sinA = std::sin(angle);
        float cosA = std::cos(angle);

        float transX = (tX * (1.f - cosA)) - (tY * -sinA);
        float transY = (tX * -sinA) + (tY * (1.f - cosA));

        float x = mX1;
        mX1 = (mX1 * cosA) - (mY1 * sinA) + transX;
        mY1 = (x * sinA) + (mY1 * cosA) + transY;

        x = mX2;
        mX2 = (mX2 * cosA) - (mY2 * sinA) + transX;
        mY2 = (x * sinA) + (mY2 * cosA) + transY;

        x = mX3;
        mX3 = (mX3 * cosA) - (mY3 * sinA) + transX;
        mY3 = (x * sinA) + (mY3 * cosA) + transY;
    }
    inline void scale(float x, float y) {

#ifdef DEBUG
        LOGV(LIBENG_LOG_BOUNDS, (*mLog % 100), LOG_FORMAT(" - x:%f; y:%f"), __PRETTY_FUNCTION__, __LINE__, x, y);
        assert(x > 0.f);
        assert(y > 0.f);
#endif
        mX1 *= x;
        mX2 *= x;
        mX3 *= x;

        mY1 *= y;
        mY2 *= y;
        mY3 *= y;
    }

    inline bool inTrian2D(float x, float y) const {

        bool a = LIBENG_BOUND_SIGN(x, y, mX1, mY1, mX2, mY2) < 0.f;
        bool b = LIBENG_BOUND_SIGN(x, y, mX2, mY2, mX3, mY3) < 0.f;
        bool c = LIBENG_BOUND_SIGN(x, y, mX3, mY3, mX1, mY1) < 0.f;

        return ((a == b) && (b == c));
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
class Rectangle2D;
class Circle2D;
class Polygone2D;

//
template<> inline bool Triangle2D::overlap<BorderLeft>(const BorderLeft* over) const;
template<> inline bool Triangle2D::overlap<BorderRight>(const BorderRight* over) const;
template<> inline bool Triangle2D::overlap<BorderTop>(const BorderTop* over) const;
template<> inline bool Triangle2D::overlap<BorderBottom>(const BorderBottom* over) const;
template<> inline bool Triangle2D::overlap<Rectangle2D>(const Rectangle2D* over) const;
template<> inline bool Triangle2D::overlap<Circle2D>(const Circle2D* over) const;
template<> inline bool Triangle2D::overlap<Polygone2D>(const Polygone2D* over) const;
template<> inline bool Triangle2D::overlap<Triangle2D>(const Triangle2D* over) const;

//
template<> inline float Triangle2D::overlapDist<BorderLeft>(const BorderLeft* over) const;
template<> inline float Triangle2D::overlapDist<BorderRight>(const BorderRight* over) const;
template<> inline float Triangle2D::overlapDist<BorderTop>(const BorderTop* over) const;
template<> inline float Triangle2D::overlapDist<BorderBottom>(const BorderBottom* over) const;
template<> inline float Triangle2D::overlapDist<Rectangle2D>(const Rectangle2D* over) const;
template<> inline float Triangle2D::overlapDist<Circle2D>(const Circle2D* over) const;
template<> inline float Triangle2D::overlapDist<Polygone2D>(const Polygone2D* over) const;
template<> inline float Triangle2D::overlapDist<Triangle2D>(const Triangle2D* over) const;

//
template<> inline float Triangle2D::overlapDistance<BorderLeft>(const BorderLeft* over) const;
template<> inline float Triangle2D::overlapDistance<BorderRight>(const BorderRight* over) const;
template<> inline float Triangle2D::overlapDistance<BorderTop>(const BorderTop* over) const;
template<> inline float Triangle2D::overlapDistance<BorderBottom>(const BorderBottom* over) const;
template<> inline float Triangle2D::overlapDistance<Rectangle2D>(const Rectangle2D* over) const;
template<> inline float Triangle2D::overlapDistance<Circle2D>(const Circle2D* over) const;
template<> inline float Triangle2D::overlapDistance<Polygone2D>(const Polygone2D* over) const;
template<> inline float Triangle2D::overlapDistance<Triangle2D>(const Triangle2D* over) const;

} // namespace

// Cast(s)
#define trian2DVia(b)   static_cast<const eng::Triangle2D*>(b)

#endif // __cplusplus
#endif // LIBENG_TRIANGLE2D_H_
