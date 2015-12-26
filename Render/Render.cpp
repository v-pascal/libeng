#include "Render.h"

#include <libeng/Log/Log.h>
#include <libeng/Tools/Tools.h>
#include <libeng/Advertising/Advertising.h>
#include <time.h>

#ifndef __ANDROID__
#import "OpenGLES/ES2/gl.h"
#endif

#define MIN_WAIT_DELAY          0.3f // Minimum delay allowed to be able to call 'wait' method

Render* Render::mThis = NULL;

//////
Render::Render() : mGame(NULL), mThread(NULL), mPlayer(NULL), mLastTickCount(0), mEvent(EVT_NONE), mInitEGL(false),
        mProgramObj(0), mPause(false), mInputs(NULL), mTickPerSecond(LIBENG_NO_DATA) {

    LOGV(LIBENG_LOG_RENDER, 0, LOG_FORMAT(), __PRETTY_FUNCTION__, __LINE__);
#ifdef DEBUG
    mLogRender = 0;
#endif
    mTextures = eng::Textures::getInstance();
    mPlayer = eng::Player::getInstance();
#ifdef LIBENG_ENABLE_CAMERA
    mCamera = eng::Camera::getInstance();
#endif

    std::memset(&mScreen, 0, sizeof(eng::Screen));

#ifdef __ANDROID__
    mWindow = NULL;

    mDisplay = EGL_NO_DISPLAY;
    mSurface = EGL_NO_SURFACE;
    mContext = EGL_NO_CONTEXT;

    mLockScreen = false;
    mDraw = false;

#else
    mMainContext = nil;
    mWorkContext = nil;

    mFrameBuffer = 0;
    mRenderBuffer = 0;

#endif
}
Render::~Render() {

    LOGV(LIBENG_LOG_RENDER, 0, LOG_FORMAT(), __PRETTY_FUNCTION__, __LINE__);
    eng::Textures::freeInstance();
}

unsigned char Render::addTexture(unsigned char index, short width, short height, unsigned char* data,
        bool grayscale) {

    return mTextures->addTexture(index, width, height, data, grayscale);
}
#ifdef __ANDROID__
bool Render::initEGL() {

    LOGV(LIBENG_LOG_RENDER, 0, LOG_FORMAT(), __PRETTY_FUNCTION__, __LINE__);

    mDisplay = eglGetDisplay(EGL_DEFAULT_DISPLAY);
    if (mDisplay == EGL_NO_DISPLAY) {

        LOGE(LOG_FORMAT(" - eglGetDisplay failed"), __PRETTY_FUNCTION__, __LINE__);
        return false;
    }

    if (eglInitialize(mDisplay, 0, 0) == EGL_FALSE) {

        LOGE(LOG_FORMAT(" - eglInitialize failed (%p)"), __PRETTY_FUNCTION__, __LINE__, mDisplay);
        return false;
    }

    // Here specify the attributes of the desired configuration.
    // Below, we select an EGLConfig with at least 8 bits per color
    // component compatible with on-screen windows
    const EGLint configAttribs[] = {

        EGL_RENDERABLE_TYPE, EGL_OPENGL_ES2_BIT, // OpenGL ES 2.0
        EGL_SURFACE_TYPE, EGL_WINDOW_BIT,
        EGL_COLOR_BUFFER_TYPE, EGL_RGB_BUFFER,
        EGL_LUMINANCE_SIZE, 0,
        EGL_BLUE_SIZE, 8,
        EGL_GREEN_SIZE, 8,
        EGL_RED_SIZE, 8,
        EGL_ALPHA_SIZE, 8,
        EGL_NONE
    };

    EGLint numConfigs;
    EGLConfig config;

    // Here, the application chooses the configuration it desires. In this
    // sample, we have a very simplified selection process, where we pick
    // the first EGLConfig that matches our criteria
    if (eglChooseConfig(mDisplay, configAttribs, &config, 1, &numConfigs) == EGL_FALSE) {

        LOGE(LOG_FORMAT(" - eglChooseConfig failed (d:%p; c:%p; n:%d)"), __PRETTY_FUNCTION__, __LINE__, mDisplay, config,
                numConfigs);
        return false;
    }

    EGLint format;

    // EGL_NATIVE_VISUAL_ID is an attribute of the EGLConfig that is
    // guaranteed to be accepted by ANativeWindow_setBuffersGeometry().
    // As soon as we picked a EGLConfig, we can safely reconfigure the
    // ANativeWindow buffers to match, using EGL_NATIVE_VISUAL_ID.
    if (eglGetConfigAttrib(mDisplay, config, EGL_NATIVE_VISUAL_ID, &format) == EGL_FALSE) {

        LOGE(LOG_FORMAT(" - eglChooseConfig failed (d:%p; c:%p; f:%d)"), __PRETTY_FUNCTION__, __LINE__, mDisplay, config,
                format);
        return false;
    }

    ANativeWindow_setBuffersGeometry(mWindow, 0, 0, format);

    mSurface = eglCreateWindowSurface(mDisplay, config, mWindow, NULL);
    if (mSurface == EGL_NO_SURFACE) {

        LOGE(LOG_FORMAT(" - eglCreateWindowSurface failed (d:%p; c:%p; w:%p)"), __PRETTY_FUNCTION__, __LINE__, mDisplay,
                config, mWindow);
        return false;
    }

    const EGLint contextAttribs[] = {

        EGL_CONTEXT_CLIENT_VERSION, 2, // OpenGL ES 2.0
        EGL_NONE
    };

    mContext = eglCreateContext(mDisplay, config, NULL, contextAttribs);
    if (mContext == EGL_NO_CONTEXT) {

        LOGE(LOG_FORMAT(" - eglCreateContext failed (d:%p; c:%p)"), __PRETTY_FUNCTION__, __LINE__, mDisplay, config);
        return false;
    }

    if (eglMakeCurrent(mDisplay, mSurface, mSurface, mContext) == EGL_FALSE) {

        LOGE(LOG_FORMAT(" - eglMakeCurrent failed (d:%p; s:%p; c:%p)"), __PRETTY_FUNCTION__, __LINE__, mDisplay, mSurface,
                mContext);
        return false;
    }

    // Get surface resolution
    EGLint screenWidth, screenHeight;
    if ((eglQuerySurface(mDisplay, mSurface, EGL_WIDTH, &screenWidth) == EGL_FALSE) ||
        (eglQuerySurface(mDisplay, mSurface, EGL_HEIGHT, &screenHeight) == EGL_FALSE)) {

        LOGE(LOG_FORMAT(" - eglQuerySurface failed (d:%p; s:%p)"), __PRETTY_FUNCTION__, __LINE__, mDisplay, mSurface);
        return false;
    }

    // WARNING: Screen resolution shoud not changed between a start & a resume operation!
    if (mScreen.width) {
#ifndef LIBENG_PORT_AS_LAND
        if ((mScreen.width != static_cast<short>(screenWidth)) || (mScreen.height != static_cast<short>(screenHeight))) {
            LOGW(LOG_FORMAT(" - Screen resolution shoud not changed (o:%d;%d n:%d;%d)"), __PRETTY_FUNCTION__, __LINE__,
                    mScreen.width, mScreen.height, static_cast<short>(screenWidth), static_cast<short>(screenHeight));
        }
#else
        if ((mScreen.width != static_cast<short>(screenHeight)) || (mScreen.height != static_cast<short>(screenWidth))) {
            LOGW(LOG_FORMAT(" - Screen resolution shoud not changed (o:%d;%d n:%d;%d)"), __PRETTY_FUNCTION__, __LINE__,
                    mScreen.height, mScreen.width, static_cast<short>(screenWidth), static_cast<short>(screenHeight));
        }
#endif
    }
    mScreen.width = static_cast<short>(screenWidth);
    mScreen.height = static_cast<short>(screenHeight);

    return true;
}
#endif
bool Render::initOpenglES2x() {

    LOGV(LIBENG_LOG_RENDER, 0, LOG_FORMAT(), __PRETTY_FUNCTION__, __LINE__);
#ifdef __ANDROID__
    ////// Init EGL
    if (!initEGL()) {
        return false;
    }
#else
    if (mWorkContext != nil) {

        [mWorkContext release];
        mWorkContext = nil;
    }
    mWorkContext = [[EAGLContext alloc] initWithAPI:[mMainContext API] sharegroup:[mMainContext sharegroup]];
    [EAGLContext setCurrentContext:mWorkContext];

    glBindFramebuffer(GL_FRAMEBUFFER, mFrameBuffer);
#endif

    // Init OpenGLES elements
    if (!init())
        return false;

#ifdef DEBUG
    GLint maxTexSize;
    glGetIntegerv(GL_MAX_TEXTURE_SIZE, &maxTexSize);

    LOGI(LIBENG_LOG_RENDER, 0, LOG_FORMAT(" - Maximum texture size: %d"), __PRETTY_FUNCTION__, __LINE__, maxTexSize);
#endif
    return true;
}
bool Render::renderOpenglES2x() {

#ifdef DEBUG
    LOGV(LIBENG_LOG_RENDER, (mLogRender % 100), LOG_FORMAT(" - (w:%d; h:%d)"), __PRETTY_FUNCTION__, __LINE__,
            mScreen.width, mScreen.height);
#endif
#ifndef __ANDROID__
    [EAGLContext setCurrentContext:mWorkContext];

    glBindFramebuffer(GL_FRAMEBUFFER, mFrameBuffer);
#endif

    ////// Render
    mGame->updateFrame();
    begin();
    mGame->renderFrame();

#ifdef __ANDROID__
    if (eglSwapBuffers(mDisplay, mSurface) == EGL_FALSE) {

        LOGE(LOG_FORMAT(" - eglSwapBuffers error %d"), __PRETTY_FUNCTION__, __LINE__, eglGetError());
        return false;
    }
#else
    glBindRenderbuffer(GL_RENDERBUFFER, mRenderBuffer);
    [[EAGLContext currentContext] presentRenderbuffer:GL_RENDERBUFFER];

    glClear(GL_COLOR_BUFFER_BIT);
#endif
    return true;
}
void Render::destroyOpenglES2x() {

    LOGV(LIBENG_LOG_RENDER, 0, LOG_FORMAT(), __PRETTY_FUNCTION__, __LINE__);
    if (mPause)

        ////// Pause
        mGame->pause();

    else {

        ////// Destroy
        mGame->destroy();

#ifdef DEBUG
        // Check all texture objects has been deleted correctly
        if (mTextures->chkTextures()) {
            LOGI(LIBENG_LOG_RENDER, 0, LOG_FORMAT(" - Good textures management"), __PRETTY_FUNCTION__, __LINE__);
        }
        else {
            LOGW(LOG_FORMAT(" - Bad textures management"), __PRETTY_FUNCTION__, __LINE__);
        }
#endif
    }

    mPlayer->pause(eng::ALL_TRACK);
    mPlayer->empty();
    // -> Needed if 'PlatformOS::pause' has been called B4 having finished to start

#ifdef __ANDROID__
    ////// Destroy OpenGL ES 2.0
    if (mDisplay != EGL_NO_DISPLAY) {

        eglMakeCurrent(mDisplay, EGL_NO_SURFACE, EGL_NO_SURFACE, EGL_NO_CONTEXT);
        if (mContext != EGL_NO_CONTEXT) {
            eglDestroyContext(mDisplay, mContext);
        }
        if (mSurface != EGL_NO_SURFACE) {
            eglDestroySurface(mDisplay, mSurface);
        }
        eglTerminate(mDisplay);
    }

    mDisplay = EGL_NO_DISPLAY;
    mSurface = EGL_NO_SURFACE;
    mContext = EGL_NO_CONTEXT;

    ANativeWindow_release(mWindow);

#else
    if (!mPause) {

        // Delete the Frame and Render buffers.
        glDeleteRenderbuffers(1, &mRenderBuffer);
        glDeleteFramebuffers(1, &mFrameBuffer);
    }
#endif
    mInitEGL = false;
}

//////
#ifdef __ANDROID__
void Render::create() {

    LOGV(LIBENG_LOG_RENDER, 0, LOG_FORMAT(), __PRETTY_FUNCTION__, __LINE__);
    assert(!mThread);

    mEvent= EVT_NONE;
    mThread = new boost::thread(Render::startRenderThread, this);
}
void Render::start(ANativeWindow* window) {

    LOGV(LIBENG_LOG_RENDER, 0, LOG_FORMAT(" - w:%p (i:%s)"), __PRETTY_FUNCTION__, __LINE__, window,
            (mInitEGL)? "true":"false");
    assert(mThread);

    mLastTickCount = clock();
    if (!mInitEGL) {

        //assert(mWindow != window); // Possible case !?!
        mWindow = window;

        mMutex.lock();
        mEvent = EVT_INIT;
        boost::mutex initMutex;
        boost::unique_lock<boost::mutex> lock(initMutex);
        mMutex.unlock();
        mWaitInit.wait(lock);

        LOGI(LIBENG_LOG_RENDER, 0, LOG_FORMAT(" - Init OpenGL-ES done"), __PRETTY_FUNCTION__, __LINE__);
    }
    else { // Lock/Unlock operation

        assert(mWindow == window);

        mMutex.lock();
        switch (mEvent) {

            case EVT_INIT: break;
            case EVT_NONE: {

                LOGI(LIBENG_LOG_RENDER, 0, LOG_FORMAT(" - Let's draw"), __PRETTY_FUNCTION__, __LINE__);
                mEvent = EVT_DRAW;
                break;
            }
        }
        mMutex.unlock();
    }
}
#else
void Render::start(short screenWidth, short screenHeight, EAGLContext* context, CAEAGLLayer* layer) {
    
    LOGV(LIBENG_LOG_RENDER, 0, LOG_FORMAT(" - w:%d; h:%d; c:%p; l:%p"), __PRETTY_FUNCTION__, __LINE__, screenWidth,
            screenHeight, context, layer);
    assert(!mThread);
    
    mMainContext = context;
    mLastTickCount = clock();

    // Creates the Frame buffer.
    glGenFramebuffers(1, &mFrameBuffer);
    glBindFramebuffer(GL_FRAMEBUFFER, mFrameBuffer);

    // Creates the render buffer.
    // Here the EAGL compels us to change the default OpenGL behavior.
    // Instead to use glRenderbufferStorage(GL_RENDERBUFFER, GL_RGB565, _surfaceWidth, _surfaceHeight);
    // We need to use renderbufferStorage: defined here in the [EAGLView propertiesToCurrentColorbuffer];
    glGenRenderbuffers(1, &mRenderBuffer);
    glBindRenderbuffer(GL_RENDERBUFFER, mRenderBuffer);

    [mMainContext renderbufferStorage:GL_RENDERBUFFER fromDrawable:layer];
    glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_RENDERBUFFER, mRenderBuffer);

    //check that our configuration of the framebuffer is valid
    if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE) {

        LOGE(LOG_FORMAT(" - Failed to make GL_FRAMEBUFFER: %d"), __PRETTY_FUNCTION__, __LINE__,
             static_cast<int>(glCheckFramebufferStatus(GL_FRAMEBUFFER)));
        assert(NULL);
    }
    // WARNING: Screen resolution shoud not changed between a start & a resume operation!
    if (mScreen.width) {
#ifndef LIBENG_PORT_AS_LAND
        if ((mScreen.width != static_cast<short>(screenWidth)) || (mScreen.height != static_cast<short>(screenHeight))) {
            LOGW(LOG_FORMAT(" - Screen resolution shoud not changed (o:%d;%d n:%d;%d)"), __PRETTY_FUNCTION__, __LINE__,
                    mScreen.width, mScreen.height, static_cast<short>(screenWidth), static_cast<short>(screenHeight));
        }
#else
        if ((mScreen.width != static_cast<short>(screenHeight)) || (mScreen.height != static_cast<short>(screenWidth))) {
            LOGW(LOG_FORMAT(" - Screen resolution shoud not changed (o:%d;%d n:%d;%d)"), __PRETTY_FUNCTION__, __LINE__,
                    mScreen.height, mScreen.width, static_cast<short>(screenWidth), static_cast<short>(screenHeight));
        }
#endif
    }
    mScreen.width = screenWidth;
    mScreen.height = screenHeight;

    mEvent = EVT_INIT;
    mThread = new boost::thread(Render::startRenderThread, this);
}
#endif
#ifdef __ANDROID__
void Render::pause(bool finishing, bool lockscreen) {

    LOGV(LIBENG_LOG_RENDER, 0, LOG_FORMAT(" - f:%s; l:%s"), __PRETTY_FUNCTION__, __LINE__, (finishing)? "true":"false",
            (lockscreen)? "true":"false");
    assert(mDraw);

    mPause = !finishing;
    mLockScreen = lockscreen;
    mDraw = false; // Do it after assignment above
}
void Render::noDraw(bool wait) {

#ifdef DEBUG
    if (!wait) {
        LOGV(LIBENG_LOG_RENDER, 0, LOG_FORMAT(" - w:%s"), __PRETTY_FUNCTION__, __LINE__, (wait)? "true":"false");
    }
#endif
    if (mPlayer->isRunning())
        mPlayer->pause(eng::ALL_TRACK);
#ifdef LIBENG_ENABLE_ADVERTISING
    eng::Advertising::getInstance()->resetPubDelay();
#endif
#ifdef LIBENG_ENABLE_CAMERA
    if (mPause)
        mCamera->pause(mLockScreen);
#endif
    if (wait) {

        if ((!mGame->isScreenLocked()) && (mLockScreen))
            mGame->lockScreen(); ////// Lock screen

        boost::this_thread::sleep(boost::posix_time::milliseconds(150));
    }
#ifdef LIBENG_ENABLE_DELTA
    mGame->resetDelta();
#endif
}
#else
void Render::pause() {

    LOGV(LIBENG_LOG_RENDER, 0, LOG_FORMAT(), __PRETTY_FUNCTION__, __LINE__);
    mPause = true;
}
#endif
void Render::resume() {

#ifdef __ANDROID__
    LOGV(LIBENG_LOG_RENDER, 0, LOG_FORMAT(" - (d:%s)"), __PRETTY_FUNCTION__, __LINE__, (mDraw)? "true":"false");
    assert(!mDraw);

    mDraw = true;

#else
    LOGV(LIBENG_LOG_RENDER, 0, LOG_FORMAT(), __PRETTY_FUNCTION__, __LINE__);
    if (mPause) {

        assert(!mThread);

        mEvent = EVT_INIT;
        mThread = new boost::thread(Render::startRenderThread, this);
    }
    else {

        assert(mThread);

        mMutex.lock();
        mEvent = EVT_DRAW;
        mMutex.unlock();
    }
#endif

#ifdef LIBENG_ENABLE_DELTA
    mGame->resetDelta();
#endif
}
void Render::stop() {

    LOGV(LIBENG_LOG_RENDER, 0, LOG_FORMAT(), __PRETTY_FUNCTION__, __LINE__);
    //assert(mThread);
    if (!mThread)
        return; // Can occured when terminate an application in pause

    mMutex.lock();
    mEvent = EVT_EXIT;
    mMutex.unlock();

    mThread->join();
    delete mThread;

    mThread = NULL;
}

void Render::renderThreadRunning() { ///////////////////////////////////////////

    LOGV(LIBENG_LOG_RENDER, 0, LOG_FORMAT(" - Start loop"), __PRETTY_FUNCTION__, __LINE__);

    bool bAbort = false;
    while (!bAbort) {

        mMutex.lock();
        switch (mEvent) {

            case EVT_NONE: {

                mMutex.unlock();
#ifdef DEBUG
                ++mLogRender;
                LOGI(LIBENG_LOG_RENDER, (mLogRender % 100), LOG_FORMAT(" - EVT_NONE"), __PRETTY_FUNCTION__, __LINE__);
#endif
                boost::this_thread::sleep(boost::posix_time::milliseconds(100));
                break;
            }
            case EVT_INIT: {

#ifdef __ANDROID__
                mEvent = EVT_DRAW;
#else
                if (!mPause)
                    mEvent = EVT_NONE; // 'resume' method will set it to 'EVT_DRAW' (via 'Render::resume')
                else {

                    mEvent = EVT_DRAW;
                    // ...'resume' method has been already called
                    // -> It created this thread with this 'EVT_INIT' event (it replaced 'start' call)

                    mPause = false;
                }
#endif
                mMutex.unlock();

                LOGI(LIBENG_LOG_RENDER, 0, LOG_FORMAT(" - EVT_INIT"), __PRETTY_FUNCTION__, __LINE__);
                assert(!mInitEGL);

                // Init OpenGL ES 2.0
                mInitEGL = initOpenglES2x();

                ///// Start
                mGame->start();

                LOGI(LIBENG_LOG_RENDER, 0, LOG_FORMAT(" - Game started"), __PRETTY_FUNCTION__, __LINE__);
                mWaitInit.notify_one();
                break;
            }
            case EVT_DRAW: {

                mMutex.unlock();
#ifdef DEBUG
                ++mLogRender;
#ifdef __ANDROID__
                LOGI(LIBENG_LOG_RENDER, (mLogRender % 100), LOG_FORMAT(" - EVT_DRAW (%s)"), __PRETTY_FUNCTION__,
                        __LINE__, (mDraw)? "true":"false");
#else
                LOGI(LIBENG_LOG_RENDER, (mLogRender % 100), LOG_FORMAT(" - EVT_DRAW"), __PRETTY_FUNCTION__, __LINE__);
#endif
#endif
#ifdef __ANDROID__
                if (!mDraw) {

                    noDraw(true); // Wait
                    break;
                }
#endif
                clock_t curTickCount = clock();
                float secondsElapsed = static_cast<float>(curTickCount - mLastTickCount) / mTickPerSecond;
                if (secondsElapsed > (1.f / LIBENG_RENDER_FPS)) {

                    mLastTickCount = curTickCount;
#ifdef __ANDROID__
                    mGame->unlockScreen();
#endif
                    // Render OpenGL ES 2.0
                    renderOpenglES2x();
                }
                else {

                    float millisDelay = ((1.f / LIBENG_RENDER_FPS) - secondsElapsed) * 1000.f;
#ifdef DEBUG
                    LOGI(LIBENG_LOG_RENDER, (mLogRender % 100), LOG_FORMAT(" - Wait %f milliseconds"), __PRETTY_FUNCTION__,
                            __LINE__, millisDelay);
#endif
                    if (millisDelay > MIN_WAIT_DELAY)
                        mGame->wait(millisDelay); ////// Wait
                }
                break;
            }
            case EVT_EXIT: {

                mMutex.unlock();
                LOGI(LIBENG_LOG_RENDER, 0, LOG_FORMAT(" - EVT_EXIT"), __PRETTY_FUNCTION__, __LINE__);
#ifdef __ANDROID__
                if (!mDraw)
                    noDraw(false); // ...this method has been called one time at least (here)
#endif
                // Destroy OpenGL ES 2.0
                destroyOpenglES2x();

#ifdef __ANDROID__
                // Detach Java VM from current thread (if needed)
                assert(eng::g_JavaVM);

                JNIEnv* env = NULL;
                if (eng::g_JavaVM->GetEnv(reinterpret_cast<void**>(&env), JNI_VERSION_1_6) != JNI_EDETACHED) {
                    if (eng::g_JavaVM->DetachCurrentThread() != JNI_OK) {
                        LOGE(LOG_FORMAT(" - Failed to detach Java VM with current thread"), __PRETTY_FUNCTION__, __LINE__);
                    }
                }
#endif
                bAbort = true;
                break;
            }
        }
    }
    LOGV(LIBENG_LOG_RENDER, 0, LOG_FORMAT(" - Finished"), __PRETTY_FUNCTION__, __LINE__);
}
void Render::startRenderThread(Render* render) {

    LOGV(LIBENG_LOG_RENDER, 0, LOG_FORMAT(" - r:%p"), __PRETTY_FUNCTION__, __LINE__, render);
    render->renderThreadRunning();
}
