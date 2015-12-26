#ifndef TWITTER_H_
#define TWITTER_H_

#include <libeng/Social/Network.h>
#ifdef LIBENG_ENABLE_SOCIAL

#ifdef __ANDROID__
#include <jni.h>

#define LIBENG_TWITTER_CLASS        "/social/EngTwitter"
#else
#import <Foundation/Foundation.h>
#endif

#define LIBENG_TWITTER_URL          "https://twitter.com/"

namespace eng {

//////
class Twitter : public Network {

public:
    static const unsigned char TEXTURE_ID = 0xfc;

    struct LinkData : ShareData {

        std::string tweet;
        std::string media;
    };
#ifdef __ANDROID__
    static jobjectArray fillLinkData(JNIEnv* env, const ShareData* data) {

        jobjectArray linkData = (jobjectArray)env->NewObjectArray(2, env->FindClass("java/lang/String"), NULL);
        env->SetObjectArrayElement(linkData, 0, env->NewStringUTF(static_cast<const LinkData*>(data)->tweet.c_str()));
        env->SetObjectArrayElement(linkData, 1, env->NewStringUTF(static_cast<const LinkData*>(data)->media.c_str()));
        return linkData;
    }
    static void delLinkData(JNIEnv* env, jobjectArray array) {

        env->DeleteLocalRef(env->GetObjectArrayElement(array, 0));
        env->DeleteLocalRef(env->GetObjectArrayElement(array, 1));

        env->DeleteLocalRef(array);
    }
#else
    static NSArray* fillLinkData(const ShareData* data) {




        //NotImplemented();



        
        return NULL;
    }
#endif

    typedef enum {

        // Login
        // Info
        // Picture
        // Share

    } Error;
};

#ifdef __ANDROID__
extern jclass g_TwitterClass;
#endif

} // namespace

#endif // LIBENG_ENABLE_SOCIAL
#endif // TWITTER_H_
