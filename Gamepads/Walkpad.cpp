#include "Walkpad.h"

#ifdef LIBENG_ENABLE_GAMEPAD
#include <libeng/Inputs/Touch/Touch.h>

#define NO_TOUCH    static_cast<char>(LIBENG_NO_DATA)

namespace eng {

//////
Walkpad::Walkpad() : Gamepad(), mPrevL(0), mPrevR(0), mTurnTop(0), mTurnBottom(0), mCalTop(0), mCalBottom(0),
        mTurning(false) {

    LOGV(LIBENG_LOG_GAMEPAD, 0, LOG_FORMAT(), __PRETTY_FUNCTION__, __LINE__);
    std::memset(&mConfig, 0, sizeof(WalkConfig));
    std::memset(&mCurData, 0, sizeof(Data));

#ifdef DEBUG
    mLog = NULL;
#endif
}
Walkpad::~Walkpad() { LOGV(LIBENG_LOG_GAMEPAD, 0, LOG_FORMAT(), __PRETTY_FUNCTION__, __LINE__); }

void Walkpad::refresh() {

#ifdef DEBUG
    LOGV(LIBENG_LOG_GAMEPAD, (*mLog % 100), LOG_FORMAT(), __PRETTY_FUNCTION__, __LINE__);
#endif
    char leftTouch = NO_TOUCH;
    char rightTouch = NO_TOUCH;
    char midTouch = NO_TOUCH;

    mCurData.status = WPS_NONE;
    mCurData.leftFoot = mCurData.rightFoot = false;
    mCurData.speed = 0;

    // Update data according current finger(s) position
    unsigned char touchCount = mGame->mTouchCount;
    while (touchCount--) {

#ifndef LIBENG_PORT_AS_LAND
        if ((mGame->mTouchData[touchCount].Y > mConfig.top) && (mGame->mTouchData[touchCount].Y < mConfig.bottom)) {
            if ((mGame->mTouchData[touchCount].X > mConfig.leftL) && (mGame->mTouchData[touchCount].X < mConfig.rightL))
                leftTouch = touchCount;
            else if ((mGame->mTouchData[touchCount].X > mConfig.leftR) && (mGame->mTouchData[touchCount].X < mConfig.rightR))
                rightTouch = touchCount;
            else if ((mTurning) &&
                    (mGame->mTouchData[touchCount].X > mConfig.rightL) && (mGame->mTouchData[touchCount].X < mConfig.leftR))
                midTouch = static_cast<char>(touchCount);
        }
#else
        if ((mGame->mTouchData[touchCount].X < mConfig.top) && (mGame->mTouchData[touchCount].X > mConfig.bottom)) {
            if ((mGame->mTouchData[touchCount].Y > mConfig.leftL) && (mGame->mTouchData[touchCount].Y < mConfig.rightL))
                leftTouch = static_cast<char>(touchCount);
            else if ((mGame->mTouchData[touchCount].Y > mConfig.leftR) && (mGame->mTouchData[touchCount].Y < mConfig.rightR))
                rightTouch = static_cast<char>(touchCount);
            else if ((mTurning) &&
                    (mGame->mTouchData[touchCount].Y > mConfig.rightL) && (mGame->mTouchData[touchCount].Y < mConfig.leftR))
                midTouch = static_cast<char>(touchCount);
        }
#endif
    }

#ifndef LIBENG_PORT_AS_LAND
    bool leftInTurn = (leftTouch != NO_TOUCH)?
            ((mGame->mTouchData[leftTouch].Y > mTurnTop) && (mGame->mTouchData[leftTouch].Y < mTurnBottom)):false;
    bool rightInTurn = (rightTouch != NO_TOUCH)?
            ((mGame->mTouchData[rightTouch].Y > mTurnTop) && (mGame->mTouchData[rightTouch].Y < mTurnBottom)):false;
#else
    bool leftInTurn = (leftTouch != NO_TOUCH)?
            ((mGame->mTouchData[leftTouch].X < mTurnTop) && (mGame->mTouchData[leftTouch].X > mTurnBottom)):false;
    bool rightInTurn = (rightTouch != NO_TOUCH)?
            ((mGame->mTouchData[rightTouch].X < mTurnTop) && (mGame->mTouchData[rightTouch].X > mTurnBottom)):false;
#endif

    // Check stopping
    if ((leftInTurn) && (rightInTurn)) {

        mCurData.status = WPS_STOP;
        mTurning = false;
    }
    // Check turning
    else if ((mTurning) ||
            ((!mCurData.direction) && (rightInTurn) && (mGame->mTouchData[rightTouch].Type == TouchInput::TOUCH_DOWN)) ||
            ((mCurData.direction) && (leftInTurn) && (mGame->mTouchData[leftTouch].Type == TouchInput::TOUCH_DOWN))) {

        if (midTouch != NO_TOUCH) {

            mCurData.status = (!mCurData.direction)? WPS_TURNLEFT:WPS_TURNRIGHT;
            mCurData.direction = !mCurData.direction;
            mTurning = false;
        }
        else {

            mCurData.status = WPS_UNDEFINED; // Not defined yet
            mTurning = (rightInTurn || leftInTurn);
        }
    }
    // Check walking
    else {

        walk(leftTouch, true);
        walk(rightTouch, false);

        if ((mCurData.rightFoot) && (mCurData.leftFoot))
            mCurData.status = WPS_UNEXPECTED; // Walking with both feet on the floor !?!?
    }
}

} // namespace

#endif // LIBENG_ENABLE_GAMEPAD
