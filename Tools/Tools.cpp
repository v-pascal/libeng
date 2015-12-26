#include "Tools.h"

#ifdef __ANDROID__
#include <GLES2/gl2.h>
#else
#include <OpenGLES/ES2/gl.h>
#include <unistd.h>
#endif

#include <libeng/Log/Log.h>

#include <cctype>
#include <iomanip>
#include <sstream>

namespace eng {

#ifdef __ANDROID__
extern "C" JNIEnv* getJavaEnv(unsigned char logLevel, const char* function, int line) {

    JNIEnv* env = NULL;
    if (g_JavaVM->GetEnv(reinterpret_cast<void**>(&env), JNI_VERSION_1_6) == JNI_EDETACHED) {

        LOGI(logLevel, 0, LOG_FORMAT(" - JNI environment is detached"), function, line);
        if (g_JavaVM->AttachCurrentThread(&env, NULL) != JNI_OK) {

            LOGE(LOG_FORMAT(" - Failed to attach Java VM with current thread"), function, line);
            assert(NULL);
            return NULL;
        }
    }
    if (!env) {

        LOGE(LOG_FORMAT(" - Failed to get JNI environment"), function, line);
        assert(NULL);
        return NULL;
    }
    return env;
}
extern "C" void detachThreadJVM(unsigned char logLevel) {

    LOGV(logLevel, 0, LOG_FORMAT(" - (g:%p)"), __PRETTY_FUNCTION__, __LINE__, g_JavaVM);
    assert(g_JavaVM);

    JNIEnv* env = NULL;
    if (g_JavaVM->GetEnv(reinterpret_cast<void**>(&env), JNI_VERSION_1_6) != JNI_EDETACHED) {
        if (g_JavaVM->DetachCurrentThread() != JNI_OK) {

            LOGE(LOG_FORMAT(" - Failed to detach Java VM with current thread"), __PRETTY_FUNCTION__, __LINE__);
        }
    }
}
extern "C" wchar_t* java2wstring(const jchar* utf16, jsize len) {

    assert(!(len % 2));
    len >>= 1;

    wchar_t* utf32 = new wchar_t[len + 1];
    memset(utf32, 0, sizeof(wchar_t) * (len + 1));

    for (jsize i = 0; i < len; ++i)
        utf32[i] = static_cast<wchar_t>(utf16[(i << 1)] << 16) | static_cast<wchar_t>(utf16[(i << 1) + 1]);

    return utf32;
}
extern "C" jbyte* wstring2byteArray(const std::wstring& wString) {

    size_t len = wString.size() << 1;
    jbyte* res = new jbyte[len];
    memset(res, 0, sizeof(jbyte) * len);

    for (size_t i = 0; i < wString.size(); ++i) {

        if (static_cast<unsigned int>(wString.at(i)) & 0xFFFF0000) {
            LOGE(LOG_FORMAT(), __PRETTY_FUNCTION__, __LINE__);
            assert(NULL);
        }
        res[i << 1] = static_cast<jbyte>((static_cast<unsigned int>(wString.at(i)) >> 16) & 0x0000FF00);
        res[(i << 1) + 1] = static_cast<jbyte>(static_cast<unsigned int>(wString.at(i)) & 0x000000FF);
    }
    return res;
}
#endif

GLuint loadShader(GLenum type, const char* prog) {

    LOGV(LIBENG_LOG_RENDER, 0, LOG_FORMAT(" - %s: p:%p"), __PRETTY_FUNCTION__, __LINE__,
            (type == GL_VERTEX_SHADER)? "GL_VERTEX_SHADER":"GL_FRAGMENT_SHADER", prog);

    // Create the shader object
    GLuint shaderId = glCreateShader(type);
    if (!shaderId) {

        LOGE(LOG_FORMAT(" - glCreateShader failed"), __PRETTY_FUNCTION__, __LINE__);
        return 0;
    }

    // Load the shader source
    glShaderSource(shaderId, 1, &prog, NULL);

    // Compile the vertex shader
    glCompileShader(shaderId);

    // Check the compile status
    GLint compileStatus;
    glGetShaderiv(shaderId, GL_COMPILE_STATUS, &compileStatus);

    if (compileStatus == GL_FALSE) {

        GLsizei logLen;
        GLchar infoLog[128];
        glGetShaderInfoLog(shaderId, 128, &logLen, infoLog);
        glDeleteShader(shaderId);

        LOGE(LOG_FORMAT(" - Failed to compile shader: %s"), __PRETTY_FUNCTION__, __LINE__, infoLog);
        return 0;
    }
    return shaderId;
}

extern "C" GLuint loadProgram(const char* vertexProg, const char* fragmentProg) {

    LOGV(LIBENG_LOG_RENDER, 0, LOG_FORMAT(" - v:%p; f:%x"), __PRETTY_FUNCTION__, __LINE__, vertexProg, fragmentProg);

    // Load vertex shader
    GLuint vertexShaderId = loadShader(GL_VERTEX_SHADER, vertexProg);
    if (!vertexShaderId)
        return 0;

    // Load fragment shader
    GLuint fragmentShaderId = loadShader(GL_FRAGMENT_SHADER, fragmentProg);
    if (!fragmentShaderId)
        return 0;

    // Create the program object
    GLuint progObj = glCreateProgram();
    if (!progObj) {

        LOGE(LOG_FORMAT(" - glCreateProgram failed"), __PRETTY_FUNCTION__, __LINE__);
        return 0;
    }

    glAttachShader(progObj, vertexShaderId);
    glAttachShader(progObj, fragmentShaderId);

    // Link the program
    glLinkProgram(progObj);

    // Check the link status
    GLint linked;
    glGetProgramiv(progObj, GL_LINK_STATUS, &linked);

    if (linked == GL_FALSE) {

        GLsizei logLen;
        GLchar infoLog[128];
        glGetShaderInfoLog(vertexShaderId, 128, &logLen, infoLog);
        glDeleteProgram(progObj);

        LOGE(LOG_FORMAT(" - Error linking program: %s"), __PRETTY_FUNCTION__, __LINE__, infoLog);
        return 0;
    }

    // Free up no longer needed shader resources
    glDeleteShader(vertexShaderId);
    glDeleteShader(fragmentShaderId);

    return progObj;
}

#ifdef __ANDROID__
extern std::string getUID(UID::Type type, unsigned char logLevel) {

    LOGV(logLevel, 0, LOG_FORMAT(" - (j:%p; a:%p)"), __PRETTY_FUNCTION__, __LINE__, g_JavaVM, g_ActivityObject);
    assert(g_JavaVM);
    assert(g_ActivityObject);

    std::string uid(LIBENG_UID_ERROR); // Error
    JNIEnv* env = getJavaEnv(logLevel, __PRETTY_FUNCTION__, __LINE__);
    if (!env)
        return uid;

    jclass clss = env->GetObjectClass(g_ActivityObject);
    if (!clss) {

        LOGE(LOG_FORMAT(" - Failed to get activity object class"), __PRETTY_FUNCTION__, __LINE__);
        assert(NULL);
        return uid;
    }
    jmethodID mthd = env->GetMethodID(clss, "getUID", "(S)Ljava/lang/String;");
    if (!mthd) {

        LOGE(LOG_FORMAT(" - Failed to get 'getUID' java method"), __PRETTY_FUNCTION__, __LINE__);
        assert(NULL);
        return uid;
    }

    jstring strUID = static_cast<jstring>(env->CallObjectMethod(g_ActivityObject, mthd, static_cast<short>(type)));
    if (strUID) {

        const char* lpUID = env->GetStringUTFChars(strUID, 0);
        uid.assign(lpUID);
        env->ReleaseStringUTFChars(strUID, lpUID);

        // Check avoid #JNI1 issue case
        if (uid == " ") uid.clear(); // null
    }
#ifdef DEBUG
    else { // #JNI1 issue found

        LOGF(LOG_FORMAT(" - #JNI1 issue found"), __PRETTY_FUNCTION__, __LINE__);
        //assert(NULL); // Let's #JNI1 crash
        uid.clear();
    }
#endif
    return uid;
}

extern std::string getCountry(unsigned char logLevel) {

    LOGV(logLevel, 0, LOG_FORMAT(" - (j:%p; a:%p)"), __PRETTY_FUNCTION__, __LINE__, g_JavaVM, g_ActivityObject);
    assert(g_JavaVM);
    assert(g_ActivityObject);

    std::string country(LIBENG_COUNTRY_ERROR); // Error
    JNIEnv* env = getJavaEnv(logLevel, __PRETTY_FUNCTION__, __LINE__);
    if (!env)
        return country;

    jclass clss = env->GetObjectClass(g_ActivityObject);
    if (!clss) {

        LOGE(LOG_FORMAT(" - Failed to get activity object class"), __PRETTY_FUNCTION__, __LINE__);
        assert(NULL);
        return country;
    }
    jmethodID mthd = env->GetMethodID(clss, "getCountryCode", "()Ljava/lang/String;");
    if (!mthd) {

        LOGE(LOG_FORMAT(" - Failed to get 'getCountryCode' java method"), __PRETTY_FUNCTION__, __LINE__);
        assert(NULL);
        return country;
    }

    jstring strCountry = static_cast<jstring>(env->CallObjectMethod(g_ActivityObject, mthd));
    if (strCountry) {

        const char* lpCountry = env->GetStringUTFChars(strCountry, 0);
        country.assign(lpCountry);
        env->ReleaseStringUTFChars(strCountry, lpCountry);

        // Check avoid #JNI1 issue case
        if (country == " ") country.clear(); // null
    }
#ifdef DEBUG
    else { // #JNI1 issue found

        LOGF(LOG_FORMAT(" - #JNI1 issue found"), __PRETTY_FUNCTION__, __LINE__);
        //assert(NULL); // Let's #JNI1 crash
        country.clear();
    }
#endif
    return country;
}

#else // iOS
extern NSString* getCountry() { return [[NSLocale currentLocale] objectForKey:NSLocaleCountryCode]; }

#endif

extern std::string encodeURL(const std::string& url) {

    if (url.empty())
        return url;

    std::ostringstream encoded;
    encoded.fill('0');
    encoded << std::hex;

    for (std::string::const_iterator iter = url.begin(); iter != url.end(); ++iter) {

        std::string::value_type val = (*iter);
        if ((isalnum(val)) || (val == '-') || (val == '_') || (val == '.') || (val == '~'))
            encoded << val;
        else if (val == ' ')
            encoded << '+';
        else
            encoded << '%' << std::setw(2) << static_cast<int>(val) << std::setw(0);
    }
    return encoded.str();
}

static const char BASE_64_TABLE[] = { 'A', 'B', 'C', 'D', 'E', 'F', 'G', 'H', 'I', 'J', 'K', 'L', 'M', 'N', 'O', 'P',
                                      'Q', 'R', 'S', 'T', 'U', 'V', 'W', 'X', 'Y', 'Z', 'a', 'b', 'c', 'd', 'e', 'f',
                                      'g', 'h', 'i', 'j', 'k', 'l', 'm', 'n', 'o', 'p', 'q', 'r', 's', 't', 'u', 'v',
                                      'w', 'x', 'y', 'z', '0', '1', '2', '3', '4', '5', '6', '7', '8', '9', '+', '/' };
static const int MOD_64_TABLE[] = { 0, 2, 1 };

extern std::string encodeB64(const std::string& field) {

    if (field.empty())
        return field;

    std::string encoded(static_cast<size_t>(4 * ((field.length() + 2) / 3)), ' ');
    for (int i = 0, j = 0; i < static_cast<int>(field.length()); ) {

        uint32_t octetA = (i < static_cast<int>(field.length()))? static_cast<unsigned char>(field.at(i++)):0;
        uint32_t octetB = (i < static_cast<int>(field.length()))? static_cast<unsigned char>(field.at(i++)):0;
        uint32_t octetC = (i < static_cast<int>(field.length()))? static_cast<unsigned char>(field.at(i++)):0;

        uint32_t triple = (octetA << 0x10) + (octetB << 0x08) + octetC;

        encoded.at(j++) = BASE_64_TABLE[(triple >> 3 * 6) & 0x3F];
        encoded.at(j++) = BASE_64_TABLE[(triple >> 2 * 6) & 0x3F];
        encoded.at(j++) = BASE_64_TABLE[(triple >> 1 * 6) & 0x3F];
        encoded.at(j++) = BASE_64_TABLE[(triple >> 0 * 6) & 0x3F];
    }
    for (int i = 0; i < MOD_64_TABLE[static_cast<int>(field.length()) % 3]; ++i)
        encoded.at(static_cast<int>(encoded.length()) - 1 - i) = '=';

    return encoded;
}

//////
#ifdef DEBUG
bool Delay::isElapsed(clock_t delta, short updates, float sumRef, const char* function, int line) {

    if ((std::numeric_limits<float>::max() - static_cast<float>(delta)) < static_cast<float>(mClockSum)) {

        LOGF(LOG_FORMAT(" - Float overflow"), __PRETTY_FUNCTION__, __LINE__);
        assert(NULL);
        return true;
    }
    mClockSum += static_cast<float>(delta);
    if (sumRef > 0.f) {
        if (mClockSum > sumRef) {

            reset();
            return true;
        }
    }
    else if (++mUpdateCount > updates) {

        LOGI(1, 0, LOG_FORMAT(" - updates = %d for sumRef = %f"), function, line, updates, mClockSum);
        reset();
        return true;
    }
    return false;
}
#endif

//////
#ifdef __ANDROID__
Launcher::Launcher(std::string package, std::string activity, std::string data) : mPackage(package),
        mActivity(activity), mData(data) {

    LOGV(LIBENG_LOG_LAUNCHER, 0, LOG_FORMAT(" - p:%s; a:%s; d:%s"), __PRETTY_FUNCTION__, __LINE__, package.c_str(),
            activity.c_str(), data.c_str());
}
Launcher::~Launcher() { LOGV(LIBENG_LOG_LAUNCHER, 0, LOG_FORMAT(), __PRETTY_FUNCTION__, __LINE__); }

bool Launcher::go() {

    LOGV(LIBENG_LOG_LAUNCHER, 0, LOG_FORMAT(" - (j:%p; a:%p)"), __PRETTY_FUNCTION__, __LINE__, g_JavaVM,
            g_ActivityObject);
    assert(g_JavaVM);
    assert(g_ActivityObject);

    JNIEnv* env = getJavaEnv(LIBENG_LOG_LAUNCHER, __PRETTY_FUNCTION__, __LINE__);
    if (!env)
        return false;

    jclass clss = env->GetObjectClass(g_ActivityObject);
    if (!clss) {

        LOGE(LOG_FORMAT(" - Failed to get activity object class"), __PRETTY_FUNCTION__, __LINE__);
        assert(NULL);
        return false;
    }
    jmethodID mthd = env->GetMethodID(clss, "launchApp", "(Ljava/lang/String;Ljava/lang/String;Ljava/lang/String;)Z");
    if (!mthd) {

        LOGE(LOG_FORMAT(" - Failed to get 'launchApp' java method"), __PRETTY_FUNCTION__, __LINE__);
        assert(NULL);
        return false;
    }

    jstring jPackage = env->NewStringUTF(mPackage.c_str());
    jstring jActivity = env->NewStringUTF(mActivity.c_str());
    jstring jData = env->NewStringUTF(mData.c_str());
    bool res = static_cast<bool>(env->CallBooleanMethod(g_ActivityObject, mthd, jPackage, jActivity, jData));

    env->DeleteLocalRef(jPackage);
    env->DeleteLocalRef(jActivity);
    env->DeleteLocalRef(jData);
    return res;
}
#endif

//////
#ifdef __ANDROID__
extern bool alertMessage(unsigned char logLevel, const char* msg) {

    LOGV(logLevel, 0, LOG_FORMAT(" - m:%s (j:%p; a:%p)"), __PRETTY_FUNCTION__, __LINE__, msg, g_JavaVM, g_ActivityObject);
    assert(g_JavaVM);
    assert(g_ActivityObject);

    JNIEnv* env = getJavaEnv(logLevel, __PRETTY_FUNCTION__, __LINE__);
    if (!env)
        return false;

    jclass clss = env->GetObjectClass(g_ActivityObject);
    if (!clss) {

        LOGE(LOG_FORMAT(" - Failed to get activity object class"), __PRETTY_FUNCTION__, __LINE__);
        assert(NULL);
        return false;
    }
    jmethodID mthd = env->GetMethodID(clss, "alert", "(Ljava/lang/String;)V");
    if (!mthd) {

        LOGE(LOG_FORMAT(" - Failed to get 'alert' java method"), __PRETTY_FUNCTION__, __LINE__);
        assert(NULL);
        return false;
    }

    jstring jMessage = env->NewStringUTF(msg);
    env->CallVoidMethod(g_ActivityObject, mthd, jMessage);
    env->DeleteLocalRef(jMessage);

#else
extern bool alertMessage(unsigned char logLevel, double duration, const char* msg) {

    LOGV(LIBENG_LOG_STORAGE, 0, LOG_FORMAT(" - d:%f; m:%s (a:%p)"), __PRETTY_FUNCTION__, __LINE__, duration, msg, g_AppleOS);
    assert(g_AppleOS);
    assert(msg);

    [g_AppleOS alertMessage:msg withDuration:duration];
#endif
    return true;
}

} // namespace
