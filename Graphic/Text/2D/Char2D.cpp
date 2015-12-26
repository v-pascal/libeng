#include "Text2D.h"

#ifdef LIBENG_ENABLE_TEXT
#include <libeng/Log/Log.h>
#include <assert.h>
#include <cstring>

namespace eng {

//////
Char2D::Char2D() : Panel2D(false), mCopied(false) { // No texture management here (done in 'Text2D')

    LOGV(LIBENG_LOG_TEXT, 0, LOG_FORMAT(), __PRETTY_FUNCTION__, __LINE__);

    mTexCoords = new float[8];
    std::memset(mTexCoords, 0, sizeof(float) * 8);
}
Char2D::Char2D(const Char2D& copy) : Panel2D(false), mCopied(true) {

    LOGV(LIBENG_LOG_TEXT, 0, LOG_FORMAT(), __PRETTY_FUNCTION__, __LINE__);

    mTexCoords = new float[8];
    std::memset(mTexCoords, 0, sizeof(float) * 8);
    std::memcpy(mVertices, copy.mVertices, sizeof(float) * 8);
}
Char2D::~Char2D() { LOGV(LIBENG_LOG_TEXT, 0, LOG_FORMAT(), __PRETTY_FUNCTION__, __LINE__); }

void Char2D::start(const Fonts* fonts, short fontIdx, wchar_t character, short charRank) {

    LOGV(LIBENG_LOG_TEXT, 0, LOG_FORMAT(" - f:%p; o:%d; c:0x%X; h:%d"), __PRETTY_FUNCTION__, __LINE__, fonts, fontIdx,
            character, charRank);
    assert(mTextures->getIndex(TEXTURE_ID_FONT) != TEXTURE_IDX_INVALID);
    Object2D::start(mTextures->getIndex(TEXTURE_ID_FONT));

    fonts->getFontCoords(mTexCoords, fontIdx, character);

    if (!mCopied) {

        LOGI(LIBENG_LOG_TEXT, 0, LOG_FORMAT(), __PRETTY_FUNCTION__, __LINE__);
#ifndef LIBENG_LAND_ORIENTATION
#ifndef LIBENG_PORT_AS_LAND
        assert(mScreen->height > mScreen->width); // Portrait orientation
#else
        assert(mScreen->width > mScreen->height); // Portrait orientation as landscape (width & height have been swapped)
#endif
#else
        assert(mScreen->height < mScreen->width); // Landscape orientation
#endif
        // Font size vertices coordinate
        mVertices[1] = ((*fonts)[fontIdx]->heightPix * mScreen->ratioH);
        mVertices[4] = ((*fonts)[fontIdx]->widthPix * mScreen->ratioW);
        mVertices[6] = mVertices[4];
        mVertices[7] = mVertices[1];

        // Right shift according previous character rank (if any)
        if (charRank) {

            mVertices[0] = charRank * mVertices[4];
            mVertices[2] = mVertices[0];
            mVertices[4] += mVertices[0];
            mVertices[6] = mVertices[4];
        }
    }
    else { // Already positionned on first character

        LOGI(LIBENG_LOG_TEXT, 0, LOG_FORMAT(), __PRETTY_FUNCTION__, __LINE__);
        assert(charRank);

        // Always right shift according previous character rank
        float shift = charRank * std::abs<float>(mVertices[4] - mVertices[0]);

        mVertices[0] += shift;
        mVertices[2] = mVertices[0];
        mVertices[4] += shift;
        mVertices[6] = mVertices[4];
    }
}

void Char2D::render() const {

#ifdef DEBUG
    LOGV(LIBENG_LOG_TEXT, (*mLog % 100), LOG_FORMAT(), __PRETTY_FUNCTION__, __LINE__);
#endif
    //transform(); // Called in 'Text2D::render' method
    Panel2D::render(false);
}

} // namespace

#endif // LIBENG_ENABLE_TEXT
