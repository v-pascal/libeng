#ifndef LIBENG_CHAR2D_H_
#define LIBENG_CHAR2D_H_
#if defined(__cplusplus)

#include <libeng/Global.h>
#ifdef LIBENG_ENABLE_TEXT

#include <libeng/Global.h>
#include <libeng/Graphic/Object/2D/Panel2D.h>
#include <complex>

namespace eng {

//////
class Char2D : public Panel2D {

private:
    bool mCopied; // Positionned on first character

public:
    Char2D();
    Char2D(const Char2D& copy);
    virtual ~Char2D();

    inline void translate(float x, float y) {

    	mVertices[0] += x;
    	mVertices[1] += y;
    	mVertices[2] = mVertices[0];
    	mVertices[3] += y;
    	mVertices[4] += x;
    	mVertices[5] = mVertices[3];
    	mVertices[6] = mVertices[4];
    	mVertices[7] = mVertices[1];
    }
    inline void scale(float x, float y) {

        mVertices[0] *= x;
        mVertices[1] *= y;
        mVertices[2] = mVertices[0];
        mVertices[3] *= y;
        mVertices[4] *= x;
        mVertices[5] = mVertices[3];
        mVertices[6] = mVertices[4];
        mVertices[7] = mVertices[1];
    }
    inline void position(float left, float top, short charRank) {

        mVertices[2] = left;
        mVertices[3] = top - (mVertices[1] - mVertices[3]);
        mVertices[4] = left + (mVertices[4] - mVertices[0]);
        mVertices[5] = mVertices[3];
        mVertices[6] = mVertices[4];

        mVertices[0] = left;
        mVertices[1] = top;
        mVertices[7] = top;

        if (charRank) // Shift character according its rank
            translate(charRank * (mVertices[4] - left), 0.f);
    }

    //////
    void start(const Fonts* fonts, short fontIdx, wchar_t character, short charRank);

    inline void update(const Fonts* fonts, short fontIdx, wchar_t character) {

        LOGV(LIBENG_LOG_TEXT, 0, LOG_FORMAT(" - f:%p; o:%d; c:0x%X"), __PRETTY_FUNCTION__, __LINE__, fonts, fontIdx, character);
        fonts->getFontCoords(mTexCoords, fontIdx, character);
    }
    void render() const;

};

} // namespace

#endif // LIBENG_ENABLE_TEXT
#endif // __cplusplus
#endif // LIBENG_CHAR2D_H_
