#include "Element2D.h"

namespace eng {

//////
Element2D::Element2D(bool manage) : Static2D(manage), Dynamic2D(), mStacked(false), mTransX(0.f), mTransY(0.f) {

    LOGV(LIBENG_LOG_ELEMENT, 0, LOG_FORMAT(" - m:%s"), __PRETTY_FUNCTION__, __LINE__, (manage)? "true":"false");

    mTransform = new float[TRANSFORM_BUFFER_SIZE];
    std::memset(mTransform, 0, sizeof(float) * TRANSFORM_BUFFER_SIZE);
}
Element2D::~Element2D() { LOGV(LIBENG_LOG_ELEMENT, 0, LOG_FORMAT(), __PRETTY_FUNCTION__, __LINE__); }

void Element2D::addBound(Bounds* bound) {

#ifdef DEBUG
    LOGV(LIBENG_LOG_ELEMENT, 0, LOG_FORMAT(" - b:%p"), __PRETTY_FUNCTION__, __LINE__, bound);
    assert(bound);

    bound->setLog(mLog); // If not already done
#endif
    if (!mBounds) {

        LOGI(LIBENG_LOG_ELEMENT, 0, LOG_FORMAT(), __PRETTY_FUNCTION__, __LINE__);
        assert(!mBoundCount);

        mBoundCount = 1;
        mBounds = new Bounds*[mBoundCount];
        mBounds[0] = bound;
    }
    else {

        LOGI(LIBENG_LOG_ELEMENT, 0, LOG_FORMAT(), __PRETTY_FUNCTION__, __LINE__);
        assert(mBoundCount);

        Bounds** bounds = new Bounds*[mBoundCount + 1];
        for (unsigned char i = 0; i < mBoundCount; ++i)
            bounds[i] = mBounds[i];
        bounds[mBoundCount] = bound;

        ++mBoundCount;
        delete [] mBounds;
        mBounds = bounds;
    }
}
void Element2D::addVelocities(short count) {

    LOGV(LIBENG_LOG_ELEMENT, 0, LOG_FORMAT(" - c:%d"), __PRETTY_FUNCTION__, __LINE__, count);
    assert(count);
    assert(!mVelocities);

    mVelocities = new float[count];
    std::memset(mVelocities, 0, sizeof(float) * count);
}

void Element2D::start(unsigned char texture) {

    LOGV(LIBENG_LOG_ELEMENT, 0, LOG_FORMAT(" - t:%d"), __PRETTY_FUNCTION__, __LINE__, texture);
    Object2D::start(texture);

    std::memcpy(mTransform, Object2D::TransformBuffer, sizeof(float) * TRANSFORM_BUFFER_SIZE);
}
void Element2D::start(unsigned char red, unsigned char green, unsigned char blue) {

    LOGV(LIBENG_LOG_ELEMENT, 0, LOG_FORMAT(" - r:%d; g:%d; b:%d"), __PRETTY_FUNCTION__, __LINE__, red, green, blue);
    Object2D::start(red, green, blue);

    std::memcpy(mTransform, Object2D::TransformBuffer, sizeof(float) * TRANSFORM_BUFFER_SIZE);
}

void Element2D::render(bool resetUniform) const {

#ifdef DEBUG
    LOGV(LIBENG_LOG_ELEMENT, (*mLog % 100), LOG_FORMAT(" - r:%s (u:%s)"), __PRETTY_FUNCTION__, __LINE__,
            (resetUniform)? "true":"false", (mUpdated)? "true":"false");
    if ((resetUniform) || (mUpdated))
        Dynamic2D::transform(getShader2D(), mLog);
#else
    if ((resetUniform) || (mUpdated))
        Dynamic2D::transform(getShader2D());
#endif
    Panel2D::render(false);
}

} // namespace
