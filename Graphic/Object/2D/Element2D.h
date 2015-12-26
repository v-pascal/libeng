#ifndef LIBENG_ELEMENT2D_H_
#define LIBENG_ELEMENT2D_H_
#if defined(__cplusplus)

#include <libeng/Graphic/Object/2D/Static2D.h>
#include <libeng/Graphic/Object/2D/Dynamic2D.h>
#include <libeng/Log/Log.h>

namespace eng {

//////
class Element2D : public Static2D, public Dynamic2D { // Dynamic surface with texture coordinates, bounds & velocities management
                                                        // -> Nothing more to implement
private:
    bool mStacked;

    float mTransX;
    float mTransY;

public:
    Element2D(bool manage = true);
    virtual ~Element2D();

    void addBound(Bounds* bound);
    void addVelocities(short count);

    //
    inline void stack() {

        LOGV(LIBENG_LOG_ELEMENT, 0, LOG_FORMAT(), __PRETTY_FUNCTION__, __LINE__);
        assert(!mStacked);

#ifndef LIBENG_PORT_AS_LAND
        mVertices[0] += mTransform[Dynamic2D::TRANS_X];
        mVertices[2] += mTransform[Dynamic2D::TRANS_X];
        mVertices[4] += mTransform[Dynamic2D::TRANS_X];
        mVertices[6] += mTransform[Dynamic2D::TRANS_X];
#else
        mVertices[0] -= mTransform[Dynamic2D::TRANS_X];
        mVertices[2] -= mTransform[Dynamic2D::TRANS_X];
        mVertices[4] -= mTransform[Dynamic2D::TRANS_X];
        mVertices[6] -= mTransform[Dynamic2D::TRANS_X];
#endif
        mVertices[1] += mTransform[Dynamic2D::TRANS_Y];
        mVertices[3] += mTransform[Dynamic2D::TRANS_Y];
        mVertices[5] += mTransform[Dynamic2D::TRANS_Y];
        mVertices[7] += mTransform[Dynamic2D::TRANS_Y];

        mTransX = mTransform[Dynamic2D::TRANS_X];
        mTransY = mTransform[Dynamic2D::TRANS_Y];

        mTransform[Dynamic2D::TRANS_X] = 0.f;
        mTransform[Dynamic2D::TRANS_Y] = 0.f;

        mStacked = true;

        // Do not update bounds coz unchanged
        // -> A translation using vertices or using transformation should be managed in same way at bounds level
    };
    inline void unstack() {

        LOGV(LIBENG_LOG_ELEMENT, 0, LOG_FORMAT(), __PRETTY_FUNCTION__, __LINE__);
        assert(mStacked);

#ifndef LIBENG_PORT_AS_LAND
        mVertices[0] -= mTransX;
        mVertices[2] -= mTransX;
        mVertices[4] -= mTransX;
        mVertices[6] -= mTransX;
#else
        mVertices[0] += mTransX;
        mVertices[2] += mTransX;
        mVertices[4] += mTransX;
        mVertices[6] += mTransX;
#endif
        mVertices[1] -= mTransY;
        mVertices[3] -= mTransY;
        mVertices[5] -= mTransY;
        mVertices[7] -= mTransY;

        mTransform[Dynamic2D::TRANS_X] += mTransX;
        mTransform[Dynamic2D::TRANS_Y] += mTransY;
        // '+=' and not '=' coz translation matrix can have meanwhile changed

        mStacked = false;
    };
    inline bool isStacked() const { return mStacked; }

    //////
    void start(unsigned char texture);
    void start(unsigned char red, unsigned char green, unsigned char blue);

    inline void update(const Game* game, const Level* level) { assert(NULL); }

    inline bool checkCollision(const Bounds* bounds, unsigned char boundsId) { assert(NULL); return false; }
    inline bool checkCollision(const Object* object, unsigned char objectId) { assert(NULL); return false; }

    void render(bool resetUniform) const;

};

} // namespace

// Cast(s)
#define elem2DVia(o)    static_cast<const eng::Element2D*>(o)

#endif // __cplusplus
#endif // LIBENG_ELEMENT2D_H_
