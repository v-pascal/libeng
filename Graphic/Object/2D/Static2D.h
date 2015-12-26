#ifndef LIBENG_STATIC2D_H_
#define LIBENG_STATIC2D_H_
#if defined(__cplusplus)

#include <libeng/Graphic/Object/2D/Panel2D.h>

namespace eng {

//////
class Static2D : public Panel2D {   // Static surface with texture coordinates management
                                    // -> Nothing more to implement
public:
    Static2D(bool manage = true);
    virtual ~Static2D();

    inline void setVertices(const float coords[8]) {

        mVertices[0] = coords[0];
        mVertices[1] = coords[1];
        mVertices[2] = coords[2];
        mVertices[3] = coords[3];
        mVertices[4] = coords[4];
        mVertices[5] = coords[5];
        mVertices[6] = coords[6];
        mVertices[7] = coords[7];
    };
    void setVertices(short x0, short y0, short x2, short y2);
    // Position the surface according a left/top & right/bottom screen coordinates (in pixel)
    // -> With [0;0]: Bottom/left coordinates; [g_ScreenWidth;g_ScreenHeight]: Top/right coordinates

    // WARNING: Low performance! Use only when loading

    inline void size(float x, float y) {

        assert(x > 0.f);
        assert(y > 0.f);

        mVertices[0] *= x;
        mVertices[1] *= y;
        mVertices[2] *= x;
        mVertices[3] *= y;
        mVertices[4] *= x;
        mVertices[5] *= y;
        mVertices[6] *= x;
        mVertices[7] *= y;
    }
    // Same as 'Dynamic2D::scale' method but applied on 'mVertices' buffer

    inline void setTexCoords(const float coords[8]) {

        mTexCoords[0] = coords[0];
        mTexCoords[1] = coords[1];
        mTexCoords[2] = coords[2];
        mTexCoords[3] = coords[3];
        mTexCoords[4] = coords[4];
        mTexCoords[5] = coords[5];
        mTexCoords[6] = coords[6];
        mTexCoords[7] = coords[7];
    };
    inline void copyTexCoords(const Static2D &copy) { setTexCoords(copy.mTexCoords); }
    inline void copyVertices(const Static2D &copy) { setVertices(copy.mVertices); }

    //////
    virtual void render(bool resetUniform) const;

};

} // namespace

// Cast(s)
#define stat2DVia(o)    static_cast<const eng::Static2D*>(o)

#endif // __cplusplus
#endif // LIBENG_STATIC2D_H_
