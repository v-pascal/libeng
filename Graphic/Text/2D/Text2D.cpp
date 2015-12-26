#include "Text2D.h"

#ifdef LIBENG_ENABLE_TEXT
#include <libeng/Graphic/Object/2D/Dynamic2D.h>
#include <libeng/Game/2D/Game2D.h>
#include <algorithm>
#include <assert.h>

#ifndef __ANDROID__
#include "OpenGLES/ES2/gl.h"
#endif

namespace eng {

//////
Text2D::Text2D() : mGame(NULL), mTextures(NULL), mFonts(NULL), mFontIdx(0), mStarted(false) {

    LOGV(LIBENG_LOG_TEXT, 0, LOG_FORMAT(), __PRETTY_FUNCTION__, __LINE__);
}
Text2D::Text2D(short font) : mGame(NULL), mTextures(NULL), mFonts(NULL), mFontIdx(font), mStarted(false) {

    LOGV(LIBENG_LOG_TEXT, 0, LOG_FORMAT(" - f:%d"), __PRETTY_FUNCTION__, __LINE__, font);
}
Text2D::~Text2D() {

#ifdef DEBUG
    LOGV(LIBENG_LOG_TEXT, 0, LOG_FORMAT(), __PRETTY_FUNCTION__, __LINE__);
    if (!mTextures) {
        LOGW(LOG_FORMAT(" - Text2D not initialized"), __PRETTY_FUNCTION__, __LINE__);
        return;
    }
#endif
    unsigned char textureIdx = mTextures->getIndex(TEXTURE_ID_FONT);
    if (textureIdx != TEXTURE_IDX_INVALID)
        mTextures->delTexture(textureIdx);

    for (std::list<Char2D*>::iterator iter = mText2D.begin(); iter != mText2D.end(); ++iter)
        delete (*iter);
    mText2D.clear();
}

void Text2D::scale(float x, float y) {

    LOGV(LIBENG_LOG_TEXT, 0, LOG_FORMAT(" - x:%f; y:%f"), __PRETTY_FUNCTION__, __LINE__, x, y);
    assert(mText2D.size());

    std::list<Char2D*>::iterator iter = mText2D.begin();
    float xTrans = (*iter)->getLeft();
    float yTrans = (*iter)->getTop();
    (*iter)->scale(x, y);

    xTrans = (xTrans - (*iter)->getLeft()) * 0.5f;
    yTrans = (yTrans - (*iter)->getTop()) * 0.5f;
    (*iter)->translate(xTrans, yTrans);

    ++iter;
    for ( ; iter != mText2D.end(); ++iter) {

        (*iter)->scale(x, y);
        (*iter)->translate(xTrans, yTrans);
    }
    // WARNING: Make a scale before positionning it (before 'position' method call)
}
void Text2D::position(float left, float top) {

    LOGV(LIBENG_LOG_TEXT, 0, LOG_FORMAT(" - l:%f; t:%f"), __PRETTY_FUNCTION__, __LINE__, left, top);
    assert(mText2D.size());

    std::list<Char2D*>::iterator iter = mText2D.begin();
    for (short i = 0, charRank = 0; (i < static_cast<short>(mText.size())) && (iter != mText2D.end());
            ++i, ++charRank, ++iter) {

        if (mText.at(i) != L'\n')
            (*iter)->position(left, top, charRank);
        else {

            LOGI(LIBENG_LOG_TEXT, 0, LOG_FORMAT(), __PRETTY_FUNCTION__, __LINE__);
            assert(iter != mText2D.begin());

            --iter;
            top = (*iter)->getBottom();
            charRank = -1; // Reset character rank (begin new line)
        }
    }
    // WARNING: Position text after having scale it (after 'scale' method call)
}

float Text2D::getRight() const {

    LOGV(LIBENG_LOG_TEXT, 0, LOG_FORMAT(), __PRETTY_FUNCTION__, __LINE__);
    assert(mText2D.size());
    assert(mText.size());

    float charWidth = (*mText2D.begin())->getRight() - (*mText2D.begin())->getLeft();

    size_t pos = mText.find(L'\n', 0);
    if (pos != std::wstring::npos) {

        size_t prev = 0;
        float right = 0.f;
        do {

            right = std::max<float>(right, (pos - prev) * charWidth);
            prev = ++pos;
            pos = mText.find(L'\n', pos);
        }
        while (pos != std::wstring::npos);
        return std::max<float>(right, (mText.size() - prev) * charWidth) + (*mText2D.begin())->getLeft();
    }
    return (mText.size() * charWidth) + (*mText2D.begin())->getLeft();
}

void Text2D::start(const std::wstring& text) {

#ifdef DEBUG
    LOGV(LIBENG_LOG_TEXT, 0, LOG_FORMAT(" - t:%s...(%u)"), __PRETTY_FUNCTION__, __LINE__, text.c_str(),
            static_cast<unsigned int>(text.size())); // Display first char... + length
    assert(mTextures); // Check initialization
    assert(text.size());
    assert(text.at(0) != L' ');
    assert(text.at(text.size() - 1) != L' ');
    assert(text.at(0) != L'\n');
#endif
    resume();

    assert(!mText2D.size());
    for (short i = 0, charRank = 0; i < static_cast<short>(text.size()); ++i, ++charRank) {

        if (text.at(i) != L'\n') {

            LOGI(LIBENG_LOG_TEXT, 0, LOG_FORMAT(" - Character: 0x%X"), __PRETTY_FUNCTION__, __LINE__, text.at(i));

            Char2D* character = new Char2D;
            character->initialize(mGame);
            character->start(mFonts, mFontIdx, text.at(i), charRank);

            mText2D.push_back(character);
        }
        else {

            LOGI(LIBENG_LOG_TEXT, 0, LOG_FORMAT(" - New line"), __PRETTY_FUNCTION__, __LINE__);
            assert((i + 1) < static_cast<short>(text.size()));

            translate(0.f, std::abs<float>((*mText2D.rbegin())->getBottom() - (*mText2D.rbegin())->getTop()));
            charRank = -1; // Reset character rank (begin new line)
        }
    }
    mText = text;
    mStarted = true;
}
void Text2D::update(const std::wstring& text) {

    LOGV(LIBENG_LOG_TEXT, 0, LOG_FORMAT(" - t:%s... (T:%s...[%u])"), __PRETTY_FUNCTION__, __LINE__, text.c_str(),
            mText.c_str(), static_cast<unsigned int>(mText.size()));
    assert(text.size());
    assert(text.at(0) != L' ');
    assert(text.at(text.size() - 1) != L' ');
    assert(text.at(0) != L'\n');
    assert(text.find(L'\n', 0) == std::wstring::npos);
    assert(mText.size());
    assert(mText.find(L'\n', 0) == std::wstring::npos);
    assert(mText != text);

    std::list<Char2D*>::iterator iter = mText2D.begin();
    short newIdx = 0;
    for ( ; (newIdx < static_cast<short>(text.size())) && (newIdx < static_cast<short>(mText.size())); ++newIdx, ++iter) {
        if (mText.at(newIdx) != text.at(newIdx))
            (*iter)->update(mFonts, mFontIdx, text.at(newIdx));
    }
    for ( ; (newIdx < static_cast<short>(text.size())); ++newIdx)
        addCharacter(text.at(newIdx), newIdx);
    for ( ; (newIdx < static_cast<short>(mText.size())); ++newIdx) {
        delete (*iter);
        iter = mText2D.erase(iter);
    }

    mText = text;
}
void Text2D::render(bool resetUniform) const {

#ifdef DEBUG
    LOGV(LIBENG_LOG_TEXT, (*mGame->getLog() % 100), LOG_FORMAT(" - r:%s"), __PRETTY_FUNCTION__, __LINE__,
            (resetUniform)? "true":"false");
#endif
    // Reset transform matrix (if needed)
    if (resetUniform) {

        glUniform1f(mGame->getShader2D()->transXLoc, Object2D::TransformBuffer[Dynamic2D::TRANS_X]);
        glUniform1f(mGame->getShader2D()->transYLoc, Object2D::TransformBuffer[Dynamic2D::TRANS_Y]);
        glUniform1f(mGame->getShader2D()->scaleXLoc, Object2D::TransformBuffer[Dynamic2D::SCALE_X]);
        glUniform1f(mGame->getShader2D()->scaleYLoc, Object2D::TransformBuffer[Dynamic2D::SCALE_Y]);
        glUniform1f(mGame->getShader2D()->rotateLoc, Object2D::TransformBuffer[Dynamic2D::ROTATE_C]);
    }
    for (std::list<Char2D*>::const_iterator iter = mText2D.begin(); iter != mText2D.end(); ++iter)
        (*iter)->render();
}

} // namespace

#endif // LIBENG_ENABLE_TEXT
