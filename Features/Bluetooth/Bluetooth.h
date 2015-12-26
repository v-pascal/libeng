#ifndef LIBENG_BLUETOOTH_H_
#define LIBENG_BLUETOOTH_H_
#if defined(__cplusplus)

#include <libeng/Global.h>
#ifdef LIBENG_ENABLE_BLUETOOTH

#include <boost/thread.hpp>
#include <string>

namespace eng {

static const char NO_BLUETOOTH_DEVICE[] = "#BT-NONE#";

class PlatformOS;

class Bluetooth { // WARNING: Create, use & free Bluetooth instance from the same thread

    friend class PlatformOS;

private:
    Bluetooth();
    virtual ~Bluetooth();

    static Bluetooth* mThis;

public:
    static Bluetooth* getInstance() {
        if (!mThis)
            mThis = new Bluetooth;
        return mThis;
    }
    static void freeInstance() {
        if (mThis) {
            delete mThis;
            mThis = NULL;
        }
    }

    static bool isEnabled();

protected:
    bool mConnected;

#ifdef __ANDROID__
    JNIEnv* mEnv;
    jobject mObject;
    jclass mClass;

#else











#endif

private:
    unsigned char* mBuffer; // Read buffer
    int mLength; // Current read data length
    int mCurSize; // Current read buffer size
    boost::mutex mMutex;

    void newLen(int size) throw(std::bad_alloc);

    bool create();
    bool read(int len, unsigned char* data);
    inline void setConnection(bool connected) { mConnected = connected; }

    //////
public:
    inline bool isConnected() const { return mConnected; }

    enum {

        STATUS_ERROR = 0, // No Bluetooth/disabled
        STATUS_NONE,
        STATUS_LISTENING,
        STATUS_CONNECTING,
        STATUS_CONNECTED
    };
    unsigned char getStatus() const;
    bool reset();

    bool discover();
    bool isDiscovering();
    std::string getDevice(unsigned char index);

    bool listen(bool secure, const std::string &uuid, const std::string &name);
    bool connect(bool secure, const std::string &uuid, const std::string &address);

    bool write(int len, const unsigned char* data);

    // Read
    bool lock() { return mMutex.try_lock(); }
    inline const unsigned char* getBuffer() const { return mBuffer; }
    inline int getLength() const { return mLength; }
    void clear(int len); // WARNING: Method call can release memories (heavy processus)
    void unlock() { mMutex.unlock(); }

};

#ifdef __ANDROID__
extern jclass g_BluetoothClass;
#endif

} // namespace

#endif // LIBENG_ENABLE_BLUETOOTH
#endif // __cplusplus
#endif // LIBENG_BLUETOOTH_H_
