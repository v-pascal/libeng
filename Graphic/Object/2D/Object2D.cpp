#include "Object2D.h"
#include "Dynamic2D.h"

#ifndef __ANDROID__
#include "OpenGLES/ES2/gl.h"
#endif

#include <libeng/Game/2D/Game2D.h>

namespace eng {

//////
Object2D::Object2D(bool manage) : Object(), mTextureIdx(TEXTURE_IDX_INVALID), mShaderParam(NULL),
        mShaderParam2D(NULL), mManageTexture(manage), mTextures(NULL), mRed(1.f), mGreen(1.f), mBlue(1.f),
        mAlpha(1.f), mStarted(false) {

    LOGV(LIBENG_LOG_OBJECT, 0, LOG_FORMAT(" - m:%s"), __PRETTY_FUNCTION__, __LINE__, (manage)? "true":"false");
#if defined(LIBENG_ENABLE_CAMERA) && !defined(__ANDROID__)
    mBGRA = false;
#endif
}
Object2D::~Object2D() {

    LOGV(LIBENG_LOG_OBJECT, 0, LOG_FORMAT(), __PRETTY_FUNCTION__, __LINE__);
    if ((mManageTexture) && (mTextureIdx != TEXTURE_IDX_INVALID))   // It is possible to destroy an object without having
        mTextures->delTexture(mTextureIdx);                     // generated any texture (so check valid texture index)
}

void Object2D::initialize(const Game2D* game) {

#ifdef DEBUG
    LOGV(LIBENG_LOG_OBJECT, 0, LOG_FORMAT(" - g:%p"), __PRETTY_FUNCTION__, __LINE__, game);
    Object::initialize(game->getScreen(), game->getLog());
#else
    Object::initialize(game->getScreen());
#endif
    mShaderParam = game->getShader();
    mShaderParam2D = game->getShader2D();
    mTextures = Textures::getInstance();
}

void Object2D::transform() const {

#ifdef DEBUG
    LOGV(LIBENG_LOG_OBJECT, (*mLog % 100), LOG_FORMAT(" - (T:%p)"), __PRETTY_FUNCTION__, __LINE__, TransformBuffer);
#endif
    glUniform1f(mShaderParam2D->transXLoc, TransformBuffer[Dynamic2D::TRANS_X]);
    glUniform1f(mShaderParam2D->transYLoc, TransformBuffer[Dynamic2D::TRANS_Y]);
    glUniform1f(mShaderParam2D->scaleXLoc, TransformBuffer[Dynamic2D::SCALE_X]);
    glUniform1f(mShaderParam2D->scaleYLoc, TransformBuffer[Dynamic2D::SCALE_Y]);
    glUniform1f(mShaderParam2D->rotateLoc, TransformBuffer[Dynamic2D::ROTATE_C]);
}

void Object2D::resume(unsigned char texture) {

    LOGV(LIBENG_LOG_OBJECT, 0, LOG_FORMAT(" - t:%d (m:%s)"), __PRETTY_FUNCTION__, __LINE__, texture,
            (mManageTexture)? "true":"false");
    if (mManageTexture)
        Object2D::start(texture);  // texture == texture ID

    else { // texture == texture index (already loaded)

        assert(texture != TEXTURE_IDX_INVALID);
        assert(mTextureIdx == TEXTURE_IDX_INVALID); // When paused texture index must be reseted (see 'pause' method)

        mTextureIdx = texture;
    }
}
void Object2D::resume(unsigned char red, unsigned char green, unsigned char blue) {

    LOGV(LIBENG_LOG_OBJECT, 0, LOG_FORMAT(" - r:%d; g:%d; b:%d (m:%s)"), __PRETTY_FUNCTION__, __LINE__, red, green,
            blue, (mManageTexture)? "true":"false");
    if (mManageTexture)
        Object2D::start(red, green, blue);

    else {

        assert(mTextures->getIndex(TEXTURE_ID_COLOR) != TEXTURE_IDX_INVALID);
        assert(mTextureIdx == TEXTURE_IDX_INVALID); // When paused texture index must be reseted (see 'pause' method)

        mRed = static_cast<float>(red) / 0xff;
        mGreen = static_cast<float>(green) / 0xff;
        mBlue = static_cast<float>(blue) / 0xff;

        mTextureIdx = mTextures->getIndex(TEXTURE_ID_COLOR);
    }
}

void Object2D::start(unsigned char texture) {

#ifdef DEBUG
    LOGV(LIBENG_LOG_OBJECT, 0, LOG_FORMAT(" - t:%d (m:%s)"), __PRETTY_FUNCTION__, __LINE__, texture,
            (mManageTexture)? "true":"false");
    if ((!mScreen) || (!mShaderParam) || (!mShaderParam2D) || (!mTextures)) {

        LOGW(LOG_FORMAT(" - Start object that has not been initialized"), __PRETTY_FUNCTION__, __LINE__);
        assert(NULL);
    }
#endif
    if (mManageTexture) { // texture == texture ID

        assert(texture != TEXTURE_ID_COLOR);
        assert(texture != TEXTURE_ID_CAM);
#ifdef LIBENG_ENABLE_SOCIAL
        assert(texture != Facebook::TEXTURE_ID);
        assert(texture != Twitter::TEXTURE_ID);
        assert(texture != Google::TEXTURE_ID);
#endif
        mTextureIdx = mTextures->getIndex(texture);
        if (mTextureIdx == TEXTURE_IDX_INVALID)
            mTextureIdx = Textures::loadTexture(texture); // Load texture

        mTextures->genTexture(mTextureIdx);
    }
    else { // texture == texture index (already loaded)

        assert(texture != TEXTURE_IDX_INVALID);
        mTextureIdx = texture;
    }
    mStarted = true;
}
void Object2D::start(unsigned char red, unsigned char green, unsigned char blue) {

#ifdef DEBUG
    LOGV(LIBENG_LOG_OBJECT, 0, LOG_FORMAT(" - r:%d; g:%d; b:%d (m:%s)"), __PRETTY_FUNCTION__, __LINE__, red, green,
            blue, (mManageTexture)? "true":"false");
    if ((!mScreen) || (!mShaderParam) || (!mShaderParam2D) || (!mTextures)) {

        LOGW(LOG_FORMAT(" - Start object that has not been initialized"), __PRETTY_FUNCTION__, __LINE__);
        assert(NULL);
    }
#endif
    if (mManageTexture) {

        mTextureIdx = mTextures->getIndex(TEXTURE_ID_COLOR);
        if (mTextureIdx == TEXTURE_IDX_INVALID)
            mTextureIdx = mTextures->addTexColor();

        mTextures->genTexture(mTextureIdx);
    }
    else {

        assert(mTextures->getIndex(TEXTURE_ID_COLOR) != TEXTURE_IDX_INVALID);
        mTextureIdx = mTextures->getIndex(TEXTURE_ID_COLOR);
    }
    mRed = static_cast<float>(red) / 0xff;
    mGreen = static_cast<float>(green) / 0xff;
    mBlue = static_cast<float>(blue) / 0xff;

    mStarted = true;
}

void Object2D::render(bool unused) const {

#ifdef DEBUG
    if (mTextureIdx == TEXTURE_IDX_INVALID) {

        LOGE(LOG_FORMAT(" - Invalid texture index"), __PRETTY_FUNCTION__, __LINE__);
        assert(NULL);
        return;
    }
    if (!(*mTextures)[mTextureIdx]->refCount) {
        
        if (mTextureIdx == mTextures->getIndex(TEXTURE_ID_CAM)) {
            LOGW(LOG_FORMAT(" - Render with ungenerated camera texture (i:%d)"), __PRETTY_FUNCTION__, __LINE__, mTextureIdx);
        }
        else {
            LOGE(LOG_FORMAT(" - Render with ungenerated texture (i:%d)"), __PRETTY_FUNCTION__, __LINE__, mTextureIdx);
            assert(NULL);
        }
        return;
    }
#else
    if (!(*mTextures)[mTextureIdx]->refCount)
        return; // Should be usefull for camera texture only
#endif

    // Load the vertex & texture position
    glVertexAttribPointer(mShaderParam->positionLoc, 2, GL_FLOAT, GL_FALSE, 0, mVertices);
    glVertexAttribPointer(mShaderParam->texcoordLoc, 2, GL_FLOAT, GL_FALSE, 0, mTexCoords);

    // Load color filter attributes
    glVertexAttrib1f(mShaderParam->redLoc, mRed);
    glVertexAttrib1f(mShaderParam->greenLoc, mGreen);
    glVertexAttrib1f(mShaderParam->blueLoc, mBlue);
    glVertexAttrib1f(mShaderParam->alphaLoc, mAlpha);
#if defined(LIBENG_ENABLE_CAMERA) && !defined(__ANDROID__)
    glVertexAttrib1f(mShaderParam->bgraLoc, static_cast<GLfloat>(mBGRA));
#endif

    // Bind the texture
    glBindTexture(GL_TEXTURE_2D, (*mTextures)[mTextureIdx]->textureId);

    // Set the sampler texture unit to 0
    glUniform1i(mShaderParam->samplerLoc, 0);
}

} // namespace
