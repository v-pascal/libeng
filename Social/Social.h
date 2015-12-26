#ifndef SOCIAL_H_
#define SOCIAL_H_
#if defined(__cplusplus)

#include <libeng/Global.h>
#ifdef LIBENG_ENABLE_SOCIAL

#include <libeng/Social/Networks/Facebook.h>
#include <libeng/Social/Networks/Twitter.h>
#include <libeng/Social/Networks/Google.h>

#include <libeng/Social/Session.h>
#include <libeng/Social/Network.h>

#include <cstddef>
#include <map>

namespace eng {

//////
class Social {

    friend class PlatformOS;
    friend class Textures;

private:
    Social(bool displayError);
    virtual ~Social();

    static Social* mThis;

    typedef std::map<Network::ID, Session*> SocialNetworks;
    SocialNetworks mNetworks;

    bool mDisplayError; // TRUE: An alert message is displayed when error occured; FALSE: No error message is displayed
    Session* getSession(Network::ID id);

public:
    static Social* getInstance(bool displayError) {
        if (!mThis)
            mThis = new Social(displayError);
        return mThis;
    }
    static void freeInstance() {
        if (mThis) {
            delete mThis;
            mThis = NULL;
        }
    }

    const Session* _getSession(Network::ID id) const;
    // WARNING: Check return value coz can return NULL without any assertion
    // BUG: Try to use 'private:getSession' method instead of this one when it called from a none friend class !?!?
    // -> Has been renamed from 'getSession' to '_getSession' in order to compile

    //////
    bool addSession(Network::ID id, bool logout);
#ifdef LIBENG_ENABLE_ADVERTISING
    bool setAdReqInfo(Network::ID id) const;
#endif

#ifdef __ANDROID__
    void start(jobject intent) const;
    void stop() const;
    void intent(int req, int res, jobject data) const;
#else
    void resume();
    void terminate();
#endif

    bool request(Network::ID id, Session::RequestID request, const ShareData* data);
    // WARNING: Return result of the request sent (see 'getResult' to get request result)

};

} // namespace

#endif // LIBENG_ENABLE_SOCIAL
#endif // __cplusplus
#endif // SOCIAL_H_
