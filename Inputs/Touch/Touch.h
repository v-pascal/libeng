#ifndef LIBENG_TOUCH_H_
#define LIBENG_TOUCH_H_
#if defined(__cplusplus)

#include <libeng/Global.h>
#include <libeng/Inputs/Input.h>

namespace eng {

static const unsigned char DEFAULT_TOUCH_PRECISION = 10;	// 10 pixels of precision
static const unsigned char MAX_TOUCH_COUNT = 10;			// 10 fingers maximum

struct TouchData;
typedef std::map<unsigned int, TouchData*> TouchMap;
inline void assignTouch(TouchData* destination, const TouchData* source);

//////
class TouchInput : public Input {

public:
    typedef enum {

        TOUCH_DOWN = 0,
        TOUCH_UP,
        TOUCH_MOVE,
        TOUCH_CANCEL

    } TouchType;

private:
    short mPrecision;
    TouchMap mTouchs;

    mutable TouchMap::const_iterator mWalkIter;

public:
    TouchInput();
    TouchInput(const void* data);
    virtual ~TouchInput();

    //////
    inline void setConfig(const void* config) { }
    inline void setPrecision(const void* precision) { mPrecision = *static_cast<const short*>(precision); }

    inline const void* getConfig() const { return NULL; }
    inline bool getData(void* data, unsigned char index = 0) const {

#ifdef DEBUG
        ++mLogGet;
        LOGV(LIBENG_LOG_TOUCH, (mLogGet % 100), LOG_FORMAT(" - d:%p; i:%d"), __PRETTY_FUNCTION__, __LINE__, data, index);
        if (index >= MAX_TOUCH_COUNT) {

            LOGE(LOG_FORMAT(" - Max touch count overflow"), __PRETTY_FUNCTION__, __LINE__);
            assert(NULL);
        }
#endif
        if (!index)
            mWalkIter = mTouchs.begin();

        if (mWalkIter != mTouchs.end()) {

            assignTouch(static_cast<TouchData*>(data), mWalkIter->second);
            mWalkIter++;
            return true;
        }
        return false;
    }
    void updateData(const void* data);
    void purgeData();

};

struct TouchData {

    TouchInput::TouchType Type;
    unsigned int Id;

    short X;
    short Y;

};
typedef struct TouchData TouchData;

inline void assignTouch(TouchData* destination, const TouchData* source) {

    destination->Type = source->Type;
    destination->Id = source->Id;
    destination->X = source->X;
    destination->Y = source->Y;
}

} // namespace

#endif // __cplusplus
#endif // LIBENG_TOUCH_H_
