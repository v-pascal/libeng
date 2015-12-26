#ifndef LIBENG_SHADER_H_
#define LIBENG_SHADER_H_

#ifdef __ANDROID__
#include <GLES2/gl2.h>
#else
typedef int32_t GLint;
// -> Avoid to include "OpenGLES/ES2/gl.h" in header file
// BUG: 'CVOpenGLESTexture.h' compilation error (from iOS SDK 8.1)

#endif

namespace eng {

//////
typedef struct {

    GLint positionLoc;
    GLint texcoordLoc;

    GLint redLoc;
    GLint greenLoc;
    GLint blueLoc;
    GLint alphaLoc;
#if defined(LIBENG_ENABLE_CAMERA) && !defined(__ANDROID__)
    GLint bgraLoc;
#endif

    GLint samplerLoc;

} ShaderParam;

} // namespace

#endif // SHADER_H_
