#include "Render2D.h"

#include <libeng/Graphic/Object/2D/Object2D.h>
#include <libeng/Graphic/Object/2D/Dynamic2D.h>
#include <libeng/Tools/Tools.h>
#include <libeng/Log/Log.h>

#ifndef __ANDROID__
#include "OpenGLES/ES2/gl.h"
#endif

static const char gs_vertexShader[] =
        "precision highp float;"
        "attribute vec4 a_position;"
        "attribute vec2 a_texCoord;"
        "attribute float a_red;"
        "attribute float a_green;"
        "attribute float a_blue;"
        "attribute float a_alpha;"
        "attribute float a_bgra;"
        "uniform float u_ratioX;"
        "uniform float u_ratioY;"
        "uniform float u_transX;"
        "uniform float u_transY;"
        "uniform float u_scaleX;"
        "uniform float u_scaleY;"
        "uniform float u_rotate;"
        "varying vec2 v_texCoord;"
        "varying float v_red;"
        "varying float v_green;"
        "varying float v_blue;"
        "varying float v_alpha;"
        "varying float v_bgra;"
        "void main()"
        "{"
#ifndef LIBENG_PORT_AS_LAND
            "float rotX1=1.0;"
            "float rotY1=0.0;"
            "float rotX2=0.0;"
            "float rotY2=1.0;"
#else
            "float rotX1=0.0;"
            "float rotY1=-1.0;"
            "float rotX2=1.0;"
            "float rotY2=0.0;"
#endif
            "if(u_rotate!=0.0)"
            "{"
                "rotX1=cos(u_rotate);"
                "rotY1=sin(u_rotate);"
                "rotX2=-rotY1;"
                "rotY2=rotX1;"
            "}"
            "v_red=a_red;"
            "v_green=a_green;"
            "v_blue=a_blue;"
            "v_alpha=a_alpha;"
            "v_bgra=a_bgra;"
            "v_texCoord=a_texCoord;"
            "gl_Position=mat4(rotX1,rotY1,0,0,"
                             "rotX2,rotY2,0,0,"
                             "0,0,1,0,"
                             "u_transX*(1.0/u_scaleX),u_transY*(1.0/u_scaleY),0,1)*a_position*"
                        "mat4(u_ratioX*u_scaleX,0,0,0,"
                             "0,u_ratioY*u_scaleY,0,0,"
                             "0,0,1,0,"
                             "0,0,0,1);"
            "}";

static const char gs_fragmentShader[] =
        "precision highp float;"
        "varying vec2 v_texCoord;"
        "varying float v_red;"
        "varying float v_green;"
        "varying float v_blue;"
        "varying float v_alpha;"
        "varying float v_bgra;"
        "uniform sampler2D s_texture;"
        "void main()"
        "{"
#if defined(LIBENG_ENABLE_CAMERA) && !defined(__ANDROID__)
            "if(v_bgra>0.5)"
            "{"
                "gl_FragColor=texture2D(s_texture,v_texCoord).bgra*vec4(v_blue,v_green,v_red,v_alpha);"
            "}"
            "else"
            "{"
                "gl_FragColor=texture2D(s_texture,v_texCoord)*vec4(v_red,v_green,v_blue,v_alpha);"
            "}"
#else
            "gl_FragColor=texture2D(s_texture,v_texCoord)*vec4(v_red,v_green,v_blue,v_alpha);"
#endif
        "}";

//////
Render2D::Render2D() : Render() {

    LOGV(LIBENG_LOG_RENDER, 0, LOG_FORMAT(), __PRETTY_FUNCTION__, __LINE__);

    std::memset(&mShaderParam, 0, sizeof(eng::ShaderParam));
    std::memset(&mShaderParam2D, 0, sizeof(eng::ShaderParam2D));
}
Render2D::~Render2D() { LOGV(LIBENG_LOG_RENDER, 0, LOG_FORMAT(), __PRETTY_FUNCTION__, __LINE__); }

bool Render2D::init() {

    ////// Load program
    mProgramObj = eng::loadProgram(gs_vertexShader, gs_fragmentShader);
    if (!mProgramObj)
        return false;

    // Get the attribute & uniform locations
    mShaderParam.positionLoc = glGetAttribLocation(mProgramObj, "a_position");
    mShaderParam.texcoordLoc = glGetAttribLocation(mProgramObj, "a_texCoord");

    mShaderParam.redLoc = glGetAttribLocation(mProgramObj, "a_red");
    mShaderParam.greenLoc = glGetAttribLocation(mProgramObj, "a_green");
    mShaderParam.blueLoc = glGetAttribLocation(mProgramObj, "a_blue");
    mShaderParam.alphaLoc = glGetAttribLocation(mProgramObj, "a_alpha");
#if defined(LIBENG_ENABLE_CAMERA) && !defined(__ANDROID__)
    mShaderParam.bgraLoc = glGetAttribLocation(mProgramObj, "a_bgra");
#endif

    mShaderParam2D.ratioXLoc = glGetUniformLocation(mProgramObj, "u_ratioX");
    mShaderParam2D.ratioYLoc = glGetUniformLocation(mProgramObj, "u_ratioY");

#ifndef LIBENG_PORT_AS_LAND
    mShaderParam2D.transXLoc = glGetUniformLocation(mProgramObj, "u_transX");
    mShaderParam2D.transYLoc = glGetUniformLocation(mProgramObj, "u_transY");
    mShaderParam2D.scaleXLoc = glGetUniformLocation(mProgramObj, "u_scaleX");
    mShaderParam2D.scaleYLoc = glGetUniformLocation(mProgramObj, "u_scaleY");
#else
    mShaderParam2D.transXLoc = glGetUniformLocation(mProgramObj, "u_transY");
    mShaderParam2D.transYLoc = glGetUniformLocation(mProgramObj, "u_transX");
    mShaderParam2D.scaleXLoc = glGetUniformLocation(mProgramObj, "u_scaleY");
    mShaderParam2D.scaleYLoc = glGetUniformLocation(mProgramObj, "u_scaleX");
#endif
    mShaderParam2D.rotateLoc = glGetUniformLocation(mProgramObj, "u_rotate");
    //GLenum err = glGetError();
    //if (err != GL_NO_ERROR) {
    //    LOGW(LOG_FORMAT(" - glGetAttribLocation error: %d"), __PRETTY_FUNCTION__, __LINE__, err);
    //}

    // Get the sampler location
    mShaderParam.samplerLoc = glGetUniformLocation(mProgramObj, "s_texture");

    // Use tightly packed data
    glPixelStorei(GL_UNPACK_ALIGNMENT, 1);

    // Enable alpha blending (as default)
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    glClearColor(0.f, 0.f, 0.f, 1.f); // background color
    
    glEnableVertexAttribArray(mShaderParam.positionLoc);
    glEnableVertexAttribArray(mShaderParam.texcoordLoc);

    return true;
}
void Render2D::begin() {

    // Set the viewport
#ifndef LIBENG_PORT_AS_LAND
    glViewport(0, 0, mScreen.width, mScreen.height);
#else
    glViewport(0, 0, mScreen.height, mScreen.width);
#endif
    // Clear the color buffer
    glClear(GL_COLOR_BUFFER_BIT);

    // Use the program object
    glUseProgram(mProgramObj);

    // Reset uniform for the current frame
    glUniform1f(mShaderParam2D.ratioXLoc, eng::Object2D::TransformBuffer[eng::Dynamic2D::RATIO_X]);
    glUniform1f(mShaderParam2D.ratioYLoc, eng::Object2D::TransformBuffer[eng::Dynamic2D::RATIO_Y]);

    glUniform1f(mShaderParam2D.transXLoc, eng::Object2D::TransformBuffer[eng::Dynamic2D::TRANS_X]);
    glUniform1f(mShaderParam2D.transYLoc, eng::Object2D::TransformBuffer[eng::Dynamic2D::TRANS_Y]);
    glUniform1f(mShaderParam2D.scaleXLoc, eng::Object2D::TransformBuffer[eng::Dynamic2D::SCALE_X]);
    glUniform1f(mShaderParam2D.scaleYLoc, eng::Object2D::TransformBuffer[eng::Dynamic2D::SCALE_Y]);
    glUniform1f(mShaderParam2D.rotateLoc, eng::Object2D::TransformBuffer[eng::Dynamic2D::ROTATE_C]);
}
