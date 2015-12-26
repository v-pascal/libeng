#ifndef LIBENG_BORDERRIGHT_H_
#define LIBENG_BORDERRIGHT_H_
#if defined(__cplusplus)

#include <libeng/Graphic/Bounds/Bounds.h>
#include <libeng/Log/Log.h>

namespace eng {

//////
class BorderRight : public Bounds {

public:
    BorderRight(float right) : Bounds(), mRight(right) {

        LOGV(LIBENG_LOG_BOUNDS, 0, LOG_FORMAT(" - r:%f"), __PRETTY_FUNCTION__, __LINE__, right);
    }
    virtual ~BorderRight() { LOGV(LIBENG_LOG_BOUNDS, 0, LOG_FORMAT(), __PRETTY_FUNCTION__, __LINE__); }

    float mRight;

    //////
    template<class T> inline bool overlap(const T* over) const { assert(NULL); return false; }
    template<class T> inline float overlapDist(const T* over) const { assert(NULL); return NO_BOUNDS_DIST; }
    template<class T> inline float overlapDistance(const T* over) const { assert(NULL); return NO_BOUNDS_DIST; }
    // WARNING: Only use template specializations! Defined in 'Overlap.h' header file
    // -> No way to put the definition of an inline member function into its implementation file (forward declaration needs)

};

class BorderLeft;
class Rectangle2D;
class Circle2D;
class Polygone2D;

//
template<> inline bool BorderRight::overlap<BorderLeft>(const BorderLeft* over) const;
template<> inline bool BorderRight::overlap<Rectangle2D>(const Rectangle2D* over) const;
template<> inline bool BorderRight::overlap<Circle2D>(const Circle2D* over) const;
template<> inline bool BorderRight::overlap<Polygone2D>(const Polygone2D* over) const;

//
template<> inline float BorderRight::overlapDist<BorderLeft>(const BorderLeft* over) const;
template<> inline float BorderRight::overlapDist<Rectangle2D>(const Rectangle2D* over) const;
template<> inline float BorderRight::overlapDist<Circle2D>(const Circle2D* over) const;
template<> inline float BorderRight::overlapDist<Polygone2D>(const Polygone2D* over) const;

//
template<> inline float BorderRight::overlapDistance<BorderLeft>(const BorderLeft* over) const;
template<> inline float BorderRight::overlapDistance<Rectangle2D>(const Rectangle2D* over) const;
template<> inline float BorderRight::overlapDistance<Circle2D>(const Circle2D* over) const;
template<> inline float BorderRight::overlapDistance<Polygone2D>(const Polygone2D* over) const;

} // namespace

// Cast(s)
#define bRightVia(b)    static_cast<const eng::BorderRight*>(b)

#endif // __cplusplus
#endif // LIBENG_BORDERRIGHT_H_
