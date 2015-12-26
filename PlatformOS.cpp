#ifdef __ANDROID__
#include <libeng/Global.h>  // Force to be the first file to be included
#endif
#include <libeng/PlatformOS.h>

#include <libeng/Log/Log.h>
#include <libeng/Render/Render.h>
#include <libeng/Render/2D/Render2D.h>
#include <libeng/Player/Player.h>
#include <libeng/Storage/Storage.h>
#include <libeng/Features/Camera/Camera.h>
#include <libeng/Features/Mic/Mic.h>
#include <libeng/Features/Internet/Internet.h>
#include <libeng/Features/Bluetooth/Bluetooth.h>
#include <libeng/Advertising/Advertising.h>
#include <libeng/Social/Social.h>
#include <libeng/Tools/Tools.h>

#include <libeng/Inputs/Touch/Touch.h>
#include <libeng/Inputs/Accel/Accel.h>

#ifdef __ANDROID__
#include <android/native_window.h>
#endif

namespace eng {

//////
const char* g_LogTag = NULL;
const char* g_GameFile = NULL;

unsigned char Inputs::InputUse = 0;

#ifdef __ANDROID__
JavaVM* g_JavaVM = NULL;
const char* g_JavaProject = NULL;
jclass g_ActivityClass = NULL;
jobject g_ActivityObject = NULL;

jclass g_FacebookClass = NULL;
jclass g_TwitterClass = NULL;
jclass g_GoogleClass = NULL;

#ifdef LIBENG_ENABLE_BLUETOOTH
jclass g_BluetoothClass = NULL;
#endif

#else // !__ANDROID__
id<LibengOS> g_AppleOS = nil;

#endif
template<class T> RecuChainFact<T>* RecuChainFact<T>::mThis = NULL;

long initTime = 0;
clock_t initClock = 0;

//////
class PlatformOS {

private:
    bool mDimension;

    Render* mRender;
    Player* mPlayer;

    //
    Inputs* mInputs;
    float mAccelRange;

#ifdef LIBENG_ENABLE_MIC
    Mic* mMic;
#endif
#ifdef LIBENG_ENABLE_CAMERA
    Camera* mCamera;
#endif
    static PlatformOS* mThis;

private:
    PlatformOS(const PlatformData* data);
    virtual ~PlatformOS();

public:
    static PlatformOS* getInstance(const PlatformData* data) {
        if (!mThis)
            mThis = new PlatformOS(data);
        return mThis;
    }
    static void freeInstance() {
        if (mThis) {
            delete mThis;
            mThis = NULL;
        }
    }

#ifdef LIBENG_ENABLE_SOCIAL
    Session* getNetwork(Network::ID id) {

#ifdef DEBUG
        LOGV(LIBENG_LOG_PLATFORM, 0, LOG_FORMAT(" - i:%d (s:%p)"), __PRETTY_FUNCTION__, __LINE__,
                static_cast<unsigned char>(id), Social::mThis);

        assert(Social::mThis);
        Session* session = Social::mThis->getSession(id);
        if (!session) {

            LOGE(LOG_FORMAT(" - Session %d not found"), __PRETTY_FUNCTION__, __LINE__, id);
            assert(NULL);
        }
        return session;
#else
        return Social::getInstance(false)->getSession(id);
#endif
    }
#endif // LIBENG_ENABLE_SOCIAL

    //////
#ifdef __ANDROID__
    void create() {

        LOGV(LIBENG_LOG_PLATFORM, 0, LOG_FORMAT(), __PRETTY_FUNCTION__, __LINE__);
        mRender->create();
    }
    void start(jobject intent) {

        LOGV(LIBENG_LOG_PLATFORM, 0, LOG_FORMAT(" - i:%p"), __PRETTY_FUNCTION__, __LINE__, intent);
#ifdef LIBENG_ENABLE_SOCIAL
        if (Social::mThis) // Existing social instance (in use)
            Social::mThis->start(intent);
#endif
    }
    void change(ANativeWindow* window) {

        LOGV(LIBENG_LOG_PLATFORM, 0, LOG_FORMAT(" - w:%p"), __PRETTY_FUNCTION__, __LINE__, window);
        mRender->start(window);
    }
#else // !__ANDROID__
    void start(short screenWidth, short screenHeight, EAGLContext* context, CAEAGLLayer* layer) {

        LOGV(LIBENG_LOG_PLATFORM, 0, LOG_FORMAT(), __PRETTY_FUNCTION__, __LINE__);
        mRender->start(screenWidth, screenHeight, context, layer);
    }
#endif
    void resume(long millis) {

        LOGV(LIBENG_LOG_PLATFORM, 0, LOG_FORMAT(" - m:%d"), __PRETTY_FUNCTION__, __LINE__, millis);
        if (mRender->getTickPerSecond() < 0.f) {

            float tickPerSecond = delta<clock_t>(initClock, clock()) / static_cast<float>(millis - initTime);
            tickPerSecond = std::abs<float>(tickPerSecond) * 1000.f; // In second
            LOGI(LIBENG_LOG_PLATFORM, 0, LOG_FORMAT(" - Tick per second: %f"), __PRETTY_FUNCTION__, __LINE__,
                    tickPerSecond);

            mRender->setTickPerSecond(tickPerSecond);
        }
#if !defined(__ANDROID__) && defined(LIBENG_ENABLE_SOCIAL)
        if (Social::mThis) // Existing social instance (in use)
            Social::mThis->resume();
#endif
        mRender->resume();
    }
#ifdef __ANDROID__
    void result(int req, int res, jobject intent) {

        LOGV(LIBENG_LOG_PLATFORM, 0, LOG_FORMAT(" - r:%d; r:%d; i:%p"), __PRETTY_FUNCTION__, __LINE__, req, res, intent);
#ifdef LIBENG_ENABLE_SOCIAL
        if (Social::mThis)
            Social::mThis->intent(req, res, intent);
#endif
    }
    void pause(bool finishing, bool lockScreen) {

        LOGV(LIBENG_LOG_PLATFORM, 0, LOG_FORMAT(" - f:%s; l:%s"), __PRETTY_FUNCTION__, __LINE__,
                (finishing)? "true":"false", (lockScreen)? "true":"false");
        mRender->pause(finishing, lockScreen);

#else // !__ANDROID__
    void pause() {

        LOGV(LIBENG_LOG_PLATFORM, 0, LOG_FORMAT(), __PRETTY_FUNCTION__, __LINE__);
        mRender->pause();
#endif
        mPlayer->pause(ALL_TRACK); // Stop playing track(s) immediately

#ifdef __ANDROID__
#ifdef LIBENG_ENABLE_MIC
        if (!finishing)
            mMic->stop();
#endif
#else // !__ANDROID__
#ifdef LIBENG_ENABLE_CAMERA
        mCamera->pause();
#endif
        mRender->stop();
#endif
    }
#ifdef __ANDROID__
    void destroy() { // Only called throught 'surfaceDestroyed' java method under ANDROID

        LOGV(LIBENG_LOG_PLATFORM, 0, LOG_FORMAT(), __PRETTY_FUNCTION__, __LINE__);
        mRender->stop();
    }
    void stop() {

        LOGV(LIBENG_LOG_PLATFORM, 0, LOG_FORMAT(), __PRETTY_FUNCTION__, __LINE__);
#ifdef LIBENG_ENABLE_SOCIAL
        if (Social::mThis)
            Social::mThis->stop();
#endif
    }
#endif
    void free() {

        LOGV(LIBENG_LOG_PLATFORM, 0, LOG_FORMAT(), __PRETTY_FUNCTION__, __LINE__);
        mPlayer->free();
#ifndef __ANDROID__ // iOS
#ifdef LIBENG_ENABLE_SOCIAL
        if (Social::mThis)
            Social::mThis->terminate();
#endif
#ifdef LIBENG_ENABLE_CAMERA
        if (mCamera->isStarted())
            mCamera->stop();
#endif
        mRender->stop();
#endif
    }

    //////
    unsigned char loadTexture(unsigned char id, short width, short height, unsigned char* data,
            bool grayscale) {

        LOGV(LIBENG_LOG_PLATFORM, 0, LOG_FORMAT(" - i:%d; w:%d; h:%d; d:%p; g:%s"), __PRETTY_FUNCTION__, __LINE__, id,
                width, height, data, (grayscale)? "true":"false");
        return mRender->addTexture(id, width, height, data, grayscale);
    }
    void loadOgg(unsigned char id, int len, unsigned char* data, bool queued) {

        LOGV(LIBENG_LOG_PLATFORM, 0, LOG_FORMAT(" - i:%d; l:%d; d:%p; q:%s"), __PRETTY_FUNCTION__, __LINE__, id, len,
                data, (queued)? "true":"false");
        mPlayer->addSound(id, len, data, queued);
    }
#ifdef LIBENG_ENABLE_STORAGE
    void loadFile(const char* file, const wchar_t* content) {

        LOGV(LIBENG_LOG_PLATFORM, 0, LOG_FORMAT(" - f:%s; c:%p"), __PRETTY_FUNCTION__, __LINE__, (file)? file:"NULL",
                content);
        Storage::getInstance()->addFile(file, content);
    }
#endif
#ifdef LIBENG_ENABLE_CAMERA
    void loadCamera(const unsigned char* data) { mCamera->updateBuffer(data); }
#endif
#ifdef LIBENG_ENABLE_MIC
    void loadMic(int len, const short* data) { mMic->updateBuffer(len, data); }
#endif
#ifdef LIBENG_ENABLE_BLUETOOTH
    bool loadBluetooth(int len, unsigned char* data) {

        LOGV(LIBENG_LOG_PLATFORM, 0, LOG_FORMAT(" - l:%d; d:%p"), __PRETTY_FUNCTION__, __LINE__, len, data);
        if (len > 0)
            return Bluetooth::getInstance()->read(len, data);

        Bluetooth::getInstance()->setConnection(!len);
        return true;
    }
#endif

    void touch(unsigned int id, unsigned char type, short x, short y);
    void accelerometer(float xRate, float yRate, float zRate);
};

//////
PlatformOS::PlatformOS(const PlatformData* data) : mRender(NULL), mDimension(data->dimension),
        mAccelRange(data->accelRange) {

    LOGV(LIBENG_LOG_PLATFORM, 0, LOG_FORMAT(" - d:%p (g:%p; d:%s; r:%f; x:%f; y:%f)"), __PRETTY_FUNCTION__, __LINE__,
            data, data->game, (data->dimension)? "true":"false", data->accelRange, data->xDpi, data->yDpi);
    if (!mDimension)
        mRender = Render2D::getInstance();
    else {

        LOGE(LOG_FORMAT(" - No 3D render yet!"), __PRETTY_FUNCTION__, __LINE__);
        assert(NULL);
    }
    mPlayer = Player::getInstance();
    mInputs = Inputs::getInstance();
#ifdef LIBENG_ENABLE_CAMERA
    mCamera = Camera::getInstance();
#ifdef LIBENG_ENABLE_CAMERA
    mCamera = Camera::getInstance();
#endif
#endif
#ifdef LIBENG_ENABLE_MIC
    mMic = Mic::getInstance();
#endif
    mRender->initialize(static_cast<Game*>(data->game), data->xDpi, data->yDpi);
}
PlatformOS::~PlatformOS() {

    LOGV(LIBENG_LOG_PLATFORM, 0, LOG_FORMAT(), __PRETTY_FUNCTION__, __LINE__);
    if (!mDimension)
        Render2D::freeInstance();

    Player::freeInstance();
    Inputs::freeInstance();
#ifdef LIBENG_ENABLE_STORAGE
    Storage::freeInstance();
#endif
#ifdef LIBENG_ENABLE_CAMERA
    Camera::freeInstance();
#endif
#ifdef LIBENG_ENABLE_MIC
    Mic::freeInstance();
#endif
#ifdef LIBENG_ENABLE_INTERNET
    Internet::freeInstance();
#endif
#ifdef LIBENG_ENABLE_ADVERTISING
    Advertising::freeInstance();
#endif
#ifdef LIBENG_ENABLE_SOCIAL
    Social::freeInstance();
#endif
}

void PlatformOS::touch(unsigned int id, unsigned char type, short x, short y) {

    TouchInput::TouchType touchType = TouchInput::TOUCH_CANCEL;
#ifdef __ANDROID__
    switch (type) {
        case 0: touchType = TouchInput::TOUCH_DOWN; break;      // MotionEvent.ACTION_DOWN
        case 1: touchType = TouchInput::TOUCH_UP; break;        // MotionEvent.ACTION_UP
        case 2: touchType = TouchInput::TOUCH_MOVE; break;      // MotionEvent.ACTION_MOVE
        case 3: touchType = TouchInput::TOUCH_CANCEL; break;    // MotionEvent.ACTION_CANCEL
        case 5: touchType = TouchInput::TOUCH_DOWN; break;      // MotionEvent.ACTION_POINTER_DOWN
        case 6: touchType = TouchInput::TOUCH_UP; break;        // MotionEvent.ACTION_POINTER_UP
#ifdef DEBUG
        default: {

            LOGE(LOG_FORMAT(" - Wrong touch type"), __PRETTY_FUNCTION__, __LINE__);
            assert(NULL);
            break;
        }
#endif
    }
#else
    switch (type) {
        case TOUCH_BEGAN:        touchType = TouchInput::TOUCH_DOWN; break;
        case TOUCH_MOVED:        touchType = TouchInput::TOUCH_MOVE; break;
        case TOUCH_ENDED:        touchType = TouchInput::TOUCH_UP; break;
        case TOUCH_CANCELLED:    touchType = TouchInput::TOUCH_CANCEL; break;
#ifdef DEBUG
        default: {

            LOGE(LOG_FORMAT(" - Wrong touch type"), __PRETTY_FUNCTION__, __LINE__);
            assert(NULL);
            break;
        }
#endif
    }
#endif

    TouchData data;

    data.Id = id;
    data.Type = touchType;
    data.X = x;
    data.Y = y;

    mInputs->update<TouchInput>(Inputs::TOUCH, static_cast<const void*>(&data));
}
void PlatformOS::accelerometer(float xRate, float yRate, float zRate) {

    AccelData data;

#ifndef __ANDROID__
// LIBENG_PORT_AS_LAND cannot be defined (see 'Global.h')
#ifndef LIBENG_LAND_ORIENTATION
    data.X = xRate;
    data.Y = yRate;
#else
    data.X = yRate;
    data.Y = xRate;
    // In LIBENG_LAND_ORIENTATION reverse X & Y on iOS
#endif
#else // Android
#ifndef LIBENG_PORT_AS_LAND
    data.X = xRate;
    data.Y = yRate;
    // Even if in LIBENG_LAND_ORIENTATION
#else
    data.X = yRate;
    data.Y = xRate;
#endif
#endif
    data.Z = zRate;

    mInputs->update<AccelInput>(Inputs::ACCEL, static_cast<const void*>(&data));
    mInputs->get(Inputs::ACCEL)->setConfig(static_cast<const void*>(&mAccelRange));
}

//////
PlatformOS* PlatformOS::mThis = NULL;
PlatformOS* platform; // Avoid to check existing instance each time it is used

extern "C" {

void platformInit(long millis, const PlatformData* data) {

    LOGV(LIBENG_LOG_PLATFORM, 0, LOG_FORMAT(" - m:%d; d:%p"), __PRETTY_FUNCTION__, __LINE__, millis, data);
    assert(data);

#ifdef __ANDROID__
    g_JavaVM = data->jvm;
    g_JavaProject = data->jpn;
    g_ActivityClass = data->cls;
    g_ActivityObject = data->obj;

#ifdef LIBENG_ENABLE_SOCIAL
    g_FacebookClass = data->facebook;
    g_TwitterClass = data->twitter;
    g_GoogleClass = data->google;
#endif
#ifdef LIBENG_ENABLE_BLUETOOTH
    g_BluetoothClass = data->bluetooth;
#endif

#else // !__ANDROID__
    g_AppleOS = data->os;

#endif
    initTime = millis;
    initClock = clock();

    g_LogTag = data->project;
    g_GameFile = data->file;

    platform = PlatformOS::getInstance(data);
}
#ifdef __ANDROID__
void platformCreate() { platform->create(); }
void platformStart(jobject intent) { platform->start(intent); }
void platformChange(ANativeWindow* window) { platform->change(window); }
#else
void platformStart(short screenWidth, short screenHeight, EAGLContext* context, CAEAGLLayer* layer) {

    platform->start(screenWidth, screenHeight, context, layer);
}
#endif
void platformResume(long millis) { platform->resume(millis); }
#ifdef __ANDROID__
void platformResult(int req, int res, jobject intent) { platform->result(req, res, intent); }
void platformPause(bool finishing, bool lockScreen) { platform->pause(finishing, lockScreen); }
void platformDestroy() { platform->destroy(); }
void platformStop() { platform->stop(); }
#else
void platformPause() { platform->pause(); }
#endif
void platformFree() {

    platform->free();
    PlatformOS::freeInstance();
}

unsigned char platformLoadTexture(unsigned char id, short width, short height, unsigned char* data,
        bool grayscale) {

    return platform->loadTexture(id, width, height, data, grayscale);
}
void platformLoadOgg(unsigned char id, int len, unsigned char* data, bool queued) {

    platform->loadOgg(id, len, data, queued);
}
void platformLoadFile(const char* file, const wchar_t* content) {

#ifdef LIBENG_ENABLE_STORAGE
    platform->loadFile(file, content);
#else
    LOGF(LOG_FORMAT(" - LIBENG_ENABLE_STORAGE flag is not defined"), __PRETTY_FUNCTION__, __LINE__);
    assert(NULL);
#endif
}
void platformLoadCamera(const unsigned char* data) {

#ifdef LIBENG_ENABLE_CAMERA
    platform->loadCamera(data);
#else
    LOGF(LOG_FORMAT(" - LIBENG_ENABLE_CAMERA flag is not defined"), __PRETTY_FUNCTION__, __LINE__);
    assert(NULL);
#endif
}
void platformLoadMic(int len, const short* data) {

#ifdef LIBENG_ENABLE_MIC
    platform->loadMic(len, data);
#else
    LOGF(LOG_FORMAT(" - LIBENG_ENABLE_MIC flag is not defined"), __PRETTY_FUNCTION__, __LINE__);
    assert(NULL);
#endif
}
#ifdef LIBENG_ENABLE_SOCIAL
void platformLoadSocial(unsigned char id, unsigned char request, unsigned char result, short width,
        short height, unsigned char* data) {

    LOGV(LIBENG_LOG_PLATFORM, 0, LOG_FORMAT(" - i:%d; r:%d; r:%d; w:%d; h:%d; d:%p"), __PRETTY_FUNCTION__, __LINE__, id,
            request, result, width, height, data);
    switch (static_cast<Session::RequestID>(request)) {

        case Session::REQUEST_LOGIN:
        case Session::REQUEST_INFO:
        case Session::REQUEST_SHARE_LINK:
        case Session::REQUEST_SHARE_VIDEO: {
            platform->getNetwork(static_cast<Network::ID>(id))->setResult(static_cast<Session::RequestID>(request),
                    result);
            break;
        }
        case Session::REQUEST_PICTURE: {
            platform->getNetwork(static_cast<Network::ID>(id))->setPicture(result, width, height, data);
            break;
        }
        default: {

            LOGW(LOG_FORMAT(" - Invalid request ID: %d"), __PRETTY_FUNCTION__, __LINE__, id);
            assert(NULL);
            break;
        }
    }
}
#endif
#ifdef LIBENG_ENABLE_STORAGE
void platformLoadStore(unsigned char result) { Storage::getInstance()->setStatus(result); }
#endif
#ifdef LIBENG_ENABLE_BLUETOOTH
bool platformLoadBluetooth(int len, unsigned char* data) { return platform->loadBluetooth(len, data); }
#endif

void platformTouch(unsigned int id, unsigned char type, short x, short y) {

    if (Inputs::InputUse & USE_INPUT_TOUCH)
        platform->touch(id, type, x, y);
}
void platformAccelerometer(float xRate, float yRate, float zRate) {

    if (Inputs::InputUse & USE_INPUT_ACCEL)
        platform->accelerometer(xRate, yRate, zRate);
}

} // extern "C"
} // namespace
