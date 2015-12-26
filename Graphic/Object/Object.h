#ifndef LIBENG_OBJECT_H_
#define LIBENG_OBJECT_H_
#if defined(__cplusplus)

#include <libeng/Game/Screen.h>
#include <libeng/Log/Log.h>

namespace eng {

static const float FULL_TEXCOORD_BUFFER[] = { 0.f, 0.f, 0.f, 1.f, 1.f, 1.f, 1.f, 0.f };

//////
class Object {

public:
    Object();
    virtual ~Object();

#ifdef DEBUG
    inline void initialize(const Screen* screen, const unsigned int* log) {

        LOGV(LIBENG_LOG_OBJECT, 0, LOG_FORMAT(" - s:%p; l:%p"), __PRETTY_FUNCTION__, __LINE__, screen, log);
        mLog = log;
#else
    inline void initialize(const Screen* screen) {
#endif
        mScreen = screen;
    }

    bool isFullCoordsBuffer() const; // Check if 'mTexCoords' == FULL_TEXCOORD_BUFFER
                                        // -> Needed coz FULL_TEXCOORD_BUFFER address is different in the other modules
    //////
    virtual void start(unsigned char texture) = 0;
    virtual void start(unsigned char red, unsigned char green, unsigned char blue) = 0;

    virtual void render(bool resetUniform) const = 0;

protected:
#ifdef DEBUG
    const unsigned int* mLog;
#endif
    const Screen* mScreen;

    float* mTexCoords; // Deleted in destructor (if not NULL and != FULL_TEXCOORD_BUFFER)
    float* mVertices;

};

} // namespace

#endif // __cplusplus
#endif // LIBENG_OBJECT_H_
