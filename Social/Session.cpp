#include "Session.h"

#ifdef LIBENG_ENABLE_SOCIAL
#include <libeng/Tools/Tools.h>
#include <libeng/Textures/Textures.h>

#include <cstddef>
#include <string>

#ifdef __ANDROID__
#define JAVA_METHOD_LOGIN       "login"
#define JAVA_METHOD_ISLOGGED    "isLogged"
#define JAVA_METHOD_INFO        "getUserInfo"
#define JAVA_METHOD_PICTURE     "getUserPicture"
#define JAVA_METHOD_START       "start"
#define JAVA_METHOD_STOP        "stop"
#define JAVA_METHOD_LOGOUT      "logout"
#define JAVA_METHOD_USERID      "getUserID"
#define JAVA_METHOD_BIRTHDAY    "getUserBirthday"
#endif

namespace eng {

//////
Session::Session(Network::ID id, bool displayError, bool close) : mNetworkID(id), mRequest(REQUEST_NONE),
        mDisplayError(displayError), mUserPic(NULL), mLogout(close), mPicWidth(0), mPicHeight(0), mObject(NULL),
        mResult(static_cast<unsigned char>(Request::RESULT_NONE)) {

    LOGV(LIBENG_LOG_SOCIAL, 0, LOG_FORMAT(" - i:%d"), __PRETTY_FUNCTION__, __LINE__, static_cast<unsigned char>(id));
}
Session::~Session() {

    LOGV(LIBENG_LOG_SOCIAL, 0, LOG_FORMAT(), __PRETTY_FUNCTION__, __LINE__);
#ifdef __ANDROID__
    if (mObject) {

        JNIEnv* env = getJavaEnv(LIBENG_LOG_SOCIAL, __PRETTY_FUNCTION__, __LINE__);
        if (env) {

            if (mLogout)
                close();
            env->DeleteGlobalRef(mObject);
        }
    }
#else
    assert(g_AppleOS);
    if (mObject) {

        if (mLogout)
            close();
        [mObject remove];
    }
#endif
    unsigned char picTexIdx = TEXTURE_IDX_INVALID;
    switch (mNetworkID) {

        case Network::FACEBOOK: picTexIdx = Textures::getInstance()->getIndex(Facebook::TEXTURE_ID); break;
        case Network::TWITTER: picTexIdx = Textures::getInstance()->getIndex(Twitter::TEXTURE_ID); break;
        case Network::GOOGLE: picTexIdx = Textures::getInstance()->getIndex(Google::TEXTURE_ID); break;
    }
    if (picTexIdx != TEXTURE_IDX_INVALID)
        Textures::getInstance()->delTexture(picTexIdx); // The social picture texture is managed here

    if (mUserPic)
        delete [] mUserPic;
}

bool Session::create() {

#ifdef __ANDROID__
    LOGV(LIBENG_LOG_SOCIAL, 0, LOG_FORMAT(" - (d:%s; i:%d; j:%p; a:%p; f:%p; t:%p; g:%p)"), __PRETTY_FUNCTION__,
            __LINE__, (mDisplayError)? "true":"false", static_cast<unsigned char>(mNetworkID), g_JavaVM,
                    g_ActivityObject, g_FacebookClass, g_TwitterClass, g_GoogleClass);
    assert(g_JavaVM);
    assert(g_ActivityObject);
    assert(g_FacebookClass);
    assert(g_TwitterClass);
    assert(g_GoogleClass);
    assert(!mObject);

    JNIEnv* env = getJavaEnv(LIBENG_LOG_SOCIAL, __PRETTY_FUNCTION__, __LINE__);
    if (!env)
        return false;

    std::string mthdSign("(L");
    mthdSign.append(g_JavaProject);
    mthdSign.append(LIBENG_ACTIVITY_CLASS);
    mthdSign.append(";Z)V");

    jmethodID mthd = NULL;
    switch (mNetworkID) {

        case Network::FACEBOOK: mthd = env->GetMethodID(g_FacebookClass, "<init>", mthdSign.c_str()); break;
        case Network::TWITTER: mthd = env->GetMethodID(g_TwitterClass, "<init>", mthdSign.c_str()); break;
        case Network::GOOGLE: mthd = env->GetMethodID(g_GoogleClass, "<init>", mthdSign.c_str()); break;
        default: {

            LOGE(LOG_FORMAT(" - Unexpected social network ID: %d"), __PRETTY_FUNCTION__, __LINE__,
                    static_cast<unsigned char>(mNetworkID));
            assert(NULL);
            return false;
        }
    }
    if (!mthd) {

        LOGE(LOG_FORMAT(" - Failed to get social network class constructor (id:%d)"), __PRETTY_FUNCTION__, __LINE__,
                static_cast<unsigned char>(mNetworkID));
        assert(NULL);
        return false;
    }

    mObject = NULL;
    switch (mNetworkID) {

        case Network::FACEBOOK: {
            mObject = env->NewGlobalRef(env->NewObject(g_FacebookClass, mthd, g_ActivityObject, mDisplayError));
            break;
        }
        case Network::TWITTER: {
            mObject = env->NewGlobalRef(env->NewObject(g_TwitterClass, mthd, g_ActivityObject, mDisplayError));
            break;
        }
        case Network::GOOGLE: {
            mObject = env->NewGlobalRef(env->NewObject(g_GoogleClass, mthd, g_ActivityObject, mDisplayError));
            break;
        }
    }

#else
    LOGV(LIBENG_LOG_SOCIAL, 0, LOG_FORMAT(" - (i:%d; a:%p)"), __PRETTY_FUNCTION__, __LINE__,
         static_cast<unsigned char>(mNetworkID), g_AppleOS);
    assert(g_AppleOS);
    assert(!mObject);

    mObject = [g_AppleOS create:static_cast<unsigned char>(mNetworkID) byDisplayingError:mDisplayError];

#endif
    if (!mObject) {
        
        LOGW(LOG_FORMAT(" - Failed to get social network instance (id:%d)"), __PRETTY_FUNCTION__, __LINE__,
             static_cast<unsigned char>(mNetworkID));
        assert(NULL);
        return false;
    }
    return true;
}

#ifdef __ANDROID__
bool Session::callJavaMethod(const char* mthdName, bool boolean, jobject intent) const {

    LOGV(LIBENG_LOG_SOCIAL, 0, LOG_FORMAT(" - s:%s; i:%p"), __PRETTY_FUNCTION__, __LINE__, mthdName, intent);
    JNIEnv* env = getJavaEnv(LIBENG_LOG_SOCIAL, __PRETTY_FUNCTION__, __LINE__);
    if (!env)
        return false;

    jclass clss = env->GetObjectClass(mObject);
    if (!clss) {

        LOGE(LOG_FORMAT(" - Failed to get social network class (id:%d)"), __PRETTY_FUNCTION__, __LINE__,
                static_cast<unsigned char>(mNetworkID));
        assert(NULL);
        return false;
    }
    jmethodID mthd = env->GetMethodID(clss, mthdName, (boolean)? "()Z":((intent)? "(Landroid/content/Intent;)V":"()V"));
    if (!mthd) {

        LOGE(LOG_FORMAT(" - Failed to get '%s' java method (id:%d; it:%p)"), __PRETTY_FUNCTION__, __LINE__, mthdName,
                static_cast<unsigned char>(mNetworkID), intent);
        assert(NULL);
        return false;
    }
    if (boolean)
        return env->CallBooleanMethod(mObject, mthd);
    if (intent)
        env->CallVoidMethod(mObject, mthd, intent);
    else
        env->CallVoidMethod(mObject, mthd);
    return true;
}
bool Session::callJavaMethod(const char* mthdName, std::string& result) const {

    LOGV(LIBENG_LOG_SOCIAL, 0, LOG_FORMAT(" - s:%s"), __PRETTY_FUNCTION__, __LINE__, mthdName);
    JNIEnv* env = getJavaEnv(LIBENG_LOG_SOCIAL, __PRETTY_FUNCTION__, __LINE__);
    if (!env)
        return false;

    jclass clss = env->GetObjectClass(mObject);
    if (!clss) {

        LOGE(LOG_FORMAT(" - Failed to get social network class (id:%d)"), __PRETTY_FUNCTION__, __LINE__,
                static_cast<unsigned char>(mNetworkID));
        assert(NULL);
        return false;
    }
    jmethodID mthd = env->GetMethodID(clss, mthdName, "()Ljava/lang/String;");
    if (!mthd) {

        LOGE(LOG_FORMAT(" - Failed to get '%s' java method (id:%d)"), __PRETTY_FUNCTION__, __LINE__, mthdName,
                static_cast<unsigned char>(mNetworkID));
        assert(NULL);
        return false;
    }

    jstring str = static_cast<jstring>(env->CallObjectMethod(mObject, mthd));
    if (str) { // Defined (not null)

        const char* lpStr = env->GetStringUTFChars(str, 0);
        result.append(lpStr);
        env->ReleaseStringUTFChars(str, lpStr);

        // Check avoid #JNI1 issue case
        if (result == " ") result.clear(); // null
    }
#ifdef DEBUG
    else { // #JNI1 issue found

        LOGF(LOG_FORMAT(" - #JNI1 issue found"), __PRETTY_FUNCTION__, __LINE__);
        //assert(NULL); // Let's #JNI1 crash
    }
#endif
    return true;
}
#endif

bool Session::open() {

    if (!checkObject(__PRETTY_FUNCTION__, __LINE__))
        return false;

    mRequest = REQUEST_LOGIN;
    mResult = static_cast<unsigned char>(Request::RESULT_WAITING);
    // Set B4 calling method below coz can reply B4 leaving it

#ifdef __ANDROID__
    if (callJavaMethod(JAVA_METHOD_LOGIN, true))
        return true;
#else
    if ([mObject login])
        return true;
#endif
    mRequest = REQUEST_NONE;
    mResult = static_cast<unsigned char>(Request::RESULT_NONE);
    return false;
}
bool Session::close() const {

    LOGV(LIBENG_LOG_SOCIAL, 0, LOG_FORMAT(" - (i:%d; o:%p)"), __PRETTY_FUNCTION__, __LINE__,
         static_cast<unsigned char>(mNetworkID), mObject);
    if (mUserPic) {
        delete [] mUserPic;
        mUserPic = NULL;
    }
    if (isOpened())
#ifdef __ANDROID__
        return callJavaMethod(JAVA_METHOD_LOGOUT, false);
#else
        [mObject logout];
#endif
#ifdef DEBUG
    else {
        LOGW(LOG_FORMAT(" - Try to close a none opened session (id:%d)"), __PRETTY_FUNCTION__, __LINE__,
                static_cast<unsigned char>(mNetworkID));
    }
#endif
    return true;
}
bool Session::isOpened() const {

    if (!checkObject(__PRETTY_FUNCTION__, __LINE__))
        return false;
#ifdef __ANDROID__
    return callJavaMethod(JAVA_METHOD_ISLOGGED, true);
#else
    return [mObject isLogged];
#endif
}

bool Session::info() {

    if (!checkObject(__PRETTY_FUNCTION__, __LINE__))
        return false;

    mRequest = REQUEST_INFO;
    mResult = static_cast<unsigned char>(Request::RESULT_WAITING);
    // Set B4 calling method below coz can reply B4 leaving it

#ifdef __ANDROID__
    if (callJavaMethod(JAVA_METHOD_INFO, true))
        return true;
#else
    if ([mObject getUserInfo])
        return true;
#endif
    mRequest = REQUEST_NONE;
    mResult = static_cast<unsigned char>(Request::RESULT_NONE);
    return false;
}
bool Session::picture() {

    if (!checkObject(__PRETTY_FUNCTION__, __LINE__))
        return false;

    mRequest = REQUEST_PICTURE;
    mResult = static_cast<unsigned char>(Request::RESULT_WAITING);
    // Set B4 calling method below coz can reply B4 leaving it

#ifdef __ANDROID__
    if (callJavaMethod(JAVA_METHOD_PICTURE, true))
        return true;
#else
    if ([mObject getUserPicture])
        return true;
#endif
    mRequest = REQUEST_NONE;
    mResult = static_cast<unsigned char>(Request::RESULT_NONE);
    return false;
}

bool Session::shareLink(const ShareData* data) {

    if (!checkObject(__PRETTY_FUNCTION__, __LINE__))
        return false;

    LOGI(LIBENG_LOG_SOCIAL, 0, LOG_FORMAT(" - data:%p"), __PRETTY_FUNCTION__, __LINE__, data);
#ifdef __ANDROID__
    JNIEnv* env = getJavaEnv(LIBENG_LOG_SOCIAL, __PRETTY_FUNCTION__, __LINE__);
    if (!env)
        return false;

    jclass clss = env->GetObjectClass(mObject);
    if (!clss) {

        LOGE(LOG_FORMAT(" - Failed to get social network class (id:%d)"), __PRETTY_FUNCTION__, __LINE__,
                static_cast<unsigned char>(mNetworkID));
        assert(NULL);
        return false;
    }
    jmethodID mthd = env->GetMethodID(clss, "shareLink", "([Ljava/lang/String;)Z");
    if (!mthd) {

        LOGE(LOG_FORMAT(" - Failed to get 'shareLink' java method"), __PRETTY_FUNCTION__, __LINE__);
        assert(NULL);
        return false;
    }

    jobjectArray dataArray = NULL;
    if (data) {
        switch (mNetworkID) {

            case Network::FACEBOOK: dataArray = Facebook::fillLinkData(env, data); break;
            case Network::TWITTER: dataArray = Twitter::fillLinkData(env, data); break;
            case Network::GOOGLE: dataArray = Google::fillLinkData(env, data); break;
        }
    }
    mRequest = REQUEST_SHARE_LINK;
    mResult = static_cast<unsigned char>(Request::RESULT_WAITING);
    // Set B4 calling method below coz can reply B4 leaving it

    bool res = env->CallBooleanMethod(mObject, mthd, dataArray);
    if (data) {
        switch (mNetworkID) {

            case Network::FACEBOOK: Facebook::delLinkData(env, dataArray); break;
            case Network::TWITTER: Twitter::delLinkData(env, dataArray); break;
            case Network::GOOGLE: Google::delLinkData(env, dataArray); break;
        }
    }
    if (res)
        return true;
#else
    NSArray* dataArray = nil;
    if (data) {
        switch (mNetworkID) {

            case Network::FACEBOOK: dataArray = Facebook::fillLinkData(data); break;
            case Network::TWITTER: dataArray = Twitter::fillLinkData(data); break;
            case Network::GOOGLE: dataArray = Google::fillLinkData(data); break;
        }
    }
    mRequest = REQUEST_SHARE_LINK;
    mResult = static_cast<unsigned char>(Request::RESULT_WAITING);
    // Set B4 calling method below coz can reply B4 leaving it

    if ([mObject shareLink:dataArray])
        return true;
#endif
    mRequest = REQUEST_NONE;
    mResult = static_cast<unsigned char>(Request::RESULT_NONE);
    return false;
}
bool Session::shareVideo(const ShareData* data) {

    if (!checkObject(__PRETTY_FUNCTION__, __LINE__))
        return false;

    LOGI(LIBENG_LOG_SOCIAL, 0, LOG_FORMAT(" - data:%p"), __PRETTY_FUNCTION__, __LINE__, data);
#ifdef __ANDROID__
    JNIEnv* env = getJavaEnv(LIBENG_LOG_SOCIAL, __PRETTY_FUNCTION__, __LINE__);
    if (!env)
        return false;

    jclass clss = env->GetObjectClass(mObject);
    if (!clss) {

        LOGE(LOG_FORMAT(" - Failed to get social network class (id:%d)"), __PRETTY_FUNCTION__, __LINE__,
                static_cast<unsigned char>(mNetworkID));
        assert(NULL);
        return false;
    }
    jmethodID mthd = env->GetMethodID(clss, "shareVideo", "([Ljava/lang/String;)Z");
    if (!mthd) {

        LOGE(LOG_FORMAT(" - Failed to get 'shareVideo' java method"), __PRETTY_FUNCTION__, __LINE__);
        assert(NULL);
        return false;
    }

    jobjectArray dataArray = NULL;
    if (data) {
        switch (mNetworkID) {

            case Network::FACEBOOK: dataArray = Facebook::fillLinkData(env, data); break;
            case Network::TWITTER: dataArray = Twitter::fillLinkData(env, data); break;
            case Network::GOOGLE: dataArray = Google::fillLinkData(env, data); break;
        }
    }
    mRequest = REQUEST_SHARE_VIDEO;
    mResult = static_cast<unsigned char>(Request::RESULT_WAITING);
    // Set B4 calling method below coz can reply B4 leaving it

    bool res = env->CallBooleanMethod(mObject, mthd, dataArray);
    if (data) {
        switch (mNetworkID) {

            case Network::FACEBOOK: Facebook::delLinkData(env, dataArray); break;
            case Network::TWITTER: Twitter::delLinkData(env, dataArray); break;
            case Network::GOOGLE: Google::delLinkData(env, dataArray); break;
        }
    }
    if (res)
        return true;
#else
    NSArray* dataArray = nil;
    if (data) {
        switch (mNetworkID) {

            case Network::FACEBOOK: dataArray = Facebook::fillLinkData(data); break;
            case Network::TWITTER: dataArray = Twitter::fillLinkData(data); break;
            case Network::GOOGLE: dataArray = Google::fillLinkData(data); break;
        }
    }
    mRequest = REQUEST_SHARE_VIDEO;
    mResult = static_cast<unsigned char>(Request::RESULT_WAITING);
    // Set B4 calling method below coz can reply B4 leaving it

    if ([mObject shareVideo:dataArray])
        return true;

#endif
    mRequest = REQUEST_NONE;
    mResult = static_cast<unsigned char>(Request::RESULT_NONE);
    return false;
}

#ifdef __ANDROID__
void Session::start(jobject intent) const {

    if (!checkObject(__PRETTY_FUNCTION__, __LINE__))
        return;
    callJavaMethod(JAVA_METHOD_START, false, intent);
}
void Session::stop() const {

    if (!checkObject(__PRETTY_FUNCTION__, __LINE__))
        return;
    callJavaMethod(JAVA_METHOD_STOP, false);
}
void Session::intent(int req, int res, jobject data) const {

    if (!checkObject(__PRETTY_FUNCTION__, __LINE__))
        return;

    LOGI(LIBENG_LOG_SOCIAL, 0, LOG_FORMAT(" - r:%d; r:%d; d:%p"), __PRETTY_FUNCTION__, __LINE__, req, res, data);
    JNIEnv* env = getJavaEnv(LIBENG_LOG_SOCIAL, __PRETTY_FUNCTION__, __LINE__);
    if (!env)
        return;

    jclass clss = env->GetObjectClass(mObject);
    if (!clss) {

        LOGE(LOG_FORMAT(" - Failed to get social network class (id:%d)"), __PRETTY_FUNCTION__, __LINE__,
                static_cast<unsigned char>(mNetworkID));
        assert(NULL);
        return;
    }
    jmethodID mthd = env->GetMethodID(clss, "launch", "(IILandroid/content/Intent;)V");
    if (!mthd) {

        LOGE(LOG_FORMAT(" - Failed to get 'launch' java method"), __PRETTY_FUNCTION__, __LINE__);
        assert(NULL);
        return;
    }
    env->CallVoidMethod(mObject, mthd, req, res, data);
}
#else // !__ANDROID__
void Session::resume() {

    if (!checkObject(__PRETTY_FUNCTION__, __LINE__))
        return;
    [mObject resume];
}
void Session::terminate() {

    if (!checkObject(__PRETTY_FUNCTION__, __LINE__))
        return;
    [mObject terminate];
}
#endif

std::string Session::getUserID() const {

    std::string userID; // Empty string -> Error (cannot be NULL)
    if (!checkObject(__PRETTY_FUNCTION__, __LINE__))
        return userID;

#ifdef __ANDROID__
    callJavaMethod(JAVA_METHOD_USERID, userID);
#else
    userID.append([mObject.userID cStringUsingEncoding:NSASCIIStringEncoding]);
#endif
    return userID;
}
std::wstring Session::getUserName() const {

    std::wstring userName;
    if (!checkObject(__PRETTY_FUNCTION__, __LINE__))
        return userName;

#ifdef __ANDROID__
    JNIEnv* env = getJavaEnv(LIBENG_LOG_SOCIAL, __PRETTY_FUNCTION__, __LINE__);
    if (!env)
        return userName;

    jclass clss = env->GetObjectClass(mObject);
    if (!clss) {

        LOGE(LOG_FORMAT(" - Failed to get social network class (id:%d)"), __PRETTY_FUNCTION__, __LINE__,
                static_cast<unsigned char>(mNetworkID));
        assert(NULL);
        return userName;
    }
    jmethodID mthd = env->GetMethodID(clss, "getUserName", "()Ljava/lang/String;");
    if (!mthd) {

        LOGE(LOG_FORMAT(" - Failed to get 'getUserName' java method"), __PRETTY_FUNCTION__, __LINE__);
        assert(NULL);
        return userName;
    }

    jstring str = static_cast<jstring>(env->CallObjectMethod(mObject, mthd));
    if (str) { // Defined (not null)

        jsize len = env->GetStringLength(str);
        const jchar* jstr = env->GetStringChars(str, 0);

        wchar_t* wstr = new wchar_t[len + 1];
        memset(wstr, 0, sizeof(wchar_t) * (len + 1));
        for (jsize i = 0; i < len; ++i)
            wstr[i] = static_cast<wchar_t>(jstr[i]);
        userName.append(wstr, len);

        delete [] wstr;
        env->ReleaseStringChars(str, jstr);

        // Check avoid #JNI1 issue case
        if (userName == L" ") userName.clear(); // null
    }
#ifdef DEBUG
    else { // #JNI1 issue found

        LOGF(LOG_FORMAT(" - #JNI1 issue found"), __PRETTY_FUNCTION__, __LINE__);
        //assert(NULL); // Let's #JNI1 crash
    }
#endif
#else
    if (mObject.userName != nil)
        for (short i = 0; i < static_cast<short>([mObject.userName length]); ++i)
            userName += [mObject.userName characterAtIndex:i];
#endif
    return userName;
}
unsigned char Session::getUserGender() const {

    if (!checkObject(__PRETTY_FUNCTION__, __LINE__))
        return GENDER_NONE;

#ifdef __ANDROID__
    JNIEnv* env = getJavaEnv(LIBENG_LOG_SOCIAL, __PRETTY_FUNCTION__, __LINE__);
    if (!env)
        return GENDER_NONE;

    jclass clss = env->GetObjectClass(mObject);
    if (!clss) {

        LOGE(LOG_FORMAT(" - Failed to get social network class (id:%d)"), __PRETTY_FUNCTION__, __LINE__,
                static_cast<unsigned char>(mNetworkID));
        assert(NULL);
        return GENDER_NONE;
    }
    jmethodID mthd = env->GetMethodID(clss, "getUserGender", "()S");
    if (!mthd) {

        LOGE(LOG_FORMAT(" - Failed to get 'getUserGender' java method"), __PRETTY_FUNCTION__, __LINE__);
        assert(NULL);
        return GENDER_NONE;
    }
    return static_cast<unsigned char>(env->CallShortMethod(mObject, mthd));

#else
    return mObject.userGender;
#endif
}
std::string Session::getUserBirthday() const {

    std::string birthday;
    if (!checkObject(__PRETTY_FUNCTION__, __LINE__))
        return birthday;

#ifdef __ANDROID__
    callJavaMethod(JAVA_METHOD_BIRTHDAY, birthday);
#else
    if (mObject.userBirthday != nil) {

        NSDateFormatter* dateFormatter = [[NSDateFormatter alloc] init];
        [dateFormatter setDateFormat:@"MM/dd/yyyy"];

        birthday.append([[dateFormatter stringFromDate:mObject.userBirthday] cStringUsingEncoding:NSASCIIStringEncoding]);
    }
#endif
    return birthday;
}

void Session::setPicture(unsigned char result, short width, short height, unsigned char* data) {

    LOGV(LIBENG_LOG_SOCIAL, 0, LOG_FORMAT(" - r:%d; w:%d; h:%d; d:%p (i:%d)"), __PRETTY_FUNCTION__, __LINE__, result,
            width, height, data, static_cast<unsigned char>(mNetworkID));

    mResult = result;
    if (mResult != static_cast<unsigned char>(Request::RESULT_SUCCEEDED))
        return;

#ifdef DEBUG
    LOGI(LIBENG_LOG_SOCIAL, 0, LOG_FORMAT(), __PRETTY_FUNCTION__, __LINE__);
    assert(width);
    assert(height);
    assert(data);

    switch (mNetworkID) {

        case Network::FACEBOOK: {
            if (Textures::getInstance()->getIndex(Facebook::TEXTURE_ID) == TEXTURE_IDX_INVALID) {
                LOGW(LOG_FORMAT(" - Default Facebook picture not loaded"), __PRETTY_FUNCTION__, __LINE__);
                assert(NULL);
            }
            break;
        }
        case Network::TWITTER: {
            if (Textures::getInstance()->getIndex(Twitter::TEXTURE_ID) == TEXTURE_IDX_INVALID) {
                LOGW(LOG_FORMAT(" - Default Twitter picture not loaded"), __PRETTY_FUNCTION__, __LINE__);
                assert(NULL);
            }
            break;
        }
        case Network::GOOGLE: {
            if (Textures::getInstance()->getIndex(Google::TEXTURE_ID) == TEXTURE_IDX_INVALID) {
                LOGW(LOG_FORMAT(" - Default Google picture not loaded"), __PRETTY_FUNCTION__, __LINE__);
                assert(NULL);
            }
            break;
        }
    }
#endif
    mMutex.lock();
    mPicWidth = width;
    mPicHeight = height;

    if (mUserPic)
        delete [] mUserPic;
    mUserPic = data;
    mMutex.unlock();
}

} // namespace

#endif // LIBENG_ENABLE_SOCIAL
