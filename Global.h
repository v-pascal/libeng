#ifndef LIBENG_GLOBAL_H_
#define LIBENG_GLOBAL_H_

#define LIBENG_VERSION_1_2_8
#define LIBENG_VERSION              "1.2.8"

#define LIBENG_NO_DATA              -1

#ifdef __ANDROID__
////// DEBUG | RELEASE

// Debug
#ifndef DEBUG
#define DEBUG
#endif
#undef NDEBUG

// Relase
//#ifndef NDEBUG
//#define NDEBUG
//#endif
//#undef DEBUG

// -> Not needed at compilation (only to display the code correctly)
// -> Works for all sources coz this file is the first included file (see 'PlatformOS.cpp')

// Needed coz the actions below do not define or undefine definitions above:
// * Following flags not working: LOCAL_CFLAGS/APP_CFLAGS/NDK_APP_CFLAGS = -DDEBUG -UNDEBUG | -DNDEBUG -UDEBUG
// * android:debuggable="false | true" in the manifest file does not work as well
// * NDK_DEBUG=0 | 1 definition has no effect

#endif // __ANDROID__


////// Flags
#ifdef __ANDROID__
#define LIBENG_USE_RTTI             // Allow to use RTTI (RunTime Type Information) - Needed in some classes when used
                                    // -> When defined remove "-fno-rtti" from LOCAL_CFLAGS and add "-frtti" flag instead
#endif
#ifdef DEBUG
#define LIBENG_ENABLE_FPS           // Enable to display FPS logs
#endif
#define LIBENG_ENABLE_DELTA         // Enable to calculate frame delta time
#define LIBENG_ENABLE_GAMEPAD       // Enable gamepad management (see 'Game' class)
#define LIBENG_ENABLE_SPRITE        // Enable sprite object declaration
#define LIBENG_ENABLE_TEXT          // Enable text object declaration
#define LIBENG_ENABLE_SCROLLING     // Enable scroll object declaration
#define LIBENG_ENABLE_PROGRESS      // Enable progress bar object declaration
#define LIBENG_ENABLE_STORAGE       // Enable storage management
#define LIBENG_ENABLE_CAMERA        // Enable using camera (or: Remove 'CoreMedia', 'CoreVideo' & 'AVFoundation' from iOS project)
#ifdef LIBENG_ENABLE_CAMERA
//#define LIBENG_CAM_FREEZER          // Enable freeze camera option (allow freeze & save picture management)
#endif
#define LIBENG_ENABLE_MIC           // Enable using mic (or: Remove 'AudioToolbox' framework from iOS project)
#ifdef LIBENG_ENABLE_MIC
#define LIBENG_DB_CALC              // Implement level/decibel calculation
#endif
#define LIBENG_ENABLE_INTERNET      // Enable Internet network use
#define LIBENG_ENABLE_ADVERTISING   // Enable advertising management
#define LIBENG_ENABLE_SOCIAL        // Enable social network management
#define LIBENG_ENABLE_BLUETOOTH     // Enable Bluetooth management


#if defined(LIBENG_ENABLE_ADVERTISING) && !defined(LIBENG_ENABLE_INTERNET)
#define LIBENG_ENABLE_INTERNET      // Always define Internet flag when advertising is enabled
#endif
#if defined(LIBENG_ENABLE_SOCIAL) && !defined(LIBENG_ENABLE_INTERNET)
#define LIBENG_ENABLE_INTERNET      // Always define Internet flag when social network is enabled
#endif

////// Orientation
//#define LIBENG_LAND_ORIENTATION     // Applications should not be set in landscape orientation on Android. This is due to
#ifndef LIBENG_LAND_ORIENTATION     // title & status bar removal failure.
#define LIBENG_PORT_AS_LAND         // Landscape application with a portrait orientation (needed in 'Render2D', 'Game2D', etc.)
#endif                              // WARNING: This flag should be checked in source code only when...
                                    //          * Touch input is managed (replace X by Y & Y by -X)
                                    //          * A 2D rotation is requested (start rotation from -Pi/2 instead of 0)
                                    //          * Accelerometer input is managed (replace -X by X & Y by -Y)
                                    //          * TRANS_X should be replaced with -TRANS_X (using 'Dynamic2D::getTransform' method)
#ifndef __ANDROID__
#ifdef LIBENG_PORT_AS_LAND          // When using this flag to make a landscape game with a portrait orientation on
#undef LIBENG_PORT_AS_LAND          // Android (see reasons above), on iOS set a landscape orientation instead with setting
#define LIBENG_LAND_ORIENTATION     // this flag would be preferable (to avoid displaying title bar when using a gamepad)
#endif // LIBENG_PORT_AS_LAND

#endif // !__ANDROID__

// WARNING: Landscape orientation should be defined as 'Right home button' type only!
//          Portrait orientation should be defined as 'Bottom home button' type only!

////// Log levels (< 5 to log)
#define LIBENG_LOG_PLATFORM         4
#define LIBENG_LOG_RENDER           4
#define LIBENG_LOG_GAME             4
#define LIBENG_LOG_TEXTURE          4
#define LIBENG_LOG_PLAYER           4
#define LIBENG_LOG_INPUT            4
#define LIBENG_LOG_ACCEL            4
#define LIBENG_LOG_TOUCH            4
#define LIBENG_LOG_BOUNDS           4
#define LIBENG_LOG_OBJECT           5
#define LIBENG_LOG_PANEL            5
#define LIBENG_LOG_DYNAMIC          4
#define LIBENG_LOG_ELEMENT          5
#define LIBENG_LOG_STATIC           5
#define LIBENG_LOG_SPRITE           4
#define LIBENG_LOG_TEXT             5
#define LIBENG_LOG_SCROLLING        4
#define LIBENG_LOG_GAMEPAD          4
#define LIBENG_LOG_INTRO            4
#define LIBENG_LOG_LEVEL            4
#define LIBENG_LOG_PROGRESS         4
#define LIBENG_LOG_STORAGE          4
#define LIBENG_LOG_CAMERA           4
#define LIBENG_LOG_MIC              4
#define LIBENG_LOG_ADVERTISING      4
#define LIBENG_LOG_INTERNET         4
#define LIBENG_LOG_SOCIAL           4
#define LIBENG_LOG_LAUNCHER         4
#define LIBENG_LOG_BLUETOOTH        4

////// Common
#define LIBENG_FILE_NOT_FOUND       L"#FILE_NOT_FOUND#" // Data of the file when not exists
#define LIBENG_RENDER_FPS           60 // Frames per second

namespace eng {

extern const char* g_LogTag; // Project log name
#ifdef LIBENG_ENABLE_STORAGE
extern const char* g_GameFile; // Game file name where to store data (if needed)
#endif

} // namespace

#ifdef LIBENG_ENABLE_SOCIAL
#include <libeng/Social/Networks/Facebook.h>
#include <libeng/Social/Networks/Twitter.h>
#include <libeng/Social/Networks/Google.h>
#endif

#ifdef __ANDROID__
#include <jni.h>

#define LIBENG_ACTIVITY_CLASS       "/EngActivity"
#define LIBENG_BLUETOOTH_CLASS      "/EngBluetooth"

namespace eng {

extern JavaVM* g_JavaVM;
extern const char* g_JavaProject;
extern jclass g_ActivityClass;
extern jobject g_ActivityObject;

} // namespace

#else // !__ANDROID__
#include <libeng/AppleOS.h>

namespace eng {

extern id<LibengOS> g_AppleOS;

} // namespace

#endif // __ANDROID__

#endif // LIBENG_GLOBAL_H_
