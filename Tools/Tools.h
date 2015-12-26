#ifndef LIBENG_TOOLS_H_
#define LIBENG_TOOLS_H_
#if defined(__cplusplus)

#ifndef __ANDROID__
#import <Foundation/Foundation.h>
#endif

#include <limits>
#include <cmath>
#include <cstdarg>
#include <sstream>
#include <string>
#include <assert.h>
#include <time.h>
#include <float.h>

#define LIBENG_MEM_SCALE            16 // Multiple of 2
#define LIBENG_GET_MEM2ALLOC(n)     (unsigned int)((((unsigned int)(n / LIBENG_MEM_SCALE)) * LIBENG_MEM_SCALE) + \
                                        (((n % LIBENG_MEM_SCALE))? LIBENG_MEM_SCALE:0))

namespace eng {

static const float PI_F = 3.14159265358979f;

//////
inline bool fequal(float a, float b) { return (fabs(a - b) <= (FLT_EPSILON * std::max<float>(fabs(a), fabs(b)))); }

//////
template<typename T>
inline T delta(T lastVal, T newVal) {

    if ((newVal < 0) && (lastVal > 0)) {
        return (newVal - std::numeric_limits<T>::min()) + (std::numeric_limits<T>::max() - lastVal);
    }
#ifdef __ANDROID__
    return std::abs<T>(newVal - lastVal);
#else
    T res = newVal - lastVal;
    return (res > 0)? res:-res;
#endif
};

//////
template<typename T>
inline std::string numToStr(T number) {

    std::ostringstream oss;
    oss << number;
    return oss.str();
};
template<typename T>
inline std::wstring numToWStr(T number) {

    std::wostringstream oss;
    oss << number;
    return oss.str();
};
// WARNING: Use type size >= sizeof(short) only

template<typename T>
inline T strToNum(const std::string& str) {

    std::istringstream iss(str);
    T res;
    return (iss >> res)? res:0;
};
template<typename T>
inline T wstrToNum(const std::wstring& str) {

    std::wistringstream iss(str);
    T res;
    return (iss >> res)? res:0;
};

template<typename T>
inline std::string numToHex(T number) {

    std::stringstream res;
    res << std::hex << number;
    return res.str();
};

//////
template<class T>
struct CellChain {

    T data;
    CellChain* next;
};

template<class T>
class RecuChainFact {

private:
    static RecuChainFact<T>* mThis;

public:
    static RecuChainFact<T>* getInstance() {
        if (!mThis)
            mThis = new RecuChainFact<T>();
        return mThis;
    };
    static void freeInstance() {
        if (mThis)
            delete mThis;
        mThis = NULL;
    };
    ~RecuChainFact() {

        assert(!mHead);
        assert(!mCur);
    };

    //////
    void begin(T data) {

        assert(!mHead);
        assert(!mCur);

        mHead = new struct CellChain<T>;
        mHead->data = data;
        mCur = mHead;
    };
    void add(T data) {

        assert(mHead);
        assert(mCur);

        mCur->next = new struct CellChain<T>;
        mCur = mCur->next;
        mCur->data = data;
    };
    struct CellChain<T>* end(T data) {  // Return the last cell chain (i.e ret->next == head)

        assert(mHead);
        assert(mCur);

        add(data);
        mCur->next = mHead;
        return mCur;
    };
    static void destroy(struct CellChain<T>* prevData, struct CellChain<T>* nextData) {

        while (nextData->next != prevData) {

            struct CellChain<T>* backup = nextData->next;
            delete nextData;
            nextData = backup;
        }
        delete nextData;
        delete prevData;
    };

private:
    RecuChainFact() : mHead(NULL), mCur(NULL) { }

    struct CellChain<T>* mHead;
    struct CellChain<T>* mCur;

};

//////
#ifdef __ANDROID__
inline long getTickCount() {    // Not really a tick counter but nanoseconds counter
                                   // -> Use 'clock' function instead (for about 1 millisecond duration)

    // CLOCK_REALTIME: System-wide realtime clock. Setting this clock requires appropriate privileges.
    // CLOCK_MONOTONIC: Clock that cannot be set and represents monotonic time since some unspecified starting point.
    // CLOCK_PROCESS_CPUTIME_ID: High-resolution per-process timer from the CPU.
    // CLOCK_THREAD_CPUTIME_ID:  Thread-specific CPU-time clock.

    timespec tickCount;
    clock_gettime(CLOCK_THREAD_CPUTIME_ID, &tickCount);
    return tickCount.tv_nsec;
};
#endif

//////
class Line { // Bresenham algorithm

    short mDecision;

    short mDeltaX;
    short mDeltaYX;

    unsigned char mOctant; // n° octant (anticlockwise rotation)
    // 1: [0°:45°]
    // 2: [45°:90°]
    // 3: [90°:135°]
    // 4: [135°:180°]
    // 5: [180°;225°]
    // 6: [225°:270°]
    // 7: [270°:315°]
    // 8: [315°:360°]

    // ...from [x0;y0] to [x1;y1] coordinates

public:
    short X; // x0
    short Y; // y0

    inline void init(short x1, short y1) {

        short dx = (x1 >= X)? x1 - X:X - x1;
        short dy = (y1 >= Y)? y1 - Y:Y - y1;

        if (dx >= dy) {

            mDeltaX = dy << 1;
            mDecision = mDeltaX - dx;
            mDeltaYX = mDeltaX - (dx << 1);

            mOctant = (x1 >= X)? ((y1 >= Y)? 5:3):((y1 >= Y)? 8:1);
        }
        else {

            mDeltaX = dx << 1;
            mDecision = mDeltaX - dy;
            mDeltaYX = mDeltaX - (dy << 1);

            mOctant = (x1 >= X)? ((y1 >= Y)? 6:4):((y1 >= Y)? 7:2);
        }
    }
    inline void next() {

        if (mDecision <= 0) {

            mDecision += mDeltaX;
            switch (mOctant) {

                case 1:
                case 8: --X; break;
                case 2:
                case 4: --Y; break;
                case 3:
                case 5: ++X; break;
                case 6:
                case 7: ++Y; break;
            }
        }
        else {

            mDecision += mDeltaYX;
            switch (mOctant) {

                case 1:
                case 2: { --X; --Y; break; }
                case 3:
                case 4: { ++X; --Y; break; }
                case 5:
                case 6: { ++X; ++Y; break; }
                case 8:
                case 7: { --X; ++Y; break; }
            }
        }
    }

};

//////
#ifdef __ANDROID__
#include <jni.h>

extern "C" JNIEnv* getJavaEnv(unsigned char logLevel, const char* function, int line);
extern "C" void detachThreadJVM(unsigned char logLevel);

extern "C" wchar_t* java2wstring(const jchar* utf16, jsize len);
extern "C" jbyte* wstring2byteArray(const std::wstring& wString);

#endif

//////
#ifdef __ANDROID__
#include <GLES2/gl2.h>
#else
typedef uint32_t GLuint;
// -> Avoid to include "OpenGLES/ES2/gl.h" in header file
// BUG: 'CVOpenGLESTexture.h' compilation error (from iOS SDK 8.1)

#endif

extern "C" GLuint loadProgram(const char* vertexProg, const char* fragmentProg);

//////
#ifdef __ANDROID__
#define LIBENG_UID_ERROR        "#UID-ERROR#"
#define LIBENG_COUNTRY_ERROR    "#CTR-ERROR#"

struct UID {
    typedef enum {
        ANDROID_ID = 0,
        DEVICE_ID
    } Type;
};
extern std::string getUID(UID::Type type, unsigned char logLevel);
extern std::string getCountry(unsigned char logLevel); // // ISO 3166-1 alpha-3

#else // iOS
extern NSString* getCountry(); // // ISO 3166-1 alpha-2

#endif

//////
extern std::string encodeURL(const std::string& url);
extern std::string encodeB64(const std::string& field);

//////
class Delay {

private:
#ifdef DEBUG
    short mUpdateCount;
#endif
    float mClockSum;

public:
#ifdef DEBUG
    Delay() : mUpdateCount(0), mClockSum(0.f) { }
#else
    Delay() : mClockSum(0.f) { }
#endif
    virtual ~Delay() { }

    //////
#ifdef DEBUG
    bool isElapsed(clock_t delta, short updates, float sumRef, const char* function, int line);
    // To display 'sumRef' expected value by using only 'updates' count set negative value to 'sumRef'
#else
    inline bool isElapsed(clock_t delta, float sumRef) {

        mClockSum += static_cast<float>(delta);
        if (mClockSum > sumRef) {

            reset();
            return true;
        }
        return false;
    }
#endif
    inline void reset() {

#ifdef DEBUG
        mUpdateCount = 0;
#endif
        mClockSum = 0.f;
    }
};

//////
#ifdef __ANDROID__
class Launcher {

private:
    std::string mPackage;
    std::string mActivity;
    std::string mData;

public:
    Launcher(std::string package, std::string activity, std::string data);
    virtual ~Launcher();

    //////
    bool go();
    // WARNING: Calling this method will pause the current application

};
#endif

//////
#ifdef __ANDROID__
extern "C" bool alertMessage(unsigned char logLevel, const char* msg);
#else
extern "C" bool alertMessage(unsigned char logLevel, double duration, const char* msg);
// 'duration' in seconds
#endif

} // namespace

#endif // __cplusplus
#endif // LIBENG_TOOLS_H_
