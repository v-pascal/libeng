#ifndef LIBENG_TEXT2D_H_
#define LIBENG_TEXT2D_H_
#if defined(__cplusplus)

#include <libeng/Global.h>
#ifdef LIBENG_ENABLE_TEXT

#include <libeng/Graphic/Text/Fonts.h>
#include <libeng/Log/Log.h>
#include <libeng/Graphic/Object/2D/Panel2D.h>
#include <libeng/Graphic/Text/2D/Char2D.h>
#include <string>
#include <list>

namespace eng {

//////
class Text2D {

private:
    Textures* mTextures;

    const Fonts* mFonts;
    short mFontIdx;
    std::list<Char2D*> mText2D;

    inline void addCharacter(wchar_t character, short charRank) {

        Char2D* newChar = new Char2D(*(*mText2D.begin()));
        newChar->initialize(mGame);
        newChar->start(mFonts, mFontIdx, character, charRank);

        mText2D.push_back(newChar);
    }
    bool mStarted; // FALSE: Need to start object (call 'start' method); TRUE: Need to resume object (call 'resume' method)

protected:
    const Game2D* mGame;
    std::wstring mText;

public:
    Text2D();
    Text2D(short font);
    virtual ~Text2D();

    void initialize(const Game2D* game) {

        LOGV(LIBENG_LOG_TEXT, 0, LOG_FORMAT(" - g:%p"), __PRETTY_FUNCTION__, __LINE__, game);

        mTextures = Textures::getInstance();
        mFonts = Fonts::getInstance();
        // Avoid to check existing instance each time it is used

        mGame = game;
    }
    inline const std::wstring& getText() const { return mText; }
    inline bool isStarted() const { return mStarted; }

    inline void setAlpha(float alpha) {

#ifdef DEBUG
        if (!mText2D.size()) {
            LOGE(LOG_FORMAT(), __PRETTY_FUNCTION__, __LINE__);
            assert(NULL);
        }
#endif
        for (std::list<Char2D*>::iterator iter = mText2D.begin(); iter != mText2D.end(); ++iter)
            (*iter)->setAlpha(alpha);
    }
    inline void setColor(float red, float green, float blue, short idx = LIBENG_NO_DATA) {

        LOGV(LIBENG_LOG_TEXT, 0, LOG_FORMAT(" - r:%f; g:%f; b:%f; i:%d"), __PRETTY_FUNCTION__, __LINE__, red, green,
                blue, idx);
        assert(mText2D.size());

        if (idx != LIBENG_NO_DATA) {

            assert(idx > LIBENG_NO_DATA);
            assert(mText2D.size() > idx);

            std::list<Char2D*>::iterator iter = mText2D.begin();
            while (idx--) ++iter;

            (*iter)->setRed(red);
            (*iter)->setGreen(green);
            (*iter)->setBlue(blue);
        }
        else for (std::list<Char2D*>::iterator iter = mText2D.begin(); iter != mText2D.end(); ++iter) {

            (*iter)->setRed(red);
            (*iter)->setGreen(green);
            (*iter)->setBlue(blue);
        }
    }
    inline void translate(float x, float y) {

        LOGV(LIBENG_LOG_TEXT, 0, LOG_FORMAT(" - x:%f; y:%f"), __PRETTY_FUNCTION__, __LINE__, x, y);
        assert(mText2D.size());

        for (std::list<Char2D*>::iterator iter = mText2D.begin(); iter != mText2D.end(); ++iter)
            (*iter)->translate(x, y);
    }
    void scale(float x, float y);
    // WARNING: Make a scale before positionning it (before 'position' method call)

    void position(float left, float top);
    // WARNING: Position text after having scale it (after 'scale' method call)

    inline float getLeft() const { return (*mText2D.begin())->getLeft(); }
    inline float getTop() const { return (*mText2D.begin())->getTop(); }
    float getRight() const;
    inline float getBottom() const { return (*mText2D.rbegin())->getBottom(); }

    inline void pause() {

        LOGV(LIBENG_LOG_TEXT, 0, LOG_FORMAT(), __PRETTY_FUNCTION__, __LINE__);
        for (std::list<Char2D*>::iterator iter = mText2D.begin(); iter != mText2D.end(); ++iter)
            (*iter)->pause();
    }
    void resume() {

        LOGV(LIBENG_LOG_TEXT, 0, LOG_FORMAT(), __PRETTY_FUNCTION__, __LINE__);
        unsigned char textureIdx = mTextures->getIndex(TEXTURE_ID_FONT);
        if (textureIdx == TEXTURE_IDX_INVALID)
            textureIdx = Textures::loadTexture(TEXTURE_ID_FONT); // Load texture
        mTextures->genTexture(textureIdx);

        // Call 'start' method of all characters
        for (std::list<Char2D*>::iterator iter = mText2D.begin(); iter != mText2D.end(); ++iter)
            (*iter)->Object2D::start(textureIdx);
    }

    //////
    void start(const std::wstring& text);
    void update(const std::wstring& text);
    virtual void render(bool resetUniform) const;

};

} // namespace

#endif // LIBENG_ENABLE_TEXT
#endif // __cplusplus
#endif // LIBENG_TEXT2D_H_
