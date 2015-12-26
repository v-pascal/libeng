#include "Camera.h"

#ifdef LIBENG_ENABLE_CAMERA

#include <libeng/Tools/Tools.h>
#include <libeng/Textures/Textures.h>
#include <libeng/Log/Log.h>
#include <assert.h>

#define CLIP(Color)             ((Color) < 0 ? 0 : (Color) > 262143 ? 262143 : Color)

namespace eng {

Camera* Camera::mThis = NULL;

inline void NV21toRGBA(int* rgb, int len, short width, short height, const unsigned char* nv21) {

    // len == width * height! :p ...calculate it one time in a static varaible declaration is faster !?!
    for (int y = 0, yPix = 0; y < static_cast<int>(height); ++y) {
        int uvPix = len + (y >> 1) * width, U = 0, V = 0;
        for (int x = 0; x < static_cast<int>(width); ++x, ++yPix) {

            // NV21 to YUV
            if (!(x & 1)) {

                U = nv21[uvPix++] - static_cast<int>(128);
                V = nv21[uvPix++] - static_cast<int>(128);
            }
            int Y = nv21[yPix] - static_cast<int>(16);

            // YUV to RGBA
            int Y1192 = 0;
            if (Y > 0) Y1192 = 1192 * Y;

            rgb[yPix] = 0xff000000 | // Alpha
                    ((CLIP(Y1192 + 1634 * V) << 6) & 0xff0000) | // Red
                    ((CLIP(Y1192 - 833 * V - 400 * U) >> 2) & 0xff00) | // Green
                    ((CLIP(Y1192 + 2066 * U) >> 10) & 0xff); // Blue
        }
    }
}

//////
Camera::Camera() : mCamBuffer(NULL), mStarted(false), mUpdated(false), mWidth(0), mHeight(0), mMemBuffer(NULL),
        mTextureIdx(TEXTURE_IDX_INVALID) {

    LOGV(LIBENG_LOG_CAMERA, 0, LOG_FORMAT(), __PRETTY_FUNCTION__, __LINE__);
    mTextures = Textures::getInstance();

#ifndef __ANDROID__
    mPaused = false;
#endif
#ifdef DEBUG
    mLog = 0;
#endif
#ifdef LIBENG_CAM_FREEZER
    mFrozen = false;

    mBuffered = true;
    mBuffer = NULL;
    mBufferLen = 0;
#endif
}
Camera::~Camera() {

    LOGV(LIBENG_LOG_CAMERA, 0, LOG_FORMAT(), __PRETTY_FUNCTION__, __LINE__);
    if (mCamBuffer)
        delete [] mCamBuffer;
    if (mMemBuffer)
        delete [] mMemBuffer;
#ifdef LIBENG_CAM_FREEZER
    if (mBuffer)
        delete [] mBuffer;
#endif
}

unsigned char Camera::start(short width, short height) {

#ifdef DEBUG
    static short prevWidth = 0;
    static short prevHeight = 0;
#endif
#ifdef __ANDROID__
    LOGV(LIBENG_LOG_CAMERA, 0, LOG_FORMAT(" - w:%d; h:%d (j:%p; a:%p; s:%s)"), __PRETTY_FUNCTION__, __LINE__, width,
            height, g_JavaVM, g_ActivityClass, (mStarted)? "true":"false");
    assert(g_JavaVM);
    assert(g_ActivityClass);
    assert(!mStarted);

    JNIEnv* env = getJavaEnv(LIBENG_LOG_CAMERA, __PRETTY_FUNCTION__, __LINE__);
    if (!env)
        return TEXTURE_IDX_INVALID;

    jmethodID mthd = env->GetStaticMethodID(g_ActivityClass, "startCamera", "(SS)Z");
    if (!mthd) {

        LOGE(LOG_FORMAT(" - Failed to get 'startCamera' java method"), __PRETTY_FUNCTION__, __LINE__);
        assert(NULL);
        return TEXTURE_IDX_INVALID;
    }

    mWidth = width;
    mHeight = height;

    if (!env->CallStaticBooleanMethod(g_ActivityClass, mthd, width, height)) {
#else
    LOGV(LIBENG_LOG_CAMERA, 0, LOG_FORMAT(" - w:%d; h:%d (a:%p; s:%s)"), __PRETTY_FUNCTION__, __LINE__, width, height,
         g_AppleOS, (mStarted)? "true":"false");
    assert(g_AppleOS);
    assert(!mStarted);

    mWidth = width;
    mHeight = height;

    if (![g_AppleOS startCamera:width andHeight:height]) {
#endif
        LOGE(LOG_FORMAT(" - Failed to start camera with format %dx%d"), __PRETTY_FUNCTION__, __LINE__, width, height);
        //assert(NULL); // Can occured when Create/Stop operation is done quickly
        return TEXTURE_IDX_INVALID;
    }
    mStarted = true;

    LOGI(LIBENG_LOG_CAMERA, 0, LOG_FORMAT(), __PRETTY_FUNCTION__, __LINE__);
    assert(mTextureIdx == TEXTURE_IDX_INVALID);

    mTextureIdx = mTextures->getIndex(TEXTURE_ID_CAM);
    if (mTextureIdx == TEXTURE_IDX_INVALID)
        mTextureIdx = mTextures->addTexCam(mWidth, mHeight);
#ifdef DEBUG
    else {

        LOGI(LIBENG_LOG_CAMERA, 0, LOG_FORMAT(), __PRETTY_FUNCTION__, __LINE__);
        assert((prevWidth == mWidth) && (prevHeight == mHeight));

        mTextures->addTexCam(mWidth, mHeight); // Needed to define texture buffer B4 being able to generate it

        prevWidth = mWidth;
        prevHeight = mHeight;
    }
#else
    else
        mTextures->addTexCam(mWidth, mHeight);
#endif
    mTextures->genTexture(mTextureIdx);
    return mTextureIdx;
}
#ifdef __ANDROID__
void Camera::pause(bool lockScreen) {

#ifdef DEBUG
    LOGV(LIBENG_LOG_CAMERA, (mLog % 100), LOG_FORMAT(" - l:%s (s:%s)"), __PRETTY_FUNCTION__, __LINE__,
            (lockScreen)? "true":"false", (mStarted)? "true":"false");
#endif
#else
void Camera::pause() {

    LOGV(LIBENG_LOG_CAMERA, 0, LOG_FORMAT(" - (s:%s)"), __PRETTY_FUNCTION__, __LINE__, (mStarted)? "true":"false");
#endif
    if (!mStarted)
        return;

#ifndef __ANDROID__
    assert(!mPaused);
    mPaused = true;
#endif

    mMutex.lock();
    if (mCamBuffer) {

        delete [] mCamBuffer;
        mCamBuffer = NULL;
    }
    mUpdated = false;
    mMutex.unlock();

#ifdef __ANDROID__
    if ((!lockScreen) && (mTextureIdx != TEXTURE_IDX_INVALID)) { // Pause/Resume operation

        mTextures->delTexture(mTextureIdx);
        mTextureIdx = TEXTURE_IDX_INVALID;
    }
#ifdef DEBUG
    else if (mTextureIdx == TEXTURE_IDX_INVALID) {
        LOGI(LIBENG_LOG_CAMERA, (mLog % 10), LOG_FORMAT(" - Camera already paused"), __PRETTY_FUNCTION__, __LINE__);
    }
#endif
#else // iOS
    assert(mTextureIdx != TEXTURE_IDX_INVALID);

    mTextures->delTexture(mTextureIdx);
    mTextureIdx = TEXTURE_IDX_INVALID;
#endif
}
void Camera::resume() {

    LOGV(LIBENG_LOG_CAMERA, 0, LOG_FORMAT(" - (s:%s; w:%d; h:%d)"), __PRETTY_FUNCTION__, __LINE__,
            (mStarted)? "true":"false", mWidth, mHeight);
    if (!mStarted)
        return;

    assert(mTextureIdx == TEXTURE_IDX_INVALID);
    assert(mTextures->getIndex(TEXTURE_ID_CAM) == TEXTURE_IDX_INVALID);

    mTextureIdx = mTextures->addTexCam(mWidth, mHeight);
    mTextures->genTexture(mTextureIdx);

#ifndef __ANDROID__
    assert(mPaused);
    mPaused = false;
#endif
}

#if defined(__ANDROID__) && defined(LIBENG_ENABLE_ADVERTISING)
void Camera::interstitialAdCamResume() {

    LOGV(LIBENG_LOG_CAMERA, 0, LOG_FORMAT(" - (s:%s; w:%d; h:%d)"), __PRETTY_FUNCTION__, __LINE__,
            (mStarted)? "true":"false", mWidth, mHeight);
    if (!mStarted)
        return;

    assert(mTextureIdx == TEXTURE_IDX_INVALID);
    assert(mTextures->getIndex(TEXTURE_ID_CAM) != TEXTURE_IDX_INVALID);

    mTextureIdx = mTextures->addTexCam(mWidth, mHeight);
    mTextures->genTexture(mTextureIdx);
}
#endif

bool Camera::stop() {

#ifdef __ANDROID__
    LOGV(LIBENG_LOG_CAMERA, 0, LOG_FORMAT(" - (j:%p; a:%p; s:%s)"), __PRETTY_FUNCTION__, __LINE__, g_JavaVM,
            g_ActivityClass, (mStarted)? "true":"false");
    assert(g_JavaVM);
    assert(g_ActivityClass);
    assert(mStarted);

    JNIEnv* env = getJavaEnv(LIBENG_LOG_CAMERA, __PRETTY_FUNCTION__, __LINE__);
    if (!env)
        return false;

    jmethodID mthd = env->GetStaticMethodID(g_ActivityClass, "stopCamera", "()Z");
    if (!mthd) {

        LOGE(LOG_FORMAT(" - Failed to get 'stopCamera' java method"), __PRETTY_FUNCTION__, __LINE__);
        assert(NULL);
        return false;
    }
    if (!env->CallStaticBooleanMethod(g_ActivityClass, mthd)) {
#else
    LOGV(LIBENG_LOG_CAMERA, 0, LOG_FORMAT(" - (a:%p; s:%s)"), __PRETTY_FUNCTION__, __LINE__, g_AppleOS,
            (mStarted)? "true":"false");
    assert(g_AppleOS);
    assert(mStarted);

    if (![g_AppleOS stopCamera]) {
#endif
        // Fail & display error only when not in pause operation coz at least it is impossible to know
        // if the camera has already been started or not
        LOGE(LOG_FORMAT(" - Failed to stop camera"), __PRETTY_FUNCTION__, __LINE__);
        assert(NULL);
        return false;
    }
    mStarted = false;

    mTextures->delTexture(mTextureIdx);
    mTextureIdx = TEXTURE_IDX_INVALID;
    return true;
}

#ifdef LIBENG_CAM_FREEZER
void Camera::freeze() {

    LOGV(LIBENG_LOG_CAMERA, 0, LOG_FORMAT(" - (f:%s; b:%s; w:%d; h:%d)"), __PRETTY_FUNCTION__, __LINE__,
            (mFrozen)? "true":"false", (mBuffered)? "true":"false", mWidth, mHeight);
    assert(!mFrozen);
    assert(mBuffered);

    if (mBuffer)
        delete [] mBuffer;

    mBufferLen = mWidth * mHeight; // RGBA buffer length
    mBuffer = new int[mBufferLen];

    mFrozen = true;
}
void Camera::unfreeze() {

    LOGV(LIBENG_LOG_CAMERA, 0, LOG_FORMAT(" - (f:%s; b:%s)"), __PRETTY_FUNCTION__, __LINE__, (mFrozen)? "true":"false",
            (mBuffered)? "true":"false");
    assert(mFrozen);
    assert(mBuffered);

    mBuffered = false;
    mFrozen = false;
}
#endif

void Camera::updateBuffer(const unsigned char* data) {

#ifdef DEBUG
    LOGV(LIBENG_LOG_CAMERA, (mLog % 100), LOG_FORMAT(" - d:%p (w:%d; h:%d)"), __PRETTY_FUNCTION__, __LINE__, data,
            mWidth, mHeight);
    //assert(mStarted); // This method can be called before this flag has been set
#endif
#ifndef __ANDROID__
    if (mPaused)
        return;
#endif

#ifdef LIBENG_CAM_FREEZER
    if (mFrozen)
        return; // Let's get next frame

    if (!mBuffered) {

        assert(mBuffer);
#ifdef __ANDROID__
        // Convert from NV21 (YUV420sp) to YUV and then to RGBA (ARGB)
        NV21toRGBA(mBuffer, mBufferLen, mWidth, mHeight, data);

#else
        // Convert from BGRA to RGBA (ARGB)
        unsigned char* buffer = reinterpret_cast<unsigned char*>(mBuffer);
        for (int y = 0, pix = 0; y < static_cast<int>(mHeight); ++y)
            for (int x = 0; x < static_cast<int>(mWidth); ++x, pix += 4) {

                buffer[pix] = data[pix + 2];
                buffer[pix + 1] = data[pix + 1];
                buffer[pix + 2] = data[pix];
                buffer[pix + 3] = data[pix + 3];
            }
#endif
        mBuffered = true;
        return;
    }
#endif
    static int camBufferLen = 0;

    mMutex.lock();
    if (!mCamBuffer) {

        camBufferLen = mWidth * mHeight;
        mCamBuffer = new int[camBufferLen];
    }

#ifdef DEBUG
    if (camBufferLen != (mWidth * mHeight)) {
        LOGE(LOG_FORMAT(" - Wrong camera buffer length (%d)"), __PRETTY_FUNCTION__, __LINE__, camBufferLen);
        assert(NULL);
    }
    if (mTextureIdx == TEXTURE_IDX_INVALID) {
        LOGW(LOG_FORMAT(" - No camera texture generated"), __PRETTY_FUNCTION__, __LINE__);
        //assert(NULL); // Can occured when a pause/resume operation is repeated quickly (pause/resume issue)
    }
#endif

#ifdef __ANDROID__
    // Convert from NV21 (YUV420sp) to YUV and then to RGBA (ARGB)
    NV21toRGBA(mCamBuffer, camBufferLen, mWidth, mHeight, data);

#else
    std::memcpy(reinterpret_cast<unsigned char*>(mCamBuffer), data, camBufferLen * 4);
    // Convert from BGRA to RGBA into the shader using 'Object2D::mBGRA' member
#endif
    mUpdated = true;
    mMutex.unlock();
}
bool Camera::updateTexture(bool memorize) {

#ifdef DEBUG
    ++mLog;
    LOGV(LIBENG_LOG_CAMERA, (mLog % 100), LOG_FORMAT(" - s:%s (w:%d; h:%d; u:%s)"), __PRETTY_FUNCTION__, __LINE__,
            (memorize)? "true":"false",mWidth, mHeight, (mUpdated)? "true":"false");
    //assert(mStarted); // This method can be called before this flag has been set
#endif
    mMutex.lock();
    if ((!mUpdated) || (!mCamBuffer) || (mTextureIdx == TEXTURE_IDX_INVALID)) {

        mMutex.unlock();
        return false;
    }
    mTextures->updateTexCam(mTextureIdx, mWidth, mHeight, reinterpret_cast<const unsigned char*>(mCamBuffer));
    mUpdated = false;
    if (memorize) {

        int len = mWidth * mHeight * 4;
        if (!mMemBuffer)
            mMemBuffer = new unsigned char[len];

        std::memcpy(mMemBuffer, mCamBuffer, len);
    }
    mMutex.unlock();
    return true;
}

} // namespace

#endif // LIBENG_ENABLE_CAMERA
