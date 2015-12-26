#ifndef LIBENG_BOUNDS_H_
#define LIBENG_BOUNDS_H_
#if defined(__cplusplus)

#include <stddef.h>

namespace eng {

static const float NO_BOUNDS_DIST = 0.f;

//////
class Bounds {

protected:
#ifdef DEBUG
    const unsigned int* mLog;
#endif
    typedef struct {
        float X;
        float Y;
    } Point;

public:
    Bounds() {
#ifdef DEBUG
        mLog = NULL;
#endif
    }
    virtual ~Bounds() { }

#ifdef DEBUG
    inline void setLog(const unsigned int* log) { mLog = log; }
#endif

};

} // namespace

#endif // __cplusplus
#endif // LIBENG_BOUNDS_H_
