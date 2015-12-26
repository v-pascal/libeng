#ifndef LIBENG_CIRCLE2D_H_
#define LIBENG_CIRCLE2D_H_
#if defined(__cplusplus)

#include <libeng/Graphic/Bounds/2D/Bounds2D.h>
#include <libeng/Log/Log.h>

namespace eng {

//////
class Circle2D : public Bounds2D {

public:
    Circle2D(float x, float y, float radius) : Bounds2D(), X(x), Y(y), mRadius(radius) {

        LOGV(LIBENG_LOG_BOUNDS, 0, LOG_FORMAT(" - x:%f; y:%f; r:%f"), __PRETTY_FUNCTION__, __LINE__, x, y, radius);
    }
    virtual ~Circle2D() { LOGV(LIBENG_LOG_BOUNDS, 0, LOG_FORMAT(), __PRETTY_FUNCTION__, __LINE__); }

    float X;
    float Y;
    float mRadius;

    //
    inline void translate(float x, float y) {

#ifdef DEBUG
        LOGV(LIBENG_LOG_BOUNDS, (*mLog % 100), LOG_FORMAT(" - x:%f; y:%f"), __PRETTY_FUNCTION__, __LINE__, x, y);
#endif
        X += x;
        Y += y;
    }
    inline void rotate(float angle, float tX, float tY) { } // Nothing do (according that the rotation is at circle center)
    inline void scale(float x, float y) {

#ifdef DEBUG
        LOGV(LIBENG_LOG_BOUNDS, (*mLog % 100), LOG_FORMAT(" - x:%f; y:%f"), __PRETTY_FUNCTION__, __LINE__, x, y);
        if (x != y) {

            LOGW(LOG_FORMAT(" - Use 'Rectangle2D' or 'Polygone2D' instead"), __PRETTY_FUNCTION__, __LINE__);
            assert(NULL);
        }
#endif
        X *= x;
        Y *= y;

        mRadius *= x;   // -> Same as *= y
    }

    inline void getPerimeterCoords(float outX, float outY, float &onX, float &onY) const { // out -> Outside circle;
                                                                                           // on -> On circle perimeter
#ifdef DEBUG
        LOGV(LIBENG_LOG_BOUNDS, (*mLog % 100), LOG_FORMAT(" - ox:%f; oy:%f (x:%f; y:%f; r:%f)"), __PRETTY_FUNCTION__,
                __LINE__, outX, outY, X, Y, mRadius);
#endif
        if (outX > X) {

            float factor = (outY - Y) / (outX - X);
            float constant = outY - (factor * outX);

            float yDiff = (Y > outY)? ((factor * (outX - mRadius)) + constant) - outY:
                    outY - ((factor * (outX - mRadius)) + constant);

            onX = X + ((mRadius * mRadius) / std::sqrt((mRadius * mRadius) + (yDiff * yDiff)));
            onY = (factor * onX) + constant;
        }
        else if (outX < X) {

            float factor = (Y - outY) / (X - outX);
            float constant = outY - (factor * outX);

            float yDiff = (outY > Y)? ((factor * (X - mRadius)) + constant) - Y:
                    Y - ((factor * (X - mRadius)) + constant);

            onX = X - ((mRadius * mRadius) / std::sqrt((mRadius * mRadius) + (yDiff * yDiff)));
            onY = (factor * onX) + constant;
        }
        else {

            onX = outX;
            onY = (outY > Y)? outY + mRadius:outY - mRadius;
        }
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
class Polygone2D;
class Triangle2D;

//
template<> inline bool Circle2D::overlap<BorderLeft>(const BorderLeft* over) const;
template<> inline bool Circle2D::overlap<BorderRight>(const BorderRight* over) const;
template<> inline bool Circle2D::overlap<BorderTop>(const BorderTop* over) const;
template<> inline bool Circle2D::overlap<BorderBottom>(const BorderBottom* over) const;
template<> inline bool Circle2D::overlap<Rectangle2D>(const Rectangle2D* over) const;
template<> inline bool Circle2D::overlap<Circle2D>(const Circle2D* over) const;
template<> inline bool Circle2D::overlap<Polygone2D>(const Polygone2D* over) const;
template<> inline bool Circle2D::overlap<Triangle2D>(const Triangle2D* over) const;

//
template<> inline float Circle2D::overlapDist<BorderLeft>(const BorderLeft* over) const;
template<> inline float Circle2D::overlapDist<BorderRight>(const BorderRight* over) const;
template<> inline float Circle2D::overlapDist<BorderTop>(const BorderTop* over) const;
template<> inline float Circle2D::overlapDist<BorderBottom>(const BorderBottom* over) const;
template<> inline float Circle2D::overlapDist<Rectangle2D>(const Rectangle2D* over) const;
template<> inline float Circle2D::overlapDist<Circle2D>(const Circle2D* over) const;
template<> inline float Circle2D::overlapDist<Polygone2D>(const Polygone2D* over) const;
template<> inline float Circle2D::overlapDist<Triangle2D>(const Triangle2D* over) const;

//
template<> inline float Circle2D::overlapDistance<BorderLeft>(const BorderLeft* over) const;
template<> inline float Circle2D::overlapDistance<BorderRight>(const BorderRight* over) const;
template<> inline float Circle2D::overlapDistance<BorderTop>(const BorderTop* over) const;
template<> inline float Circle2D::overlapDistance<BorderBottom>(const BorderBottom* over) const;
template<> inline float Circle2D::overlapDistance<Rectangle2D>(const Rectangle2D* over) const;
template<> inline float Circle2D::overlapDistance<Circle2D>(const Circle2D* over) const;
template<> inline float Circle2D::overlapDistance<Polygone2D>(const Polygone2D* over) const;
template<> inline float Circle2D::overlapDistance<Triangle2D>(const Triangle2D* over) const;

} // namespace

// Cast(s)
#define circ2DVia(b)    static_cast<const eng::Circle2D*>(b)

#endif // __cplusplus
#endif // LIBENG_CIRCLE2D_H_
