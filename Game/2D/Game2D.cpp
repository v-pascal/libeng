#include "Game2D.h"

#include <libeng/Global.h>

#include <libeng/Log/Log.h>
#include <libeng/Graphic/Object/2D/Dynamic2D.h>
#include <libeng/Graphic/Object/2D/Object2D.h>

namespace eng {

float* Object2D::TransformBuffer = NULL;

//////
Game2D::Game2D(unsigned char level) : Game(level), mShaderParam2D(NULL), mGameIntro(NULL) {

    LOGV(LIBENG_LOG_GAME, 0, LOG_FORMAT(" - l:%d"), __PRETTY_FUNCTION__, __LINE__, level);

    Object2D::TransformBuffer = new float[TRANSFORM_BUFFER_SIZE];
    std::memcpy(Object2D::TransformBuffer, DEFAULT_TRANSFORM_BUFFER, sizeof(float) * TRANSFORM_BUFFER_SIZE);
}
Game2D::~Game2D() {

    LOGV(LIBENG_LOG_GAME, 0, LOG_FORMAT(), __PRETTY_FUNCTION__, __LINE__);
    assert(!mGameIntro); // Should be deleted in 'Game::destroy' method

    delete [] Object2D::TransformBuffer;
}

void Game2D::updateIntro() const {

    LOGV(LIBENG_LOG_GAME, 0, LOG_FORMAT(), __PRETTY_FUNCTION__, __LINE__);
    assert(!mGameLevel);
    assert(mGameIntro);

    mGameIntro->update();
}
void Game2D::renderIntro() const {

    LOGV(LIBENG_LOG_GAME, 0, LOG_FORMAT(), __PRETTY_FUNCTION__, __LINE__);
    assert(!mGameLevel);
    assert(mGameIntro);

    mGameIntro->render();
    if (mGameIntro->isFinished()) {

        delete mGameIntro;
        mGameIntro = NULL;

        // Remove logo texture
        mTextures->rmvTextures(1);
        ++mGameLevel;
    }
}

void Game2D::pause() {

    LOGV(LIBENG_LOG_GAME, 0, LOG_FORMAT(), __PRETTY_FUNCTION__, __LINE__);
    Game::pause();

    if ((mGameIntro) && (!mGameLevel))
        mGameIntro->pause();
}

bool Game2D::start() {

    LOGV(LIBENG_LOG_GAME, 0, LOG_FORMAT(), __PRETTY_FUNCTION__, __LINE__);
    Game::start();

    static bool bStarted = false;
    if (!bStarted) {

        ////// Start operation
        if (mScreen->height > mScreen->width) { // Portrait orientation

            LOGI(LIBENG_LOG_GAME, 0, LOG_FORMAT(" - Portrait orientation"), __PRETTY_FUNCTION__, __LINE__);
            float screenRatio = static_cast<float>(mScreen->height) / mScreen->width;

            Object2D::TransformBuffer[Dynamic2D::RATIO_Y] = 1.f / screenRatio;
#ifndef LIBENG_PORT_AS_LAND
            mScreen->left = -1.f;
            mScreen->right = 1.f;
            mScreen->top = screenRatio;
            mScreen->bottom = -screenRatio;
#else
            mScreen->left = -screenRatio;
            mScreen->right = screenRatio;
            mScreen->top = 1.f;
            mScreen->bottom = -1.f;

            std::swap<short>(mScreen->width, mScreen->height);
#endif
        }
        else { // Landscape orientation

#ifdef LIBENG_LAND_ORIENTATION
            LOGI(LIBENG_LOG_GAME, 0, LOG_FORMAT(" - Landscape orientation"), __PRETTY_FUNCTION__, __LINE__);
            float screenRatio = static_cast<float>(mScreen->width) / mScreen->height;

            Object2D::TransformBuffer[Dynamic2D::RATIO_X] = 1.f / screenRatio;

            mScreen->left = -screenRatio;
            mScreen->right = screenRatio;
            mScreen->top = 1.f;
            mScreen->bottom = -1.f;
#else
            LOGF(LOG_FORMAT(" - Landscape orientation not allowed!"), __PRETTY_FUNCTION__, __LINE__);
            assert(NULL);
#endif
        }
        LOGI(LIBENG_LOG_GAME, 0, LOG_FORMAT(" - Screen: w:%d; h:%d"), __PRETTY_FUNCTION__, __LINE__, mScreen->width,
                mScreen->height);
        mScreen->ratioW = 2.f * mScreen->right / mScreen->width;
        mScreen->ratioH = 2.f * mScreen->top / mScreen->height;

        // Get logo texture to start introduction (if needed)
        if ((mGameIntro) && (!mGameLevel)) {

            LOGI(LIBENG_LOG_GAME, 0, LOG_FORMAT(" - Start introduction"), __PRETTY_FUNCTION__, __LINE__);
            mGameIntro->start(false);
        }

        bStarted = true;
        return true;
    }

    ////// Resume operation
#ifdef __ANDROID__
#ifdef LIBENG_PORT_AS_LAND
    // Switch screen resolution coz they have been reassigned
    std::swap<short>(mScreen->width, mScreen->height);
#endif
#endif
    // Restart introduction (if needed)
    if ((mGameIntro) && (!mGameLevel)) {

        LOGI(LIBENG_LOG_GAME, 0, LOG_FORMAT(" - Restart introduction"), __PRETTY_FUNCTION__, __LINE__);
        mGameIntro->start(true);
    }
    return false; // Already started
}
void Game2D::destroy() {

    LOGV(LIBENG_LOG_GAME, 0, LOG_FORMAT(), __PRETTY_FUNCTION__, __LINE__);
    Game::destroy();

    if (mGameIntro) delete mGameIntro;
    mGameIntro = NULL;
}

} // namespace
