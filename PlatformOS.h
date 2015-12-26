#ifndef LIBENG_PLATFORM_H_
#define LIBENG_PLATFORM_H_
#if defined(__cplusplus)

#include <libeng/Game/Game.h>

#ifdef __ANDROID__
#include <android/native_window.h>
#include <android/native_window_jni.h>

namespace eng {

typedef struct {

    const char* project; // Project name (used for logs)
    const char* file; // Game file (NULL if not needed)

    // Java Virtual Machine data access
    JavaVM* jvm;
    const char* jpn; // Java Project Name (path & name)
    jclass cls;
    jobject obj;

#ifdef LIBENG_ENABLE_SOCIAL
    jclass facebook;
    jclass twitter;
    jclass google;
#endif
#ifdef LIBENG_ENABLE_BLUETOOTH
    jclass bluetooth;
#endif

    Game* game; // Game instance

    float xDpi; // Horizontal DPI
    float yDpi; // Vertical DPI
    float accelRange; // Maximum accelerometer range

    bool dimension; // TRUE: 'Game3D' instance; FALSE: 'Game2D' instance (avoid using RTTI)

} PlatformData;

extern "C" {

void platformInit(long millis, const PlatformData* data);
void platformCreate();
void platformStart(jobject intent);
void platformChange(ANativeWindow* window);

#else // !__ANDROID__
#import <QuartzCore/CAEAGLLayer.h>
#import "OpenGLES/EAGL.h"

namespace eng {

typedef struct {

    const char* project; // Project name (used for logs)
    const char* file; // Game file (NULL if not needed)

    id<LibengOS> os;

    Game* game; // Game instance

    float xDpi; // Horizontal DPI
    float yDpi; // Vertical DPI
    float accelRange; // Maximum accelerometer range

    bool dimension; // TRUE: Game3D instance; FALSE: Game2D instance (avoid using RTTI when not needed)

} PlatformData;

extern "C" {

void platformInit(long millis, const PlatformData* data);
void platformStart(short screenWidth, short screenHeight, EAGLContext* context, CAEAGLLayer* layer);

#endif

void platformResume(long millis);
#ifdef __ANDROID__
void platformResult(int req, int res, jobject intent);
void platformPause(bool finishing, bool lockScreen);
void platformDestroy();
void platformStop();
#else
void platformPause();
#endif
void platformFree();

unsigned char platformLoadTexture(unsigned char id, short width, short height, unsigned char* data,
        bool grayscale = false);
void platformLoadOgg(unsigned char id, int len, unsigned char* data, bool queued = true);
void platformLoadFile(const char* file, const wchar_t* content);
void platformLoadCamera(const unsigned char* data);
void platformLoadMic(int len, const short* data);
#ifdef LIBENG_ENABLE_SOCIAL
void platformLoadSocial(unsigned char id, unsigned char request, unsigned char result, short width, short height,
        unsigned char* data);
#endif
#ifdef LIBENG_ENABLE_STORAGE
void platformLoadStore(unsigned char result);
#endif
#ifdef LIBENG_ENABLE_BLUETOOTH
bool platformLoadBluetooth(int len, unsigned char* data);
#endif

#ifndef __ANDROID__
static const unsigned char TOUCH_BEGAN = 0;
static const unsigned char TOUCH_MOVED = 1;
static const unsigned char TOUCH_ENDED = 2;
static const unsigned char TOUCH_CANCELLED = 3;
#endif
void platformTouch(unsigned int id, unsigned char type, short x, short y);
void platformAccelerometer(float xRate, float yRate, float zRate);

} // extern "C"
} // namespace

#endif // __cplusplus
#endif // LIBENG_PLATFORM_H_
