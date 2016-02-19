#ifndef LIBENG_INPUT_H_
#define LIBENG_INPUT_H_
#if defined(__cplusplus)

#include <libeng/Global.h>
#include <libeng/Log/Log.h>
#include <map>
#include <boost/thread/mutex.hpp>

namespace eng {

static const unsigned char MAX_INPUT_COUNT = 2; // TOUCH & ACCEL

// Inputs use mask
static const unsigned char USE_INPUT_TOUCH = 0x01;
static const unsigned char USE_INPUT_ACCEL = 0x02;

//////
class Input {

protected:
#ifdef DEBUG
    unsigned int mLogAdd;
    mutable unsigned int mLogGet;
    unsigned int mLogUpd;
    unsigned int mLogPurge;
#endif

public:
    virtual ~Input() { }

    //////
    virtual void setConfig(const void* config) = 0;
    virtual void setPrecision(const void* precision) = 0;

    virtual const void* getConfig() const = 0;
    virtual bool getData(void* data, unsigned char index = 0) const = 0;
    virtual void updateData(const void* data) = 0;
    virtual void purgeData() = 0;

};

//////
class Inputs {

private:
    Input* mInputs[MAX_INPUT_COUNT];
    boost::mutex mMutex;

#ifdef DEBUG
    unsigned int mLogAdd;
    mutable unsigned int mLogGet;
    unsigned int mLogPurge;
#endif

public:
    typedef enum {

        TOUCH = 0,
        ACCEL

    } InputType;

private:
    Inputs();
    virtual ~Inputs();

    static Inputs* mThis;

public:
    static Inputs* getInstance() {
        if (!mThis)
            mThis = new Inputs;
        return mThis;
    }
    static void freeInstance() {
        if (mThis) {
            delete mThis;
            mThis = NULL;
        }
    }
    static unsigned char InputUse; // See 'Inputs use mask' definitions

    void lock() { mMutex.lock(); }
    void unlock() { mMutex.unlock(); }

    inline Input* get(InputType type) { return mInputs[type]; }

    //////
    inline bool get(InputType type, void* data, unsigned char index = 0) const {

#ifdef DEBUG
        ++mLogGet;
        switch (type) {
            case TOUCH: {
                LOGV(LIBENG_LOG_INPUT, (mLogGet % 100), LOG_FORMAT(" - TOUCH (d:%p; i:%d)"), __PRETTY_FUNCTION__, __LINE__, data, index);
                break;
            }
            case ACCEL: {
                LOGV(LIBENG_LOG_INPUT, (mLogGet % 100), LOG_FORMAT(" - ACCEL (d:%p; i:%d)"), __PRETTY_FUNCTION__, __LINE__, data, index);
                break;
            }
            default: {
                LOGD(LIBENG_LOG_INPUT + 1, 0, LOG_FORMAT(" - t:%d"), __PRETTY_FUNCTION__, __LINE__, static_cast<int>(type));
                break;
            }
        }
#endif
        if (mInputs[type])
            return mInputs[type]->getData(data, index);

        return false;
    }

    template<class inputClass>
    inline void update(InputType type, const void* data) {

        mMutex.lock();
        if (mInputs[type])
            mInputs[type]->updateData(data);
#ifdef DEBUG
        else {

            ++mLogAdd;
            switch (type) {
                case TOUCH: {
                    LOGI(LIBENG_LOG_INPUT, (mLogAdd % 100), LOG_FORMAT(" - Create TOUCH input: %p"), __PRETTY_FUNCTION__,
                            __LINE__, data);
                    break;
                }
                case ACCEL: {
                    LOGI(LIBENG_LOG_INPUT, (mLogAdd % 100), LOG_FORMAT(" - Create ACCEL input: %p"), __PRETTY_FUNCTION__,
                            __LINE__, data);
                    break;
                }
                default: {
                    LOGD(LIBENG_LOG_INPUT + 1, 0, LOG_FORMAT(" - t:%d; d:%p"), __PRETTY_FUNCTION__, __LINE__, static_cast<int>(type), data);
                    break;
                }
            }
            mInputs[type] = new inputClass(data);
        }
#else
        else
            mInputs[type] = new inputClass(data);
#endif
        mMutex.unlock();
    }
    inline void purge() {

#ifdef DEBUG
        ++mLogPurge;
        LOGV(LIBENG_LOG_INPUT, (mLogPurge % 100), LOG_FORMAT(), __PRETTY_FUNCTION__, __LINE__);
#endif
        for (short i = 0; i < MAX_INPUT_COUNT; ++i)
            if (mInputs[i])
                mInputs[i]->purgeData();
    }
    void free();

};

} // namespace

#endif // __cplusplus
#endif // LIBENG_INPUT_H_
