#ifndef LIBENG_OVERLAP_H_
#define LIBENG_OVERLAP_H_
#if defined(__cplusplus)

#ifdef __ANDROID__
#include <boost/math/tr1.hpp>
#else
#include <algorithm>
#endif

// To include when 'overlap*' methods of border bounds are needed
// -> Template method specializations are defined here

#include <libeng/Graphic/Bounds/BorderLeft.h>
#include <libeng/Graphic/Bounds/BorderRight.h>
#include <libeng/Graphic/Bounds/BorderTop.h>
#include <libeng/Graphic/Bounds/BorderBottom.h>

#include <libeng/Graphic/Bounds/2D/Rectangle2D.h>
#include <libeng/Graphic/Bounds/2D/Circle2D.h>
#include <libeng/Graphic/Bounds/2D/Polygone2D.h>

namespace eng {

//////
template<> inline bool BorderLeft::overlap<BorderRight>(const BorderRight* over) const { assert(over); return (mLeft > over->mRight); }
template<> inline bool BorderLeft::overlap<Rectangle2D>(const Rectangle2D* over) const { assert(over); return (mLeft > over->mLeft); }
template<> inline bool BorderLeft::overlap<Circle2D>(const Circle2D* over) const { assert(over); return (mLeft > (over->X - over->mRadius)); }
template<> inline bool BorderLeft::overlap<Polygone2D>(const Polygone2D* over) const {

    assert(over);
#ifdef __ANDROID__
    using namespace boost::math;
    return (mLeft > tr1::fminf(tr1::fminf(over->mX1, over->mX2), tr1::fminf(over->mX3, over->mX4)));
#else
    return (mLeft > std::min<float>(std::min<float>(over->mX1, over->mX2), std::min<float>(over->mX3, over->mX4)));
#endif
}

template<> inline bool BorderRight::overlap<BorderLeft>(const BorderLeft* over) const { assert(over); return (mRight < over->mLeft); }
template<> inline bool BorderRight::overlap<Rectangle2D>(const Rectangle2D* over) const { assert(over); return (mRight < over->mRight); }
template<> inline bool BorderRight::overlap<Circle2D>(const Circle2D* over) const { assert(over); return (mRight < (over->X + over->mRadius)); }
template<> inline bool BorderRight::overlap<Polygone2D>(const Polygone2D* over) const {

    assert(over);
#ifdef __ANDROID__
    using namespace boost::math;
    return (mRight < tr1::fmaxf(tr1::fmaxf(over->mX1, over->mX2), tr1::fmaxf(over->mX3, over->mX4)));
#else
    return (mRight < std::max<float>(std::max<float>(over->mX1, over->mX2), std::max<float>(over->mX3, over->mX4)));
#endif
}

template<> inline bool BorderTop::overlap<BorderBottom>(const BorderBottom* over) const { assert(over); return (mTop < over->mBottom); }
template<> inline bool BorderTop::overlap<Rectangle2D>(const Rectangle2D* over) const { assert(NULL); return false; } // Not implemented yet!
template<> inline bool BorderTop::overlap<Circle2D>(const Circle2D* over) const { assert(over); return (mTop < (over->Y + over->mRadius)); }
template<> inline bool BorderTop::overlap<Polygone2D>(const Polygone2D* over) const {

    assert(over);
#ifdef __ANDROID__
    using namespace boost::math;
    return (mTop < tr1::fmaxf(tr1::fmaxf(over->mY1, over->mY2), tr1::fmaxf(over->mY3, over->mY4)));
#else
    return (mTop < std::max<float>(std::max<float>(over->mY1, over->mY2), std::max<float>(over->mY3, over->mY4)));
#endif
}

template<> inline bool BorderBottom::overlap<BorderTop>(const BorderTop* over) const { assert(over); return (mBottom > over->mTop); }
template<> inline bool BorderBottom::overlap<Rectangle2D>(const Rectangle2D* over) const { assert(NULL); return false; }
template<> inline bool BorderBottom::overlap<Circle2D>(const Circle2D* over) const { assert(over); return (mBottom > (over->Y - over->mRadius)); }
template<> inline bool BorderBottom::overlap<Polygone2D>(const Polygone2D* over) const {

    assert(over);
#ifdef __ANDROID__
    using namespace boost::math;
    return (mBottom > tr1::fminf(tr1::fminf(over->mY1, over->mY2), tr1::fminf(over->mY3, over->mY4)));
#else
    return (mBottom > std::min<float>(std::min<float>(over->mY1, over->mY2), std::min<float>(over->mY3, over->mY4)));
#endif
}

//////
template<> inline float BorderLeft::overlapDist<BorderRight>(const BorderRight* over) const { assert(NULL); return NO_BOUNDS_DIST; }
template<> inline float BorderLeft::overlapDist<Rectangle2D>(const Rectangle2D* over) const { assert(NULL); return NO_BOUNDS_DIST; }
template<> inline float BorderLeft::overlapDist<Circle2D>(const Circle2D* over) const { assert(NULL); return NO_BOUNDS_DIST; }
template<> inline float BorderLeft::overlapDist<Polygone2D>(const Polygone2D* over) const { assert(NULL); return NO_BOUNDS_DIST; }

template<> inline float BorderRight::overlapDist<BorderLeft>(const BorderLeft* over) const { assert(NULL); return NO_BOUNDS_DIST; }
template<> inline float BorderRight::overlapDist<Rectangle2D>(const Rectangle2D* over) const { assert(NULL); return NO_BOUNDS_DIST; }
template<> inline float BorderRight::overlapDist<Circle2D>(const Circle2D* over) const { assert(NULL); return NO_BOUNDS_DIST; }
template<> inline float BorderRight::overlapDist<Polygone2D>(const Polygone2D* over) const { assert(NULL); return NO_BOUNDS_DIST; }

template<> inline float BorderTop::overlapDist<BorderBottom>(const BorderBottom* over) const { assert(NULL); return NO_BOUNDS_DIST; }
template<> inline float BorderTop::overlapDist<Rectangle2D>(const Rectangle2D* over) const { assert(NULL); return NO_BOUNDS_DIST; }
template<> inline float BorderTop::overlapDist<Circle2D>(const Circle2D* over) const { assert(over); assert(NULL); return NO_BOUNDS_DIST; }
template<> inline float BorderTop::overlapDist<Polygone2D>(const Polygone2D* over) const { assert(over); assert(NULL); return NO_BOUNDS_DIST; }

template<> inline float BorderBottom::overlapDist<BorderTop>(const BorderTop* over) const { assert(NULL); return NO_BOUNDS_DIST; }
template<> inline float BorderBottom::overlapDist<Rectangle2D>(const Rectangle2D* over) const { assert(NULL); return NO_BOUNDS_DIST; }
template<> inline float BorderBottom::overlapDist<Circle2D>(const Circle2D* over) const { assert(NULL); return NO_BOUNDS_DIST; }
template<> inline float BorderBottom::overlapDist<Polygone2D>(const Polygone2D* over) const { assert(NULL); return NO_BOUNDS_DIST; }

//////
template<> inline float BorderLeft::overlapDistance<BorderRight>(const BorderRight* over) const { assert(NULL); return NO_BOUNDS_DIST; }
template<> inline float BorderLeft::overlapDistance<Rectangle2D>(const Rectangle2D* over) const { assert(NULL); return NO_BOUNDS_DIST; }
template<> inline float BorderLeft::overlapDistance<Circle2D>(const Circle2D* over) const { assert(NULL); return NO_BOUNDS_DIST; }
template<> inline float BorderLeft::overlapDistance<Polygone2D>(const Polygone2D* over) const { assert(NULL); return NO_BOUNDS_DIST; }

template<> inline float BorderRight::overlapDistance<BorderLeft>(const BorderLeft* over) const { assert(NULL); return NO_BOUNDS_DIST; }
template<> inline float BorderRight::overlapDistance<Rectangle2D>(const Rectangle2D* over) const { assert(NULL); return NO_BOUNDS_DIST; }
template<> inline float BorderRight::overlapDistance<Circle2D>(const Circle2D* over) const { assert(NULL); return NO_BOUNDS_DIST; }
template<> inline float BorderRight::overlapDistance<Polygone2D>(const Polygone2D* over) const { assert(NULL); return NO_BOUNDS_DIST; }

template<> inline float BorderTop::overlapDistance<BorderBottom>(const BorderBottom* over) const { assert(NULL); return NO_BOUNDS_DIST; }
template<> inline float BorderTop::overlapDistance<Rectangle2D>(const Rectangle2D* over) const { assert(NULL); return NO_BOUNDS_DIST; }
template<> inline float BorderTop::overlapDistance<Circle2D>(const Circle2D* over) const { assert(over); assert(NULL); return NO_BOUNDS_DIST; }
template<> inline float BorderTop::overlapDistance<Polygone2D>(const Polygone2D* over) const { assert(over); assert(NULL); return NO_BOUNDS_DIST; }

template<> inline float BorderBottom::overlapDistance<BorderTop>(const BorderTop* over) const { assert(NULL); return NO_BOUNDS_DIST; }
template<> inline float BorderBottom::overlapDistance<Rectangle2D>(const Rectangle2D* over) const { assert(NULL); return NO_BOUNDS_DIST; }
template<> inline float BorderBottom::overlapDistance<Circle2D>(const Circle2D* over) const { assert(NULL); return NO_BOUNDS_DIST; }
template<> inline float BorderBottom::overlapDistance<Polygone2D>(const Polygone2D* over) const { assert(NULL); return NO_BOUNDS_DIST; }

} // namespace

#endif // __cplusplus
#endif // LIBENG_OVERLAP_H_
