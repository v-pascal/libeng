#ifndef LIBENG_SPRITE2D_H_
#define LIBENG_SPRITE2D_H_
#if defined(__cplusplus)

#include <libeng/Global.h>
#ifdef LIBENG_ENABLE_SPRITE

#include <libeng/Graphic/Object/2D/Panel2D.h>
#include <libeng/Graphic/Object/2D/Dynamic2D.h>

namespace eng {

//////
class Sprite2D : public Panel2D, public Dynamic2D { // Dynamic surface with animations management
                                                    // -> Need to implement 'Dynamic2D' abstract methods
                                                    // -> Need to implement 'fillVerticesAnims' method
    unsigned char mAnimCount;

protected:
    float** mVerticesAnims;
    unsigned char mCurAnim; // Current animation index

public:
    Sprite2D(unsigned char animCount);
    virtual ~Sprite2D();

    inline void animVertices(unsigned char animIdx) {

        assert(animIdx < mAnimCount);

        mVertices[0] = mVerticesAnims[animIdx][0];
        mVertices[1] = mVerticesAnims[animIdx][1];
        mVertices[2] = mVerticesAnims[animIdx][2];
        mVertices[3] = mVerticesAnims[animIdx][3];
        mVertices[4] = mVerticesAnims[animIdx][4];
        mVertices[5] = mVerticesAnims[animIdx][5];
        mVertices[6] = mVerticesAnims[animIdx][6];
        mVertices[7] = mVerticesAnims[animIdx][7];
        // ...in original position (centered)
    };
    inline void animTexCoords(const float texcoordAnims[][8], unsigned char animIdx, bool reversed) {

        assert(animIdx < mAnimCount);
        if (reversed) {

            mTexCoords[0] = texcoordAnims[animIdx][6];
            mTexCoords[1] = texcoordAnims[animIdx][1];
            mTexCoords[2] = texcoordAnims[animIdx][4];
            mTexCoords[3] = texcoordAnims[animIdx][3];
            mTexCoords[4] = texcoordAnims[animIdx][2];
            mTexCoords[5] = texcoordAnims[animIdx][5];
            mTexCoords[6] = texcoordAnims[animIdx][0];
            mTexCoords[7] = texcoordAnims[animIdx][7];
        }
        else {

            mTexCoords[0] = texcoordAnims[animIdx][0];
            mTexCoords[1] = texcoordAnims[animIdx][1];
            mTexCoords[2] = texcoordAnims[animIdx][2];
            mTexCoords[3] = texcoordAnims[animIdx][3];
            mTexCoords[4] = texcoordAnims[animIdx][4];
            mTexCoords[5] = texcoordAnims[animIdx][5];
            mTexCoords[6] = texcoordAnims[animIdx][6];
            mTexCoords[7] = texcoordAnims[animIdx][7];
        }
    };
    inline void reverse(bool horizontal) {

        if (horizontal) { // According a horizontal axis

            std::swap<float>(mVertices[1], mVertices[3]);
            std::swap<float>(mVertices[7], mVertices[5]);
        }
        else { // According a vertical axis

            std::swap<float>(mVertices[0], mVertices[6]);
            std::swap<float>(mVertices[2], mVertices[4]);
        }
    };

    //////
protected:
    virtual void fillVerticesAnims(unsigned char animIdx) = 0;

public:
    void start(unsigned char texture);
    void render(bool resetUniform) const;

};

} // namepsace

// Cast(s)
#define sprit2DVia(o)   static_cast<const eng::Sprite2D*>(o)

#endif // LIBENG_ENABLE_SPRITE
#endif // __cplusplus
#endif // LIBENG_SPRITE2D_H_
