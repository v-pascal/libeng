#ifndef LIBENG_BOUNDS2D_H_
#define LIBENG_BOUNDS2D_H_
#if defined(__cplusplus)

#include <libeng/Graphic/Bounds/Bounds.h>
#include <libeng/Global.h>
#include <libeng/Log/Log.h>

#define LIBENG_BOUND_SIGN(x1, y1, x2, y2, x3, y3)   ((float)(((x1 - x3) * (y2 - y3)) - ((x2 - x3) * (y1 - y3))))

namespace eng {

//////
class Bounds2D : public Bounds {

public:
    Bounds2D() : Bounds(), mOverlapX(NO_BOUNDS_DIST), mOverlapY(NO_BOUNDS_DIST) {

        LOGV(LIBENG_LOG_BOUNDS, 0, LOG_FORMAT(), __PRETTY_FUNCTION__, __LINE__);
    }
    virtual ~Bounds2D() { LOGV(LIBENG_LOG_BOUNDS, 0, LOG_FORMAT(), __PRETTY_FUNCTION__, __LINE__); }

    mutable float mOverlapX; // Horizontal overlapped distance
    mutable float mOverlapY; // Vertical overlapped distance
    // -> Variables that can be used for other reasons (e.g previous X or Y bound)

    //////
    virtual void translate(float x, float y) = 0;
    virtual void rotate(float angle, float tX, float tY) = 0;
    virtual void scale(float x, float y) = 0;

};

} // namespace

#endif // __cplusplus
#endif // LIBENG_BOUNDS2D_H_
