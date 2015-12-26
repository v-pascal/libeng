#include "Fonts.h"

#ifdef LIBENG_ENABLE_TEXT
#include <libeng/Textures/Textures.h>

namespace eng {

Fonts* Fonts::mThis = NULL;

//////
Fonts::Fonts() { LOGV(LIBENG_LOG_TEXT, 0, LOG_FORMAT(), __PRETTY_FUNCTION__, __LINE__); }
Fonts::~Fonts() {

    LOGV(LIBENG_LOG_TEXT, 0, LOG_FORMAT(), __PRETTY_FUNCTION__, __LINE__);
    for (std::vector<Font*>::iterator iter = mFonts.begin(); iter != mFonts.end(); ++iter)
        delete (*iter);
    mFonts.clear();
}

void Fonts::addFont(short index, short width, short height, short texWidth, short texHeight, float offset) {

    LOGV(LIBENG_LOG_TEXT, 0, LOG_FORMAT(" - i:%d; w:%d; h:%d; W:%d; H:%d; o:%f"), __PRETTY_FUNCTION__, __LINE__, index,
            width, height, texWidth, texHeight, offset);
    assert(static_cast<short>(mFonts.size()) == index);

    Font* font = new Font;
    font->width = width / static_cast<float>(texWidth);
    font->height = height / static_cast<float>(texHeight);
    font->offset = 0.f;

    if (offset > -0.5f) // != LIBENG_NO_DATA
        font->offset = offset;

    else if (index-- > 0) { // Previous index (if any)

        font->offset += mFonts[index]->offset;
        font->offset += mFonts[index]->height * 3; // Font is defined on 3 lines
    }
    font->widthPix = width;
    font->heightPix = height;

    mFonts.push_back(font);
}

} // namespace

#endif // LIBENG_ENABLE_TEXT
