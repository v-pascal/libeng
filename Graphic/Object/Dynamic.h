#ifndef LIBENG_DYNAMIC_H_
#define LIBENG_DYNAMIC_H_
#if defined(__cplusplus)

#include <libeng/Game/Level.h>
#include <libeng/Graphic/Bounds/Bounds.h>
#include <libeng/Graphic/Object/Object.h>
#include <assert.h>

namespace eng {

//////
class Dynamic {

protected:
    Bounds** mBounds;
    unsigned char mBoundCount;
    // WARNING: Set bounds after centered vertices buffer & before any translation/rotation/scaling operation

    bool mUpdated; // TRUE: If any changes have been done ('mTransform' has changed); FALSE: No change
    // WARNING: This flag is associated with the 'resetUniform' parameter of the 'Object::render' method

public:
    Dynamic() : mBounds(NULL), mBoundCount(0), mUpdated(false) { }
    virtual ~Dynamic() {

        LOGV(LIBENG_LOG_DYNAMIC, 0, LOG_FORMAT(" - (b:%p; c:%d)"), __PRETTY_FUNCTION__, __LINE__, mBounds, mBoundCount);
        if (mBounds) {
            if (mBoundCount > 0) {
                do {
                    delete mBounds[--mBoundCount];
                } while (mBoundCount);
                delete [] mBounds;
            }
            else
                assert(NULL);
        }
    }

    inline const Bounds* getBounds(unsigned char index) const {

        assert(index < mBoundCount);
        return mBounds[index];
    }
    inline unsigned char getBoundCount() const { return mBoundCount; }

    //////
    virtual void update(const Game* game, const Level* level) = 0;

    virtual bool checkCollision(const Bounds* bounds, unsigned char boundsId) = 0;
    virtual bool checkCollision(const Object* object, unsigned char objectId) = 0;

};

} // namespace

#endif // __cplusplus
#endif // LIBENG_DYNAMIC_H_
