#ifndef LIBENG_SCROLLING_H_
#define LIBENG_SCROLLING_H_
#if defined(__cplusplus)

#include <libeng/Global.h>
#ifdef LIBENG_ENABLE_SCROLLING

#include <libeng/Graphic/Scrolling/ScrollPanel.h>
#include <libeng/Render/2D/Shader2D.h>
#include <libeng/Textures/Textures.h>
#include <libeng/Graphic/Bounds/BorderLeft.h>
#include <libeng/Graphic/Bounds/BorderRight.h>
#include <libeng/Graphic/Bounds/BorderTop.h>
#include <libeng/Graphic/Bounds/BorderBottom.h>
#include <assert.h>

namespace eng {

class Game2D;

//////
class Scroll : public ScrollPanel {

public:
    inline float getScrollLeft() const { assert(mCenter); assert(mCenter->panel); return mCenter->panel->getScrollLeft(); }
    inline float getScrollRight() const { assert(mCenter); assert(mCenter->panel); return mCenter->panel->getScrollRight(); }
    inline float getScrollTop() const { assert(mCenter); assert(mCenter->panel); return mCenter->panel->getScrollTop(); }
    inline float getScrollBottom() const { assert(mCenter); assert(mCenter->panel); return mCenter->panel->getScrollBottom(); }

private:
    const ShaderParam2D* mShader2D;
    Textures* mTextures;

    unsigned char mSwap; // Panel move distance when swapping

#ifdef DEBUG
    const unsigned int* mLog;
#endif
    unsigned char mTextureIdx;
    unsigned char mCount; // Scroll panel count

    typedef struct Element {

        Element* prev;
        ScrollPanel* panel;
        Element* next;

    } ScrollElement;

    ScrollElement* mHead;
    ScrollElement** mPanelList; // Indexed list to make fast walk when accessing panels (see 'assign', 'operator[]', etc)

    ScrollElement* mLeft; // |mTop
    ScrollElement* mCenter;
    ScrollElement* mRight; // |mBottom
    // ...3 visible panels always center in the middle of the scroll list (see 'render' method)

    Bounds* mBounds[2];

public:
    Scroll(unsigned char count = 3, bool manage = true);
    virtual ~Scroll();

    //
    inline void copy(const ScrollPanel &src) {

        LOGV(LIBENG_LOG_SCROLLING, 0, LOG_FORMAT(), __PRETTY_FUNCTION__, __LINE__);
        assert(NULL); // Not implemented yet!
    }
    inline unsigned char getTextureIdx() const { assert(mScrollTexMan); return mTextureIdx; }

    inline void assign(unsigned char index, ScrollPanel* panel) {

        assert(index < mCount);
        assert(!mPanelList[index]->panel);
        mPanelList[index]->panel = panel;
    }
    inline ScrollPanel* operator[](unsigned char index) { assert(index < mCount); return mPanelList[index]->panel; }
    inline const ScrollPanel* operator[](unsigned char index) const {

        assert(index < mCount);
        return mPanelList[index]->panel;
    }

    //////
    void initScroll(const Game2D* game);
    void pauseScroll();
    inline void resetScroll(unsigned char count = 3, unsigned char rank = 0) {

        LOGV(LIBENG_LOG_SCROLLING, 0, LOG_FORMAT(), __PRETTY_FUNCTION__, __LINE__);
        assert(!rank); // Not implemented yet!
        assert(mCount == count);

        ScrollElement* walk = mHead;
        do {

            walk->panel->resetScroll(mCount, rank);
            walk = walk->next;
            ++rank;
        }
        while (walk != mHead);
    }
    // Check panel rank using 'ScrollPanel::getRank' method after having reseting the scroll to know panel position

    void resumeScroll(unsigned char textureId);
protected:
    void startScroll(unsigned char texture);

public:
    void startScroll(unsigned char textureId, bool horizontal, unsigned char rank = 0, unsigned char count = 3);
    // 'horizontal' == false -> vertical scrolling

    inline void scrollX(float x, bool texSwap = false) { // See 'texSwap' flag description below

#ifdef DEBUG
        LOGV(LIBENG_LOG_SCROLLING, (*mLog % 100), LOG_FORMAT(" - x:%f; t:%s"), __PRETTY_FUNCTION__, __LINE__, x,
                (texSwap)? "true":"false");
#endif
        for (unsigned char i = 0; i < mCount; ++i)
            mPanelList[i]->panel->scrollX(x, texSwap);

        if (!mOrientation) // Vertical scrolling
            return;

        // Check if needed to swap panels
        assert(mBounds[0]);
        assert(mBounds[1]);
        if (mCenter->panel->checkCollision(mBounds[0], ScrollPanel::OVF_RIGHT)) {

            LOGI(LIBENG_LOG_SCROLLING, 0, LOG_FORMAT(" - Scroll on right"), __PRETTY_FUNCTION__, __LINE__);
            mHead->panel->scrollX(static_cast<float>(mSwap), texSwap);

            mHead = mHead->next;
            mLeft = mLeft->next;
            mCenter = mCenter->next;
            mRight = mRight->next;
#ifdef DEBUG
            if (texSwap) {

                assert(mCount == 3);
                mRight->panel->copy(*mLeft->panel);
            }
#else
            if (texSwap)
                mRight->panel->copy(*mLeft->panel);
#endif
        }
        else if (mCenter->panel->checkCollision(mBounds[1], ScrollPanel::OVF_LEFT)) {

            LOGI(LIBENG_LOG_SCROLLING, 0, LOG_FORMAT(" - Scroll on left"), __PRETTY_FUNCTION__, __LINE__);
            mHead->prev->panel->scrollX(static_cast<float>(-mSwap), texSwap);

            mHead = mHead->prev;
            mLeft = mLeft->prev;
            mCenter = mCenter->prev;
            mRight = mRight->prev;
#ifdef DEBUG
            if (texSwap) {

                assert(mCount == 3);
                mLeft->panel->copy(*mRight->panel);
            }
#else
            if (texSwap)
                mLeft->panel->copy(*mRight->panel);
#endif
        }
    }
    inline void scrollY(float y, bool texSwap = false) { // See 'texSwap' flag description below

#ifdef DEBUG
        LOGV(LIBENG_LOG_SCROLLING, (*mLog % 100), LOG_FORMAT(" - y:%f; t:%s"), __PRETTY_FUNCTION__, __LINE__, y,
                (texSwap)? "true":"false");
#endif
        for (unsigned char i = 0; i < mCount; ++i)
            mPanelList[i]->panel->scrollY(y, texSwap);

        if (mOrientation) // Horizontal scrolling
            return;

        // Check if needed to swap panels
        assert(mBounds[0]);
        assert(mBounds[1]);
        if (mCenter->panel->checkCollision(mBounds[0], ScrollPanel::OVF_TOP)) {

            LOGI(LIBENG_LOG_SCROLLING, 0, LOG_FORMAT(" - Scroll on top"), __PRETTY_FUNCTION__, __LINE__);
            mHead->panel->scrollY(static_cast<float>(mSwap), texSwap);

            mHead = mHead->next;
            mLeft = mLeft->next;
            mCenter = mCenter->next;
            mRight = mRight->next;
#ifdef DEBUG
            if (texSwap) {

                assert(mCount == 3);
                mRight->panel->copy(*mLeft->panel);
            }
#else
            if (texSwap)
                mRight->panel->copy(*mLeft->panel);
#endif
        }
        else if (mCenter->panel->checkCollision(mBounds[1], ScrollPanel::OVF_BOTTOM)) {

            LOGI(LIBENG_LOG_SCROLLING, 0, LOG_FORMAT(" - Scroll on bottom"), __PRETTY_FUNCTION__, __LINE__);
            mHead->prev->panel->scrollY(static_cast<float>(-mSwap), texSwap);

            mHead = mHead->prev;
            mLeft = mLeft->prev;
            mCenter = mCenter->prev;
            mRight = mRight->prev;
#ifdef DEBUG
            if (texSwap) {

                assert(mCount == 3);
                mLeft->panel->copy(*mRight->panel);
            }
#else
            if (texSwap)
                mLeft->panel->copy(*mRight->panel);
#endif
        }
    }
    // -> 'texSwap' flag is used when there are only 2 texture coordinates for 3 panels. Needed to swap texture
    //    coordinates when swapping panel
    // WARNING: 'texSwap' case can be used only when there is no difference between panel vertices coordinates

    inline bool checkCollision(const Bounds* bounds, Overflow ovf) {

#ifdef DEBUG
        LOGV(LIBENG_LOG_SCROLLING, (*mLog % 100), LOG_FORMAT(" - b:%p; o:%d"), __PRETTY_FUNCTION__, __LINE__, bounds,
                static_cast<int>(ovf));
        if (!bounds) {

            LOGE(LOG_FORMAT(" - Missing bounds (NULL)"), __PRETTY_FUNCTION__, __LINE__);
            assert(NULL);
        }
#endif
        switch (ovf) {

            case ScrollPanel::OVF_LEFT:     return (getScrollLeft() > bRightVia(bounds)->mRight);
            case ScrollPanel::OVF_RIGHT:    return (getScrollRight() < bLeftVia(bounds)->mLeft);
            case ScrollPanel::OVF_BOTTOM:   return (getScrollBottom() > bTopVia(bounds)->mTop);
            case ScrollPanel::OVF_TOP:      return (getScrollTop() < bBotVia(bounds)->mBottom);
            default: {

                LOGF(LOG_FORMAT(" - Unknown scroll overflow: %d"), __PRETTY_FUNCTION__, __LINE__, static_cast<int>(ovf));
                assert(NULL);
                break;
            }
        }
        return false;
    }

    void renderScroll(bool resetUniform) const;

};

} // namespace

// Cast(s)
#define scrollVia(p)    static_cast<eng::Scroll*>(p)

#endif // LIBENG_ENABLE_SCROLLING
#endif // __cplusplus
#endif // LIBENG_SCROLLING_H_
