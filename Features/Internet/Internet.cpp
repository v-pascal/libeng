#include "Internet.h"

#ifdef LIBENG_ENABLE_INTERNET

#include <libeng/Log/Log.h>
#include <libeng/Tools/Tools.h>

#include <netdb.h>
#include <netinet/in.h>

#ifndef __ANDROID__
#include <arpa/inet.h>
#include <sys/socket.h>
#include <ifaddrs.h>

#define MAX_INTERFACES          16
#define WIFI_INTERFACE_NAME     "en0"
#define SEARCH_IP_PORT          65432 // Port to check existing IP address
#endif

namespace eng {

Internet* Internet::mThis = NULL;

//////
Internet::Internet() : mThread(NULL), mWaitHTTP(false), mSocket(0) {

    LOGV(LIBENG_LOG_INTERNET, 0, LOG_FORMAT(), __PRETTY_FUNCTION__, __LINE__);
#ifndef __ANDROID__
    mAbort = true;
#endif
}
Internet::~Internet() { LOGV(LIBENG_LOG_INTERNET, 0, LOG_FORMAT(), __PRETTY_FUNCTION__, __LINE__); }

unsigned char Internet::isConnected() {

#ifdef __ANDROID__
    LOGV(LIBENG_LOG_INTERNET, 0, LOG_FORMAT(" - (j:%p; a:%p)"), __PRETTY_FUNCTION__, __LINE__, g_JavaVM,
            g_ActivityObject);
    assert(g_JavaVM);
    assert(g_ActivityObject);

    JNIEnv* env = getJavaEnv(LIBENG_LOG_INTERNET, __PRETTY_FUNCTION__, __LINE__);
    if (!env)
        return CONNECTION_NONE;

    jclass clss = env->GetObjectClass(g_ActivityObject);
    if (!clss) {

        LOGE(LOG_FORMAT(" - Failed to get activity object class"), __PRETTY_FUNCTION__, __LINE__);
        assert(NULL);
        return CONNECTION_NONE;
    }
    jmethodID mthd = env->GetMethodID(clss, "isConnected", "()S");
    if (!mthd) {

        LOGE(LOG_FORMAT(" - Failed to get 'isConnected' java method"), __PRETTY_FUNCTION__, __LINE__);
        assert(NULL);
        return CONNECTION_NONE;
    }
    return static_cast<unsigned char>(env->CallShortMethod(g_ActivityObject, mthd));

#else
    LOGV(LIBENG_LOG_INTERNET, 0, LOG_FORMAT(" - (a:%p)"), __PRETTY_FUNCTION__, __LINE__, g_AppleOS);
    assert(g_AppleOS);

    return [g_AppleOS isConnected];
#endif
}
#ifdef __ANDROID__
bool Internet::isOnline(int timeOut) {

    LOGV(LIBENG_LOG_INTERNET, 0, LOG_FORMAT(" - t:%d (j:%p; a:%p)"), __PRETTY_FUNCTION__, __LINE__, timeOut, g_JavaVM,
            g_ActivityClass);
    assert(g_JavaVM);
    assert(g_ActivityClass);

    JNIEnv* env = getJavaEnv(LIBENG_LOG_INTERNET, __PRETTY_FUNCTION__, __LINE__);
    if (!env)
        return false;

    jmethodID mthd = env->GetStaticMethodID(g_ActivityClass, "isOnline", "(I)Z");
    if (!mthd) {

        LOGE(LOG_FORMAT(" - Failed to get 'isOnline' java method"), __PRETTY_FUNCTION__, __LINE__);
        assert(NULL);
        return false;
    }
    return static_cast<bool>(env->CallStaticBooleanMethod(g_ActivityClass, mthd, timeOut));

#else
bool Internet::isOnline() {

    LOGV(LIBENG_LOG_INTERNET, 0, LOG_FORMAT(" - (a:%p)"), __PRETTY_FUNCTION__, __LINE__, g_AppleOS);
    assert(g_AppleOS);

    return [g_AppleOS isOnline];
#endif
}

std::string Internet::getDeviceIP(bool IPv4) {

    LOGV(LIBENG_LOG_INTERNET, 0, LOG_FORMAT(" - v4:%s"), __PRETTY_FUNCTION__, __LINE__, (IPv4)? "true":"false");
    std::string ip; // Error (empty)

#ifdef __ANDROID__
    assert(g_JavaVM);
    assert(g_ActivityClass);

    JNIEnv* env = getJavaEnv(LIBENG_LOG_INTERNET, __PRETTY_FUNCTION__, __LINE__);
    if (!env)
        return ip;

    jmethodID mthd = env->GetStaticMethodID(g_ActivityClass, "getDeviceIP", "(Z)Ljava/lang/String;");
    if (!mthd) {

        LOGE(LOG_FORMAT(" - Failed to get 'getDeviceIP' java method"), __PRETTY_FUNCTION__, __LINE__);
        assert(NULL);
        return ip;
    }

    jstring strIP = static_cast<jstring>(env->CallStaticObjectMethod(g_ActivityClass, mthd, static_cast<jboolean>(IPv4)));
    if (strIP) {

        const char* lpIP = env->GetStringUTFChars(strIP, 0);
        ip.assign(lpIP);
        env->ReleaseStringUTFChars(strIP, lpIP);

        // Check avoid #JNI1 issue case
        if (ip == " ") ip.clear(); // null
    }
#ifdef DEBUG
    else { // #JNI1 issue found

        LOGF(LOG_FORMAT(" - #JNI1 issue found"), __PRETTY_FUNCTION__, __LINE__);
        //assert(NULL); // Let's #JNI1 crash
        ip.clear();
    }
#endif

#else
    assert(IPv4); // Not implemented yet!

    struct ifaddrs* ifap;
    if (getifaddrs(&ifap) < 0) {

        LOGW(LOG_FORMAT(" - Failed to get ip (err:%d)"), __PRETTY_FUNCTION__, __LINE__, static_cast<int>(errno));
        assert(NULL);
        return ip;
    }
    for (struct ifaddrs *ifa = ifap; ifa; ifa = ifa->ifa_next)
        if ((ifa->ifa_addr->sa_family == AF_INET) && (!std::memcmp(ifa->ifa_name, WIFI_INTERFACE_NAME, sizeof(WIFI_INTERFACE_NAME) - 1)))
            ip.assign(inet_ntoa(((struct sockaddr_in*)ifa->ifa_addr)->sin_addr));

    freeifaddrs(ifap);

#endif
    LOGI(LIBENG_LOG_INTERNET, 0, LOG_FORMAT(" - IP: %s"), __PRETTY_FUNCTION__, __LINE__, ip.c_str());
    return ip;
}

std::string Internet::getNetworkIP(int timeOut) {
        
    LOGV(LIBENG_LOG_INTERNET, 0, LOG_FORMAT(" - t:%d"), __PRETTY_FUNCTION__, __LINE__, timeOut);
    std::string ipList; // Error (empty)

#ifdef __ANDROID__
    assert(g_JavaVM);
    assert(g_ActivityClass);

    JNIEnv* env = getJavaEnv(LIBENG_LOG_INTERNET, __PRETTY_FUNCTION__, __LINE__);
    if (!env)
        return ipList;

    jmethodID mthd = env->GetStaticMethodID(g_ActivityClass, "getNetworkIP", "(I)Ljava/lang/String;");
    if (!mthd) {

        LOGE(LOG_FORMAT(" - Failed to get 'getNetworkIP' java method"), __PRETTY_FUNCTION__, __LINE__);
        assert(NULL);
        return ipList;
    }

    jstring strListIP = static_cast<jstring>(env->CallStaticObjectMethod(g_ActivityClass, mthd, static_cast<jint>(timeOut)));
    if (strListIP) {

        const char* lpListIP = env->GetStringUTFChars(strListIP, 0);
        ipList.assign(lpListIP);
        env->ReleaseStringUTFChars(strListIP, lpListIP);

        // Check avoid #JNI1 issue case
        if (ipList == " ") ipList.clear(); // null
    }
#ifdef DEBUG
    else { // #JNI1 issue found

        LOGF(LOG_FORMAT(" - #JNI1 issue found"), __PRETTY_FUNCTION__, __LINE__);
        //assert(NULL); // Let's #JNI1 crash
        ipList.clear();
    }
#endif

#else
    std::string ip(getDeviceIP(true));
    assert(!ip.empty());

    size_t addrpos = ip.rfind('.');
    if (addrpos == std::string::npos) {

        LOGE(LOG_FORMAT(" - Wrong IP address: %s"), __PRETTY_FUNCTION__, __LINE__, ip.c_str());
        assert(NULL);
        return ipList;
    }
    unsigned char curIP = static_cast<unsigned char>(strToNum<short>(ip.substr(addrpos + 1)));
    ip.resize(addrpos + 1);

    mAbort = false;
    for (unsigned char i = 1; (i < 255) && (!mAbort); ++i) {
        if (curIP == i)
            continue;

        std::string search(ip);
        search.append(numToStr<short>(static_cast<short>(i)));

        struct sockaddr_in hostAddr;
        bzero(&hostAddr, sizeof(hostAddr));
        hostAddr.sin_len = sizeof(hostAddr);
        hostAddr.sin_family = AF_INET;
        hostAddr.sin_port = htons(SEARCH_IP_PORT);
        inet_pton(AF_INET, search.c_str(), &hostAddr.sin_addr);

        int socketFD = socket(AF_INET, SOCK_STREAM, 0);
        if (socketFD < 0) {

            LOGE(LOG_FORMAT(" - Failed to create socket (err:%d)"), __PRETTY_FUNCTION__, __LINE__, static_cast<int>(errno));
            assert(NULL);
            return ipList;
        }
        fcntl(socketFD, F_SETFL, fcntl(socketFD, F_GETFL, 0) | O_NONBLOCK);

        bool exists = true;
        if (connect(socketFD, (struct sockaddr*)&hostAddr, sizeof(hostAddr)) < 0) {

            exists = false; // Not connected
            if (static_cast<int>(errno) == EINPROGRESS) {

                struct timeval timeout;
                timeout.tv_sec = 0; // microseconds
                timeout.tv_usec = timeOut * 1000;

                fd_set readFD;
                FD_ZERO(&readFD);
                FD_SET(socketFD, &readFD);
                if (select(socketFD + 1, &readFD, NULL, NULL, &timeout) > 0)
                    exists = true; // Host exists
            }
        }
        //else // Connected (exists == true)
        close(socketFD);

        if (exists) {

            if (ipList.empty())
                ipList.assign(search);

            else {

                ipList += SEPARATOR_IP_NETWORK_CHR;
                ipList.append(search);
            }
        }
    }

#endif
    LOGI(LIBENG_LOG_INTERNET, 0, LOG_FORMAT(" - IPs: %s"), __PRETTY_FUNCTION__, __LINE__, ipList.c_str());
    return ipList;
}
void Internet::abortNetworkIP() {

#ifdef __ANDROID__
    LOGV(LIBENG_LOG_INTERNET, 0, LOG_FORMAT(), __PRETTY_FUNCTION__, __LINE__);
    assert(g_JavaVM);
    assert(g_ActivityClass);

    JNIEnv* env = getJavaEnv(LIBENG_LOG_INTERNET, __PRETTY_FUNCTION__, __LINE__);
    if (!env)
        return;

    jmethodID mthd = env->GetStaticMethodID(g_ActivityClass, "abortNetworkIP", "()V");
    if (!mthd) {

        LOGE(LOG_FORMAT(" - Failed to get 'abortNetworkIP' java method"), __PRETTY_FUNCTION__, __LINE__);
        assert(NULL);
        return;
    }
    env->CallStaticVoidMethod(g_ActivityClass, mthd);

#else
    LOGV(LIBENG_LOG_INTERNET, 0, LOG_FORMAT(" - (a:%s)"), __PRETTY_FUNCTION__, __LINE__, (mAbort)? "true":"false");
    mAbort = true;

#endif
}

////// HTTP
#define HTTP_GET        "GET "
#define HTTP_HOST       " HTTP/1.0\r\nHost: "
#define HTTP_END        "\r\n\r\n"

int Internet::HTTP_PORT = 80;

bool Internet::sendHTTP(const std::string& host, const std::string& url) {

    LOGV(LIBENG_LOG_INTERNET, 0, LOG_FORMAT(" - h:%s; u:%s"), __PRETTY_FUNCTION__, __LINE__, host.c_str(), url.c_str());
#ifdef DEBUG
    if (mWaitHTTP) {

        LOGW(LOG_FORMAT(" - Failed to send HTTP request: Already in progress"), __PRETTY_FUNCTION__, __LINE__);
        return false;
    }
#else
    if (mWaitHTTP)
        return false;
#endif
    if (mThread) {

        mThread->join();
        delete mThread;
    }
    if (!isConnected()) {

        LOGW(LOG_FORMAT(" - Device not connected"), __PRETTY_FUNCTION__, __LINE__);
        return false;
    }

    mWaitHTTP = true;
    mReplyHTTP.clear();
    mThread = new boost::thread(Internet::startHttpThread, this, host, url);
    return true;
}

void Internet::httpThreadRunning(const std::string& host, const std::string& url) {

    assert(mWaitHTTP);

    // Send
    struct hostent* reqHost = gethostbyname(host.c_str());
    if ((!reqHost) || (!reqHost->h_addr)) {

        LOGW(LOG_FORMAT(" - Failed retrieving DNS information: %s"), __PRETTY_FUNCTION__, __LINE__, host.c_str());
        mWaitHTTP = false;
        return;
    }

    struct sockaddr_in reqAddr;
    std::memset(&reqAddr, 0, sizeof(sockaddr_in));

    reqAddr.sin_family = AF_INET;
    reqAddr.sin_port = htons(HTTP_PORT);
    std::memcpy(&reqAddr.sin_addr, reqHost->h_addr, reqHost->h_length);

    mSocket = socket(AF_INET, SOCK_STREAM, 0);
    if (mSocket < 0) {

        LOGE(LOG_FORMAT(" - Failed to create socket"), __PRETTY_FUNCTION__, __LINE__);
        mWaitHTTP = false;
        return;
    }
    if (connect(mSocket, reinterpret_cast<struct sockaddr*>(&reqAddr), sizeof(sockaddr_in)) < 0) {

        close(mSocket);
        LOGW(LOG_FORMAT(" - Failed to connect: %s"), __PRETTY_FUNCTION__, __LINE__, host.c_str());
        mWaitHTTP = false;
        return;
    }

    std::string request(HTTP_GET);
    request.append(url);
    request.append(HTTP_HOST);
    request.append(host);
    request.append(HTTP_END);
    LOGI(LIBENG_LOG_INTERNET, 0, LOG_FORMAT(" - Request: %s"), __PRETTY_FUNCTION__, __LINE__, request.c_str());

    if (static_cast<int>(send(mSocket, request.c_str(), request.length(), 0)) != static_cast<int>(request.length())) {

        close(mSocket);
        LOGW(LOG_FORMAT(" - Failed to send HTTP request: %s"), __PRETTY_FUNCTION__, __LINE__, request.c_str());
        mWaitHTTP = false;
        return;
    }

    // Receive
    char rcv;
    while (read(mSocket, &rcv, 1) > 0) {

        mReplyHTTP += rcv;
#ifdef DEBUG
        if (!mWaitHTTP) {

            LOGI(LIBENG_LOG_INTERNET, 0, LOG_FORMAT(" - Force to exit from HTTP thread"), __PRETTY_FUNCTION__, __LINE__);
            break; // Force to exit
        }
#else
        if (!mWaitHTTP)
            break;
#endif
    }
    close(mSocket);
    mWaitHTTP = false;
}
void Internet::startHttpThread(Internet* http, const std::string& host, const std::string& url) {

    LOGV(LIBENG_LOG_INTERNET, 0, LOG_FORMAT(" - h:%p; h:%s; u:%s"), __PRETTY_FUNCTION__, __LINE__, http,
            host.c_str(), url.c_str());
    http->httpThreadRunning(host, url);
}

} // namespace

#endif // LIBENG_ENABLE_INTERNET
