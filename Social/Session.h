#ifndef SESSION_H_
#define SESSION_H_
#if defined(__cplusplus)

#include <libeng/Global.h>
#ifdef LIBENG_ENABLE_SOCIAL

#ifdef __ANDROID__
#include <jni.h>
#else
#include <libeng/AppleOS.h>
#endif

#include <libeng/Log/Log.h>
#include <libeng/Social/Network.h>
#include <string>
#include <assert.h>

#include <boost/thread.hpp>

namespace eng {

//////
class Session {

public:
    typedef enum {

        REQUEST_NONE = 0,

        REQUEST_LOGIN,      // Login
        REQUEST_INFO,       // Get user info
        REQUEST_PICTURE,    // Get user picture
        REQUEST_SHARE_LINK, // Share a link
        REQUEST_SHARE_VIDEO // Share a video

    } RequestID;
#ifdef DEBUG
    static void check(RequestID id) {
        assert((static_cast<unsigned char>(id) == REQUEST_LOGIN) ||
            (static_cast<unsigned char>(id) == REQUEST_INFO) ||
            (static_cast<unsigned char>(id) == REQUEST_PICTURE) ||
            (static_cast<unsigned char>(id) == REQUEST_SHARE_LINK) ||
            (static_cast<unsigned char>(id) == REQUEST_SHARE_VIDEO));
    }
#endif

private:
    Network::ID mNetworkID;
    bool mDisplayError; // See 'Social::mDisplayError' variable description
    bool mLogout; // TRUE: Logout when the session is deleted; FALSE: Keep logged
#ifdef __ANDROID__
    jobject mObject;

    bool callJavaMethod(const char* mthdName, bool boolean, jobject intent = NULL) const;
    bool callJavaMethod(const char* mthdName, std::string& result) const;
#else
    id<SocialOS> mObject;
#endif
    inline bool checkObject(const char* function, int line) const {

        LOGV(LIBENG_LOG_SOCIAL, 0, LOG_FORMAT(" - (i:%d; o:%p)"), function, line,
             static_cast<unsigned char>(mNetworkID), mObject);
        if (!mObject) {
            
            LOGW(LOG_FORMAT(" - Social network instance not created (id:%d)"), function, line,
                 static_cast<unsigned char>(mNetworkID));
            assert(NULL);
            return false;
        }
        return true;
    }
    RequestID mRequest;
    unsigned char mResult;

    boost::mutex mMutex; // Picture buffer protection (see 'Textures::addTexPic' method)

    short mPicWidth;
    short mPicHeight;
    mutable unsigned char* mUserPic;

public:
    Session(Network::ID id, bool displayError, bool close);
    virtual ~Session();

    void lock() { mMutex.lock(); }
    void unlock() { mMutex.unlock(); }

    inline RequestID getRequest() const { return mRequest; }
    inline unsigned char getResult() const { return mResult; }

    inline short getPicWidth() const { return mPicWidth; }
    inline short getPicHeight() const { return mPicHeight; }
    inline unsigned char* getUserPic() const { return mUserPic; }

    //////
    bool create(); // Create social network object instance

    bool open();
    bool close() const;
    bool info();
    bool picture();

    bool shareLink(const ShareData* data);
    bool shareVideo(const ShareData* data);

#ifdef __ANDROID__
    inline jobject getObject() const { return mObject; }

    void start(jobject intent) const;
    void stop() const;
    void intent(int req, int res, jobject data) const;
#else
    inline id<SocialOS> getObject() const { return mObject; }

    void resume();
    void terminate();
#endif

    bool isOpened() const;
    std::string getUserID() const;
    std::wstring getUserName() const;
    unsigned char getUserGender() const;
    std::string getUserBirthday() const; // MM/dd/yyyy

    inline void setResult(RequestID request, unsigned char result) {

        LOGV(LIBENG_LOG_SOCIAL, 0, LOG_FORMAT(" - r:%d; rs:%d (r:%d)"), __PRETTY_FUNCTION__, __LINE__, request, result,
                mRequest);
        if (mRequest == request)
            mResult = result;
#ifdef DEBUG
        else {
            LOGW(LOG_FORMAT(" - Received %d result from previous %d request (%d)"), __PRETTY_FUNCTION__, __LINE__,
                    result, request, mRequest);
        }
#endif
    }
    void setPicture(unsigned char result, short width, short height, unsigned char* data);

};

} // namespace

#endif // LIBENG_ENABLE_SOCIAL
#endif // __cplusplus
#endif // SESSION_H_
