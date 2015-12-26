#ifndef GOOGLE_H_
#define GOOGLE_H_

#include <libeng/Social/Network.h>
#ifdef LIBENG_ENABLE_SOCIAL

#ifdef __ANDROID__
#include <jni.h>

#define LIBENG_GOOGLE_CLASS     "/social/EngGoogle"
#else
#import <Foundation/Foundation.h>
#endif

#define LIBENG_GOOGLE_URL       "https://plus.google.com/"

namespace eng {

//////
class Google : public Network {

public:
    static const unsigned char TEXTURE_ID = 0xfb;

    struct LinkData : ShareData {

        std::string url; // Link URL (REQUEST_SHARE_LINK) & File name (REQUEST_SHARE_VIDEO)
        std::string type;
        std::string title;
    };
#ifdef __ANDROID__
    static jobjectArray fillLinkData(JNIEnv* env, const ShareData* data) {

        jobjectArray linkData = (jobjectArray)env->NewObjectArray(3, env->FindClass("java/lang/String"), NULL);
        env->SetObjectArrayElement(linkData, 0, env->NewStringUTF(static_cast<const LinkData*>(data)->url.c_str()));
        env->SetObjectArrayElement(linkData, 1, env->NewStringUTF(static_cast<const LinkData*>(data)->type.c_str()));
        env->SetObjectArrayElement(linkData, 2, env->NewStringUTF(static_cast<const LinkData*>(data)->title.c_str()));
        return linkData;
    }
    static void delLinkData(JNIEnv* env, jobjectArray array) {

        env->DeleteLocalRef(env->GetObjectArrayElement(array, 0));
        env->DeleteLocalRef(env->GetObjectArrayElement(array, 1));
        env->DeleteLocalRef(env->GetObjectArrayElement(array, 2));

        env->DeleteLocalRef(array);
    }
#else
    static NSArray* fillLinkData(const ShareData* data) {

        NSArray* linkData = [NSArray arrayWithObjects:
            [NSString stringWithCString:static_cast<const LinkData*>(data)->url.c_str() encoding:NSASCIIStringEncoding],
            [NSString stringWithCString:static_cast<const LinkData*>(data)->type.c_str() encoding:NSASCIIStringEncoding],
            [NSString stringWithCString:static_cast<const LinkData*>(data)->title.c_str() encoding:NSASCIIStringEncoding],
            nil];
        return linkData;
    }
#endif

    typedef enum {

        // Login
        ERR_LOGIN_SERVICE_INVALID = Request::RESULT_FAILED + 1,
        ERR_LOGIN_SERVICE_DISABLED,
        ERR_LOGIN_SERVICE_MISSING,
        ERR_LOGIN_UPDATE_REQUIRED,
        ERR_LOGIN_DATE_INVALID,
        ERR_LOGIN_INVALID_ACCOUNT,
        ERR_LOGIN_UNLICENSED,

        // Info
        ERR_INFO_EMPTY

        // Picture
        // Share

    } Error;
};

#ifdef __ANDROID__
extern jclass g_GoogleClass;
#endif

} // namespace

#endif // LIBENG_ENABLE_SOCIAL
#endif // GOOGLE_H_
