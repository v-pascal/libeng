#ifndef INTERNET_H_
#define INTERNET_H_
#if defined(__cplusplus)

#include <libeng/Global.h>
#ifdef LIBENG_ENABLE_INTERNET

#include <cstddef>
#include <assert.h>
#include <boost/thread.hpp>

namespace eng {

static const char SEPARATOR_IP_NETWORK_CHR = '*';
static const char* SEPARATOR_IP_NETWORK_STR = "*";

//////
class Internet {

private:
    Internet();
    virtual ~Internet();

    static Internet* mThis;

    boost::thread* mThread;
    int mSocket;

public:
    static Internet* getInstance() {
        if (!mThis)
            mThis = new Internet;
        return mThis;
    }
    static void freeInstance() {
        if (mThis) {
            delete mThis;
            mThis = NULL;
        }
    }

    enum {

        CONNECTION_NONE = 0, // No network connection or error

        CONNECTION_WIFI,
        CONNECTION_UNKNOWN
    };
    static unsigned char isConnected();  // Check if device is connected to a network (Wifi, Ethernet, Mobile...)
#ifdef __ANDROID__
    static bool isOnline(int timeOut); // Return if Internet is available ('timeOut' in milliseconds)
#else
    static bool isOnline(); // ...
#endif

    static std::string getDeviceIP(bool IPv4);

    ////// SearchIP
#ifdef __ANDROID__
    static std::string getNetworkIP(int timeOut); // Using SEPARATOR_IP_NETWORK_* as separator ('timeOut' in milliseconds)
    // WARNING: This method will block caller until all valid IP on network will be found (one minute at least)
    static void abortNetworkIP(); // Interrupt 'getNetworkIP' processus

#else
    std::string getNetworkIP(int timeOut); // ...
    void abortNetworkIP(); // ...

private:
    volatile bool mAbort;

#endif

    ////// HTTP
private:
    std::string mReplyHTTP;
    bool mWaitHTTP;

    static int HTTP_PORT;

    void httpThreadRunning(const std::string& host, const std::string& url);
    static void startHttpThread(Internet* http, const std::string& host, const std::string& url);

public:
    bool sendHTTP(const std::string& host, const std::string& url); // 'url' should be encoded (see 'encodeURL')
    inline bool waitHTTP() const { return mWaitHTTP; }
    inline void stopHTTP() { assert(mWaitHTTP); mWaitHTTP = false; }
    inline std::string getHTTP() const { return mReplyHTTP; }

};

} // namespace

#endif // LIBENG_ENABLE_INTERNET
#endif // __cplusplus
#endif // INTERNET_H_
