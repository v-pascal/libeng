#include "Social.h"

#ifdef LIBENG_ENABLE_SOCIAL
#include <libeng/Log/Log.h>
#include <libeng/Features/Internet/Internet.h>
#include <libeng/Tools/Tools.h>

#ifdef __ANDROID__
#define SOCIAL_CLASS    "/social/EngSocial"
#endif
#include <cstddef>
#include <assert.h>

namespace eng {

Social* Social::mThis = NULL;

//////
Social::Social(bool displayError) : mDisplayError(displayError) {

    LOGV(LIBENG_LOG_SOCIAL, 0, LOG_FORMAT(" - d:%s"), __PRETTY_FUNCTION__, __LINE__, (displayError)? "true":"false");
}
Social::~Social() {

    LOGV(LIBENG_LOG_SOCIAL, 0, LOG_FORMAT(), __PRETTY_FUNCTION__, __LINE__);
    for (SocialNetworks::iterator iter = mNetworks.begin(); iter != mNetworks.end(); ++iter)
        delete iter->second;
    mNetworks.clear();
}

Session* Social::getSession(Network::ID id) {

    SocialNetworks::iterator network = mNetworks.find(id);
#ifdef DEBUG
    if (network != mNetworks.end()) {

        assert(network->second);
        return network->second;
    }
#else
    if (network != mNetworks.end())
        return network->second;
#endif
    return NULL;
}
const Session* Social::_getSession(Network::ID id) const {

    SocialNetworks::const_iterator network = mNetworks.find(id);
#ifdef DEBUG
    if (network != mNetworks.end()) {

        assert(network->second);
        return network->second;
    }
#else
    if (network != mNetworks.end())
        return network->second;
#endif
    return NULL;
}

bool Social::addSession(Network::ID id, bool logout) {

#ifdef DEBUG
    LOGV(LIBENG_LOG_SOCIAL, 0, LOG_FORMAT(" - i:%d; l:%s"), __PRETTY_FUNCTION__, __LINE__,
            static_cast<unsigned char>(id), (logout)? "true":"false");
    Network::check(id);

    assert(!_getSession(id));
#endif
    Session* session = new Session(id, mDisplayError, logout);

    mNetworks[id] = session;
    return session->create();
}
#ifdef LIBENG_ENABLE_ADVERTISING
bool Social::setAdReqInfo(Network::ID id) const {

#ifdef __ANDROID__
#ifdef DEBUG
    LOGV(LIBENG_LOG_SOCIAL, 0, LOG_FORMAT(" - i:%d (j:%p; a:%p)"), __PRETTY_FUNCTION__, __LINE__,
            static_cast<unsigned char>(id), g_JavaVM, g_ActivityClass);
    assert(_getSession(id));
    assert(_getSession(id)->getObject());
    assert(g_JavaVM);
    assert(g_ActivityClass);

    Network::check(id);
#endif
    if ((!_getSession(id)) || (!_getSession(id)->getObject()))
        return false;

    JNIEnv* env = getJavaEnv(LIBENG_LOG_SOCIAL, __PRETTY_FUNCTION__, __LINE__);
    if (!env)
        return false;

    std::string mthdSign("(L");
    mthdSign.append(g_JavaProject);
    mthdSign.append(SOCIAL_CLASS);
    mthdSign.append(";)V");

    jmethodID mthd = env->GetStaticMethodID(g_ActivityClass, "setAdReqInfo", mthdSign.c_str());
    if (!mthd) {

        LOGE(LOG_FORMAT(" - Failed to get 'setAdReqInfo' java method"), __PRETTY_FUNCTION__, __LINE__);
        assert(NULL);
        return false;
    }
    env->CallStaticVoidMethod(g_ActivityClass, mthd, _getSession(id)->getObject());

#else // !__ANDROID__
#ifdef DEBUG
    LOGV(LIBENG_LOG_SOCIAL, 0, LOG_FORMAT(" - i:%d (a:%p)"), __PRETTY_FUNCTION__, __LINE__, g_AppleOS);
    assert(g_AppleOS);
    assert(_getSession(id));
    assert(_getSession(id)->getObject());

    Network::check(id);
#endif
    if ((!g_AppleOS) || (!_getSession(id)) || (!_getSession(id)->getObject()))
        return false;

    [g_AppleOS setAdReqInfo:_getSession(id)->getObject()];
#endif
    return true;
}
#endif

#ifdef __ANDROID__
void Social::start(jobject intent) const {

    LOGV(LIBENG_LOG_SOCIAL, 0, LOG_FORMAT(" - i:%p (s:%u)"), __PRETTY_FUNCTION__, __LINE__, intent,
            static_cast<unsigned int>(mNetworks.size()));
    for (SocialNetworks::const_iterator iter = mNetworks.begin(); iter != mNetworks.end(); ++iter)
        iter->second->start(intent);
}
void Social::stop() const {

    LOGV(LIBENG_LOG_SOCIAL, 0, LOG_FORMAT(" - (s:%u)"), __PRETTY_FUNCTION__, __LINE__,
            static_cast<unsigned int>(mNetworks.size()));
    for (SocialNetworks::const_iterator iter = mNetworks.begin(); iter != mNetworks.end(); ++iter)
        iter->second->stop();
}
void Social::intent(int req, int res, jobject data) const {

    LOGV(LIBENG_LOG_SOCIAL, 0, LOG_FORMAT(" - r:%d; e:%d; d:%p (s:%u)"), __PRETTY_FUNCTION__, __LINE__, req, res,
         data, static_cast<unsigned int>(mNetworks.size()));
    for (SocialNetworks::const_iterator iter = mNetworks.begin(); iter != mNetworks.end(); ++iter)
        iter->second->intent(req, res, data);
}
#else // !__ANDROID__
void Social::resume() {

    LOGV(LIBENG_LOG_SOCIAL, 0, LOG_FORMAT(" - (s:%u)"), __PRETTY_FUNCTION__, __LINE__,
         static_cast<unsigned int>(mNetworks.size()));
    for (SocialNetworks::iterator iter = mNetworks.begin(); iter != mNetworks.end(); ++iter)
        iter->second->resume();
}
void Social::terminate() {

    LOGV(LIBENG_LOG_SOCIAL, 0, LOG_FORMAT(" - (s:%u)"), __PRETTY_FUNCTION__, __LINE__,
         static_cast<unsigned int>(mNetworks.size()));
    for (SocialNetworks::iterator iter = mNetworks.begin(); iter != mNetworks.end(); ++iter)
        iter->second->terminate();
}
#endif

bool Social::request(Network::ID id, Session::RequestID request, const ShareData* data) {

#ifdef DEBUG
    LOGV(LIBENG_LOG_SOCIAL, 0, LOG_FORMAT(" - i:%d; r:%d; d:%p"), __PRETTY_FUNCTION__, __LINE__,
            static_cast<unsigned char>(id), static_cast<unsigned char>(request), data);
    Network::check(id);
#endif
    //if (!Internet::isOnline())
    //    return false;
    // -> Already checked into the OS to be able to display an alert message if requested (see 'mDisplayError' member)

    Session* session = getSession(id);
#ifdef DEBUG
    if (!session) {
        LOGW(LOG_FORMAT(" - Network %d session not created"), __PRETTY_FUNCTION__, __LINE__,
                static_cast<unsigned char>(id));
        assert(NULL);
    }
#endif
    switch (request) {
        case Session::REQUEST_LOGIN: {

#ifdef DEBUG
            if (session->isOpened()) {

                LOGW(LOG_FORMAT(" - Network %d session already opened"), __PRETTY_FUNCTION__, __LINE__,
                        static_cast<unsigned char>(id));
                return true;
            }
            return session->open();
#else
            if (session->isOpened())
                return true;

            return session->open();
#endif
        }
        case Session::REQUEST_INFO: return session->info();
        case Session::REQUEST_PICTURE: return session->picture();
        case Session::REQUEST_SHARE_LINK: return session->shareLink(data);
        case Session::REQUEST_SHARE_VIDEO: return session->shareVideo(data);
        default: {

            LOGE(LOG_FORMAT(" - Unknown request ID: %d"), __PRETTY_FUNCTION__, __LINE__,
                    static_cast<unsigned char>(id));
            assert(NULL);
            break;
        }
    }
    return false;
}

} // namespace

#endif // LIBENG_ENABLE_SOCIAL
