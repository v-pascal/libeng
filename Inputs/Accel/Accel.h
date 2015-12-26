#ifndef LIBENG_ACCEL_H_
#define LIBENG_ACCEL_H_
#if defined(__cplusplus)

#include <libeng/Global.h>
#include <libeng/Log/Log.h>
#include <libeng/Inputs/Input.h>
#include <assert.h>
#include <complex>

//#ifdef __ANDROID__
#define ACCEL_LIMITS_SUP        9.80665f // Android: = 'mAccelSensor.getMaximumRange' / 2  (Samsung Galaxy Tab2)
//#else // iOS
//#endif
#define ACCEL_LIMITS_INF        -ACCEL_LIMITS_SUP
// Accelerometer limits when used in a normal way (without shaking the device)

#define ORIENTATION_SUP         (ACCEL_LIMITS_SUP / 2.f)
#define ORIENTATION_INF         -ORIENTATION_SUP
#define ACCEL_CLIP(xyz)         ((xyz > ACCEL_LIMITS_SUP)? ACCEL_LIMITS_SUP:(xyz < ACCEL_LIMITS_INF)? ACCEL_LIMITS_INF:xyz)

namespace eng {

static const float DEFAULT_ACCEL_PRECISION = 0.3f; // Acceleromteter rate precision

typedef struct {

    float X;
    float Y;
    float Z;

} AccelData;

inline void assignAccel(AccelData* destination, const AccelData* source) {

    destination->X = source->X;
    destination->Y = source->Y;
    destination->Z = source->Z;
}

enum {

    ORT_UNKNOWN = 0,

    ORT_PORTRAIT,
    ORT_REV_PORTRAIT,
    ORT_LANDSCAPE,
    ORT_REV_LANDSCAPE,
    ORT_FLAT,
    ORT_REVERSED
};

//////
class AccelInput : public Input {

private:
    const float* mMaxRange;
    float mPrecision;

    AccelData mAccel;

public:
    AccelInput(const void* data) : Input(), mMaxRange(NULL), mPrecision(DEFAULT_ACCEL_PRECISION) {

#ifdef DEBUG
        LOGV(LIBENG_LOG_ACCEL, 0, LOG_FORMAT(" - d:%p"), __PRETTY_FUNCTION__, __LINE__, data);
        mLogGet = 0;
        mLogUpd = 0;
#endif
        assignAccel(&mAccel, static_cast<const AccelData*>(data));
    }
    static unsigned char getOrientation(const AccelData* accel) {

        if (!accel)
            return ORT_UNKNOWN;

#ifdef __ANDROID__
#ifdef DEBUG
        const Input* accelInput = Inputs::getInstance()->get(Inputs::ACCEL);
        assert((*static_cast<const float*>(accelInput->getConfig())) < ((ACCEL_LIMITS_SUP * 2.f) + 1.f));
        assert((*static_cast<const float*>(accelInput->getConfig())) > ((ACCEL_LIMITS_SUP * 2.f) - 1.f));
#endif
        float x = ACCEL_CLIP(accel->X);
        float y = ACCEL_CLIP(accel->Y);
        float z = ACCEL_CLIP(accel->Z);

#else
        // No maximum accelerometer value available on iOS
        // -> Using a common range of [-11;11] values (after having * 10)

        float x = ACCEL_CLIP(-accel->X * 10.f);
        float y = ACCEL_CLIP(-accel->Y * 10.f);
        float z = ACCEL_CLIP(-accel->Z * 10.f);
#endif
        unsigned char orientation = ORT_UNKNOWN;
        if ((ORIENTATION_INF < z) && (z < ORIENTATION_SUP)) {

            if ((ORIENTATION_INF < x) && (x < ORIENTATION_SUP)) orientation = (ORIENTATION_SUP < y)? ORT_PORTRAIT:ORT_REV_PORTRAIT;
            else if ((ORIENTATION_INF < y) && (y < ORIENTATION_SUP)) orientation = (ORIENTATION_SUP < x)? ORT_LANDSCAPE:ORT_REV_LANDSCAPE;
        }
        if ((!orientation) && (ORIENTATION_INF < x) && (x < ORIENTATION_SUP) &&
                (ORIENTATION_INF < y) && (y < ORIENTATION_SUP)) orientation = (ORIENTATION_SUP < z)? ORT_FLAT:ORT_REVERSED;

        return orientation;
    }

    //////
    inline void setConfig(const void* config) { mMaxRange = static_cast<const float*>(config); }
    inline void setPrecision(const void* precision) { mPrecision = *static_cast<const float*>(precision); }

    inline const void* getConfig() const { return static_cast<const void*>(mMaxRange); }
    inline bool getData(void* data, unsigned char index = 0) const {

#ifdef DEBUG
        ++mLogGet;
        LOGV(LIBENG_LOG_ACCEL, (mLogGet % 100), LOG_FORMAT(" - d:%p; i:%d"), __PRETTY_FUNCTION__, __LINE__, data, index);
        assert(!index);
#endif
        assignAccel(static_cast<AccelData*>(data), &mAccel);
        return true;
    }
    inline void updateData(const void* data) {

#ifdef DEBUG
        ++mLogUpd;
        LOGV(LIBENG_LOG_ACCEL, (mLogUpd % 100), LOG_FORMAT(" - d:%p"), __PRETTY_FUNCTION__, __LINE__, data);
#endif
        // Check precision
        const AccelData* newAccel = static_cast<const AccelData*>(data);
        if ((mPrecision < 0.0001f) ||
                (std::abs<float>(mAccel.X - newAccel->X) > mPrecision) ||
                (std::abs<float>(mAccel.Y - newAccel->Y) > mPrecision) ||
                (std::abs<float>(mAccel.Z - newAccel->Z) > mPrecision))
            assignAccel(&mAccel, newAccel);
    }
    inline void purgeData() { }

};

} // namespace

#endif // __cplusplus
#endif // LIBENG_ACCEL_H_
