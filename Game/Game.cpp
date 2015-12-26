#include <libeng/Game/Game.h>

#include <libeng/Graphic/Object/Dynamic.h>
#include <libeng/Storage/Storage.h>
#include <libeng/Tools/Tools.h>

namespace eng {

Game* Game::mThis = NULL;

//////
Game::Game(unsigned char level) : mGameLevel(level), mScreen(NULL), mInputs(NULL), mTextures(NULL), mAccelData(NULL),
        mAccelPrecision(DEFAULT_ACCEL_PRECISION), mTouchData(NULL), mTouchCount(0), mShaderParam(NULL),
        mTouchPrecision(static_cast<short>(DEFAULT_TOUCH_PRECISION)), mTickPerSecond(LIBENG_NO_DATA) {

    LOGV(LIBENG_LOG_GAME, 0, LOG_FORMAT(" - s:%d"), __PRETTY_FUNCTION__, __LINE__, level);
#ifdef LIBENG_ENABLE_STORAGE
    mGameData = NULL;
#endif

#ifdef LIBENG_ENABLE_TEXT
    mFonts = Fonts::getInstance();
#endif

#ifdef LIBENG_ENABLE_DELTA
    mStartTime = 0;
    mDelta = 0;
#endif

#ifdef DEBUG
    mLog = 0;
#endif

#ifdef LIBENG_ENABLE_FPS
    mAbortFPS = false;
    mRenderCount = 0;
    mFPS = NULL;
#endif

#ifdef LIBENG_ENABLE_GAMEPAD
    mGamePad = NULL;
#endif

#ifdef __ANDROID__
    mScreenLocked = false;
#endif
}
Game::~Game() {

    LOGV(LIBENG_LOG_GAME, 0, LOG_FORMAT(), __PRETTY_FUNCTION__, __LINE__);
#ifdef LIBENG_ENABLE_TEXT
    Fonts::freeInstance();
#endif
    if (mTouchData)
        delete [] mTouchData;
    if (mAccelData)
        delete mAccelData;

#ifdef LIBENG_ENABLE_GAMEPAD
    if (mGamePad)
        delete mGamePad;
#endif
#ifdef LIBENG_ENABLE_STORAGE
    if (mGameData)
        delete mGameData;
#endif

#ifdef LIBENG_ENABLE_FPS
    LOGI(LIBENG_LOG_GAME, 0, LOG_FORMAT(" - Abort FPS logs"), __PRETTY_FUNCTION__, __LINE__);
    if (!mFPS) {

        mAbortFPS = true;
        LOGW(LOG_FORMAT(" - FPS log thread already aborted"), __PRETTY_FUNCTION__, __LINE__);
        //assert(NULL); // Pause/Resume issue
        return;
    }
    mAbortFPS = true;

    mFPS->join();
    delete mFPS;

#endif // LIBENG_ENABLE_FPS
}

void Game::getInputsData() {

#ifdef DEBUG
    ++mLog; // BUG: Can't increase it as following - (mLog++ % 100) or (++mLog % 100) not working !?!?!
    LOGV(LIBENG_LOG_GAME, (mLog % 100), LOG_FORMAT(" - (i: 0x%x)"), __PRETTY_FUNCTION__, __LINE__, Inputs::InputUse);
#endif
    mInputs->lock();

    ////// Touch
    if (Inputs::InputUse & USE_INPUT_TOUCH) {
        if (mTouchData) {

            unsigned char touchIdx = 0;
            while (mInputs->get(Inputs::TOUCH, static_cast<void*>(&mTouchData[touchIdx]), touchIdx))
                ++touchIdx;

            mTouchCount = touchIdx;
        }
        else {

            mTouchCount = 0;
            TouchInput* input = static_cast<TouchInput*>(mInputs->get(Inputs::TOUCH));
            if (input) {

                LOGI(LIBENG_LOG_GAME, 0, LOG_FORMAT(" - Set touch precision: %d"), __PRETTY_FUNCTION__, __LINE__,
                        mTouchPrecision);
                input->setPrecision(static_cast<void*>(&mTouchPrecision));

                mTouchData = new TouchData[MAX_TOUCH_COUNT];
                unsigned char touchIdx = 0;
                while (mInputs->get(Inputs::TOUCH, static_cast<void*>(&mTouchData[touchIdx]), touchIdx))
                    ++touchIdx;

                mTouchCount = touchIdx;
            }
        }
    }

    ////// Accelerometer
    if (Inputs::InputUse & USE_INPUT_ACCEL) {
        if (mAccelData)
            mInputs->get(Inputs::ACCEL, static_cast<void*>(mAccelData));
        else {

            AccelInput* input = static_cast<AccelInput*>(mInputs->get(Inputs::ACCEL));
            if (input) {

                LOGI(LIBENG_LOG_GAME, 0, LOG_FORMAT(" - Set accel precision: %f"), __PRETTY_FUNCTION__, __LINE__,
                        mAccelPrecision);
                input->setPrecision(static_cast<void*>(&mAccelPrecision));

                mAccelData = new AccelData;
                mInputs->get(Inputs::ACCEL, static_cast<void*>(mAccelData));
            }
        }
    }

    mInputs->purge();
    mInputs->unlock();

#ifdef LIBENG_ENABLE_GAMEPAD
    if (mGamePad) // To disable 'Gamepad::refresh' method call when not needed set it to NULL
        mGamePad->refresh();
#endif
}
void Game::resetInputs() {

    LOGV(LIBENG_LOG_GAME, 0, LOG_FORMAT(), __PRETTY_FUNCTION__, __LINE__);
    Inputs::InputUse = 0; // Lock any others input entries
    mInputs->lock();

    ////// Touch
    if (mTouchData) {
        delete [] mTouchData;
        mTouchData = NULL;
    }
    mTouchCount = 0;
    mTouchPrecision = DEFAULT_TOUCH_PRECISION;

    ////// Accelerometer
    if (mAccelData) {
        delete mAccelData;
        mAccelData = NULL;
    }
    mAccelPrecision = DEFAULT_ACCEL_PRECISION;

    mInputs->free(); // Free all existing previous inputs data (if any)
    mInputs->unlock();
}

void Game::updateFrame() {

#ifdef DEBUG
    LOGV(LIBENG_LOG_GAME, (mLog % 100), LOG_FORMAT(), __PRETTY_FUNCTION__, __LINE__);
#endif

    // Update frame
    getInputsData();
    update();
    
#ifdef LIBENG_ENABLE_DELTA
    clock_t curTime = clock();
    mDelta = delta<clock_t>(mStartTime, curTime);
    mStartTime = curTime;
#endif
}
void Game::renderFrame() {

#ifdef DEBUG
    LOGV(LIBENG_LOG_GAME, (mLog % 100), LOG_FORMAT(), __PRETTY_FUNCTION__, __LINE__);
#endif

    // Render
    render();

#ifdef LIBENG_ENABLE_FPS
    ++mRenderCount;
#endif
}

#ifdef LIBENG_ENABLE_STORAGE
bool Game::save() {

    LOGV(LIBENG_LOG_GAME, 0, LOG_FORMAT(), __PRETTY_FUNCTION__, __LINE__);
    if ((mGameData) && (g_GameFile)) {

        // Save data into Game file
        if (!Storage::saveFile(g_GameFile, mGameData)) {

            LOGW(LOG_FORMAT(" - Failed to save data into '%s' file"), __PRETTY_FUNCTION__, __LINE__, g_GameFile);
            assert(NULL);
            return false;
        }
    }
    return true;
}
#endif
#ifdef __ANDROID__
void Game::lockScreen() {

    LOGV(LIBENG_LOG_GAME, 0, LOG_FORMAT(), __PRETTY_FUNCTION__, __LINE__);
    assert(!mScreenLocked);

    mScreenLocked = true;
}
#endif
void Game::pause() {

    LOGV(LIBENG_LOG_GAME, 0, LOG_FORMAT(), __PRETTY_FUNCTION__, __LINE__);
    mTextures->delTextures();
    mTextures->rmvTextures();

#ifdef LIBENG_ENABLE_STORAGE
    save();
#endif

#ifdef LIBENG_ENABLE_FPS
    LOGI(LIBENG_LOG_GAME, 0, LOG_FORMAT(" - Abort FPS logs"), __PRETTY_FUNCTION__, __LINE__);
    if (!mFPS) {

        mAbortFPS = true;
        LOGW(LOG_FORMAT(" - FPS log thread already aborted"), __PRETTY_FUNCTION__, __LINE__);
        //assert(NULL); // Pause/Resume issue
        return;
    }
    mAbortFPS = true;

    mFPS->join();
    delete mFPS;

    mFPS = NULL;
#endif
}
void Game::destroy() {

    LOGV(LIBENG_LOG_GAME, 0, LOG_FORMAT(), __PRETTY_FUNCTION__, __LINE__);
#ifdef LIBENG_ENABLE_STORAGE
    save();
#endif
}

#ifdef LIBENG_ENABLE_FPS
void  Game::fpsThreadRunning() {

    LOGV(LIBENG_LOG_GAME, 0, LOG_FORMAT(" - Start loop"), __PRETTY_FUNCTION__, __LINE__);
    while (!mAbortFPS) {

        boost::this_thread::sleep(boost::posix_time::milliseconds(1000));
        if (mRenderCount)
            LOGW(LOG_FORMAT(" - FPS: %d"), __PRETTY_FUNCTION__, __LINE__, mRenderCount);
        mRenderCount = 0;
    }
    LOGV(LIBENG_LOG_GAME, 0, LOG_FORMAT(" - Finished"), __PRETTY_FUNCTION__, __LINE__);
}
void  Game::startThreadFPS(Game* game) {

    LOGV(LIBENG_LOG_GAME, 0, LOG_FORMAT(" - g:%p"), __PRETTY_FUNCTION__, __LINE__, game);
    game->fpsThreadRunning();
}
#endif

} // namespace
