#include "Socket.h"

#ifdef LIBENG_ENABLE_INTERNET

#include <libeng/Log/Log.h>
#include <libeng/Tools/Tools.h>

#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <fcntl.h>
#include <arpa/inet.h>

#define MAX_CLIENT_COUNT        254
#define ACCEPT_DELAY            500 // Half a second

namespace eng {

//////
Socket::Socket(bool server) : mServer(server), mThread(NULL), mSocket(LIBENG_NO_DATA), mAbort(true) {

    LOGV(LIBENG_LOG_INTERNET, 0, LOG_FORMAT(" - s:%s"), __PRETTY_FUNCTION__, __LINE__, (server)? "true":"false");
}
Socket::~Socket() {

    LOGV(LIBENG_LOG_INTERNET, 0, LOG_FORMAT(), __PRETTY_FUNCTION__, __LINE__);
    shutdown();
}

bool Socket::open() {

    LOGV(LIBENG_LOG_INTERNET, 0, LOG_FORMAT(" - (s:%d)"), __PRETTY_FUNCTION__, __LINE__, mSocket);
    assert(mSocket == LIBENG_NO_DATA);

    mSocket = socket(AF_INET, SOCK_STREAM, 0);
    if (mSocket < 0) {

        LOGE(LOG_FORMAT(" - Failed to open socket (%d)"), __PRETTY_FUNCTION__, __LINE__, mSocket);
        mSocket = LIBENG_NO_DATA;
        return false;
    }
    if (mServer)
        fcntl(mSocket, F_SETFL, fcntl(mSocket, F_GETFL, 0) | O_NONBLOCK);
    else
        fcntl(mSocket, F_SETFL, fcntl(mSocket, F_GETFL, 0) & ~O_NONBLOCK);
    return true;
}
void Socket::shutdown() {

    LOGV(LIBENG_LOG_INTERNET, 0, LOG_FORMAT(" - (t:%p)"), __PRETTY_FUNCTION__, __LINE__, mThread);
    if (mThread) {

        mAbort = true;
        mThread->join();
        delete mThread;
        mThread = NULL;
    }
    for (std::vector<int>::iterator iter = mClients.begin(); iter != mClients.end(); ++iter)
        close(*iter);
    mClients.clear();

    if (!(mSocket < 0))
        close(mSocket);

    mSocket = LIBENG_NO_DATA;
}

int Socket::receive(char* buffer, size_t max, unsigned char clientIdx) const {

    if (mSocket < 0) {

        LOGW(LOG_FORMAT(" - Socket not opened"), __PRETTY_FUNCTION__, __LINE__);
        assert(NULL);
        return LIBENG_NO_DATA;
    }
    assert((!mServer) || ((mServer) && (mClients.size() > clientIdx)));
    int socket = (!mServer)? mSocket:mClients[clientIdx];
    return read(socket, buffer, max);
}
int Socket::send(const char* buffer, size_t len, unsigned char clientIdx) const {

    LOGV(LIBENG_LOG_INTERNET, 0, LOG_FORMAT(" - b:%p; s:%d; i:%d (s:%s; l:%d)"), __PRETTY_FUNCTION__, __LINE__, buffer,
            static_cast<int>(len), clientIdx, (mServer)? "true":"false", static_cast<int>(mClients.size()));
    if (mSocket < 0) {

        LOGW(LOG_FORMAT(" - Socket not opened"), __PRETTY_FUNCTION__, __LINE__);
        assert(NULL);
        return false;
    }
    assert((!mServer) || ((mServer) && (mClients.size() > clientIdx)));
    int socket = (!mServer)? mSocket:mClients[clientIdx];
    return write(socket, buffer, len);
}

bool Socket::connexion(const std::string &host, int port) {

    LOGV(LIBENG_LOG_INTERNET, 0, LOG_FORMAT(" - h:%s; p:%d (s:%s; s:%d)"), __PRETTY_FUNCTION__, __LINE__, host.c_str(), port,
            (mServer)? "true":"false", mSocket);
    assert(!mServer);

    if (!open())
        return false;

    // Method #1
    struct in_addr ipAddr;
    inet_pton(AF_INET, host.c_str(), &ipAddr);
#ifdef __ANDROID__
    struct hostent* server = gethostbyaddr((const char*)&ipAddr, sizeof(ipAddr), AF_INET);
#else
    struct hostent* server = gethostbyaddr(&ipAddr, sizeof(ipAddr), AF_INET);
#endif
    if (!server) {

        LOGI(LIBENG_LOG_INTERNET, 0, LOG_FORMAT(" - Address %s not found"), __PRETTY_FUNCTION__, __LINE__, host.c_str());
        server = gethostbyname(host.c_str());
    }
    if (!server) {
        LOGI(LIBENG_LOG_INTERNET, 0, LOG_FORMAT(" - Host %s not found"), __PRETTY_FUNCTION__, __LINE__, host.c_str());
    }
    else {

        struct sockaddr_in servAddr;
        std::memset(&servAddr, 0, sizeof(servAddr));
        servAddr.sin_family = AF_INET;
        bcopy((char*)server->h_addr, (char*)&servAddr.sin_addr.s_addr, server->h_length);

        servAddr.sin_port = htons(port);
        if (connect(mSocket, (struct sockaddr*)&servAddr, sizeof(servAddr)) < 0) {

            LOGW(LOG_FORMAT(" - Failed to connect #1: %s:%d (%d)"), __PRETTY_FUNCTION__, __LINE__, host.c_str(), port,
                 static_cast<int>(errno));
        }
        else {

            LOGI(LIBENG_LOG_INTERNET, 0, LOG_FORMAT(" - Connected #1: %s:%d"), __PRETTY_FUNCTION__, __LINE__, host.c_str(),
                 port);
            fcntl(mSocket, F_SETFL, fcntl(mSocket, F_GETFL, 0) | O_NONBLOCK);
            return true; // Connected
        }
    }
    close(mSocket);
    mSocket = LIBENG_NO_DATA;

    // Method #2
    struct addrinfo hints, *result;
    std::memset(&hints, 0, sizeof hints);
    hints.ai_family = AF_INET;
    hints.ai_socktype = SOCK_STREAM;

    int gaiRes = getaddrinfo(host.c_str(), numToStr<int>(port).c_str(), &hints, &result);
    if (gaiRes != 0) {

        LOGW(LOG_FORMAT(" - Failed to get address info: %s"), __PRETTY_FUNCTION__, __LINE__, gai_strerror(gaiRes));
        return false;
    }
    for (struct addrinfo* walk = result; walk; walk = walk->ai_next) {
        if (!open()) {

            freeaddrinfo(result);
            return false;
        }
        if (connect(mSocket, walk->ai_addr, walk->ai_addrlen) < 0) {
            LOGW(LOG_FORMAT(" - Connexion failed (err: %d)"), __PRETTY_FUNCTION__, __LINE__, static_cast<int>(errno));
        }
        else {

            LOGI(LIBENG_LOG_INTERNET, 0, LOG_FORMAT(" - Connected #2: %s:%d"), __PRETTY_FUNCTION__, __LINE__, host.c_str(),
                 port);
            fcntl(mSocket, F_SETFL, fcntl(mSocket, F_GETFL, 0) | O_NONBLOCK);
            freeaddrinfo(result);
            return true; // Connected
        }
    }
    LOGW(LOG_FORMAT(" - Failed to connect #2: %s:%d"), __PRETTY_FUNCTION__, __LINE__, host.c_str(), port);
    freeaddrinfo(result);
    close(mSocket);
    mSocket = LIBENG_NO_DATA;
    return false;
}

bool Socket::start(int port) {

    LOGV(LIBENG_LOG_INTERNET, 0, LOG_FORMAT(" - p:%d (s:%s)"), __PRETTY_FUNCTION__, __LINE__, port,
            (mServer)? "true":"false");
    assert(mServer);
    if (mThread) {

        LOGW(LOG_FORMAT(" - Server already started"), __PRETTY_FUNCTION__, __LINE__);
        assert(NULL);
        return false;
    }
    if (mSocket < 0) {

        LOGW(LOG_FORMAT(" - Socket not opened"), __PRETTY_FUNCTION__, __LINE__);
        assert(NULL);
        return false;
    }
    struct sockaddr_in serv_addr;
    std::memset(&serv_addr, 0, sizeof(serv_addr));
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_addr.s_addr = INADDR_ANY;
    serv_addr.sin_port = htons(port);
    if (bind(mSocket, (struct sockaddr*)&serv_addr, sizeof(serv_addr)) < 0) {

        LOGW(LOG_FORMAT(" - Failed to bind (%d)"), __PRETTY_FUNCTION__, __LINE__, port);
        return false;
    }
    listen(mSocket, MAX_CLIENT_COUNT);

    mAbort = false;
    mThread = new boost::thread(Socket::startServerThread, this);
    return true;
}
bool Socket::stop() {

    LOGV(LIBENG_LOG_INTERNET, 0, LOG_FORMAT(" - (s:%s)"), __PRETTY_FUNCTION__, __LINE__, (mServer)? "true":"false");
    assert(mServer);
    if (!mThread) {

        LOGW(LOG_FORMAT(" - Server already stopped"), __PRETTY_FUNCTION__, __LINE__);
        assert(NULL);
        return false;
    }
    shutdown();
    return true;
}
void Socket::closeClient(unsigned char clientIdx) {

    LOGV(LIBENG_LOG_INTERNET, 0, LOG_FORMAT(" - i:%d (s:%d)"), __PRETTY_FUNCTION__, __LINE__, clientIdx,
            static_cast<int>(mClients.size()));
    assert(mClients.size() > clientIdx);

    mMutex.lock();
    std::vector<int>::iterator iter = (mClients.begin() + clientIdx); // Index still good even if just added one
    close(*iter);

    mClients.erase(iter);
    mMutex.unlock();
}

void Socket::serverThreadRunning() {

    LOGV(LIBENG_LOG_INTERNET, 0, LOG_FORMAT(" - Begin"), __PRETTY_FUNCTION__, __LINE__);
    while (!mAbort) {

        boost::this_thread::sleep(boost::posix_time::milliseconds(ACCEPT_DELAY));

        struct sockaddr_in cli_addr;
        socklen_t clilen = sizeof(cli_addr);
        int newSocket = accept(mSocket, (struct sockaddr*)&cli_addr, &clilen);
        if (newSocket < 0) {

            switch (errno) {
                case EWOULDBLOCK: // EAGAIN
                    break; // No new client

                default: {

                    LOGW(LOG_FORMAT(" - Failed to accept client (%d)"), __PRETTY_FUNCTION__, __LINE__, newSocket);
                    assert(NULL);
                    break;
                }
            }
            continue;
        }
        fcntl(newSocket, F_SETFL, fcntl(newSocket, F_GETFL, 0) | O_NONBLOCK);

        mMutex.lock();
        mClients.push_back(newSocket);
        mMutex.unlock();
    }
    LOGI(LIBENG_LOG_INTERNET, 0, LOG_FORMAT(" - Finish"), __PRETTY_FUNCTION__, __LINE__);
}
void Socket::startServerThread(Socket* tcp) {

    LOGV(LIBENG_LOG_INTERNET, 0, LOG_FORMAT(" - t:%p"), __PRETTY_FUNCTION__, __LINE__, tcp);
    tcp->serverThreadRunning();
}

} // namespace

#endif // LIBENG_ENABLE_INTERNET
