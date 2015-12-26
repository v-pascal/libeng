#ifndef LIBENG_CAMERA_H_
#define LIBENG_CAMERA_H_
#if defined(__cplusplus)

#include <libeng/Global.h>
#ifdef LIBENG_ENABLE_CAMERA

#include <libeng/Textures/Textures.h>
#include <boost/thread.hpp>

namespace eng {

class PlatformOS;
class Game;

//////
class Camera {

    friend class PlatformOS;
    friend class Game;

private:
    Camera();
    virtual ~Camera();

    static Camera* mThis;

    int* mCamBuffer; // Camera RGBA buffer
    unsigned char* mMemBuffer;
    boost::mutex mMutex;

    bool mStarted;
#ifndef __ANDROID__
    bool mPaused; // iOS only coz on Android the stop camera is managed B4 'pause' method call
#endif
    bool mUpdated;
#ifdef DEBUG
    unsigned int mLog;
#endif

    short mWidth;
    short mHeight;

    Textures* mTextures;
    unsigned char mTextureIdx;

#ifdef LIBENG_CAM_FREEZER
    bool mFrozen;

    bool mBuffered;
    int* mBuffer;
    int mBufferLen;
#endif

public:
    static Camera* getInstance() {
        if (!mThis)
            mThis = new Camera;
        return mThis;
    }
    static void freeInstance() {
        if (mThis) {
            delete mThis;
            mThis = NULL;
        }
    }

    inline bool isStarted() const { return mStarted; }
    inline unsigned char getCamTexIdx() const { return mTextureIdx; }
    inline const unsigned char* getCamBuffer() const { return mMemBuffer; } // getMemBuffer

    inline short getWidth() const { return mWidth; }
    inline short getHeight() const { return mHeight; }

    //////
    unsigned char start(short width, short height); // Return the camera texture index
    // WARNING:
    // * This method will cause to add a texture into 'Textures' class (camera texture)
    // * Moreover if camera is started when pause/resume operation is done camera will be restarted automatically and
    //   a texture will be generated (camera texture index == 0). So take care of the impact on textures indexes order
    // * Check systematically the camera texture index (can be TEXTURE_IDX_INVALID if Create/Stop is done quickly)

#ifdef __ANDROID__
    void pause(bool lockScreen);
    // WARNING: Do not use this method (reserved)
#else
private:
    void pause();
#endif
private:
    void resume();
    // -> Pause/Resume operation reserved

public:
    bool stop();
    // WARNING: After having stop the camera do not use camera texture anymore (texture has been deleted)

#ifdef LIBENG_CAM_FREEZER
    void freeze();
    void unfreeze();

    inline bool isBuffered() const { return mBuffered; }

    inline int getBufferLen() const { return (mBufferLen * 4); }
    inline const char* getBuffer() const { return reinterpret_cast<char*>(mBuffer); }
#endif

#if defined(__ANDROID__) && defined(LIBENG_ENABLE_ADVERTISING)

    void interstitialAdCamResume();
    // BUG: When displaying the AdMob Interstitial advertising/the Google+ connection window a lock/unlock screen operation
    //      is done except that no change on surface occurs (no 'EngSurface::surfaceChanged' java method call)
    // -> This method is needed to resume the camera manually (in 'game::update' method)
#endif

    void updateBuffer(const unsigned char* data);
    bool updateTexture(bool memorize = false); // To be called in 'Level::update' method (when used)

};

} // namespace

#endif // LIBENG_ENABLE_CAMERA
#endif // __cplusplus
#endif // LIBENG_CAMERA_H_
