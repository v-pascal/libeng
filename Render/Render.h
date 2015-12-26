#ifndef RENDER_H_
#define RENDER_H_

#include <libeng/Global.h>
#include <libeng/Game/Game.h>
#include <libeng/Player/Player.h>
#include <libeng/Textures/Textures.h>

#include <libeng/Inputs/Touch/Touch.h>
#include <libeng/Inputs/Accel/Accel.h>

#ifdef __ANDROID__
#include <android/native_window.h>
#include <GLES2/gl2.h>
#include <EGL/egl.h>

#else
#import <QuartzCore/CAEAGLLayer.h>
#include "OpenGLES/EAGL.h"

typedef uint32_t GLuint;
// -> Avoid to include "OpenGLES/ES2/gl.h" in header file
// BUG: 'CVOpenGLESTexture.h' compilation error (from iOS SDK 8.1)

#endif
#include <time.h>
#include <boost/thread.hpp>

//////
class Render {

protected:
    Render();
    static Render* mThis;

    eng::Screen mScreen;

public:
    virtual ~Render();

	virtual void initialize(eng::Game* game, float xDpi, float yDpi) {

	    mInputs = eng::Inputs::getInstance();
	    mPlayer = eng::Player::getInstance();
        // Avoid to check existing instance each time it is used

	    mScreen.xDpi = xDpi;
        mScreen.yDpi = yDpi;

	    assert(game);
	    mGame = game;
	}

    //////
#ifdef __ANDROID__
    void create();
	void start(ANativeWindow* window);
    void pause(bool finishing, bool lockscreen);
#else
	void start(short screenWidth, short screenHeight, EAGLContext* context, CAEAGLLayer* layer);
    void pause();
#endif
	void resume();
	void stop();

    inline void setTickPerSecond(float tick) {

        mGame->mTickPerSecond = tick;
        mTickPerSecond = tick;
    }
    inline float getTickPerSecond() const { return mTickPerSecond; }

	unsigned char addTexture(unsigned char index, short width, short height, unsigned char* data, bool grayscale);

	//////
	virtual bool init() = 0;
	virtual void begin() = 0;

protected:
    eng::Game* mGame;
    eng::Textures* mTextures;
    GLuint mProgramObj;

private:
//#ifdef DEBUG
	unsigned int mLogRender;
//#endif
	eng::Inputs* mInputs;
	eng::Player* mPlayer;
#ifdef LIBENG_ENABLE_CAMERA
	eng::Camera* mCamera;
#endif

    float mTickPerSecond; // Number of CPU tick per second
	clock_t mLastTickCount;

    enum RenderEvent {

        EVT_NONE = 0,   // None (waiting loop)
        EVT_INIT,       // Initialize OpenGL-ES (following with EVT_DRAW)
        EVT_DRAW,       // Render (drawing loop)
        EVT_EXIT        // Destroy OpenGL-ES + Exit (stop thread)
    };
    RenderEvent mEvent;
    boost::mutex mMutex;

    boost::condition_variable mWaitInit;
    bool mInitEGL;

    bool mPause; // Pause game

#ifdef __ANDROID__
    bool mLockScreen; // Lock screen

    volatile bool mDraw; // Draw/Wait
    void noDraw(bool wait);

    ANativeWindow* mWindow;
    
    EGLDisplay mDisplay;
    EGLSurface mSurface;
    EGLContext mContext;
    
    bool initEGL();
    
#else
    EAGLContext* mMainContext;
    EAGLContext* mWorkContext;
    
    GLuint	mFrameBuffer;
    GLuint	mRenderBuffer;

#endif

    bool initOpenglES2x();
    bool renderOpenglES2x();
    void destroyOpenglES2x();

    boost::thread* mThread;
    void renderThreadRunning();
    static void startRenderThread(Render* render);

};

#endif // RENDER_H_
