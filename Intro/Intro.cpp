#include "Intro.h"
#include "IntroCoords.h"

#ifndef __ANDROID__
#include "OpenGLES/ES2/gl.h"
#endif

#include <libeng/Features/Internet/Internet.h>
#include <libeng/Features/Bluetooth/Bluetooth.h>
#include <libeng/Game/2D/Game2D.h>
#include <boost/thread.hpp>

#define SCREEN_LOGO_RATIO       0.8f // Logo can takes 80% of the screen (in overflow cases)
#define CHANGE_SIDE_COUNT       4 // How many time the dog change the side he looks (even nnumber)

#define ALPHA_VELOCITY          0.02f
#define TRANS_VELOCITY          0.05f

#define WAIT_CONN_SIZE           (2.f / 3.f) // 2/3 of half screen width

static const short gs_presentsEN[] = { PRESENTS_EN_LEFT, PRESENTS_EN_TOP, PRESENTS_EN_RIGHT, PRESENTS_EN_BOTTOM };
static const short gs_presentsFR[] = { PRESENTS_FR_LEFT, PRESENTS_FR_TOP, PRESENTS_FR_RIGHT, PRESENTS_FR_BOTTOM };
static const short gs_presentsDE[] = { PRESENTS_DE_LEFT, PRESENTS_DE_TOP, PRESENTS_DE_RIGHT, PRESENTS_DE_BOTTOM };
static const short gs_presentsES[] = { PRESENTS_ES_LEFT, PRESENTS_ES_TOP, PRESENTS_ES_RIGHT, PRESENTS_ES_BOTTOM };
static const short gs_presentsPT[] = { PRESENTS_PT_LEFT, PRESENTS_PT_TOP, PRESENTS_PT_RIGHT, PRESENTS_PT_BOTTOM };

static const float gs_dogBackward[] = { DOG_LEFT / LOGO_SIZE, DOG_TOP / LOGO_SIZE, DOG_LEFT / LOGO_SIZE,
        DOG_BOTTOM / LOGO_SIZE, DOG_RIGHT / LOGO_SIZE, DOG_BOTTOM / LOGO_SIZE, DOG_RIGHT / LOGO_SIZE,
        DOG_TOP / LOGO_SIZE };
static const float gs_dogForward[] = { DOG_RIGHT / LOGO_SIZE, DOG_TOP / LOGO_SIZE, DOG_RIGHT / LOGO_SIZE,
        DOG_BOTTOM / LOGO_SIZE, DOG_LEFT / LOGO_SIZE, DOG_BOTTOM / LOGO_SIZE, DOG_LEFT / LOGO_SIZE,
        DOG_TOP / LOGO_SIZE };

namespace eng {

//////
WaitConn::WaitConn(bool wifi) : mMainStep(0), mConnected(false), mStoryStep(0), mForward(true), mWifi(wifi) {

#ifdef DEBUG
    LOGV(LIBENG_LOG_INTRO, 0, LOG_FORMAT(" - w:%s"), __PRETTY_FUNCTION__, __LINE__, (wifi)? "true":"false");
    mLog = NULL;
#endif
    mScreen = NULL;
}
WaitConn::~WaitConn() { LOGV(LIBENG_LOG_INTRO, 0, LOG_FORMAT(), __PRETTY_FUNCTION__, __LINE__); }

void WaitConn::initialize(const Game2D* game) {

#ifdef DEBUG
    LOGV(LIBENG_LOG_INTRO, 0, LOG_FORMAT(" - g:%p"), __PRETTY_FUNCTION__, __LINE__, game);
    mLog = game->getLog();
#endif
    mWarning.initialize(game);
    mConnL.initialize(game);
    mConnR.initialize(game);
    mDog.initialize(game);
    mPanel.initialize(game);

    mScreen = game->getScreen();
}

void WaitConn::start(const Screen* screen) {

    LOGV(LIBENG_LOG_INTRO, 0, LOG_FORMAT(" - s:%p"), __PRETTY_FUNCTION__, __LINE__, screen);
    mWarning.start(TEXTURE_ID_LOGO);

#if defined(LIBENG_LAND_ORIENTATION) || defined(LIBENG_PORT_AS_LAND)
    assert(screen->width > screen->height); // Portrait orientation as landscape (width & height have been swapped), or
                                            // landscape orientation

    short warnH = (((WARNING_BOTTOM - WARNING_TOP) * (screen->height >> 1)) / (WARNING_RIGHT - WARNING_LEFT)) >> 1; // Half
    mWarning.setVertices((screen->width >> 1) - (screen->height >> 2), (screen->height >> 1) + warnH, (screen->width >> 1) +
            (screen->height >> 2), (screen->height >> 1) - warnH);
#else
    assert(screen->height > screen->width); // Portrait orientation

    short warnH = (((WARNING_BOTTOM - WARNING_TOP) * (screen->width >> 1)) / (WARNING_RIGHT - WARNING_LEFT)) >> 1; // Half
    mWarning.setVertices(screen->width >> 2, (screen->height >> 1) + warnH, screen->width - (screen->width >> 2),
            (screen->height >> 1) - warnH);
#endif
    mWarning.size(WAIT_CONN_SIZE, WAIT_CONN_SIZE);

    float coords[8] = { WARNING_LEFT / LOGO_SIZE, WARNING_TOP / LOGO_SIZE, WARNING_LEFT / LOGO_SIZE,
            WARNING_BOTTOM / LOGO_SIZE, WARNING_RIGHT / LOGO_SIZE, WARNING_BOTTOM / LOGO_SIZE,
            WARNING_RIGHT / LOGO_SIZE, WARNING_TOP / LOGO_SIZE };
    mWarning.setTexCoords(coords);

    mConnR.start(TEXTURE_ID_LOGO);

    short wifiW = (mWifi)? ((WIFI_RIGHT - WIFI_LEFT) * warnH) / (WIFI_BOTTOM - WIFI_TOP):
            ((BLUETOOTH_RIGHT - BLUETOOTH_LEFT) * warnH) / (BLUETOOTH_BOTTOM - BLUETOOTH_TOP); // Half
    mConnR.setVertices((screen->width >> 1) - wifiW, (screen->height >> 1) + warnH, (screen->width >> 1) + wifiW,
            (screen->height >> 1) - warnH);
    mConnR.size(WAIT_CONN_SIZE, WAIT_CONN_SIZE);

    if (mWifi) {
        coords[0] = WIFI_LEFT / LOGO_SIZE;
        coords[1] = WIFI_TOP / LOGO_SIZE;
        coords[3] = WIFI_BOTTOM / LOGO_SIZE;
        coords[4] = WIFI_RIGHT / LOGO_SIZE;
    }
    else { // Bluetooth

        coords[0] = BLUETOOTH_LEFT / LOGO_SIZE;
        coords[1] = BLUETOOTH_TOP / LOGO_SIZE;
        coords[3] = BLUETOOTH_BOTTOM / LOGO_SIZE;
        coords[4] = BLUETOOTH_RIGHT / LOGO_SIZE;
    }
    coords[2] = coords[0];
    coords[5] = coords[3];
    coords[6] = coords[4];
    coords[7] = coords[1];
    mConnR.setTexCoords(coords);

    mConnL.start(TEXTURE_ID_LOGO);
    mConnL.copyVertices(mConnR);

    if (mWifi) {
        coords[0] = WIFI_RIGHT / LOGO_SIZE;
        coords[4] = WIFI_LEFT / LOGO_SIZE;
        coords[2] = coords[0];
        coords[6] = coords[4];
        mConnL.setTexCoords(coords);
    }
    else
        mConnL.copyTexCoords(mConnR);

    mDog.start(TEXTURE_ID_LOGO);

    if (mWifi) {
        warnH = (((DOG_BOTTOM - DOG_TOP) * (warnH << 1)) / (WIFI_BOTTOM - WIFI_TOP)) >> 1; // Half
        wifiW = (((DOG_RIGHT - DOG_LEFT) * (wifiW << 1)) / (WIFI_RIGHT - WIFI_LEFT)) >> 1; // Half
    }
    else {
        warnH = (((DOG_BOTTOM - DOG_TOP) * (warnH << 1)) / (BLUETOOTH_BOTTOM - BLUETOOTH_TOP)) >> 1; // Half
        wifiW = (((DOG_RIGHT - DOG_LEFT) * (wifiW << 1)) / (BLUETOOTH_RIGHT - BLUETOOTH_LEFT)) >> 1; // Half
    }
    mDog.setVertices((screen->width >> 1) - wifiW, (screen->height >> 1) + warnH, (screen->width >> 1) + wifiW,
            (screen->height >> 1) - warnH);
    mDog.size(WAIT_CONN_SIZE, WAIT_CONN_SIZE);
    mDog.setTexCoords(gs_dogForward);

    mPanel.start(TEXTURE_ID_LOGO);
    coords[0] = -0.5f;
    coords[1] = 0.5f;
    coords[2] = -0.5f;
    coords[3] = -0.5f;
    coords[4] = 0.5f;
    coords[5] = -0.5f;
    coords[6] = 0.5f;
    coords[7] = 0.5f;
    mPanel.setVertices(coords);

    coords[0] = BLACK_DOT_LEFT / LOGO_SIZE;
    coords[1] = BLACK_DOT_TOP / LOGO_SIZE;
    coords[2] = coords[0];
    coords[3] = BLACK_DOT_BOTTOM / LOGO_SIZE;
    coords[4] = BLACK_DOT_RIGHT / LOGO_SIZE;
    coords[5] = coords[3];
    coords[6] = coords[4];
    coords[7] = coords[1];
    mPanel.setTexCoords(coords);

    //
    reset();
    mPanel.translate(-0.5f, 0.f);
}
void WaitConn::resume() {

    LOGV(LIBENG_LOG_INTRO, 0, LOG_FORMAT(), __PRETTY_FUNCTION__, __LINE__);
    mWarning.resume(TEXTURE_ID_LOGO);
    mConnL.resume(TEXTURE_ID_LOGO);
    mConnR.resume(TEXTURE_ID_LOGO);
    mDog.resume(TEXTURE_ID_LOGO);
    mPanel.resume(TEXTURE_ID_LOGO);
}

void WaitConn::moveDog() {

#ifdef DEBUG
    LOGV(LIBENG_LOG_INTRO, (*mLog % 100), LOG_FORMAT(" - (d:%s; s:%d; x:%f; y:%f)"), __PRETTY_FUNCTION__, __LINE__,
         (mForward)? "true":"false", mMainStep, mDog.getTransform()[Dynamic2D::TRANS_X],
         mDog.getTransform()[Dynamic2D::TRANS_Y]);
#endif
    static unsigned char wait = 0;
    static unsigned char count = 0;

    switch (mMainStep) {
        case 1: {
            if (mForward) {

                mDog.setTexCoords(gs_dogForward);

#if defined(LIBENG_LAND_ORIENTATION) || defined(LIBENG_PORT_AS_LAND)
                mDog.translate(((mDog.getLeft() - mDog.getRight()) / 2.f) + mScreen->left, 0.f);
#else
                mDog.translate(((mDog.getLeft() - mDog.getRight()) / 2.f) - 1.f, 0.f);
#endif
            }
            else {

                mDog.setTexCoords(gs_dogBackward);
#if defined(LIBENG_LAND_ORIENTATION) || defined(LIBENG_PORT_AS_LAND)
                mDog.translate(((mDog.getRight() - mDog.getLeft()) / 2.f) + mScreen->right, 0.f);
#else
                mDog.translate(((mDog.getRight() - mDog.getLeft()) / 2.f) + 1.f, 0.f);
#endif
            }
            mDog.setAlpha(1.f);
            break;
        }
        case 2: {

#if defined(LIBENG_LAND_ORIENTATION) || defined(LIBENG_PORT_AS_LAND)
#ifdef __ANDROID__
            if (((mForward) && (mDog.getTransform()[Dynamic2D::TRANS_X] > (mScreen->left / 2.f))) ||
                    ((!mForward) && (mDog.getTransform()[Dynamic2D::TRANS_X] < (mScreen->right / 2.f)))) {
#else
            if (((!mForward) && (mDog.getTransform()[Dynamic2D::TRANS_X] > (mScreen->left / 2.f))) ||
                    ((mForward) && (mDog.getTransform()[Dynamic2D::TRANS_X] < (mScreen->right / 2.f)))) {
#endif
#else
            if (((mForward) && (mDog.getTransform()[Dynamic2D::TRANS_X] < 0.5f)) ||
                    ((!mForward) && (mDog.getTransform()[Dynamic2D::TRANS_X] > -0.5f))) {
#endif
                mDog.translate((mForward)? TRANS_VELOCITY:-TRANS_VELOCITY, 0.f);
                --mMainStep;
            }
            else {

                wait = 0;
                count = 0;
            }
            break;
        }
        case 3: {
            if (++wait == LIBENG_WAITCONN_DELAY) {

                (mForward)? mDog.setTexCoords(gs_dogBackward):mDog.setTexCoords(gs_dogForward);
                mForward = !mForward;
                wait = 0;

                if (++count == CHANGE_SIDE_COUNT)
                    break;
            }
            --mMainStep;
            break;
        }
        case 4: {
            if (++wait == LIBENG_WAITCONN_DELAY)
                break;

            --mMainStep;
            break;
        }
        case 5: {

#if defined(LIBENG_LAND_ORIENTATION) || defined(LIBENG_PORT_AS_LAND)
#ifdef __ANDROID__
            if (((mForward) && (mDog.getTransform()[Dynamic2D::TRANS_X] > (mScreen->left + ((mDog.getLeft() - mDog.getRight()) / 2.f)))) ||
                    ((!mForward) && (mDog.getTransform()[Dynamic2D::TRANS_X] < (((mDog.getRight() - mDog.getLeft()) / 2.f) + mScreen->right)))) {
#else
            if (((!mForward) && (mDog.getTransform()[Dynamic2D::TRANS_X] > (mScreen->left + ((mDog.getLeft() - mDog.getRight()) / 2.f)))) ||
                    ((mForward) && (mDog.getTransform()[Dynamic2D::TRANS_X] < (((mDog.getRight() - mDog.getLeft()) / 2.f) + mScreen->right)))) {
#endif
#else
            if (((mForward) && (mDog.getTransform()[Dynamic2D::TRANS_X] < (1.f + ((mDog.getRight() - mDog.getLeft()) / 2.f)))) ||
                    ((!mForward) && (mDog.getTransform()[Dynamic2D::TRANS_X] > (((mDog.getLeft() - mDog.getRight()) / 2.f) - 1.f)))) {
#endif
                mDog.translate((mForward)? TRANS_VELOCITY:-TRANS_VELOCITY, 0.f);
                --mMainStep;

                wait = 0;
            }
            break;
        }
        case 6: {
            if (++wait == LIBENG_WAITCONN_DELAY)
                break;

            --mMainStep;
            break;
        }
        default: {

            LOGF(LOG_FORMAT(" - Unexpected step: %d"), __PRETTY_FUNCTION__, __LINE__, mMainStep);
            assert(NULL);
            break;
        }
    }
}
void WaitConn::exitConn() {

#ifdef DEBUG
    LOGV(LIBENG_LOG_INTRO, (*mLog % 100), LOG_FORMAT(" - (b:%s; s:%d)"), __PRETTY_FUNCTION__, __LINE__,
            (!mForward)? "true":"false", mMainStep);
#endif
    static unsigned char wait = 0;

    switch (mMainStep) {
        case 7: {
            if (mForward) {

                mPanel.translate(1.f, 0.f);
                mConnL.translate(mConnL.getRight() - mConnL.getLeft(), 0.f);
                mConnR.translate(mConnR.getRight() - mConnR.getLeft(), 0.f);
            }
            else {

                mPanel.translate(-1.f, 0.f);
                mConnL.translate(mConnL.getLeft() - mConnL.getRight(), 0.f);
                mConnR.translate(mConnR.getLeft() - mConnR.getRight(), 0.f);
            }
            if (mWifi) {
                mConnL.setAlpha(1.f);
                mConnR.setAlpha(1.f);
            }
            else { // Bluetooth

                mConnL.setAlpha((mForward)? 1.f:0.f);
                mConnR.setAlpha((mForward)? 0.f:1.f);
            }
            break;
        }
        case 8: {

#if defined(LIBENG_LAND_ORIENTATION) || defined(LIBENG_PORT_AS_LAND)
#ifdef __ANDROID__
            if (((mForward) && (mConnL.getTransform()[Dynamic2D::TRANS_X] < ((mConnL.getRight() - mConnL.getLeft()) / 2.f))) ||
                    ((!mForward) && (mConnR.getTransform()[Dynamic2D::TRANS_X] > ((mConnL.getLeft() - mConnL.getRight()) / 2.f)))) {
#else
            if (((!mForward) && (mConnR.getTransform()[Dynamic2D::TRANS_X] < ((mConnL.getRight() - mConnL.getLeft()) / 2.f))) ||
                    ((mForward) && (mConnL.getTransform()[Dynamic2D::TRANS_X] > ((mConnL.getLeft() - mConnL.getRight()) / 2.f)))) {
#endif
#else
            if (((mForward) && (mConnL.getTransform()[Dynamic2D::TRANS_X] > ((mConnL.getLeft() - mConnL.getRight()) / 2.f))) ||
                    ((!mForward) && (mConnR.getTransform()[Dynamic2D::TRANS_X] < ((mConnL.getRight() - mConnL.getLeft()) / 2.f)))) {
#endif
                mConnL.translate((mForward)? -TRANS_VELOCITY:TRANS_VELOCITY, 0.f);
                mConnR.translate((mForward)? -TRANS_VELOCITY:TRANS_VELOCITY, 0.f);
                --mMainStep;
            }
            else
                wait = 0;
            break;
        }
        case 9: {
            if (++wait == LIBENG_WAITCONN_DELAY)
                break;

            --mMainStep;
            break;
        }
        case 10: {

#if defined(LIBENG_LAND_ORIENTATION) || defined(LIBENG_PORT_AS_LAND)
#ifdef __ANDROID__
            if (((mForward) && (mConnL.getTransform()[Dynamic2D::TRANS_X] < (((mConnL.getRight() - mConnL.getLeft()) * 3.f / 2.f) + mScreen->right))) ||
                    ((!mForward) && (mConnR.getTransform()[Dynamic2D::TRANS_X] > (((mConnL.getLeft() - mConnL.getRight()) * 3.f / 2.f) + mScreen->left)))) {
#else
            if (((!mForward) && (mConnR.getTransform()[Dynamic2D::TRANS_X] < (((mConnL.getRight() - mConnL.getLeft()) * 3.f / 2.f) + mScreen->right))) ||
                    ((mForward) && (mConnL.getTransform()[Dynamic2D::TRANS_X] > (((mConnL.getLeft() - mConnL.getRight()) * 3.f / 2.f) + mScreen->left)))) {
#endif
#else
            if (((mForward) && (mConnL.getTransform()[Dynamic2D::TRANS_X] > (((mConnL.getLeft() - mConnL.getRight()) * 3.f / 2.f) - 1.f))) ||
                    ((!mForward) && (mConnR.getTransform()[Dynamic2D::TRANS_X] < (((mConnL.getRight() - mConnL.getLeft()) * 3.f / 2.f) + 1.f)))) {
#endif
                mConnL.translate((mForward)? -TRANS_VELOCITY:TRANS_VELOCITY, 0.f);
                mConnR.translate((mForward)? -TRANS_VELOCITY:TRANS_VELOCITY, 0.f);
                --mMainStep;
            }
            else
                wait = 0;
            break;
        }
        case 11: {
            if (++wait == LIBENG_WAITCONN_DELAY)
                break;

            --mMainStep;
            break;
        }
        default: {

            LOGF(LOG_FORMAT(" - Unexpected step: %d"), __PRETTY_FUNCTION__, __LINE__, mMainStep);
            assert(NULL);
            break;
        }
    }
}
void WaitConn::hideConn() {

#ifdef DEBUG
    LOGV(LIBENG_LOG_INTRO, (*mLog % 100), LOG_FORMAT(" - f:%s (s:%d)"), __PRETTY_FUNCTION__, __LINE__,
            (mForward)? "true":"false", mMainStep);
#endif
    static unsigned char wait = 0;

    switch (mMainStep) {
        case 12: {
            if (mForward) {

#if defined(LIBENG_LAND_ORIENTATION) || defined(LIBENG_PORT_AS_LAND)
                mConnL.translate(mConnL.getLeft() - mConnL.getRight() + mScreen->left, 0.f);
                mConnR.translate(mConnR.getLeft() - mConnR.getRight() + mScreen->left, 0.f);
#else
                mConnL.translate(mConnL.getLeft() - mConnL.getRight() - 1.f, 0.f);
                mConnR.translate(mConnR.getLeft() - mConnR.getRight() - 1.f, 0.f);
#endif
            }
            else {

#if defined(LIBENG_LAND_ORIENTATION) || defined(LIBENG_PORT_AS_LAND)
                mConnL.translate((mConnL.getRight() - mConnL.getLeft()) + mScreen->right, 0.f);
                mConnR.translate((mConnR.getRight() - mConnR.getLeft()) + mScreen->right, 0.f);
#else
                mConnL.translate((mConnL.getRight() - mConnL.getLeft()) + 1.f, 0.f);
                mConnR.translate((mConnR.getRight() - mConnR.getLeft()) + 1.f, 0.f);
#endif
            }
            if (mWifi) {
                mConnL.setAlpha(1.f);
                mConnR.setAlpha(1.f);
            }
            else {
                mConnL.setAlpha((mForward)? 0.f:1.f);
                mConnR.setAlpha((mForward)? 1.f:0.f);
            }
            break;
        }
        case 13: {

#if defined(LIBENG_LAND_ORIENTATION) || defined(LIBENG_PORT_AS_LAND)
#ifdef __ANDROID__
            if (((mForward) && (mConnL.getTransform()[Dynamic2D::TRANS_X] > ((mConnL.getLeft() - mConnL.getRight()) / 2.f))) ||
                    ((!mForward) && (mConnR.getTransform()[Dynamic2D::TRANS_X] < ((mConnR.getRight() - mConnR.getLeft()) / 2.f)))) {
#else
            if (((!mForward) && (mConnR.getTransform()[Dynamic2D::TRANS_X] > ((mConnL.getLeft() - mConnL.getRight()) / 2.f))) ||
                    ((mForward) && (mConnL.getTransform()[Dynamic2D::TRANS_X] < ((mConnR.getRight() - mConnR.getLeft()) / 2.f)))) {
#endif
#else
            if (((mForward) && (mConnL.getTransform()[Dynamic2D::TRANS_X] < ((mConnL.getRight() - mConnL.getLeft()) / 2.f))) ||
                    ((!mForward) && (mConnR.getTransform()[Dynamic2D::TRANS_X] > ((mConnR.getLeft() - mConnR.getRight()) / 2.f)))) {
#endif
                mConnL.translate((mForward)? TRANS_VELOCITY:-TRANS_VELOCITY, 0.f);
                mConnR.translate((mForward)? TRANS_VELOCITY:-TRANS_VELOCITY, 0.f);
                --mMainStep;
            }
            else
                wait = 0;
            break;
        }
        case 14: {
            if (++wait == LIBENG_WAITCONN_DELAY)
                break;

            --mMainStep;
            break;
        }
        default: {

            LOGF(LOG_FORMAT(" - Unexpected step: %d"), __PRETTY_FUNCTION__, __LINE__, mMainStep);
            assert(NULL);
            break;
        }
    }
}
void WaitConn::update() {

#ifdef DEBUG
    LOGV(LIBENG_LOG_INTRO, (*mLog % 100), LOG_FORMAT(" - (s:%d)"), __PRETTY_FUNCTION__, __LINE__, (mMainStep + 1));
#endif
    switch (++mMainStep) {
        case 1:
        case 2:
        case 3:
        case 4:
        case 5:
        case 6: {

            assert((!mStoryStep) || (mStoryStep == 2) || (mStoryStep == 4) || (mStoryStep == 6));
            moveDog();
            break;
        }
        case 7: {

            ++mStoryStep;
            if ((mStoryStep != 1) && (mStoryStep != 5)) {

                mMainStep = 11;
                reset();

                mForward = (mStoryStep != 7);
                break;
            }
            reset();
            //break;
        }
        case 8:
        case 9:
        case 10:
        case 11: {

            assert((mStoryStep == 1) || (mStoryStep == 5));
            exitConn();

            if (mMainStep == 11) {

                mMainStep = 0;
                reset();

                mForward = (++mStoryStep == 6);
            }
            break;
        }
        case 12:
        case 13:
        case 14: {

            assert((mStoryStep == 3) || (mStoryStep == 7));
            hideConn();

            if (mMainStep == 14) {

                mMainStep = 0;
                reset();

                if (++mStoryStep == 8) {

                    mForward = true;
                    mStoryStep = 0; // Loop
                }
                else
                    mForward = false;
            }
            break;
        }
        default: {

            LOGF(LOG_FORMAT(" - Unexpected step: %d"), __PRETTY_FUNCTION__, __LINE__, mMainStep);
            assert(NULL);
            break;
        }
    }
}
void WaitConn::render() const {

#ifdef DEBUG
    LOGV(LIBENG_LOG_INTRO, (*mLog % 100), LOG_FORMAT(" - s:%d (c:%s)"), __PRETTY_FUNCTION__, __LINE__, mMainStep,
            (mConnected)? "true":"false");
#endif
    if (mConnected)
        return;

    mConnL.render(true);
    mConnR.render(true);

    glDisable(GL_BLEND);
    mPanel.render(true);
    glEnable(GL_BLEND);

    mWarning.render(true);
    mDog.render(true);
}

//////
Intro::Intro() : mDone(false), mLang(LANG_EN), mRatio(1.f), mStep(0), mOnline(false), mGame(NULL), mBluetooth(false),
        mWaitConn(NULL) {

    LOGV(LIBENG_LOG_INTRO, 0, LOG_FORMAT(), __PRETTY_FUNCTION__, __LINE__);
}
Intro::Intro(IntroLang lang) : mDone(false), mLang(lang), mRatio(1.f), mStep(0), mOnline(false), mGame(NULL),
        mBluetooth(false), mWaitConn(NULL) {

    LOGV(LIBENG_LOG_INTRO, 0, LOG_FORMAT(" - l:%d"), __PRETTY_FUNCTION__, __LINE__, static_cast<unsigned char>(lang));
}
Intro::Intro(IntroLang lang, bool online) : mDone(false), mLang(LANG_EN), mRatio(1.f), mStep(0), mOnline(online),
        mGame(NULL), mBluetooth(false), mWaitConn(NULL) {

    LOGV(LIBENG_LOG_INTRO, 0, LOG_FORMAT(" - l:%d; o:%s"), __PRETTY_FUNCTION__, __LINE__, static_cast<unsigned char>(lang),
            (online)? "true":"false");
}
Intro::Intro(IntroLang lang, bool online, bool bluetooth) : mDone(false), mLang(LANG_EN), mRatio(1.f), mStep(0),
        mOnline(online), mGame(NULL), mBluetooth(bluetooth), mWaitConn(NULL) {

    LOGV(LIBENG_LOG_INTRO, 0, LOG_FORMAT(" - l:%d; o:%s; b:%s"), __PRETTY_FUNCTION__, __LINE__,
            static_cast<unsigned char>(lang), (online)? "true":"false",  (bluetooth)? "true":"false");
}
Intro::~Intro() {

    LOGV(LIBENG_LOG_INTRO, 0, LOG_FORMAT(), __PRETTY_FUNCTION__, __LINE__);
    if (mWaitConn) delete mWaitConn;

    Player* player = Player::getInstance();
    player->pause(ALL_TRACK);
    player->free();
    player->clean();
}

void Intro::start(bool resume) {

    LOGV(LIBENG_LOG_INTRO, 0, LOG_FORMAT(" - s:%s"), __PRETTY_FUNCTION__, __LINE__, (resume)? "true":"false");
    if (!resume) { // start

        mLogo.initialize(mGame);
        mLogo.start(TEXTURE_ID_LOGO);
        float coords[8] = {0};

#if defined(LIBENG_LAND_ORIENTATION) || defined(LIBENG_PORT_AS_LAND)
        LOGV(LIBENG_LOG_INTRO, 0, LOG_FORMAT(), __PRETTY_FUNCTION__, __LINE__);
        assert(mGame->getScreen()->width > mGame->getScreen()->height);   // Portrait orientation as landscape (width &
                                                            // height have been swapped), or landscape orientation

        coords[1] = STUDIO_ARTABAN_HEIGHT / static_cast<float>(mGame->getScreen()->height);
        coords[4] = STUDIO_ARTABAN_WIDTH / static_cast<float>(mGame->getScreen()->height);
        // -> Divide by height coz in unit it is 1

        if (coords[1] > mGame->getScreen()->top)
            mRatio = (SCREEN_LOGO_RATIO * mGame->getScreen()->height) / STUDIO_ARTABAN_HEIGHT;
        if ((coords[4] * mRatio) > mGame->getScreen()->right)
            mRatio = (SCREEN_LOGO_RATIO * mGame->getScreen()->height) / STUDIO_ARTABAN_WIDTH;
#else
        LOGV(LIBENG_LOG_INTRO, 0, LOG_FORMAT(), __PRETTY_FUNCTION__, __LINE__);
        assert(mGame->getScreen()->height > mGame->getScreen()->width); // Portrait orientation

        coords[1] = STUDIO_ARTABAN_HEIGHT / static_cast<float>(mGame->getScreen()->width);
        coords[4] = STUDIO_ARTABAN_WIDTH / static_cast<float>(mGame->getScreen()->width);
        // -> Divide by width coz in unit it is 1

        if (coords[4] > mGame->getScreen()->right)
            mRatio = (SCREEN_LOGO_RATIO * mGame->getScreen()->width) / STUDIO_ARTABAN_WIDTH;
        if ((coords[1] * mRatio) > mGame->getScreen()->top)
            mRatio = (SCREEN_LOGO_RATIO * mGame->getScreen()->width) / STUDIO_ARTABAN_HEIGHT;
#endif
        mRatio *= 2.f;
        coords[1] *= mRatio;
        coords[4] *= mRatio;

        coords[6] = coords[4];
        coords[7] = coords[1];
        center(coords);

        LOGI(LIBENG_LOG_INTRO, 0, LOG_FORMAT(), __PRETTY_FUNCTION__, __LINE__);
        assert(Textures::getInstance()->getIndex(TEXTURE_ID_LOGO) == 0);

        std::memset(coords, 0, sizeof(float) * 8);
        coords[3] = 1.f;
        coords[4] = STUDIO_ARTABAN_WIDTH / static_cast<float>((*Textures::getInstance())[0]->width);
        coords[5] = 1.f;
        coords[6] = coords[4];
        mLogo.setTexCoords(coords);

        mLogo.setAlpha(0.f);
    }
    else { // resume

        if (!mWaitConn)
            mLogo.resume(TEXTURE_ID_LOGO);

        else {

            bool online = true;
#ifdef LIBENG_ENABLE_INTERNET
            if (mOnline)
#ifdef __ANDROID__
                online = mWaitConn->mConnected = Internet::isOnline(3000);
#else
                online = mWaitConn->mConnected = Internet::isOnline();
#endif
#else // !LIBENG_ENABLE_INTERNET
#ifdef DEBUG
            if (mOnline) {
                LOGF(LOG_FORMAT(" - LIBENG_ENABLE_INTERNET flag is disabled"), __PRETTY_FUNCTION__, __LINE__);
                assert(NULL);
            }
#endif
#endif
            bool bluetooth = true;
#ifdef LIBENG_ENABLE_BLUETOOTH
            if (mBluetooth)
                bluetooth = mWaitConn->mConnected = Bluetooth::isEnabled();
#else // !LIBENG_ENABLE_BLUETOOTH
#ifdef DEBUG
            if (mBluetooth) {
                LOGF(LOG_FORMAT(" - LIBENG_ENABLE_BLUETOOTH flag is disabled"), __PRETTY_FUNCTION__, __LINE__);
                assert(NULL);
            }
#endif
#endif
            if ((online) && (bluetooth)) {
                mWaitConn->resume();
                mDone = true;
            }
            else if (((!online) && (mWaitConn->isWifiConn())) || ((!bluetooth) && (!mWaitConn->isWifiConn())))
                mWaitConn->resume(); // No change

            else {

                delete mWaitConn;
                mWaitConn = new WaitConn(bluetooth);
                mWaitConn->initialize(mGame);
                mWaitConn->start(mGame->getScreen());
            }
        }
    }
    Player::loadSound(SOUND_ID_LOGO);
}

void Intro::update() {

#ifdef DEBUG
    LOGV(LIBENG_LOG_INTRO, (*mGame->getLog() % 100), LOG_FORMAT(" - s:%d"), __PRETTY_FUNCTION__, __LINE__, mStep);
#endif
    Player::getInstance()->resume();

    switch (mStep) {
        case 0: { // Display logo

            if (mLogo.getAlpha() < 1.f)
                mLogo.setAlpha(mLogo.getAlpha() + ALPHA_VELOCITY);
            else {

                Player::getInstance()->play(0);
                ++mStep;
            }
            break;
        }
        case 1: { // Wait displaying logo

            LOGI(LIBENG_LOG_INTRO, 0, LOG_FORMAT(" - Wait displaying logo..."), __PRETTY_FUNCTION__, __LINE__);
            boost::this_thread::sleep(boost::posix_time::milliseconds(3500));
            ++mStep;
            break;
        }
        case 2: { // Hide logo

            if (mLogo.getAlpha() > ALPHA_VELOCITY)
                mLogo.setAlpha(mLogo.getAlpha() - ALPHA_VELOCITY);
            else
                ++mStep;
            break;
        }
        case 3: { // Replace logo with 'presents' label

            const short* presentsCoords = NULL;
            switch (mLang) {
                case LANG_EN: presentsCoords = gs_presentsEN; break;
                case LANG_FR: presentsCoords = gs_presentsFR; break;
                case LANG_DE: presentsCoords = gs_presentsDE; break;
                case LANG_ES:
                case LANG_IT: presentsCoords = gs_presentsES; break;
                case LANG_PT: presentsCoords = gs_presentsPT; break;
                default: {

                    LOGF(LOG_FORMAT(" - Unexpected language: %d"), __PRETTY_FUNCTION__, __LINE__, static_cast<int>(mLang));
                    assert(NULL);
                    break;
                }
            }

            float coords[8] = {0};

#if defined(LIBENG_LAND_ORIENTATION) || defined(LIBENG_PORT_AS_LAND)
            LOGV(LIBENG_LOG_INTRO, 0, LOG_FORMAT(), __PRETTY_FUNCTION__, __LINE__);
            assert(mGame->getScreen()->width > mGame->getScreen()->height);   // Portrait orientation as landscape (width
                                                        // & height have been swapped), or landscape orientation

            coords[1] = ((presentsCoords[3] - presentsCoords[1]) / static_cast<float>(mGame->getScreen()->height)) * mRatio;
            coords[4] = ((presentsCoords[2] - presentsCoords[0]) / static_cast<float>(mGame->getScreen()->height)) * mRatio;
            // -> Divide by height coz in unit it is 1
#else
            LOGV(LIBENG_LOG_INTRO, 0, LOG_FORMAT(), __PRETTY_FUNCTION__, __LINE__);
            assert(mGame->getScreen()->height > mGame->getScreen()->width); // Portrait orientation

            coords[1] = ((presentsCoords[3] - presentsCoords[1]) / static_cast<float>(mGame->getScreen()->width)) * mRatio;
            coords[4] = ((presentsCoords[2] - presentsCoords[0]) / static_cast<float>(mGame->getScreen()->width)) * mRatio;
            // -> Divide by width coz in unit it is 1
#endif
            coords[6] = coords[4];
            coords[7] = coords[1];
            center(coords);

            const Textures* textures = Textures::getInstance();
            coords[0] = presentsCoords[0] / static_cast<float>((*textures)[0]->width);
            coords[1] = presentsCoords[1] / static_cast<float>((*textures)[0]->height);
            coords[2] = coords[0];
            coords[3] = presentsCoords[3] / static_cast<float>((*textures)[0]->height);
            coords[4] = presentsCoords[2] / static_cast<float>((*textures)[0]->width);
            coords[5] = coords[3];
            coords[6] = coords[4];
            coords[7] = coords[1];
            mLogo.setTexCoords(coords);

            mLogo.setAlpha(0.f);

            ++mStep;
            break;
        }
        case 4: { // Display 'presents'

            if (mLogo.getAlpha() < 1.f)
                mLogo.setAlpha(mLogo.getAlpha() + ALPHA_VELOCITY);
            else
                ++mStep;
            break;
        }
        case 5: { // Wait displaying 'presents' label

            LOGI(LIBENG_LOG_INTRO, 0, LOG_FORMAT(" - Wait displaying 'presents'..."), __PRETTY_FUNCTION__, __LINE__);
            boost::this_thread::sleep(boost::posix_time::milliseconds(1500));
            ++mStep;
            break;
        }
        case 6: { // Hide 'presents' label

            if (mLogo.getAlpha() > ALPHA_VELOCITY)
                mLogo.setAlpha(mLogo.getAlpha() - ALPHA_VELOCITY);
            else
                ++mStep;
            break;
        }
        case 7: {

#ifdef LIBENG_ENABLE_INTERNET
#ifdef __ANDROID__
            if ((!mOnline) || (Internet::isOnline(3000)))
#else
            if ((!mOnline) || (Internet::isOnline()))
#endif
                mDone = true;

            else {
                ++mStep;
                break;
            }
#else // !LIBENG_ENABLE_INTERNET
#ifdef DEBUG
            if (mOnline) {
                LOGF(LOG_FORMAT(" - LIBENG_ENABLE_INTERNET flag is disabled"), __PRETTY_FUNCTION__, __LINE__);
                assert(NULL);
            }
#endif
            mDone = true;
            break;
#endif

#ifdef LIBENG_ENABLE_BLUETOOTH
            if ((mBluetooth) && (!Bluetooth::isEnabled())) {

                mDone = false;
                ++mStep;
            }
#else // !LIBENG_ENABLE_BLUETOOTH
#ifdef DEBUG
            if (mBluetooth) {
                LOGF(LOG_FORMAT(" - LIBENG_ENABLE_BLUETOOTH flag is disabled"), __PRETTY_FUNCTION__, __LINE__);
                assert(NULL);
            }
#endif
            mDone = true;
#endif
            break;
        }
        case 8: {

            if (!mWaitConn) {

#ifdef LIBENG_ENABLE_BLUETOOTH
                mWaitConn = new WaitConn((!mBluetooth) || (Bluetooth::isEnabled()));
#else
                mWaitConn = new WaitConn(true);
#endif
                mWaitConn->initialize(mGame);
                mWaitConn->start(mGame->getScreen());
            }

            static unsigned char wait = 0;
            if (wait < (LIBENG_WAITCONN_DELAY << 2)) {

                ++wait;
                break;
            }
            mWaitConn->update();
            break;
        }
        default: {

            LOGF(LOG_FORMAT(" - Unexpected step: %d"), __PRETTY_FUNCTION__, __LINE__, mStep);
            assert(NULL);
            break;
        }
    }
}
void Intro::render() const {

#ifdef DEBUG
    LOGV(LIBENG_LOG_INTRO, (*mGame->getLog() % 100), LOG_FORMAT(" - s:%d"), __PRETTY_FUNCTION__, __LINE__, mStep);
#endif
    if (!mWaitConn)
        mLogo.render(false);
    else
        mWaitConn->render();
}

} // namespace
