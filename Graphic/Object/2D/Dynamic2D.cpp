#include "Dynamic2D.h"
#include "Object2D.h"

#ifndef __ANDROID__
#include "OpenGLES/ES2/gl.h"
#endif

namespace eng {

float* Dynamic2D::IncreaseBuffer = NULL;

//////
Dynamic2D::Dynamic2D() : Dynamic(), mVelocities(NULL) {

    LOGV(LIBENG_LOG_DYNAMIC, 0, LOG_FORMAT(" - (T:%p)"), __PRETTY_FUNCTION__, __LINE__, Object2D::TransformBuffer);
    mTransform = Object2D::TransformBuffer;
}
Dynamic2D::~Dynamic2D() {

    LOGV(LIBENG_LOG_DYNAMIC, 0, LOG_FORMAT(), __PRETTY_FUNCTION__, __LINE__);
    if (mTransform != Object2D::TransformBuffer) {

        LOGI(LIBENG_LOG_DYNAMIC, 0, LOG_FORMAT(" - Delete transform buffer: %p"), __PRETTY_FUNCTION__, __LINE__,
                mTransform);
        delete [] mTransform; // float[TRANSFORM_BUFFER_SIZE]
    }
    if (mVelocities)
        delete [] mVelocities;
}

void Dynamic2D::initIncreaseBuffer() {

    LOGV(LIBENG_LOG_DYNAMIC, 0, LOG_FORMAT(), __PRETTY_FUNCTION__, __LINE__);
    assert(!IncreaseBuffer);

    IncreaseBuffer = new float[INCREASE_BUFFER_SIZE];
    IncreaseBuffer[0] = std::cosh(0.025f) - 1.f;
    IncreaseBuffer[1] = std::cosh(0.05f) - 1.f;
    IncreaseBuffer[2] = std::cosh(0.075f) - 1.f;
    IncreaseBuffer[3] = std::cosh(0.1f) - 1.f;
    IncreaseBuffer[4] = std::cosh(0.125f) - 1.f;
    IncreaseBuffer[5] = std::cosh(0.15f) - 1.f;
    IncreaseBuffer[6] = std::cosh(0.175f) - 1.f;
    IncreaseBuffer[7] = std::cosh(0.2f) - 1.f;
    IncreaseBuffer[8] = std::cosh(0.225f) - 1.f;
    IncreaseBuffer[9] = std::cosh(0.25f) - 1.f;
    IncreaseBuffer[10] = std::cosh(0.275f) - 1.f;
    IncreaseBuffer[11] = std::cosh(0.3f) - 1.f;
    IncreaseBuffer[12] = std::cosh(0.325f) - 1.f;
    IncreaseBuffer[13] = std::cosh(0.35f) - 1.f;
    IncreaseBuffer[14] = std::cosh(0.375f) - 1.f;
    IncreaseBuffer[15] = std::cosh(0.4f) - 1.f;
    IncreaseBuffer[16] = std::cosh(0.425f) - 1.f;
    IncreaseBuffer[17] = std::cosh(0.45f) - 1.f;
    IncreaseBuffer[18] = std::cosh(0.475f) - 1.f;
    IncreaseBuffer[19] = std::cosh(0.5f) - 1.f;
    IncreaseBuffer[20] = std::cosh(0.525f) - 1.f;
    IncreaseBuffer[21] = std::cosh(0.55f) - 1.f;
    IncreaseBuffer[22] = std::cosh(0.575f) - 1.f;
    IncreaseBuffer[23] = std::cosh(0.6f) - 1.f;
    IncreaseBuffer[24] = std::cosh(0.625f) - 1.f;
    IncreaseBuffer[25] = std::cosh(0.65f) - 1.f;
    IncreaseBuffer[26] = std::cosh(0.675f) - 1.f;
    IncreaseBuffer[27] = std::cosh(0.7f) - 1.f;
    IncreaseBuffer[28] = std::cosh(0.725f) - 1.f;
    IncreaseBuffer[29] = std::cosh(0.75f) - 1.f;
    IncreaseBuffer[30] = std::cosh(0.775f) - 1.f;
    IncreaseBuffer[31] = std::cosh(0.8f) - 1.f;
    IncreaseBuffer[32] = std::cosh(0.825f) - 1.f;
    IncreaseBuffer[33] = std::cosh(0.85f) - 1.f;
    IncreaseBuffer[34] = std::cosh(0.875f) - 1.f;
    IncreaseBuffer[35] = std::cosh(0.9f) - 1.f;
    IncreaseBuffer[36] = std::cosh(0.925f) - 1.f;
    IncreaseBuffer[37] = std::cosh(0.95f) - 1.f;
    IncreaseBuffer[38] = std::cosh(0.975f) - 1.f;
    IncreaseBuffer[39] = std::cosh(1.f) - 1.f;
    IncreaseBuffer[40] = std::cosh(1.025f) - 1.f;
    IncreaseBuffer[41] = std::cosh(1.05f) - 1.f;
    IncreaseBuffer[42] = std::cosh(1.075f) - 1.f;
    IncreaseBuffer[43] = std::cosh(1.1f) - 1.f;
    IncreaseBuffer[44] = std::cosh(1.125f) - 1.f;
    IncreaseBuffer[45] = std::cosh(1.15f) - 1.f;
    IncreaseBuffer[46] = std::cosh(1.175f) - 1.f;
    IncreaseBuffer[47] = std::cosh(1.2f) - 1.f;
    IncreaseBuffer[48] = std::cosh(1.225f) - 1.f;
    IncreaseBuffer[49] = std::cosh(1.25f) - 1.f;
    IncreaseBuffer[50] = std::cosh(1.275f) - 1.f;
    IncreaseBuffer[51] = std::cosh(1.3f) - 1.f;
    IncreaseBuffer[52] = 1.f;
}
void Dynamic2D::freeIncreaseBuffer() {

    LOGV(LIBENG_LOG_DYNAMIC, 0, LOG_FORMAT(), __PRETTY_FUNCTION__, __LINE__);
    assert(IncreaseBuffer);

    delete [] IncreaseBuffer;
}

#ifndef DEBUG
void Dynamic2D::transform(const ShaderParam2D* shader2D) const {

#else
void Dynamic2D::transform(const ShaderParam2D* shader2D, const unsigned int* log) const {

    LOGV(LIBENG_LOG_DYNAMIC, (*log % 100), LOG_FORMAT(" - s:%p"), __PRETTY_FUNCTION__, __LINE__, shader2D);
#endif
    glUniform1f(shader2D->transXLoc, mTransform[TRANS_X]);
    glUniform1f(shader2D->transYLoc, mTransform[TRANS_Y]);
    glUniform1f(shader2D->scaleXLoc, mTransform[SCALE_X]);
    glUniform1f(shader2D->scaleYLoc, mTransform[SCALE_Y]);
    glUniform1f(shader2D->rotateLoc, mTransform[ROTATE_C]);
}

} // namespace
