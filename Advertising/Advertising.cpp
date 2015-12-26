#include "Advertising.h"

#ifdef LIBENG_ENABLE_ADVERTISING

#include <libeng/Log/Log.h>
#include <assert.h>

#ifdef __ANDROID__
#include <libeng/Tools/Tools.h>
#endif

namespace eng {

Advertising* Advertising::mThis = NULL;

//////
Advertising::Advertising() : mPublished(static_cast<time_t>(0)), mLoading(false) {

    LOGV(LIBENG_LOG_ADVERTISING, 0, LOG_FORMAT(), __PRETTY_FUNCTION__, __LINE__);
}
Advertising::~Advertising() { LOGV(LIBENG_LOG_ADVERTISING, 0, LOG_FORMAT(), __PRETTY_FUNCTION__, __LINE__); }

bool Advertising::load() const {

#ifdef __ANDROID__
    LOGV(LIBENG_LOG_ADVERTISING, 0, LOG_FORMAT(" - (j:%p; a:%p)"), __PRETTY_FUNCTION__, __LINE__, g_JavaVM,
            g_ActivityObject);
    assert(g_JavaVM);
    assert(g_ActivityObject);

    JNIEnv* env = getJavaEnv(LIBENG_LOG_ADVERTISING, __PRETTY_FUNCTION__, __LINE__);
    if (!env)
        return false;

    jclass clss = env->GetObjectClass(g_ActivityObject);
    if (!clss) {

        LOGE(LOG_FORMAT(" - Failed to get activity object class"), __PRETTY_FUNCTION__, __LINE__);
        assert(NULL);
        return false;
    }
    jmethodID mthd = env->GetMethodID(clss, "loadAd", "()V");
    if (!mthd) {

        LOGE(LOG_FORMAT(" - Failed to get 'loadAd' java method"), __PRETTY_FUNCTION__, __LINE__);
        assert(NULL);
        return false;
    }
    env->CallVoidMethod(g_ActivityObject, mthd);

#else
    LOGV(LIBENG_LOG_ADVERTISING, 0, LOG_FORMAT(" - (a:%p)"), __PRETTY_FUNCTION__, __LINE__, g_AppleOS);
    assert(g_AppleOS);

    [g_AppleOS loadAd];
#endif

    mLoading = true;
    return true;
}
bool Advertising::display(unsigned char id) {

#ifdef __ANDROID__
    LOGV(LIBENG_LOG_ADVERTISING, 0, LOG_FORMAT(" - i:%d (j:%p; a:%p)"), __PRETTY_FUNCTION__, __LINE__, id, g_JavaVM,
            g_ActivityObject);
    assert(g_JavaVM);
    assert(g_ActivityObject);

    JNIEnv* env = getJavaEnv(LIBENG_LOG_ADVERTISING, __PRETTY_FUNCTION__, __LINE__);
    if (!env)
        return false;

    jclass clss = env->GetObjectClass(g_ActivityObject);
    if (!clss) {

        LOGE(LOG_FORMAT(" - Failed to get activity object class"), __PRETTY_FUNCTION__, __LINE__);
        assert(NULL);
        return false;
    }
    jmethodID mthd = env->GetMethodID(clss, "displayAd", "(S)V");
    if (!mthd) {

        LOGE(LOG_FORMAT(" - Failed to get 'displayAd' java method"), __PRETTY_FUNCTION__, __LINE__);
        assert(NULL);
        return false;
    }
    env->CallVoidMethod(g_ActivityObject, mthd, static_cast<short>(id));

#else
    LOGV(LIBENG_LOG_ADVERTISING, 0, LOG_FORMAT(" - i:%d (a:%p)"), __PRETTY_FUNCTION__, __LINE__, id, g_AppleOS);
    assert(g_AppleOS);

    [g_AppleOS displayAd:id];
#endif
    return true;
}
unsigned char Advertising::refresh() {

    unsigned char adStatus = getStatus();
    if (mLoading) {
        if ((adStatus == Advertising::STATUS_DISPLAYED) || (adStatus == Advertising::STATUS_FAILED))
            resetPubDelay();
    }
    else if ((adStatus > Advertising::STATUS_LOADED) && (getPubDelay() > ADV_REFRESH_DELAY)) {

        load(); // Refresh advertising
        adStatus = Advertising::STATUS_LOADING;
    }
    return adStatus;
}
bool Advertising::hide(unsigned char id) {

#ifdef __ANDROID__
    LOGV(LIBENG_LOG_ADVERTISING, 0, LOG_FORMAT(" - i:%d (j:%p; a:%p)"), __PRETTY_FUNCTION__, __LINE__, id, g_JavaVM,
            g_ActivityObject);
    assert(g_JavaVM);
    assert(g_ActivityObject);

    JNIEnv* env = getJavaEnv(LIBENG_LOG_ADVERTISING, __PRETTY_FUNCTION__, __LINE__);
    if (!env)
        return false;

    jclass clss = env->GetObjectClass(g_ActivityObject);
    if (!clss) {

        LOGE(LOG_FORMAT(" - Failed to get activity object class"), __PRETTY_FUNCTION__, __LINE__);
        assert(NULL);
        return false;
    }
    jmethodID mthd = env->GetMethodID(clss, "hideAd", "(S)V");
    if (!mthd) {

        LOGE(LOG_FORMAT(" - Failed to get 'hideAd' java method"), __PRETTY_FUNCTION__, __LINE__);
        assert(NULL);
        return false;
    }
    env->CallVoidMethod(g_ActivityObject, mthd, static_cast<short>(id));

#else
    LOGV(LIBENG_LOG_ADVERTISING, 0, LOG_FORMAT(" - i:%d (a:%p)"), __PRETTY_FUNCTION__, __LINE__, id, g_AppleOS);
    assert(g_AppleOS);

    [g_AppleOS hideAd:id];
#endif
    return true;
}

unsigned char Advertising::getStatus() {

#ifdef __ANDROID__
    if (!g_JavaVM) {

        LOGE(LOG_FORMAT(" - No Java VM"), __PRETTY_FUNCTION__, __LINE__);
        assert(NULL);
        return STATUS_NONE;
    }
    if (!g_ActivityClass) {

        LOGE(LOG_FORMAT(" - No activity class"), __PRETTY_FUNCTION__, __LINE__);
        assert(NULL);
        return STATUS_NONE;
    }
    JNIEnv* env = getJavaEnv(LIBENG_LOG_ADVERTISING, __PRETTY_FUNCTION__, __LINE__);
    if (!env)
        return STATUS_NONE;

    jmethodID mthd = env->GetStaticMethodID(g_ActivityClass, "getAdStatus", "()S");
    if (!mthd) {

        LOGE(LOG_FORMAT(" - Failed to get 'getAdStatus' java method"), __PRETTY_FUNCTION__, __LINE__);
        assert(NULL);
        return STATUS_NONE;
    }
    return static_cast<unsigned char>(env->CallStaticShortMethod(g_ActivityClass, mthd));

#else
    assert(g_AppleOS);
    return [g_AppleOS getAdStatus];
#endif
}

void Advertising::resetPubDelay() {

    mPublished = time(NULL);
    mLoading = false;
}
unsigned char Advertising::getPubDelay() const {

    return static_cast<unsigned char>(difftime(time(NULL), mPublished));
}

} // namespace

#endif // LIBENG_ENABLE_ADVERTISING
