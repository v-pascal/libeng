#include "Sprite2D.h"

#ifdef LIBENG_ENABLE_SPRITE

namespace eng {

//////
Sprite2D::Sprite2D(unsigned char animCount) : Panel2D(), Dynamic2D(), mCurAnim(0) {

    LOGV(LIBENG_LOG_SPRITE, 0, LOG_FORMAT(" - a:%d"), __PRETTY_FUNCTION__, __LINE__, animCount);
    assert(animCount > 0);

    mTransform = new float[TRANSFORM_BUFFER_SIZE];
    std::memset(mTransform, 0, sizeof(float) * TRANSFORM_BUFFER_SIZE);

    mTexCoords = new float[8];
    std::memset(mTexCoords, 0, sizeof(float) * 8);

    mAnimCount = animCount;
    mVerticesAnims = new float*[mAnimCount];
    mVerticesAnims[0] = NULL;
}
Sprite2D::~Sprite2D() {

    LOGV(LIBENG_LOG_SPRITE, 0, LOG_FORMAT(), __PRETTY_FUNCTION__, __LINE__);
    if (mVerticesAnims) {

        if (mVerticesAnims[0] != NULL)
            for (unsigned char i = 0; i < mAnimCount; ++i)
                delete [] mVerticesAnims[i]; // float[8]
        delete [] mVerticesAnims;
    }
}

void Sprite2D::start(unsigned char texture) {

    LOGV(LIBENG_LOG_SPRITE, 0, LOG_FORMAT(" - t:%d"), __PRETTY_FUNCTION__, __LINE__, texture);
    Object2D::start(texture);

    std::memcpy(mTransform, Object2D::TransformBuffer, sizeof(float) * TRANSFORM_BUFFER_SIZE);
}
void Sprite2D::render(bool resetUniform) const {

#ifdef DEBUG
    LOGV(LIBENG_LOG_SPRITE, (*mLog % 100), LOG_FORMAT(" - r:%s (u:%s)"), __PRETTY_FUNCTION__, __LINE__,
            (resetUniform)? "true":"false", (mUpdated)? "true":"false");
    if ((mUpdated) || (resetUniform))
        Dynamic2D::transform(getShader2D(), mLog);
#else
    if ((mUpdated) || (resetUniform))
        Dynamic2D::transform(getShader2D());
#endif
    Panel2D::render(false);
}

} // namepsace

#endif // LIBENG_ENABLE_SPRITE
