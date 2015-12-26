#include "Joypad.h"

#ifdef LIBENG_ENABLE_GAMEPAD
#include <libeng/Inputs/Touch/Touch.h>

namespace eng {

//////
Joypad::Joypad() : Gamepad() {

    LOGV(LIBENG_LOG_GAMEPAD, 0, LOG_FORMAT(), __PRETTY_FUNCTION__, __LINE__);
    std::memset(&mConfig, 0, sizeof(JoyConfig));
    std::memset(&mCurData, 0, sizeof(Data));

#ifdef DEBUG
    mLog = NULL;
#endif
}
Joypad::~Joypad() {

    LOGV(LIBENG_LOG_GAMEPAD, 0, LOG_FORMAT(), __PRETTY_FUNCTION__, __LINE__);
    for (unsigned char i = 0; i < MAX_BUTTON_COUNT; ++i)
        if (mConfig.buttons[i])
            delete mConfig.buttons[i];
}

void Joypad::refresh() {

#ifdef DEBUG
    LOGV(LIBENG_LOG_GAMEPAD, (*mLog % 100), LOG_FORMAT(), __PRETTY_FUNCTION__, __LINE__);
#endif

    std::memset(&mCurData, 0, sizeof(Data));

    bool pushed = false;
    unsigned char touchCount = mGame->mTouchCount;
    while (touchCount--) {

        // Check direction (if not already done)
        if (mCurData.direction == JPD_NONE) { // ...only one direction can be set
#ifndef LIBENG_PORT_AS_LAND
            if ((mGame->mTouchData[touchCount].X > mConfig.forward.left) &&
                (mGame->mTouchData[touchCount].X < mConfig.forward.right) &&
                (mGame->mTouchData[touchCount].Y > mConfig.forward.top) &&
                (mGame->mTouchData[touchCount].Y < mConfig.forward.bottom)) {
#else
            if ((mGame->mTouchData[touchCount].Y > mConfig.forward.left) &&
                (mGame->mTouchData[touchCount].Y < mConfig.forward.right) &&
                (mGame->mTouchData[touchCount].X < mConfig.forward.top) &&
                (mGame->mTouchData[touchCount].X > mConfig.forward.bottom)) {
#endif
#ifdef DEBUG
                LOGI(LIBENG_LOG_GAMEPAD, (*mLog % 100), LOG_FORMAT(" - Direction: FORWARD"), __PRETTY_FUNCTION__, __LINE__);
#endif
                mCurData.direction = JPD_FORWARD;
                continue;
            }
#ifndef LIBENG_PORT_AS_LAND
            if ((mGame->mTouchData[touchCount].X > mConfig.backward.left) &&
                (mGame->mTouchData[touchCount].X < mConfig.backward.right) &&
                (mGame->mTouchData[touchCount].Y > mConfig.backward.top) &&
                (mGame->mTouchData[touchCount].Y < mConfig.backward.bottom)) {
#else
            if ((mGame->mTouchData[touchCount].Y > mConfig.backward.left) &&
                (mGame->mTouchData[touchCount].Y < mConfig.backward.right) &&
                (mGame->mTouchData[touchCount].X < mConfig.backward.top) &&
                (mGame->mTouchData[touchCount].X > mConfig.backward.bottom)) {
#endif
#ifdef DEBUG
                LOGI(LIBENG_LOG_GAMEPAD, (*mLog % 100), LOG_FORMAT(" - Direction: BACKWARD"), __PRETTY_FUNCTION__, __LINE__);
#endif
                mCurData.direction = JPD_BACKWARD;
                continue;
            }
#ifndef LIBENG_PORT_AS_LAND
            if ((mGame->mTouchData[touchCount].X > mConfig.down.left) &&
                (mGame->mTouchData[touchCount].X < mConfig.down.right) &&
                (mGame->mTouchData[touchCount].Y > mConfig.down.top) &&
                (mGame->mTouchData[touchCount].Y < mConfig.down.bottom)) {
#else
            if ((mGame->mTouchData[touchCount].Y > mConfig.down.left) &&
                (mGame->mTouchData[touchCount].Y < mConfig.down.right) &&
                (mGame->mTouchData[touchCount].X < mConfig.down.top) &&
                (mGame->mTouchData[touchCount].X > mConfig.down.bottom)) {
#endif
#ifdef DEBUG
                LOGI(LIBENG_LOG_GAMEPAD, (*mLog % 100), LOG_FORMAT(" - Direction: DOWN"), __PRETTY_FUNCTION__, __LINE__);
#endif
                mCurData.direction = JPD_DOWN;
                continue;
            }
#ifndef LIBENG_PORT_AS_LAND
            if ((mGame->mTouchData[touchCount].X > mConfig.up.left) &&
                (mGame->mTouchData[touchCount].X < mConfig.up.right) &&
                (mGame->mTouchData[touchCount].Y > mConfig.up.top) &&
                (mGame->mTouchData[touchCount].Y < mConfig.up.bottom)) {
#else
            if ((mGame->mTouchData[touchCount].Y > mConfig.up.left) &&
                (mGame->mTouchData[touchCount].Y < mConfig.up.right) &&
                (mGame->mTouchData[touchCount].X < mConfig.up.top) &&
                (mGame->mTouchData[touchCount].X > mConfig.up.bottom)) {
#endif
#ifdef DEBUG
                LOGI(LIBENG_LOG_GAMEPAD, (*mLog % 100), LOG_FORMAT(" - Direction: UP"), __PRETTY_FUNCTION__, __LINE__);
#endif
                mCurData.direction = JPD_UP;
                continue;
            }
        }

        // Check pushed button
        if (!pushed) { // ...only one button can be pushed at same time
            for (unsigned char i = 0; i < MAX_BUTTON_COUNT; ++i) {

                // Check defined button
                if (mConfig.buttons[i]) {
#ifndef LIBENG_PORT_AS_LAND
                    if ((mGame->mTouchData[touchCount].X > mConfig.buttons[i]->left) &&
                        (mGame->mTouchData[touchCount].X < mConfig.buttons[i]->right) &&
                        (mGame->mTouchData[touchCount].Y > mConfig.buttons[i]->top) &&
                        (mGame->mTouchData[touchCount].Y < mConfig.buttons[i]->bottom)) {
#else
                    if ((mGame->mTouchData[touchCount].Y > mConfig.buttons[i]->left) &&
                        (mGame->mTouchData[touchCount].Y < mConfig.buttons[i]->right) &&
                        (mGame->mTouchData[touchCount].X < mConfig.buttons[i]->top) &&
                        (mGame->mTouchData[touchCount].X > mConfig.buttons[i]->bottom)) {
#endif
#ifdef DEBUG
                        LOGI(LIBENG_LOG_GAMEPAD, (*mLog % 100), LOG_FORMAT(" - Pushing button %d"), __PRETTY_FUNCTION__,
                                __LINE__, i);
#endif
                        mCurData.buttonPushed[i] = true;
                        pushed = true;
                        break;
                    }
                }
                else
                    break; // No more button defined
            }
        }
    }
}

} // namespace

#endif // LIBENG_ENABLE_GAMEPAD
