#ifndef LIBENG_OVERLAP2D_H_
#define LIBENG_OVERLAP2D_H_
#if defined(__cplusplus)

// To include when 'overlap*' methods of 2D bounds are needed
// -> Template method specializations are defined here

#include <libeng/Graphic/Bounds/BorderLeft.h>
#include <libeng/Graphic/Bounds/BorderRight.h>
#include <libeng/Graphic/Bounds/BorderTop.h>
#include <libeng/Graphic/Bounds/BorderBottom.h>

#include <libeng/Graphic/Bounds/2D/Rectangle2D.h>
#include <libeng/Graphic/Bounds/2D/Circle2D.h>
#include <libeng/Graphic/Bounds/2D/Polygone2D.h>
#include <libeng/Graphic/Bounds/2D/Triangle2D.h>

#ifdef __ANDROID__
#include <boost/math/tr1.hpp>
#else
#include <math.h>
#include <algorithm>
#endif
#include <assert.h>

namespace eng {

//////
template<> inline bool Rectangle2D::overlap<BorderLeft>(const BorderLeft* over) const { assert(NULL); return false; } // Not implemented yet!
template<> inline bool Rectangle2D::overlap<BorderRight>(const BorderRight* over) const { assert(NULL); return false; }
template<> inline bool Rectangle2D::overlap<BorderTop>(const BorderTop* over) const { assert(NULL); return false; }
template<> inline bool Rectangle2D::overlap<BorderBottom>(const BorderBottom* over) const { assert(NULL); return false; }
template<> inline bool Rectangle2D::overlap<Rectangle2D>(const Rectangle2D* over) const { assert(NULL); return false; }
template<> inline bool Rectangle2D::overlap<Circle2D>(const Circle2D* over) const { assert(NULL); return false; }
template<> inline bool Rectangle2D::overlap<Polygone2D>(const Polygone2D* over) const {

    assert(over);
    return ((inRect2D(over->mX1, over->mY1)) || (inRect2D(over->mX2, over->mY2)) ||
            (inRect2D(over->mX3, over->mY3)) || (inRect2D(over->mX4, over->mY4)) ||
            (over->inPoly2D(mLeft, mTop)) || (over->inPoly2D(mRight, mTop)) ||
            (over->inPoly2D(mRight, mBottom)) || (over->inPoly2D(mLeft, mBottom)));
}
template<> inline bool Rectangle2D::overlap<Triangle2D>(const Triangle2D* over) const { assert(NULL); return false; }

template<> inline bool Circle2D::overlap<BorderLeft>(const BorderLeft* over) const { assert(over); return ((X - mRadius) < over->mLeft); }
template<> inline bool Circle2D::overlap<BorderRight>(const BorderRight* over) const { assert(over); return ((X + mRadius) > over->mRight); }
template<> inline bool Circle2D::overlap<BorderTop>(const BorderTop* over) const { assert(over); return ((Y + mRadius) > over->mTop); }
template<> inline bool Circle2D::overlap<BorderBottom>(const BorderBottom* over) const { assert(over); return ((Y - mRadius) < over->mBottom); }
template<> inline bool Circle2D::overlap<Rectangle2D>(const Rectangle2D* over) const { assert(NULL); return false; }
template<> inline bool Circle2D::overlap<Circle2D>(const Circle2D* over) const{

    assert(over);
#ifdef __ANDROID__
    return (boost::math::tr1::hypotf(X - over->X, Y - over->Y) < (mRadius + over->mRadius));
#else
    return (hypotf(X - over->X, Y - over->Y) < (mRadius + over->mRadius));
#endif
}
template<> inline bool Circle2D::overlap<Polygone2D>(const Polygone2D* over) const { assert(NULL); return false; }
template<> inline bool Circle2D::overlap<Triangle2D>(const Triangle2D* over) const { assert(NULL); return false; }

template<> inline bool Polygone2D::overlap<BorderLeft>(const BorderLeft* over) const {

    assert(over);
#ifdef __ANDROID__
    using namespace boost::math;
    return (over->mLeft > tr1::fminf(tr1::fminf(mX1, mX2), tr1::fminf(mX3, mX4)));
#else
    return (over->mLeft > std::min<float>(std::min<float>(mX1, mX2), std::min<float>(mX3, mX4)));
#endif
}
template<> inline bool Polygone2D::overlap<BorderRight>(const BorderRight* over) const {

    assert(over);
#ifdef __ANDROID__
    using namespace boost::math;
    return (over->mRight < tr1::fmaxf(tr1::fmaxf(mX1, mX2), tr1::fmaxf(mX3, mX4)));
#else
    return (over->mRight < std::max<float>(std::max<float>(mX1, mX2), std::max<float>(mX3, mX4)));
#endif
}
template<> inline bool Polygone2D::overlap<BorderTop>(const BorderTop* over) const {

    assert(over);
#ifdef __ANDROID__
    using namespace boost::math;
    return (over->mTop < tr1::fmaxf(tr1::fmaxf(mY1, mY2), tr1::fmaxf(mY3, mY4)));
#else
    return (over->mTop < std::max<float>(std::max<float>(mY1, mY2), std::max<float>(mY3, mY4)));
#endif
}
template<> inline bool Polygone2D::overlap<BorderBottom>(const BorderBottom* over) const {

    assert(over);
#ifdef __ANDROID__
    using namespace boost::math;
    return (over->mBottom > tr1::fminf(tr1::fminf(mY1, mY2), tr1::fminf(mY3, mY4)));
#else
    return (over->mBottom > std::min<float>(std::min<float>(mY1, mY2), std::min<float>(mY3, mY4)));
#endif
}
template<> inline bool Polygone2D::overlap<Rectangle2D>(const Rectangle2D* over) const { assert(NULL); return false; }
template<> inline bool Polygone2D::overlap<Circle2D>(const Circle2D* over) const {

    assert(over);
#ifdef __ANDROID__
    bool res = ((boost::math::tr1::hypotf(mX1 - over->X, mY1 - over->Y) < over->mRadius) ||
                (boost::math::tr1::hypotf(mX2 - over->X, mY2 - over->Y) < over->mRadius) ||
                (boost::math::tr1::hypotf(mX3 - over->X, mY3 - over->Y) < over->mRadius) ||
                (boost::math::tr1::hypotf(mX4 - over->X, mY4 - over->Y) < over->mRadius));
#else
    bool res = ((hypotf(mX1 - over->X, mY1 - over->Y) < over->mRadius) ||
                (hypotf(mX2 - over->X, mY2 - over->Y) < over->mRadius) ||
                (hypotf(mX3 - over->X, mY3 - over->Y) < over->mRadius) ||
                (hypotf(mX4 - over->X, mY4 - over->Y) < over->mRadius));
#endif
    if (res)
        return true;

    float x, y;
    over->getPerimeterCoords(mX1, mY1, x, y);
    if (inPoly2D(x, y))
        return true;

    over->getPerimeterCoords(mX2, mY2, x, y);
    if (inPoly2D(x, y))
        return true;

    over->getPerimeterCoords(mX3, mY3, x, y);
    if (inPoly2D(x, y))
        return true;

    over->getPerimeterCoords(mX4, mY4, x, y);
    return inPoly2D(x, y);
}
template<> inline bool Polygone2D::overlap<Polygone2D>(const Polygone2D* over) const { assert(NULL); return false; }
template<> inline bool Polygone2D::overlap<Triangle2D>(const Triangle2D* over) const {

    assert(over);
    return ((over->inTrian2D(mX1, mY1)) || (over->inTrian2D(mX2, mY2)) || (over->inTrian2D(mX3, mY3)) ||
            (over->inTrian2D(mX4, mY4)) || (inPoly2D(over->mX1, over->mY1)) || (inPoly2D(over->mX2, over->mY2)) ||
            (inPoly2D(over->mX3, over->mY3)));
}

template<> inline bool Triangle2D::overlap<BorderLeft>(const BorderLeft* over) const {

    assert(over);
#ifdef __ANDROID__
    using namespace boost::math;
    return (over->mLeft > tr1::fminf(tr1::fminf(mX1, mX2), mX3));
#else
    return (over->mLeft > std::min<float>(std::min<float>(mX1, mX2), mX3));
#endif
}
template<> inline bool Triangle2D::overlap<BorderRight>(const BorderRight* over) const {

    assert(over);
#ifdef __ANDROID__
    using namespace boost::math;
    return (over->mRight < tr1::fmaxf(tr1::fmaxf(mX1, mX2), mX3));
#else
    return (over->mRight < std::max<float>(std::max<float>(mX1, mX2), mX3));
#endif
}
template<> inline bool Triangle2D::overlap<BorderTop>(const BorderTop* over) const {

    assert(over);
#ifdef __ANDROID__
    using namespace boost::math;
    return (over->mTop < tr1::fmaxf(tr1::fmaxf(mY1, mY2), mY3));
#else
    return (over->mTop < std::max<float>(std::max<float>(mY1, mY2), mY3));
#endif
}
template<> inline bool Triangle2D::overlap<BorderBottom>(const BorderBottom* over) const {

    assert(over);
#ifdef __ANDROID__
    using namespace boost::math;
    return (over->mBottom > tr1::fminf(tr1::fminf(mY1, mY2), mY3));
#else
    return (over->mBottom > std::min<float>(std::min<float>(mY1, mY2), mY3));
#endif
}
template<> inline bool Triangle2D::overlap<Rectangle2D>(const Rectangle2D* over) const { assert(NULL); return false; }
template<> inline bool Triangle2D::overlap<Circle2D>(const Circle2D* over) const {

    assert(over);
#ifdef __ANDROID__
    bool res = ((boost::math::tr1::hypotf(mX1 - over->X, mY1 - over->Y) < over->mRadius) ||
                (boost::math::tr1::hypotf(mX2 - over->X, mY2 - over->Y) < over->mRadius) ||
                (boost::math::tr1::hypotf(mX3 - over->X, mY3 - over->Y) < over->mRadius));
#else
    bool res = ((hypotf(mX1 - over->X, mY1 - over->Y) < over->mRadius) ||
                (hypotf(mX2 - over->X, mY2 - over->Y) < over->mRadius) ||
                (hypotf(mX3 - over->X, mY3 - over->Y) < over->mRadius));
#endif
    if (res)
        return true;

    float x, y;
    over->getPerimeterCoords(mX1, mY1, x, y);
    if (inTrian2D(x, y))
        return true;

    over->getPerimeterCoords(mX2, mY2, x, y);
    if (inTrian2D(x, y))
        return true;

    over->getPerimeterCoords(mX3, mY3, x, y);
    return inTrian2D(x, y);
}
template<> inline bool Triangle2D::overlap<Polygone2D>(const Polygone2D* over) const { assert(NULL); return false; }
template<> inline bool Triangle2D::overlap<Triangle2D>(const Triangle2D* over) const { assert(NULL); return false; }

//////
template<> inline float Rectangle2D::overlapDist<BorderLeft>(const BorderLeft* over) const { assert(NULL); return NO_BOUNDS_DIST; }
template<> inline float Rectangle2D::overlapDist<BorderRight>(const BorderRight* over) const { assert(NULL); return NO_BOUNDS_DIST; }
template<> inline float Rectangle2D::overlapDist<BorderTop>(const BorderTop* over) const { assert(NULL); return NO_BOUNDS_DIST; }
template<> inline float Rectangle2D::overlapDist<BorderBottom>(const BorderBottom* over) const { assert(NULL); return NO_BOUNDS_DIST; }
template<> inline float Rectangle2D::overlapDist<Rectangle2D>(const Rectangle2D* over) const { assert(NULL); return NO_BOUNDS_DIST; }
template<> inline float Rectangle2D::overlapDist<Circle2D>(const Circle2D* over) const { assert(NULL); return NO_BOUNDS_DIST; }
template<> inline float Rectangle2D::overlapDist<Polygone2D>(const Polygone2D* over) const { assert(NULL); return NO_BOUNDS_DIST; }
template<> inline float Rectangle2D::overlapDist<Triangle2D>(const Triangle2D* over) const { assert(NULL); return NO_BOUNDS_DIST; }

template<> inline float Circle2D::overlapDist<BorderLeft>(const BorderLeft* over) const {

    assert(over);
    float xLeftPos = X - mRadius;
    if (xLeftPos < over->mLeft)
        return (over->mLeft - xLeftPos);

    return NO_BOUNDS_DIST;
}
template<> inline float Circle2D::overlapDist<BorderRight>(const BorderRight* over) const {

    assert(over);
    float xRightPos = X + mRadius;
    if (xRightPos > over->mRight)
        return (xRightPos - over->mRight);

    return NO_BOUNDS_DIST;
}
template<> inline float Circle2D::overlapDist<BorderTop>(const BorderTop* over) const {

    assert(over);
    float yTopPos = Y + mRadius;
    if (yTopPos > over->mTop)
        return (yTopPos - over->mTop);

    return NO_BOUNDS_DIST;
}
template<> inline float Circle2D::overlapDist<BorderBottom>(const BorderBottom* over) const {

    assert(over);
    float yBottomPos = Y - mRadius;
    if (yBottomPos < over->mBottom)
        return (over->mBottom - yBottomPos);

    return NO_BOUNDS_DIST;
}
template<> inline float Circle2D::overlapDist<Rectangle2D>(const Rectangle2D* over) const { assert(NULL); return NO_BOUNDS_DIST; }
template<> inline float Circle2D::overlapDist<Circle2D>(const Circle2D* over) const {

    assert(over);
#ifdef __ANDROID__
    float dist = boost::math::tr1::hypotf(X - over->X, Y - over->Y);
#else
    float dist = hypotf(X - over->X, Y - over->Y);
#endif
    if (dist < (mRadius + over->mRadius))
        return dist;

    return NO_BOUNDS_DIST;
}
template<> inline float Circle2D::overlapDist<Polygone2D>(const Polygone2D* over) const { assert(NULL); return NO_BOUNDS_DIST; }
template<> inline float Circle2D::overlapDist<Triangle2D>(const Triangle2D* over) const { assert(NULL); return NO_BOUNDS_DIST; }

template<> inline float Polygone2D::overlapDist<BorderLeft>(const BorderLeft* over) const { assert(NULL); return NO_BOUNDS_DIST; }
template<> inline float Polygone2D::overlapDist<BorderRight>(const BorderRight* over) const { assert(NULL); return NO_BOUNDS_DIST; }
template<> inline float Polygone2D::overlapDist<BorderTop>(const BorderTop* over) const { assert(NULL); return NO_BOUNDS_DIST; }
template<> inline float Polygone2D::overlapDist<BorderBottom>(const BorderBottom* over) const { assert(NULL); return NO_BOUNDS_DIST; }
template<> inline float Polygone2D::overlapDist<Rectangle2D>(const Rectangle2D* over) const { assert(NULL); return NO_BOUNDS_DIST; }
template<> inline float Polygone2D::overlapDist<Circle2D>(const Circle2D* over) const { assert(NULL); return NO_BOUNDS_DIST; }
template<> inline float Polygone2D::overlapDist<Polygone2D>(const Polygone2D* over) const { assert(NULL); return NO_BOUNDS_DIST; }
template<> inline float Polygone2D::overlapDist<Triangle2D>(const Triangle2D* over) const { assert(NULL); return NO_BOUNDS_DIST; }

template<> inline float Triangle2D::overlapDist<BorderLeft>(const BorderLeft* over) const { assert(NULL); return NO_BOUNDS_DIST; }
template<> inline float Triangle2D::overlapDist<BorderRight>(const BorderRight* over) const { assert(NULL); return NO_BOUNDS_DIST; }
template<> inline float Triangle2D::overlapDist<BorderTop>(const BorderTop* over) const { assert(NULL); return NO_BOUNDS_DIST; }
template<> inline float Triangle2D::overlapDist<BorderBottom>(const BorderBottom* over) const { assert(NULL); return NO_BOUNDS_DIST; }
template<> inline float Triangle2D::overlapDist<Rectangle2D>(const Rectangle2D* over) const { assert(NULL); return NO_BOUNDS_DIST; }
template<> inline float Triangle2D::overlapDist<Circle2D>(const Circle2D* over) const { assert(NULL); return NO_BOUNDS_DIST; }
template<> inline float Triangle2D::overlapDist<Polygone2D>(const Polygone2D* over) const { assert(NULL); return NO_BOUNDS_DIST; }
template<> inline float Triangle2D::overlapDist<Triangle2D>(const Triangle2D* over) const { assert(NULL); return NO_BOUNDS_DIST; }

//////
template<> inline float Rectangle2D::overlapDistance<BorderLeft>(const BorderLeft* over) const { assert(NULL); return NO_BOUNDS_DIST; }
template<> inline float Rectangle2D::overlapDistance<BorderRight>(const BorderRight* over) const { assert(NULL); return NO_BOUNDS_DIST; }
template<> inline float Rectangle2D::overlapDistance<BorderTop>(const BorderTop* over) const { assert(NULL); return NO_BOUNDS_DIST; }
template<> inline float Rectangle2D::overlapDistance<BorderBottom>(const BorderBottom* over) const { assert(NULL); return NO_BOUNDS_DIST; }
template<> inline float Rectangle2D::overlapDistance<Rectangle2D>(const Rectangle2D* over) const { assert(NULL); return NO_BOUNDS_DIST; }
template<> inline float Rectangle2D::overlapDistance<Circle2D>(const Circle2D* over) const { assert(NULL); return NO_BOUNDS_DIST; }
template<> inline float Rectangle2D::overlapDistance<Polygone2D>(const Polygone2D* over) const { assert(NULL); return NO_BOUNDS_DIST; }
template<> inline float Rectangle2D::overlapDistance<Triangle2D>(const Triangle2D* over) const { assert(NULL); return NO_BOUNDS_DIST; }

template<> inline float Circle2D::overlapDistance<BorderLeft>(const BorderLeft* over) const { assert(NULL); return NO_BOUNDS_DIST; }
template<> inline float Circle2D::overlapDistance<BorderRight>(const BorderRight* over) const { assert(NULL); return NO_BOUNDS_DIST; }
template<> inline float Circle2D::overlapDistance<BorderTop>(const BorderTop* over) const { assert(NULL); return NO_BOUNDS_DIST; }
template<> inline float Circle2D::overlapDistance<BorderBottom>(const BorderBottom* over) const { assert(NULL); return NO_BOUNDS_DIST; }
template<> inline float Circle2D::overlapDistance<Rectangle2D>(const Rectangle2D* over) const { assert(NULL); return NO_BOUNDS_DIST; }
template<> inline float Circle2D::overlapDistance<Circle2D>(const Circle2D* over) const { assert(NULL); return NO_BOUNDS_DIST; }
template<> inline float Circle2D::overlapDistance<Polygone2D>(const Polygone2D* over) const { assert(NULL); return NO_BOUNDS_DIST; }
template<> inline float Circle2D::overlapDistance<Triangle2D>(const Triangle2D* over) const { assert(NULL); return NO_BOUNDS_DIST; }

template<> inline float Polygone2D::overlapDistance<BorderLeft>(const BorderLeft* over) const { assert(NULL); return NO_BOUNDS_DIST; }
template<> inline float Polygone2D::overlapDistance<BorderRight>(const BorderRight* over) const { assert(NULL); return NO_BOUNDS_DIST; }
template<> inline float Polygone2D::overlapDistance<BorderTop>(const BorderTop* over) const { assert(NULL); return NO_BOUNDS_DIST; }
template<> inline float Polygone2D::overlapDistance<BorderBottom>(const BorderBottom* over) const { assert(NULL); return NO_BOUNDS_DIST; }
template<> inline float Polygone2D::overlapDistance<Rectangle2D>(const Rectangle2D* over) const { assert(NULL); return NO_BOUNDS_DIST; }
template<> inline float Polygone2D::overlapDistance<Circle2D>(const Circle2D* over) const { assert(NULL); return NO_BOUNDS_DIST; }
template<> inline float Polygone2D::overlapDistance<Polygone2D>(const Polygone2D* over) const { assert(NULL); return NO_BOUNDS_DIST; }
template<> inline float Polygone2D::overlapDistance<Triangle2D>(const Triangle2D* over) const { assert(NULL); return NO_BOUNDS_DIST; }

template<> inline float Triangle2D::overlapDistance<BorderLeft>(const BorderLeft* over) const { assert(NULL); return NO_BOUNDS_DIST; }
template<> inline float Triangle2D::overlapDistance<BorderRight>(const BorderRight* over) const { assert(NULL); return NO_BOUNDS_DIST; }
template<> inline float Triangle2D::overlapDistance<BorderTop>(const BorderTop* over) const { assert(NULL); return NO_BOUNDS_DIST; }
template<> inline float Triangle2D::overlapDistance<BorderBottom>(const BorderBottom* over) const { assert(NULL); return NO_BOUNDS_DIST; }
template<> inline float Triangle2D::overlapDistance<Rectangle2D>(const Rectangle2D* over) const { assert(NULL); return NO_BOUNDS_DIST; }
template<> inline float Triangle2D::overlapDistance<Circle2D>(const Circle2D* over) const { assert(NULL); return NO_BOUNDS_DIST; }
template<> inline float Triangle2D::overlapDistance<Polygone2D>(const Polygone2D* over) const { assert(NULL); return NO_BOUNDS_DIST; }
template<> inline float Triangle2D::overlapDistance<Triangle2D>(const Triangle2D* over) const { assert(NULL); return NO_BOUNDS_DIST; }

} // namespace

#endif // __cplusplus
#endif // LIBENG_OVERLAP2D_H_
