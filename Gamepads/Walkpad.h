#ifndef LIBENG_WALKPAD_H_
#define LIBENG_WALKPAD_H_
#if defined(__cplusplus)

#include <libeng/Global.h>
#ifdef LIBENG_ENABLE_GAMEPAD

#include <libeng/Game/Game.h>
#include <libeng/Log/Log.h>
#include <cstring>

namespace eng {

typedef struct {

    // Finger bounds in pixel (Suffix 'L': Left finger; 'R': Right finger)
    short leftL;
    short rightL;
    short leftR;
    short rightR;

    short top;
    short bottom;

    // ...with all data above in touch coordinates (in pixel)
    // -> With [0;0]: Top/left coordinates; [g_ScreenWidth;g_ScreenHeight]: Bottom/right coordinates

} WalkConfig; // ...data needed in 'Walkpad' class run time

//////
class Walkpad : public Gamepad {

public:
    typedef enum {

        WPS_UNDEFINED = 0, // Undefined status yet (e.g turn opearation in progress)
        WPS_WALK,
        WPS_CROUCH,
        WPS_JUMP,
        WPS_STOP,
        WPS_NONE,
        WPS_TURNLEFT,
        WPS_TURNRIGHT,
        WPS_UNEXPECTED // Unkown behaviour (e.g walking with both feet on the floor)

    } Status;
    typedef struct {

        Status status;      // Walk status
        short speed;       // Walk speed (in pixel): [0;(bottom - top) / 2]

        bool leftFoot;     // Left foot is on the floor
        bool rightFoot;    // Right foot is on the floor

        bool direction;    // TRUE: Backward direction; FALSE: Forward direction (default)

    } Data;

private:
#ifdef DEBUG
    const unsigned int* mLog;
#endif
    WalkConfig mConfig;
    Data mCurData;

    short mPrevL; // Previous left finger Y (or X in LIBENG_PORT_AS_LAND mode)
    short mPrevR; // Previous right finger Y (or X in LIBENG_PORT_AS_LAND mode)

    // Vertical coordinates from which speed can be calculated (or horizontal coordinates for LIBENG_PORT_AS_LAND)
    // -> Avoid to include in the speed calculation a finger acceleration increase just after a touch down
    short mCalTop;
    short mCalBottom;

    short mTurnTop;     // Upper touch down limit for a turn action
    short mTurnBottom;  // Lower touch down limit for a turn action
    // ...also used as stop action bounds

    bool mTurning;

    inline void walk(char touchIdx, bool leftTouch) {

        short* prevPos = (leftTouch)? &mPrevL:&mPrevR;
        if (touchIdx != static_cast<char>(LIBENG_NO_DATA)) {
            switch (mGame->mTouchData[touchIdx].Type) {
                case TouchInput::TOUCH_MOVE: {

                    if (leftTouch) mCurData.leftFoot = true;
                    else mCurData.rightFoot = true;

#ifndef LIBENG_PORT_AS_LAND
                    if ((*prevPos) && (*prevPos < mGame->mTouchData[touchIdx].Y) &&
                            (mGame->mTouchData[touchIdx].Y > mCalTop) && (mGame->mTouchData[touchIdx].Y < mCalBottom)) {

                        mCurData.speed = mGame->mTouchData[touchIdx].Y - (*prevPos);
#else
                    if ((*prevPos) && (*prevPos > mGame->mTouchData[touchIdx].X) &&
                            (mGame->mTouchData[touchIdx].X < mCalTop) && (mGame->mTouchData[touchIdx].X > mCalBottom)) {

                        mCurData.speed = (*prevPos) - mGame->mTouchData[touchIdx].X;
#endif
                        mCurData.status = WPS_WALK;
                        LOGI(LIBENG_LOG_GAMEPAD, 0, LOG_FORMAT(" - Walk (%s): %d"), __PRETTY_FUNCTION__, __LINE__,
                                (leftTouch)? "left":"right", mCurData.speed);
                    }
                    //break;
                }
                case TouchInput::TOUCH_DOWN: {

#ifndef LIBENG_PORT_AS_LAND
                    *prevPos = mGame->mTouchData[touchIdx].Y;
#else
                    *prevPos = mGame->mTouchData[touchIdx].X;
#endif
                    break;
                }
            }
        }
        else
            *prevPos = 0;
    }

public:
    Walkpad();
    virtual~ Walkpad();

    // WARNING: Should call 'initialize' method below after 'Game2D::start' method call (portrait as landscape management)
#ifdef DEBUG
    void initialize(const Game* game, const WalkConfig* config, const unsigned int* log) {

        LOGV(LIBENG_LOG_GAMEPAD, 0, LOG_FORMAT(" - g:%p; c:%p; l:%p"), __PRETTY_FUNCTION__, __LINE__, game, config, log);
        assert(config->leftL < config->rightL);
        assert(config->leftR < config->rightR);
        assert(config->rightL < config->leftR);
        assert(config->top < config->bottom);
#else
    void initialize(const Game* game, const WalkConfig* config) {
#endif
        init(game);

#ifndef LIBENG_PORT_AS_LAND
        std::memcpy(&mConfig, config, sizeof(WalkConfig));

        short halfH = (config->bottom - config->top) >> 1;
        short halfW = (config->rightL - config->leftL) >> 1;
        mTurnTop = halfH + config->top - halfW;
        mTurnBottom = halfH + config->top + halfW;
        // ...square bounds

        // Include speed calculation when finger is in half height position from center
        // -> Max speed available: Half height (in pixel)
        halfH >>= 1;
        mCalTop = config->top + halfH;
        mCalBottom = config->bottom - halfH;

#else
        mConfig.leftL = config->leftL;
        mConfig.rightL = config->rightL;
        mConfig.leftR = config->leftR;
        mConfig.rightR = config->rightR;

        mConfig.top = game->getScreen()->height - config->top;
        mConfig.bottom = game->getScreen()->height - config->bottom;

        short halfH = (config->bottom - config->top) >> 1;
        short halfW = (config->rightL - config->leftL) >> 1;
        mTurnTop = game->getScreen()->height - (halfH + config->top - halfW);
        mTurnBottom = game->getScreen()->height - (halfH + config->top + halfW);
        // ...square bounds

        // Include speed calculation when finger is in half height position from center
        // -> Max speed available: Half height (in pixel)
        halfH >>= 1;
        mCalTop = game->getScreen()->height - (config->top + halfH);
        mCalBottom = game->getScreen()->height - (config->bottom - halfH);

        // WARNING: Loss of performance occured when 'g_ScreenHeight' is substracted (above)
#endif

        LOGI(LIBENG_LOG_GAMEPAD, 0, LOG_FORMAT(" - ll:%d; rl:%d; lr:%d; rr:%d; t:%d; b:%d; tt:%d; tb:%d; ct:%d; cb:%d"),
                __PRETTY_FUNCTION__, __LINE__, mConfig.leftL, mConfig.rightL, mConfig.leftR, mConfig.rightR, mConfig.top,
                mConfig.bottom, mTurnTop, mTurnBottom, mCalTop, mCalBottom);

#ifdef DEBUG
        mLog = log;
#endif
    }

    //////
    void refresh();
    inline const void* getData() const { return static_cast<const void*>(&mCurData); }

};

} // namespace

// Cast(s)
#define walkpadVia(p)   static_cast<const eng::Walkpad*>(p)
#define walkdataVia(d)  static_cast<const eng::Walkpad::Data*>(d)

#endif // LIBENG_ENABLE_GAMEPAD
#endif // __cplusplus
#endif // LIBENG_WALKPAD_H_
