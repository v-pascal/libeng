#include "Mic.h"

#ifdef LIBENG_ENABLE_MIC
#include <libeng/Tools/Tools.h>

#ifdef __ANDROID__
#define SAMPLE_DELAY    10 // 10 * 5 ms before updating mic buffer/level
#endif

namespace eng {

Mic* Mic::mThis = NULL;

//////
Mic::Mic() : mBuffer(NULL), mBufferSize(0) {

#ifdef DEBUG
    LOGV(LIBENG_LOG_MIC, 0, LOG_FORMAT(), __PRETTY_FUNCTION__, __LINE__);
    mLog = 0;
#endif

#ifdef LIBENG_DB_CALC
    mLevel = 0.f;
#endif
#ifdef __ANDROID__
    mEnv = NULL;
    mLoadMthd = NULL;

    mThread = NULL;
    mAbort = true;
#endif
}
Mic::~Mic() {

    LOGV(LIBENG_LOG_MIC, 0, LOG_FORMAT(), __PRETTY_FUNCTION__, __LINE__);
#ifdef __ANDROID__
    if (mThread)
        stop();
#else
    stop();
#endif
}

#ifdef __ANDROID__
bool Mic::initRecorder(const std::string &file, int format) {

    LOGV(LIBENG_LOG_MIC, 0, LOG_FORMAT(" - f:%s; f:%d (j:%p; a:%p)"), __PRETTY_FUNCTION__, __LINE__, file.c_str(), format,
            g_JavaVM, g_ActivityObject);
    assert(g_JavaVM);
    assert(g_ActivityObject);

    JNIEnv* env = getJavaEnv(LIBENG_LOG_INTERNET, __PRETTY_FUNCTION__, __LINE__);
    if (!env)
        return false;

    jclass clss = env->GetObjectClass(g_ActivityObject);
    if (!clss) {

        LOGE(LOG_FORMAT(" - Failed to get activity object class"), __PRETTY_FUNCTION__, __LINE__);
        assert(NULL);
        return false;
    }
    jmethodID mthd = env->GetMethodID(clss, "initMicRecorder", "(Ljava/lang/String;I)Z");
    if (!mthd) {

        LOGE(LOG_FORMAT(" - Failed to get 'initMicRecorder' java method"), __PRETTY_FUNCTION__, __LINE__);
        assert(NULL);
        return false;
    }
    jstring jFile = env->NewStringUTF(file.c_str());
    bool res = static_cast<bool>(env->CallBooleanMethod(g_ActivityObject, mthd, jFile, format));
    env->DeleteLocalRef(jFile);

    return res;
}
#else
void Mic::initRecorder(const std::string &file, int format, float rate, unsigned char channels) {

    LOGV(LIBENG_LOG_MIC, 0, LOG_FORMAT(" - f:%s; f:%d; r:%f; c:%d (a:%p)"), __PRETTY_FUNCTION__, __LINE__, file.c_str(),
         format, rate, channels, g_AppleOS);
    assert(g_AppleOS);

    [g_AppleOS initMicRecorder:[NSString stringWithUTF8String:file.c_str()] withFormat:format withSampleRate:rate
               withNumChannels:static_cast<int>(channels)];
}
#endif
bool Mic::startRecorder() {

#ifdef __ANDROID__
    LOGV(LIBENG_LOG_MIC, 0, LOG_FORMAT(" - (j:%p; a:%p)"), __PRETTY_FUNCTION__, __LINE__, g_JavaVM, g_ActivityObject);
    assert(g_JavaVM);
    assert(g_ActivityObject);

    JNIEnv* env = getJavaEnv(LIBENG_LOG_INTERNET, __PRETTY_FUNCTION__, __LINE__);
    if (!env)
        return false;

    jclass clss = env->GetObjectClass(g_ActivityObject);
    if (!clss) {

        LOGE(LOG_FORMAT(" - Failed to get activity object class"), __PRETTY_FUNCTION__, __LINE__);
        assert(NULL);
        return false;
    }
    jmethodID mthd = env->GetMethodID(clss, "startMicRecorder", "()Z");
    if (!mthd) {

        LOGE(LOG_FORMAT(" - Failed to get 'startMicRecorder' java method"), __PRETTY_FUNCTION__, __LINE__);
        assert(NULL);
        return false;
    }
    return static_cast<bool>(env->CallBooleanMethod(g_ActivityObject, mthd));

#else
    LOGV(LIBENG_LOG_MIC, 0, LOG_FORMAT(" - (a:%p)"), __PRETTY_FUNCTION__, __LINE__, g_AppleOS);
    assert(g_AppleOS);

    return static_cast<bool>([g_AppleOS startMicRecorder]);
#endif
}
bool Mic::stopRecorder() {

#ifdef __ANDROID__
    LOGV(LIBENG_LOG_MIC, 0, LOG_FORMAT(" - (j:%p; a:%p)"), __PRETTY_FUNCTION__, __LINE__, g_JavaVM, g_ActivityObject);
    assert(g_JavaVM);
    assert(g_ActivityObject);

    JNIEnv* env = getJavaEnv(LIBENG_LOG_INTERNET, __PRETTY_FUNCTION__, __LINE__);
    if (!env)
        return false;

    jclass clss = env->GetObjectClass(g_ActivityObject);
    if (!clss) {

        LOGE(LOG_FORMAT(" - Failed to get activity object class"), __PRETTY_FUNCTION__, __LINE__);
        assert(NULL);
        return false;
    }
    jmethodID mthd = env->GetMethodID(clss, "stopMicRecorder", "()Z");
    if (!mthd) {

        LOGE(LOG_FORMAT(" - Failed to get 'stopMicRecorder' java method"), __PRETTY_FUNCTION__, __LINE__);
        assert(NULL);
        return false;
    }
    return static_cast<bool>(env->CallBooleanMethod(g_ActivityObject, mthd));
#else
    LOGV(LIBENG_LOG_MIC, 0, LOG_FORMAT(" - (a:%p)"), __PRETTY_FUNCTION__, __LINE__, g_AppleOS);
    assert(g_AppleOS);

    return static_cast<bool>([g_AppleOS stopMicRecorder]);
#endif
}

bool Mic::start() {

#ifdef __ANDROID__
    LOGV(LIBENG_LOG_MIC, 0, LOG_FORMAT(" - (j:%p; a:%p; t:%p)"), __PRETTY_FUNCTION__, __LINE__, g_JavaVM,
            g_ActivityClass, mThread);
    assert(g_JavaVM);
    assert(g_ActivityClass);

    if (mThread) {

        LOGI(LIBENG_LOG_MIC, 0, LOG_FORMAT(" - Mic already started"), __PRETTY_FUNCTION__, __LINE__);
        mAbort = false;
        return true;
    }
    JNIEnv* env = getJavaEnv(LIBENG_LOG_MIC, __PRETTY_FUNCTION__, __LINE__);
    if (!env)
        return false;

    jmethodID mthd = env->GetStaticMethodID(g_ActivityClass, "startMic", "()Z");
    if (!mthd) {

        LOGE(LOG_FORMAT(" - Failed to get 'startMic' java method"), __PRETTY_FUNCTION__, __LINE__);
        assert(NULL);
        return false;
    }
    if (!env->CallStaticBooleanMethod(g_ActivityClass, mthd)) {

        LOGE(LOG_FORMAT(" - Failed to start mic"), __PRETTY_FUNCTION__, __LINE__);
        assert(NULL);
        return false;
    }
    mAbort = false;
    mThread = new boost::thread(Mic::startMicThread, this);

#else
    LOGV(LIBENG_LOG_MIC, 0, LOG_FORMAT(" - (a:%p)"), __PRETTY_FUNCTION__, __LINE__, g_AppleOS);
    assert(g_AppleOS);
    assert(!mBuffer);

    if (![g_AppleOS startMic]) {

        LOGE(LOG_FORMAT(" - Failed to start mic"), __PRETTY_FUNCTION__, __LINE__);
        assert(NULL);
        return false;
    }
#endif
    return true;
}
#ifdef __ANDROID__
void Mic::update() {

#ifdef DEBUG
    ++mLog;
    LOGV(LIBENG_LOG_MIC, (mLog % 100), LOG_FORMAT(" - (e:%p; u:%p)"), __PRETTY_FUNCTION__, __LINE__, mEnv,
         mLoadMthd);
    assert(mEnv);
    assert(mLoadMthd);
#endif
    mMutex.lock();
    mEnv->CallStaticVoidMethod(g_ActivityClass, mLoadMthd);
    mMutex.unlock();

#ifdef LIBENG_DB_CALC
    updateLevel();
#endif
}
#else
void Mic::updateBuffer(int len, const short* data) {

#ifdef DEBUG
    ++mLog;
    LOGV(LIBENG_LOG_MIC, (mLog % 100), LOG_FORMAT(" - l:%d; d:%p)"), __PRETTY_FUNCTION__, __LINE__, len, data);
#endif
    if (!mBuffer) {

        mBufferSize = len;
        mBuffer = new short[mBufferSize];
    }
#ifdef DEBUG
    if (mBufferSize != len) {

        LOGE(LOG_FORMAT(" - Wrong mic buffer length"), __PRETTY_FUNCTION__, __LINE__);
        assert(NULL);
    }
#endif

    mMutex.lock();
    std::memcpy(mBuffer, data, sizeof(short) * len);
    mMutex.unlock();

#ifdef LIBENG_DB_CALC
    updateLevel();
#endif
}
#endif // __ANDROID__

bool Mic::stop() {

#ifdef __ANDROID__
    LOGV(LIBENG_LOG_MIC, 0, LOG_FORMAT(" - (j:%p; a:%p; t:%p)"), __PRETTY_FUNCTION__, __LINE__, g_JavaVM,
            g_ActivityClass, mThread);
    assert(g_JavaVM);
    assert(g_ActivityClass);

    if (!mThread) {

        LOGI(LIBENG_LOG_MIC, 0, LOG_FORMAT(" - Mic already stopped"), __PRETTY_FUNCTION__, __LINE__);
        mAbort = true;
        return true;
    }

    mAbort = true;

    mThread->join();
    delete mThread;
    mThread = NULL;

    JNIEnv* env = getJavaEnv(LIBENG_LOG_MIC, __PRETTY_FUNCTION__, __LINE__);
    if (!env)
        return false;

    jmethodID mthd = env->GetStaticMethodID(g_ActivityClass, "stopMic", "()V");
    if (!mthd) {

        LOGE(LOG_FORMAT(" - Failed to get 'stopMic' java method"), __PRETTY_FUNCTION__, __LINE__);
        assert(NULL);
        return false;
    }
    env->CallStaticVoidMethod(g_ActivityClass, mthd);

#else
    LOGV(LIBENG_LOG_MIC, 0, LOG_FORMAT(" - (a:%p)"), __PRETTY_FUNCTION__, __LINE__, g_AppleOS);
    assert(g_AppleOS);

    [g_AppleOS stopMic];
    if (mBuffer) {

        delete [] mBuffer;
        mBuffer = NULL;
    }
    //else
    //    assert(mBuffer); // Can be NULL if 'updateBuffer' has not been called yet
#endif
    return true;
}

#ifdef __ANDROID__
void Mic::micThreadRunning() {

    mEnv = getJavaEnv(LIBENG_LOG_MIC, __PRETTY_FUNCTION__, __LINE__);
    if (!mEnv)
        mLoadMthd = NULL;

    else {

        mLoadMthd = mEnv->GetStaticMethodID(g_ActivityClass, "loadMic", "()V");
        if (!mLoadMthd) {

            LOGE(LOG_FORMAT(" - Failed to get 'loadMic' java method"), __PRETTY_FUNCTION__, __LINE__);
            assert(NULL);
        }
    }
    if (mLoadMthd) {

        LOGV(LIBENG_LOG_MIC, 0, LOG_FORMAT(" - Start loop"), __PRETTY_FUNCTION__, __LINE__);
        unsigned char delay = 0;
        while (!mAbort) {

            if (!(++delay % SAMPLE_DELAY))
                update();
            else
                boost::this_thread::sleep(boost::posix_time::milliseconds(5));
        }
        LOGV(LIBENG_LOG_MIC, 0, LOG_FORMAT(" - Finished"), __PRETTY_FUNCTION__, __LINE__);
        g_JavaVM->DetachCurrentThread();
    }
#ifdef LIBENG_DB_CALC
    else
        mLevel = NO_MIC_LEVEL;
#endif
}
void Mic::startMicThread(Mic* mic) {

    LOGV(LIBENG_LOG_MIC, 0, LOG_FORMAT(" - m:%p"), __PRETTY_FUNCTION__, __LINE__, mic);
    mic->micThreadRunning();
}
#endif

} // namespace

#endif // LIBENG_ENABLE_MIC
