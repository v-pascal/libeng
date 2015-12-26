#ifndef LIBENG_MIC_H_
#define LIBENG_MIC_H_
#if defined(__cplusplus)

#include <libeng/Global.h>
#ifdef LIBENG_ENABLE_MIC

#ifdef __ANDROID__
#include <jni.h>
#endif
#include <boost/thread.hpp>
#include <libeng/Log/Log.h>

namespace eng {

#ifdef LIBENG_DB_CALC
static const float NO_MIC_LEVEL = -1.f;
#endif

//////
class Mic {

private:
    Mic();
    virtual ~Mic();

    static Mic* mThis;

#ifdef __ANDROID__
    JNIEnv* mEnv;
    jmethodID mLoadMthd;

    const short* mBuffer;

    void update();
#else
    short* mBuffer;
#endif
    int mBufferSize;

#ifdef LIBENG_DB_CALC
    float mLevel;
    inline void updateLevel() {

#ifdef DEBUG
        float prevLevel = mLevel;
#endif
        if (!mBufferSize) {

            mLevel = NO_MIC_LEVEL; // Error
            return;
        }
        assert(mBufferSize < 65537); // Overflow risk on the 'sum' variable below

        int sum = 0;
        for (int i = 0; i < mBufferSize; ++i)
            sum += mBuffer[i];
        mLevel = std::abs<float>(sum / static_cast<float>(mBufferSize));

#ifdef DEBUG
        LOGI(LIBENG_LOG_MIC, (mLog % 100), LOG_FORMAT(" - Previous/New level: %f; %f"), __PRETTY_FUNCTION__, __LINE__,
             prevLevel, mLevel);
#endif
    }
#endif

public:
    static Mic* getInstance() {
        if (!mThis)
            mThis = new Mic;
        return mThis;
    }
    static void freeInstance() {
        if (mThis) {
            delete mThis;
            mThis = NULL;
        }
    }

#ifdef __ANDROID__
    static bool initRecorder(const std::string &file, int format);
#else
    static void initRecorder(const std::string &file, int format, float rate, unsigned char channels);
#endif
    static bool startRecorder();
    static bool stopRecorder();

    //
    bool lock() {

#ifdef __ANDROID__
        if ((mThread) && (!mAbort))
            return mMutex.try_lock();

        return false;
#else
        return mMutex.try_lock();
#endif
    }
    inline const short* getBuffer() const {

#ifdef __ANDROID__
        assert(mThread);
#endif
        return mBuffer;
    }
    inline int getBufferSize() const { return mBufferSize; }
    void unlock() { mMutex.unlock(); }

#ifdef LIBENG_DB_CALC
    inline float getLevel() const { return mLevel; } // Return NO_MIC_LEVEL when error (< 0.f)
#endif

    //////
    bool start(); // Resume
    // WARNING: On Android call it each time B4 any acquisition. Lock/Unlock operation need (e.g into 'LEVEL::update' method)

#ifdef __ANDROID__
    inline void updateBuffer(int len, const short* data) {

#ifdef DEBUG
        LOGV(LIBENG_LOG_MIC, (mLog % 100), LOG_FORMAT(" - l:%d; d:%x)"), __PRETTY_FUNCTION__, __LINE__, len, data);
#endif
        mBufferSize = len;
        mBuffer = data;
    }
#else
    void updateBuffer(int len, const short* data);
#endif // __ANDROID__
    bool stop(); // Pause

private:
#ifdef DEBUG
    unsigned int mLog;
#endif

    boost::mutex mMutex;
#ifdef __ANDROID__
    boost::thread* mThread;
    volatile bool mAbort;

    void micThreadRunning();
    static void startMicThread(Mic* mic);
#endif

};

} // namespace

#endif // LIBENG_ENABLE_MIC
#endif // __cplusplus
#endif // LIBENG_MIC_H_
