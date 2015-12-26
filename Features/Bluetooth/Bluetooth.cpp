#include "Bluetooth.h"

#ifdef LIBENG_ENABLE_BLUETOOTH
#include <libeng/Log/Log.h>
#include <libeng/Tools/Tools.h>

namespace eng {

Bluetooth* Bluetooth::mThis = NULL;

//////
Bluetooth::Bluetooth() : mBuffer(NULL), mLength(0), mCurSize(0), mConnected(false) {

    LOGV(LIBENG_LOG_BLUETOOTH, 0, LOG_FORMAT(), __PRETTY_FUNCTION__, __LINE__);
#ifdef __ANDROID__
    mEnv = NULL;
    mObject = NULL;
    mClass = NULL;
#else

    LOGF(LOG_FORMAT(" - Bluetooth not implemented yet!"), __PRETTY_FUNCTION__, __LINE__);
    assert(NULL);

#endif
}
Bluetooth::~Bluetooth() {

    LOGV(LIBENG_LOG_BLUETOOTH, 0, LOG_FORMAT(), __PRETTY_FUNCTION__, __LINE__);
#ifdef __ANDROID__
    if (mObject) {

        jmethodID mthd = mEnv->GetMethodID(mClass, "destroy", "()V");
        if (!mthd) {
            LOGE(LOG_FORMAT(" - Failed to get 'destroy' java method"), __PRETTY_FUNCTION__, __LINE__);
            assert(NULL);
        }
        else
            mEnv->CallVoidMethod(mObject, mthd);

        mEnv->DeleteGlobalRef(mObject);
    }
#else

    LOGF(LOG_FORMAT(" - Bluetooth not implemented yet!"), __PRETTY_FUNCTION__, __LINE__);
    assert(NULL);

#endif
}

bool Bluetooth::isEnabled() {

#ifdef __ANDROID__
    LOGV(LIBENG_LOG_BLUETOOTH, 0, LOG_FORMAT(" - (j:%p; a:%p)"), __PRETTY_FUNCTION__, __LINE__, g_JavaVM, g_ActivityClass);
    assert(g_JavaVM);
    assert(g_ActivityClass);

    JNIEnv* env = getJavaEnv(LIBENG_LOG_BLUETOOTH, __PRETTY_FUNCTION__, __LINE__);
    if (!env)
        return false;

    jmethodID mthd = env->GetStaticMethodID(g_ActivityClass, "isBluetooth", "()Z");
    if (!mthd) {

        LOGE(LOG_FORMAT(" - Failed to get 'isBluetooth' java method"), __PRETTY_FUNCTION__, __LINE__);
        assert(NULL);
        return false;
    }
    return static_cast<bool>(env->CallStaticBooleanMethod(g_ActivityClass, mthd));

#else

    LOGF(LOG_FORMAT(" - Bluetooth not implemented yet!"), __PRETTY_FUNCTION__, __LINE__);
    assert(NULL);
    return false;

#endif
}

unsigned char Bluetooth::getStatus() const {

    LOGV(LIBENG_LOG_BLUETOOTH, 0, LOG_FORMAT(), __PRETTY_FUNCTION__, __LINE__);
#ifdef __ANDROID__
    if (!mObject)
        return (isEnabled())? STATUS_NONE:STATUS_ERROR;

    jmethodID mthd = mEnv->GetMethodID(mClass, "getStatus", "()S");
    if (!mthd) {

        LOGE(LOG_FORMAT(" - Failed to get 'getStatus' java method"), __PRETTY_FUNCTION__, __LINE__);
        assert(NULL);
        return STATUS_ERROR;
    }
    return static_cast<unsigned char>(mEnv->CallShortMethod(mObject, mthd));

#else
    
    LOGF(LOG_FORMAT(" - Bluetooth not implemented yet!"), __PRETTY_FUNCTION__, __LINE__);
    assert(NULL);
    return 0;

#endif
}
bool Bluetooth::create() {

#ifdef __ANDROID__
    LOGV(LIBENG_LOG_BLUETOOTH, 0, LOG_FORMAT(" - (j:%p; a:%p; c:%p)"), __PRETTY_FUNCTION__, __LINE__, g_JavaVM,
            g_ActivityObject, g_BluetoothClass);
    assert(g_JavaVM);
    assert(g_ActivityObject);
    assert(g_BluetoothClass);

    if ((mObject) && (getStatus())) // != STATUS_ERROR
        return true;

    if (mObject) {
        reset();
        return (getStatus() == STATUS_NONE);
    }
    mEnv = getJavaEnv(LIBENG_LOG_BLUETOOTH, __PRETTY_FUNCTION__, __LINE__);
    if (!mEnv)
        return false;

    std::string mthdSign("(L");
    mthdSign.append(g_JavaProject);
    mthdSign.append(LIBENG_ACTIVITY_CLASS);
    mthdSign.append(";)V");

    jmethodID mthd = mEnv->GetMethodID(g_BluetoothClass, "<init>", mthdSign.c_str());
    if (!mthd) {

        LOGE(LOG_FORMAT(" - Failed to get '<init>' java method (signature: %s)"), __PRETTY_FUNCTION__, __LINE__,
                mthdSign.c_str());
        assert(NULL);
        return false;
    }
    mObject = mEnv->NewGlobalRef(mEnv->NewObject(g_BluetoothClass, mthd, g_ActivityObject));
    if (!mObject) {

        LOGE(LOG_FORMAT(" - Failed to create Bluetooth object"), __PRETTY_FUNCTION__, __LINE__);
        assert(NULL);
        return false;
    }
    mClass = mEnv->GetObjectClass(mObject);
    if (!mClass) {

        LOGE(LOG_FORMAT(" - Failed to get Bluetooth class"), __PRETTY_FUNCTION__, __LINE__);
        mEnv->DeleteGlobalRef(mObject);
        mObject = NULL;
        assert(NULL);
        return false;
    }
    return static_cast<bool>(getStatus());

#else
    
    LOGF(LOG_FORMAT(" - Bluetooth not implemented yet!"), __PRETTY_FUNCTION__, __LINE__);
    assert(NULL);
    return false;

#endif
}
bool Bluetooth::reset() {

#ifdef __ANDROID__
    LOGV(LIBENG_LOG_BLUETOOTH, 0, LOG_FORMAT(" - (j:%p; o:%p)"), __PRETTY_FUNCTION__, __LINE__, g_JavaVM, mObject);
    assert(g_JavaVM);

    if (!mObject) {
        create();
        if (!mObject)
            return false;
    }
    jmethodID mthd = mEnv->GetMethodID(mClass, "reset", "()V");
    if (!mthd) {

        LOGE(LOG_FORMAT(" - Failed to get 'reset' java method"), __PRETTY_FUNCTION__, __LINE__);
        assert(NULL);
        return false;
    }
    mEnv->CallVoidMethod(mObject, mthd);

#else
    
    LOGF(LOG_FORMAT(" - Bluetooth not implemented yet!"), __PRETTY_FUNCTION__, __LINE__);
    assert(NULL);

#endif
    if (mLength) {

        mMutex.lock();
        clear(mLength);
        mLength = 0;
        mMutex.unlock();
    }
    mConnected = false;
    return true;
}

bool Bluetooth::discover() {

    LOGV(LIBENG_LOG_BLUETOOTH, 0, LOG_FORMAT(), __PRETTY_FUNCTION__, __LINE__);
#ifdef __ANDROID__
    if (!create())
        return false;

    jmethodID mthd = mEnv->GetMethodID(mClass, "discover", "()V");
    if (!mthd) {

        LOGE(LOG_FORMAT(" - Failed to get 'discover' java method"), __PRETTY_FUNCTION__, __LINE__);
        assert(NULL);
        return false;
    }
    mEnv->CallVoidMethod(mObject, mthd);
    return true;

#else
    
    LOGF(LOG_FORMAT(" - Bluetooth not implemented yet!"), __PRETTY_FUNCTION__, __LINE__);
    assert(NULL);
    return false;

#endif
}
bool Bluetooth::isDiscovering() {

    LOGV(LIBENG_LOG_BLUETOOTH, 0, LOG_FORMAT(), __PRETTY_FUNCTION__, __LINE__);
#ifdef __ANDROID__
    if (!create())
        return false;

    jmethodID mthd = mEnv->GetMethodID(mClass, "isDiscovering", "()Z");
    if (!mthd) {

        LOGE(LOG_FORMAT(" - Failed to get 'isDiscovering' java method"), __PRETTY_FUNCTION__, __LINE__);
        assert(NULL);
        return false;
    }
    return static_cast<bool>(mEnv->CallBooleanMethod(mObject, mthd));

#else

    LOGF(LOG_FORMAT(" - Bluetooth not implemented yet!"), __PRETTY_FUNCTION__, __LINE__);
    assert(NULL);
    return false;

#endif
}
std::string Bluetooth::getDevice(unsigned char index) {

    LOGV(LIBENG_LOG_BLUETOOTH, 0, LOG_FORMAT(" - i:%d"), __PRETTY_FUNCTION__, __LINE__, index);
    std::string device; // Error

#ifdef __ANDROID__
    if (!create())
        return device;

    jmethodID mthd = mEnv->GetMethodID(mClass, "getDevice", "(S)Ljava/lang/String;");
    if (!mthd) {

        LOGE(LOG_FORMAT(" - Failed to get 'getDevice' java method"), __PRETTY_FUNCTION__, __LINE__);
        assert(NULL);
        return device;
    }
    jstring strDevice = static_cast<jstring>(mEnv->CallObjectMethod(mObject, mthd, static_cast<short>(index)));
    if (strDevice) {

        const char* lpDevice = mEnv->GetStringUTFChars(strDevice, 0);
        device.assign(lpDevice);
        mEnv->ReleaseStringUTFChars(strDevice, lpDevice);

        // Check avoid #JNI1 issue case
        if (device == " ")
            device.assign(NO_BLUETOOTH_DEVICE); // No more device
    }
#ifdef DEBUG
    else { // #JNI1 issue found

        LOGF(LOG_FORMAT(" - #JNI1 issue found"), __PRETTY_FUNCTION__, __LINE__);
        //assert(NULL); // Let's #JNI1 crash
    }
#endif

#else
    
    LOGF(LOG_FORMAT(" - Bluetooth not implemented yet!"), __PRETTY_FUNCTION__, __LINE__);
    assert(NULL);

#endif
    return device;
}

void Bluetooth::newLen(int size) throw(std::bad_alloc) {

    LOGV(LIBENG_LOG_BLUETOOTH, 0, LOG_FORMAT(" - s:%d (c:%d)"), __PRETTY_FUNCTION__, __LINE__, size, mCurSize);
    assert(!mMutex.try_lock());
    assert(size > 0);

    mLength = size++;
    if (mCurSize < size) {

        unsigned char* store = new unsigned char[mCurSize];
        std::memcpy(store, mBuffer, mCurSize);

        delete [] mBuffer;
        int prevSize = mCurSize;
        mCurSize = LIBENG_GET_MEM2ALLOC(size);
        mBuffer = new unsigned char[mCurSize];

        std::memcpy(mBuffer, store, prevSize);
        delete [] store;
    }
    else if (mCurSize > size) {

        mCurSize = LIBENG_GET_MEM2ALLOC(size);
        unsigned char* store = new unsigned char[mCurSize];
        std::memcpy(store, mBuffer, mCurSize);

        delete [] mBuffer;
        mBuffer = new unsigned char[mCurSize];

        std::memcpy(mBuffer, store, mCurSize);
        delete [] store;
    }
}

bool Bluetooth::listen(bool secure, const std::string &uuid, const std::string &name) {

    LOGV(LIBENG_LOG_BLUETOOTH, 0, LOG_FORMAT(" - s:%s; u:%s; n:%s"), __PRETTY_FUNCTION__, __LINE__,
            (secure)? "true":"false", uuid.c_str(), name.c_str());
    assert(uuid.length() == 36); // fa87c0d0-afac-11de-8a39-0800200c9a66
    assert(name.length() > 0);

#ifdef __ANDROID__
    if (!create())
        return false;

    jmethodID mthd = mEnv->GetMethodID(mClass, "listen", "(ZLjava/lang/String;Ljava/lang/String;)Z");
    if (!mthd) {

        LOGE(LOG_FORMAT(" - Failed to get 'listen' java method"), __PRETTY_FUNCTION__, __LINE__);
        assert(NULL);
        return false;
    }
    jstring jUUID = mEnv->NewStringUTF(uuid.c_str());
    jstring jName = mEnv->NewStringUTF(name.c_str());
    bool res = static_cast<bool>(mEnv->CallBooleanMethod(mObject, mthd, static_cast<jboolean>(secure), jUUID, jName));
    mEnv->DeleteLocalRef(jUUID);
    mEnv->DeleteLocalRef(jName);
    return res;

#else

    LOGF(LOG_FORMAT(" - Bluetooth not implemented yet!"), __PRETTY_FUNCTION__, __LINE__);
    assert(NULL);
    return false;

#endif
}
bool Bluetooth::connect(bool secure, const std::string &uuid, const std::string &address) {

    LOGV(LIBENG_LOG_BLUETOOTH, 0, LOG_FORMAT(" - s:%s; u:%s; a:%s"), __PRETTY_FUNCTION__, __LINE__,
            (secure)? "true":"false", uuid.c_str(), address.c_str());
    assert(uuid.length() == 36); // fa87c0d0-afac-11de-8a39-0800200c9a66
    assert(address.length() > 0);

#ifdef __ANDROID__
    if (!create())
        return false;

    jmethodID mthd = mEnv->GetMethodID(mClass, "connect", "(ZLjava/lang/String;Ljava/lang/String;)Z");
    if (!mthd) {

        LOGE(LOG_FORMAT(" - Failed to get 'connect' java method"), __PRETTY_FUNCTION__, __LINE__);
        assert(NULL);
        return false;
    }
    jstring jUUID = mEnv->NewStringUTF(uuid.c_str());
    jstring jAddress = mEnv->NewStringUTF(address.c_str());
    bool res = static_cast<bool>(mEnv->CallBooleanMethod(mObject, mthd, static_cast<jboolean>(secure), jUUID, jAddress));
    mEnv->DeleteLocalRef(jUUID);
    mEnv->DeleteLocalRef(jAddress);
    return res;

#else

    LOGF(LOG_FORMAT(" - Bluetooth not implemented yet!"), __PRETTY_FUNCTION__, __LINE__);
    assert(NULL);
    return false;

#endif
}

bool Bluetooth::read(int len, unsigned char* data) {

    LOGV(LIBENG_LOG_BLUETOOTH, 0, LOG_FORMAT(" - l:%d; d:%p"), __PRETTY_FUNCTION__, __LINE__, len, data);
    assert(len > 0);
    int prevLen = mLength;

    mMutex.lock();
    try {
        if (!mBuffer) {

            mBuffer = new unsigned char[mCurSize = LIBENG_GET_MEM2ALLOC(len)];
            mLength = len;
        }
        else
            newLen(mLength + len);
    }
    catch (const std::bad_alloc &e) {

        LOGE(LOG_FORMAT(" - Full read buffer: %s"), __PRETTY_FUNCTION__, __LINE__, e.what());
        mMutex.unlock();
        assert(NULL);
        delete [] data;
        return false;
    }
    std::memcpy(mBuffer + prevLen, data, len);
    mMutex.unlock();

    delete [] data;
    return true;
}
bool Bluetooth::write(int len, const unsigned char* data) {

    LOGV(LIBENG_LOG_BLUETOOTH, 0, LOG_FORMAT(" - l:%d; d:%p"), __PRETTY_FUNCTION__, __LINE__, len, data);
    assert(len > 0);
    assert(data);

#ifdef __ANDROID__
    if (!create())
        return false;

    jmethodID mthd = mEnv->GetMethodID(mClass, "write", "([B)Z");
    if (!mthd) {

        LOGE(LOG_FORMAT(" - Failed to get 'write' java method"), __PRETTY_FUNCTION__, __LINE__);
        assert(NULL);
        return false;
    }
    jbyteArray jBuffer = mEnv->NewByteArray(static_cast<jsize>(len));
    mEnv->SetByteArrayRegion(jBuffer, 0, len, reinterpret_cast<const jbyte*>(data));
    bool res = static_cast<bool>(mEnv->CallBooleanMethod(mObject, mthd, jBuffer));
    mEnv->DeleteLocalRef(jBuffer);
    return res;

#else

    LOGF(LOG_FORMAT(" - Bluetooth not implemented yet!"), __PRETTY_FUNCTION__, __LINE__);
    assert(NULL);
    return false;

#endif
}

void Bluetooth::clear(int len) {

    LOGV(LIBENG_LOG_BLUETOOTH, 0, LOG_FORMAT(" - l:%d (e:%d)"), __PRETTY_FUNCTION__, __LINE__, len, mLength);
    assert(!mMutex.try_lock());

    if (len <= 0) {

        LOGE(LOG_FORMAT(" - Wrong read buffer length to clear: %d"), __PRETTY_FUNCTION__, __LINE__, len);
        assert(NULL);
        return;
    }
    if (mLength <= len) {

        mLength = 0;
        mCurSize = 0;
        delete [] mBuffer;
        mBuffer = NULL;
    }
    else
        newLen(mLength - len);
}

} // namespace

#endif // LIBENG_ENABLE_BLUETOOTH
