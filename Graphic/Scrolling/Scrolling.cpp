#include "Scrolling.h"

#ifdef LIBENG_ENABLE_SCROLLING
#include <libeng/Graphic/Object/2D/Object2D.h>
#include <libeng/Graphic/Object/2D/Dynamic2D.h>
#include <libeng/Game/2D/Game2D.h>
#include <libeng/Log/Log.h>

#ifndef __ANDROID__
#include "OpenGLES/ES2/gl.h"
#endif

namespace eng {

//////
Scroll::Scroll(unsigned char count, bool manage) : ScrollPanel(manage), mShader2D(NULL), mTextures(NULL), mCount(count),
        mTextureIdx(TEXTURE_IDX_INVALID), mLeft(NULL), mCenter(NULL), mRight(NULL) {

    LOGV(LIBENG_LOG_SCROLLING, 0, LOG_FORMAT(" - c:%d; m:%s"), __PRETTY_FUNCTION__, __LINE__, count,
            (manage)? "true":"false");
    assert(count >= 3); // See 'render' method
    assert(count % 2); // Count must be odd

    mHead = new ScrollElement;
    mHead->panel = NULL;

    unsigned char leftPos = (mCount >> 1) - 1; // |bottomPos
    if (!leftPos)
        mLeft = mHead;

    mPanelList = new ScrollElement*[mCount];
    memset(mPanelList, 0, sizeof(ScrollElement*) * mCount);
    mPanelList[0] = mHead;

    ScrollElement* Prev;
    ScrollElement* Next = mHead;
    for (unsigned char i = 1; i < mCount; ++i) {

        Prev = Next;
        Next->next = new ScrollElement;
        Next = Next->next;
        Next->panel = NULL;
        Next->prev = Prev;

        mPanelList[i] = Next;

        if (leftPos == i)
            mLeft = Next;
        else if ((leftPos + 1) == i)
            mCenter = Next;
        else if ((leftPos + 2) == i)
            mRight = Next;
    }
    Next->next = mHead;
    mHead->prev = Next;

    if (!mRight) //if ((leftPos + 2) == mCount)
        mRight = Next;

    mBounds[0] = NULL;
    mBounds[1] = NULL;

    mSwap = UNIT_SIZE * mCount;

#ifdef DEBUG
    mLog = NULL;

    LOGI(LIBENG_LOG_SCROLLING, 0, LOG_FORMAT(), __PRETTY_FUNCTION__, __LINE__);
    assert(mLeft);
    assert(mCenter);
    assert(mRight);
#endif
}
Scroll::~Scroll() {

    LOGV(LIBENG_LOG_SCROLLING, 0, LOG_FORMAT(), __PRETTY_FUNCTION__, __LINE__);
    assert(mTextures);

    if (mTextureIdx != TEXTURE_IDX_INVALID)
        mTextures->delTexture(mTextureIdx);

    for (unsigned char i = 0; i < mCount; ++i) {
        if (mPanelList[i]->panel)
            delete mPanelList[i]->panel; // Delete 'ScrollPanel'
        delete mPanelList[i]; // Delete 'ScrollElement'
    }
    delete [] mPanelList;

    if (mBounds[0]) delete mBounds[0];
    if (mBounds[1]) delete mBounds[1];
}

void Scroll::initScroll(const Game2D* game) {

    LOGV(LIBENG_LOG_SCROLLING, 0, LOG_FORMAT(" - g:%p"), __PRETTY_FUNCTION__, __LINE__, game);

    // Check if already initialized
    if (mScrollStarted) // ... has been initialized before having been started
        return; // Ony occurs when initializing a scroll in a scroll of scrolls

    mTextures = Textures::getInstance();
    mShader2D = game->getShader2D();

#ifdef DEBUG
    mLog = game->getLog();
    for (unsigned char i = 0; i < mCount; ++i) {

        assert(mPanelList[i]);
        assert(mPanelList[i]->panel); // Should be defined before calling this (using 'assign' method)
        mPanelList[i]->panel->initScroll(game);
    }
#else
    for (unsigned char i = 0; i < mCount; ++i)
        mPanelList[i]->panel->initScroll(game);
#endif
}
void Scroll::pauseScroll() {

    LOGV(LIBENG_LOG_SCROLLING, 0, LOG_FORMAT(), __PRETTY_FUNCTION__, __LINE__);
    mTextureIdx = TEXTURE_IDX_INVALID;

    for (unsigned char i = 0; i < mCount; ++i)
        mPanelList[i]->panel->pauseScroll();
}
void Scroll::resumeScroll(unsigned char textureId) {

    LOGV(LIBENG_LOG_SCROLLING, 0, LOG_FORMAT(" - t:%d"), __PRETTY_FUNCTION__, __LINE__, textureId);
    assert(mScrollTexMan); // Should resume each panel instead of calling this (using 'Object2D::resume' panels method)

    mTextureIdx = mTextures->getIndex(textureId);
    if (mTextureIdx == TEXTURE_IDX_INVALID)
        mTextureIdx = Textures::loadTexture(textureId); // Load texture

    mTextures->genTexture(mTextureIdx);
    startScroll(textureId); // This parameter is unused in this context
}

void Scroll::startScroll(unsigned char texture) { // 'texture' is unused in this context

    LOGV(LIBENG_LOG_SCROLLING, 0, LOG_FORMAT(" - t:%d"), __PRETTY_FUNCTION__, __LINE__, texture);
    assert(mScrollTexMan);

    for (unsigned char i = 0; i < mCount; ++i)
        mPanelList[i]->panel->startScroll(mTextureIdx);
}

void Scroll::startScroll(unsigned char textureId, bool horizontal, unsigned char rank, unsigned char count) {

#ifdef DEBUG
    LOGV(LIBENG_LOG_SCROLLING, 0, LOG_FORMAT(" - t:%d h:%s; r:%d; c:%d"), __PRETTY_FUNCTION__, __LINE__, textureId,
            (horizontal)? "Horizontal":"Vertical", rank, count);
    if (mScrollStarted)
        assert(mOrientation != horizontal); // Cannot start a scroll of scrolls that are oriented in same direction
    assert(mCount == count);
    assert(rank < count);
#endif
    if (mScrollStarted) {

        LOGI(LIBENG_LOG_SCROLLING, 0, LOG_FORMAT(" - Already started (r:%d)"), __PRETTY_FUNCTION__, __LINE__, rank);
        for (unsigned char i = 0; i < mCount; ++i)
            mPanelList[i]->panel->startScroll(textureId, horizontal, rank, mCount);
        // Keep panel 'startScroll' method call order
        return;
    }

    if (mScrollTexMan)
        resumeScroll(textureId);
    //else // ...'textureId' is unused in this context
    mRank = rank;
    mOrientation = horizontal;

    LOGI(LIBENG_LOG_SCROLLING, 0, LOG_FORMAT(), __PRETTY_FUNCTION__, __LINE__);
    assert(mPanelList[0]);
    assert(mPanelList[0]->panel); // Check if 'initScroll' has been called

    for (unsigned char i = 0; i < mCount; ++i)
        mPanelList[i]->panel->startScroll(textureId, mOrientation, i, mCount);
    // Keep panel 'startScroll' method call order

    if (mOrientation) {

        mBounds[0] = new BorderLeft(0.f);
        mBounds[1] = new BorderRight(0.f);
    }
    else {

        mBounds[0] = new BorderBottom(0.f);
        mBounds[1] = new BorderTop(0.f);
    }
    mScrollStarted = true;
}

void Scroll::renderScroll(bool resetUniform) const {

#ifdef DEBUG
    LOGV(LIBENG_LOG_SCROLLING, (*mLog % 100), LOG_FORMAT(" - r:%s"), __PRETTY_FUNCTION__, __LINE__,
            (resetUniform)? "true":"false");
    assert(mBounds[0]);
#endif
    // Reset transform matrix (if needed)
    if (resetUniform) {

        glUniform1f(mShader2D->transXLoc, Object2D::TransformBuffer[Dynamic2D::TRANS_X]);
        glUniform1f(mShader2D->transYLoc, Object2D::TransformBuffer[Dynamic2D::TRANS_Y]);
        glUniform1f(mShader2D->scaleXLoc, Object2D::TransformBuffer[Dynamic2D::SCALE_X]);
        glUniform1f(mShader2D->scaleYLoc, Object2D::TransformBuffer[Dynamic2D::SCALE_Y]);
        glUniform1f(mShader2D->rotateLoc, Object2D::TransformBuffer[Dynamic2D::ROTATE_C]);
    }
    mLeft->panel->renderScroll(false);
    mCenter->panel->renderScroll(false);
    mRight->panel->renderScroll(false);
    // Only render the left/bottom, center and right/top panels (3 visible panels only)
}

} // namespace

#endif // LIBENG_ENABLE_SCROLLING
