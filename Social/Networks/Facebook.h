#ifndef FACEBOOK_H_
#define FACEBOOK_H_
#if defined(__cplusplus)

#include <libeng/Social/Network.h>
#ifdef LIBENG_ENABLE_SOCIAL

#ifdef __ANDROID__
#include <jni.h>

#define LIBENG_FACEBOOK_CLASS   "/social/EngFacebook"
#else
#import <Foundation/Foundation.h>
#endif
#include <string>

#define LIBENG_FACEBOOK_URL     "https://www.facebook.com/app_scoped_user_id/"

namespace eng {

//////
class Facebook : public Network {

public:
    static const unsigned char TEXTURE_ID = 0xfd;

    struct LinkData : ShareData {

        std::string name; // Link name (REQUEST_SHARE_LINK) & File name (REQUEST_SHARE_VIDEO)
        std::string caption;
        std::string description;
        std::string link;
        std::string picture;
    };
#ifdef __ANDROID__
    static jobjectArray fillLinkData(JNIEnv* env, const ShareData* data) {

        jobjectArray linkData = (jobjectArray)env->NewObjectArray(5, env->FindClass("java/lang/String"), NULL);
        env->SetObjectArrayElement(linkData, 0, env->NewStringUTF(static_cast<const LinkData*>(data)->name.c_str()));
        env->SetObjectArrayElement(linkData, 1, env->NewStringUTF(static_cast<const LinkData*>(data)->caption.c_str()));
        env->SetObjectArrayElement(linkData, 2, env->NewStringUTF(static_cast<const LinkData*>(data)->description.c_str()));
        env->SetObjectArrayElement(linkData, 3, env->NewStringUTF(static_cast<const LinkData*>(data)->link.c_str()));
        env->SetObjectArrayElement(linkData, 4, env->NewStringUTF(static_cast<const LinkData*>(data)->picture.c_str()));
        return linkData;
    }
    static void delLinkData(JNIEnv* env, jobjectArray array) {

        env->DeleteLocalRef(env->GetObjectArrayElement(array, 0));
        env->DeleteLocalRef(env->GetObjectArrayElement(array, 1));
        env->DeleteLocalRef(env->GetObjectArrayElement(array, 2));
        env->DeleteLocalRef(env->GetObjectArrayElement(array, 3));
        env->DeleteLocalRef(env->GetObjectArrayElement(array, 4));

        env->DeleteLocalRef(array);
    }
#else
    static NSArray* fillLinkData(const ShareData* data) {

        NSArray* linkData = [NSArray arrayWithObjects:
            [NSString stringWithCString:static_cast<const LinkData*>(data)->name.c_str() encoding:NSASCIIStringEncoding],
            [NSString stringWithCString:static_cast<const LinkData*>(data)->caption.c_str() encoding:NSASCIIStringEncoding],
            [NSString stringWithCString:static_cast<const LinkData*>(data)->description.c_str() encoding:NSASCIIStringEncoding],
            [NSString stringWithCString:static_cast<const LinkData*>(data)->link.c_str() encoding:NSASCIIStringEncoding],
            [NSString stringWithCString:static_cast<const LinkData*>(data)->picture.c_str() encoding:NSASCIIStringEncoding],
            nil];
        return linkData;
    }
#endif

    typedef enum {

        // Login
        ERR_LOGIN_AUTHORIZATION = Request::RESULT_FAILED + 1,
        ERR_LOGIN_DIALOG,
        ERR_LOGIN_SERVICE,

        // Info
        ERR_INFO_JSON,
        ERR_INFO_EMPTY

        // Picture
        // Share

    } Error;
};

#ifdef __ANDROID__
extern jclass g_FacebookClass;
#endif

} // namespace

#endif // LIBENG_ENABLE_SOCIAL
#endif // __cplusplus
#endif // FACEBOOK_H_
