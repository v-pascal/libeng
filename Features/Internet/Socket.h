#ifndef LIBENG_SOCKET_H_
#define LIBENG_SOCKET_H_
#if defined(__cplusplus)

#include <libeng/Global.h>
#ifdef LIBENG_ENABLE_INTERNET

#include <string>
#include <boost/thread.hpp>
#include <vector>
#include <time.h>

namespace eng {

//////
class Socket {

private:
    bool mServer;
    int mSocket;
    std::vector<int> mClients;

    volatile bool mAbort;
    boost::mutex mMutex;
    boost::thread* mThread;

    void serverThreadRunning();
    static void startServerThread(Socket* tcp);

public:
    Socket(bool server);
    virtual ~Socket();

    inline bool isServer() const { return mServer; }

    //////
    bool open();
    void shutdown();

    int receive(char* buffer, size_t max, unsigned char clientIdx = 0) const;
    int send(const char* buffer, size_t len, unsigned char clientIdx = 0) const;
    // -> Return respectively 'read' & 'write' function result
    // -> 'clientIdx' is usefull for client mode only

    // Client
    bool connexion(const std::string &host, int port);

    // Server
    bool start(int port);
    bool stop();
    void closeClient(unsigned char clientIdx);

    inline bool isRunning() const { return !mAbort; }
    inline unsigned char getClientCount() const { return static_cast<unsigned char>(mClients.size()); }

};

} // namespace

#endif // LIBENG_ENABLE_INTERNET
#endif // __cplusplus
#endif // LIBENG_SOCKET_H_
