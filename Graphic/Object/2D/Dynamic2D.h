#ifndef LIBENG_DYNAMIC2D_H_
#define LIBENG_DYNAMIC2D_H_
#if defined(__cplusplus)

#include <libeng/Log/Log.h>

#include <libeng/Graphic/Object/Dynamic.h>
#include <libeng/Render/Shader.h>
#include <libeng/Render/2D/Shader2D.h>
#include <libeng/Graphic/Bounds/2D/Bounds2D.h>

namespace eng {

static const unsigned char TRANSFORM_BUFFER_SIZE = 7;
static const unsigned char INCREASE_BUFFER_SIZE = 53;
static const unsigned char INCREASE_TANGENT_IDX = 30; // Increase buffer index of the hyperbolic tangent
                                                      // -> From which the curve changes significantly (used to change animations)

static const float DEFAULT_TRANSFORM_BUFFER[] = { 1.f, 1.f, 0.f, 0.f, 1.f, 1.f, 0.f };

//////
class Dynamic2D : public Dynamic {

protected:
    float* mVelocities;

    float* mTransform;
#ifdef DEBUG
    void transform(const ShaderParam2D* shader, const unsigned int* log) const;
#else
    void transform(const ShaderParam2D* shader) const;
#endif

public:
    Dynamic2D();
    virtual ~Dynamic2D();

    typedef enum {

        RATIO_X = 0,
        RATIO_Y,
        TRANS_X,
        TRANS_Y,
        SCALE_X,
        SCALE_Y,
        ROTATE_C    // Clockwise rotation

    } TransformType;

    inline const float* getTransform() const { return mTransform; }

    inline float* getVelocities() { return mVelocities; }
    inline const float* getVelocities() const { return mVelocities; }

    static float* IncreaseBuffer; // Hyperbolic increase buffer (gravity simulation)
    static void initIncreaseBuffer();
    static void freeIncreaseBuffer();

    //////
    inline void reset() {

        for (unsigned char i = 0; i < mBoundCount; ++i)
#ifndef LIBENG_PORT_AS_LAND
            static_cast<Bounds2D*>(mBounds[i])->translate(-mTransform[TRANS_X], -mTransform[TRANS_Y]);
#else
            static_cast<Bounds2D*>(mBounds[i])->translate(mTransform[TRANS_X], -mTransform[TRANS_Y]);
#endif
        mTransform[TRANS_X] = 0.f;
        mTransform[TRANS_Y] = 0.f;
        mUpdated = true;
    }
    inline void resetY() {

        for (unsigned char i = 0; i < mBoundCount; ++i)
            static_cast<Bounds2D*>(mBounds[i])->translate(0.f, -mTransform[TRANS_Y]);

        mTransform[TRANS_Y] = 0.f;
        mUpdated = true;
    }

    inline void translate(float x, float y) {

#ifndef LIBENG_PORT_AS_LAND
        mTransform[TRANS_X] += x;
#else
        mTransform[TRANS_X] -= x;
#endif
        mTransform[TRANS_Y] += y;
        mUpdated = true;

        for (unsigned char i = 0; i < mBoundCount; ++i)
            static_cast<Bounds2D*>(mBounds[i])->translate(x, y);
    }
    inline void rotate(float angle) {

        for (unsigned char i = 0; i < mBoundCount; ++i)
            static_cast<Bounds2D*>(mBounds[i])->rotate(angle - mTransform[ROTATE_C], mTransform[TRANS_X],
                    mTransform[TRANS_Y]);

#ifndef LIBENG_PORT_AS_LAND
        mTransform[ROTATE_C] = angle;
#else
        //mTransform[ROTATE_C] = angle - (boost::math::constants::pi<float>() / 2.f);
        // WARNING: Do not substract here but start rotate from -Pi/2!
        // -> [0;-2Pi] => [-Pi/2;-5Pi/2] for a clockwise rotation / [0;2Pi] => [-Pi/2;3Pi/2] for a counter-clockwise rotation

        mTransform[ROTATE_C] = angle;
#endif
        mUpdated = true;
    }
    inline void scale(float x, float y) {

#ifdef DEBUG
        if ((!(x > 0.f)) || (!(y > 0.f))) {

            LOGW(LOG_FORMAT(" - Scale must be > 0"), __PRETTY_FUNCTION__, __LINE__);
            assert(NULL);
        }
#endif

        if (mBounds) {

            float scaleX = x / mTransform[SCALE_X];
            float scaleY = y / mTransform[SCALE_Y];

            assert(mBoundCount > 0);
            for (unsigned char i = 0; i < mBoundCount; ++i)
                static_cast<Bounds2D*>(mBounds[i])->scale(scaleX, scaleY);
        }
        mTransform[SCALE_X] = x;
        mTransform[SCALE_Y] = y;
        mUpdated = true;

        // WARNING: When changing the scale using matrix verify the 2D object is centered in the screen (according
        //          the initial vertices buffer)
        // -> If needed to move it use 'Dynamic2D::translate' method
    }

};

} // namespace

#endif // __cplusplus
#endif // LIBENG_DYNAMIC2D_H_
