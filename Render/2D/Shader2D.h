#ifndef LIBENG_SHADER2D_H_
#define LIBENG_SHADER2D_H_

#ifdef __ANDROID__
#include <GLES2/gl2.h>
#else
typedef int GLint;
#endif

namespace eng {

//////
typedef struct {

    GLint ratioXLoc;
    GLint ratioYLoc;

    GLint transXLoc;
    GLint transYLoc;
    GLint scaleXLoc;
    GLint scaleYLoc;
    GLint rotateLoc;

} ShaderParam2D;

} // namespace

#endif // LIBENG_SHADER2D_H_
