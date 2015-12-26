#ifndef LIBENG_GAME_H_
#define LIBENG_GAME_H_
#if defined(__cplusplus)

#include <libeng/Log/Log.h>
#include <libeng/Game/Screen.h>
#include <libeng/Player/Player.h>
#include <libeng/Textures/Textures.h>
#include <libeng/Render/Shader.h>
#include <libeng/Graphic/Text/Fonts.h>
#include <libeng/Storage/StoreData.h>
#include <libeng/Features/Camera/Camera.h>

#include <libeng/Inputs/Touch/Touch.h>
#include <libeng/Inputs/Accel/Accel.h>

#ifdef LIBENG_ENABLE_GAMEPAD
#include <libeng/Gamepads/Gamepad.h>
#endif
#include <time.h>

#ifdef LIBENG_ENABLE_FPS
#include <boost/thread.hpp>
#endif

namespace eng {

//////
class Game {

protected:
#ifdef DEBUG
    unsigned int mLog;
#endif
    Screen* mScreen;
    Textures* mTextures;
#ifdef LIBENG_ENABLE_TEXT
    Fonts* mFonts;
#endif

    mutable unsigned char mGameLevel; // == 0: Reserved to display game introduction (see 'Game2D::mIntro' data)
    // RULES: Odd -> Reserved to create, initialize & start objects / restart objects for resume
    //        Pair -> Reserved to update & check objects collision

    void getInputsData();

#ifdef __ANDROID__
    // Screen lock management
private:
    bool mScreenLocked;

public:
    inline bool isScreenLocked() const { return mScreenLocked; }
    inline void unlockScreen() { mScreenLocked = false; }
#endif

protected:
    Game(unsigned char level);
    static Game* mThis;

#ifdef LIBENG_ENABLE_STORAGE
    mutable StoreData* mGameData;
#endif

public:
    virtual ~Game();

    void initialize(Screen* screen, const ShaderParam* shader) {

        mInputs = Inputs::getInstance();
        mTextures = Textures::getInstance();
        // Avoid to check existing instance each time it is used

        mScreen = screen;
        mShaderParam = shader;
    }
    void updateFrame();
    void renderFrame();

#ifdef DEBUG
    inline const unsigned int* getLog() const { return static_cast<const unsigned int*>(&mLog); }
#endif
    inline const Screen* getScreen() const { return mScreen; }
    inline const ShaderParam* getShader() const { return mShaderParam; }
#ifdef LIBENG_ENABLE_STORAGE
    inline void* getData(wchar_t type, const char* key) const { assert(mGameData); return mGameData->get(type, key); }
#endif

    inline void setGameLevel(unsigned char level) const { mGameLevel = level; }

    virtual void updateIntro() const = 0;
    virtual void renderIntro() const = 0;

    //////
    virtual void init() = 0; // Initialize objects if needed ('Object::initialize' methods)  -> Main thread
    virtual bool start() { // Prepare objects to render if needed ('Object::start' methods)  -> Render thread

#ifdef LIBENG_ENABLE_DELTA
        mStartTime = clock();
#endif
#ifdef LIBENG_ENABLE_CAMERA
       Camera::getInstance()->resume();
#endif

#ifdef LIBENG_ENABLE_FPS
       LOGI(LIBENG_LOG_GAME, 0, LOG_FORMAT(" - Create FPS log thread"), __PRETTY_FUNCTION__, __LINE__);
       if (mFPS) {

           mAbortFPS = false;
           LOGW(LOG_FORMAT(" - FPS log thread already started"), __PRETTY_FUNCTION__, __LINE__);
           //assert(NULL); // Pause/Resume issue
           return true;
       }
       // Called when game is about to move from inactive to active state

       mAbortFPS = false;
       mRenderCount = 0;
       mFPS = new boost::thread(Game::startThreadFPS, this);
#endif
       return true;
    };
    virtual void pause(); // Pause objects (needed only for texture management)
#ifdef __ANDROID__
    virtual void lockScreen(); // Inform game enter in a lock screen processus
#endif
    virtual void destroy(); // Destroy objects if needed (delete objects before close)

    virtual void wait(float millis) = 0; // Process any traitment during 'millis' idle delay
    virtual void update() = 0; // Update game if needed ('Dynamic::update' methods)
    virtual void render() const = 0; // Render objects if needed ('Object::render' methods)

#ifdef LIBENG_ENABLE_DELTA
    inline clock_t getDelta() const { return mDelta; }
    inline void resetDelta() { mStartTime = clock(); }
#endif
    float mTickPerSecond;

    ////// Inputs
    void resetInputs();

    AccelData* mAccelData;
    float mAccelPrecision;

    TouchData* mTouchData;
    unsigned char mTouchCount;
    short mTouchPrecision;

#ifdef LIBENG_ENABLE_GAMEPAD
    ////// Gamepad
    mutable Gamepad* mGamePad;
#endif

private:
    Inputs* mInputs;
    const ShaderParam* mShaderParam;

#ifdef LIBENG_ENABLE_DELTA
    clock_t mStartTime;
    clock_t mDelta;
#endif

#ifdef LIBENG_ENABLE_FPS
    boost::thread* mFPS;
    unsigned char mRenderCount;
    bool mAbortFPS;

    void fpsThreadRunning();
    static void startThreadFPS(Game* game);
#endif
#ifdef LIBENG_ENABLE_STORAGE
    bool save();
#endif

};

} // namespace

#endif // __cplusplus
#endif // LIBENG_GAME_H_
