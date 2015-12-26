#include <libeng/Inputs/Touch/Touch.h>

#include <libeng/Log/Log.h>
#include <assert.h>
#include <complex>

namespace eng {

//////
TouchInput::TouchInput() : mPrecision(static_cast<short>(DEFAULT_TOUCH_PRECISION)) {

    LOGV(LIBENG_LOG_TOUCH, 0, LOG_FORMAT(), __PRETTY_FUNCTION__, __LINE__);
    mWalkIter = mTouchs.end();
}
TouchInput::TouchInput(const void* data) : Input(), mPrecision(static_cast<short>(DEFAULT_TOUCH_PRECISION)) {

    LOGV(LIBENG_LOG_TOUCH, 0, LOG_FORMAT(" - d:%p (id:%u; type:%d)"), __PRETTY_FUNCTION__, __LINE__, data,
            (data)? static_cast<const TouchData*>(data)->Id:0,
            static_cast<int>((data)? static_cast<const TouchData*>(data)->Type:LIBENG_NO_DATA));

    //assert(static_cast<const TouchData*>(data)->Type == TOUCH_DOWN);
    // -> Can occured if 'Inputs::free' method is called with a pending touch

    TouchData* touchData = new TouchData;
    assignTouch(touchData, static_cast<const TouchData*>(data));
    mTouchs.insert(std::pair<unsigned int, TouchData*>(touchData->Id, touchData));
    LOGI(LIBENG_LOG_TOUCH, 0, LOG_FORMAT(" - Touch added (id:%u; x:%d; y:%d)"), __PRETTY_FUNCTION__, __LINE__, touchData->Id,
            touchData->X, touchData->Y);
}
TouchInput::~TouchInput() {

    LOGV(LIBENG_LOG_TOUCH, 0, LOG_FORMAT(), __PRETTY_FUNCTION__, __LINE__);
    for (TouchMap::iterator iter = mTouchs.begin(); iter != mTouchs.end(); ++iter)
        delete iter->second;
    mTouchs.clear();
}

void TouchInput::updateData(const void* data) {

#ifdef DEBUG
    ++mLogUpd;
    LOGV(LIBENG_LOG_TOUCH, (mLogUpd % 20), LOG_FORMAT(" - d:%p (id:%u; type:%d)"), __PRETTY_FUNCTION__, __LINE__, data,
            (data)? static_cast<const TouchData*>(data)->Id:666,
            static_cast<int>((data)? static_cast<const TouchData*>(data)->Type:LIBENG_NO_DATA));

    // Allow to display all touch pointers
    if (!(mLogUpd % 20))
        ++mLogUpd;
#endif

    const TouchData* newTouch = static_cast<const TouchData*>(data);
    TouchMap::iterator iter = mTouchs.find(newTouch->Id);
    if (iter == mTouchs.end()) {

        LOGI(LIBENG_LOG_TOUCH, 0, LOG_FORMAT(" - Touch added (id:%u; x:%d; y:%d)"), __PRETTY_FUNCTION__, __LINE__, newTouch->Id,
                newTouch->X, newTouch->Y);
        assert(newTouch->Type == TOUCH_DOWN);

        TouchData* touchData = new TouchData;
        assignTouch(touchData, static_cast<const TouchData*>(data));
        mTouchs.insert(std::pair<unsigned int, TouchData*>(touchData->Id, touchData));
    }
    else {

#ifdef DEBUG
        if (iter->second->Id != newTouch->Id) {

            LOGE(LOG_FORMAT(" - Different ID"), __PRETTY_FUNCTION__, __LINE__);
            assert(NULL);
        }
#endif
        // Check precision
        if ((iter->second->Type != newTouch->Type) || (mPrecision == 0) ||
                (std::abs<short>(iter->second->X - newTouch->X) > mPrecision) ||
                (std::abs<short>(iter->second->Y - newTouch->Y) > mPrecision))
            assignTouch(iter->second, newTouch);
    }
}
void TouchInput::purgeData() {

#ifdef DEBUG
    ++mLogPurge;
    LOGV(LIBENG_LOG_TOUCH, (mLogPurge % 100), LOG_FORMAT(), __PRETTY_FUNCTION__, __LINE__);
#endif
    if (mTouchs.size()) {

        unsigned int touchKey[MAX_TOUCH_COUNT];
        short idxKey = 0;
        for (TouchMap::const_iterator iter = mTouchs.begin(); iter != mTouchs.end(); ++iter) {

            if ((iter->second->Type == TOUCH_UP) || (iter->second->Type == TOUCH_CANCEL))
                touchKey[idxKey++] = iter->first;
        }

        while (idxKey--) {
            
            LOGI(LIBENG_LOG_TOUCH, 0, LOG_FORMAT(" - Touch removed (id:%u)"), __PRETTY_FUNCTION__, __LINE__, touchKey[idxKey]);

            delete mTouchs[touchKey[idxKey]];
            mTouchs.erase(touchKey[idxKey]);
        }
    }
}

} // namespace
