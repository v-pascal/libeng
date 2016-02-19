#ifndef LIBENG_PANEL_H_
#define LIBENG_PANEL_H_
#if defined(__cplusplus)

#include <libeng/Global.h>
#ifdef LIBENG_ENABLE_SCROLLING

#include <libeng/Graphic/Object/2D/Static2D.h>
#include <libeng/Graphic/Scrolling/ScrollPanel.h>
#include <libeng/Graphic/Bounds/BorderLeft.h>
#include <libeng/Graphic/Bounds/BorderRight.h>
#include <libeng/Graphic/Bounds/BorderTop.h>
#include <libeng/Graphic/Bounds/BorderBottom.h>
#include <assert.h>

namespace eng {

//////
class Panel : public Static2D, public ScrollPanel {

public:
    Panel(bool manage = false);
    virtual ~Panel();

    inline float getScrollLeft() const { return mVertices[0]; }
    inline float getScrollRight() const { return mVertices[4]; }
    inline float getScrollTop() const { return mVertices[1]; }
    inline float getScrollBottom() const { return mVertices[3]; }

    inline void copy(const ScrollPanel &src) {

#ifdef LIBENG_USE_RTTI
        copyTexCoords(static_cast<const Static2D&>(dynamic_cast<const Panel&>(src)));
#else
        assert(NULL); // Enable RTTI to be able to use this method
#endif
    }

    inline void setVerticesH(const float verticesY[4]) {

#ifdef DEBUG
        if (!mOrientation) {
            LOGW(LOG_FORMAT(" - Should be used only to set initial position"), __PRETTY_FUNCTION__, __LINE__);
            // Avoid to change horizontal position which is a 'Scroll' class charge (in that orientation)
        }
#endif
        mVertices[1] = verticesY[0]; // y0
        mVertices[3] = verticesY[1]; // y1
        mVertices[5] = verticesY[2]; // y2
        mVertices[7] = verticesY[3]; // y3
    }
    inline void setVerticesV(const float verticesX[4]) {

#ifdef DEBUG
        if (mOrientation) {
            LOGW(LOG_FORMAT(" - Should be used only to set initial position"), __PRETTY_FUNCTION__, __LINE__);
            // Avoid to change vertical position which is a 'Scroll' class charge (in that orientation)
        }
#endif
        mVertices[0] = verticesX[0]; // x0
        mVertices[2] = verticesX[1]; // x1
        mVertices[4] = verticesX[2]; // x2
        mVertices[6] = verticesX[3]; // x3
    }

    //////
    inline void initScroll(const Game2D* game) {

        LOGV(LIBENG_LOG_SCROLLING, 0, LOG_FORMAT(" - g:%p"), __PRETTY_FUNCTION__, __LINE__, game);
        Object2D::initialize(game);
    }
    void pauseScroll() { Object2D::pause(); }
    inline void resetScroll(unsigned char count = 3, unsigned char rank = 0) {

        LOGV(LIBENG_LOG_SCROLLING, 0, LOG_FORMAT(" - c:%d; r:%d"), __PRETTY_FUNCTION__, __LINE__, count, rank);
        assert(rank < count);

        // Set to middle position
        if (mOrientation) {

            mVertices[0] = UNIT_VERTEX_BUFFER[0];
            mVertices[2] = UNIT_VERTEX_BUFFER[2];
            mVertices[4] = UNIT_VERTEX_BUFFER[4];
            mVertices[6] = UNIT_VERTEX_BUFFER[6];
        }
        else {

            mVertices[1] = UNIT_VERTEX_BUFFER[1];
            mVertices[3] = UNIT_VERTEX_BUFFER[3];
            mVertices[5] = UNIT_VERTEX_BUFFER[5];
            mVertices[7] = UNIT_VERTEX_BUFFER[7];
        }
        mRank = rank;

        // Position the panel according its rank
        count = count >> 1;
        if (mRank != count)
            (mOrientation)? scrollX(static_cast<float>((mRank - count) * UNIT_SIZE)):
                    scrollY(static_cast<float>((mRank - count) * UNIT_SIZE));
        //else // Rank #1 stay in middle position
    }
    void resumeScroll(unsigned char textureId) { assert(NULL); } // Use 'Object2D::resume' instead
protected:
    void startScroll(unsigned char textureIdx) { assert(mScrollTexMan); Object2D::start(textureIdx); }

public:
    void startScroll(unsigned char textureId, bool horizontal, unsigned char rank = 0, unsigned char count = 3);
    // ...'textureId' is unused in this context

    inline void scrollX(float x, bool texSwap = false) {

        mVertices[0] += x;
        mVertices[2] += x;
        mVertices[4] += x;
        mVertices[6] += x;
    }
    inline void scrollY(float y, bool texSwap = false) {

        mVertices[1] += y;
        mVertices[3] += y;
        mVertices[5] += y;
        mVertices[7] += y;
    }
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

            case ScrollPanel::OVF_LEFT:     return (mVertices[0] > bRightVia(bounds)->mRight);
            case ScrollPanel::OVF_RIGHT:    return (mVertices[4] < bLeftVia(bounds)->mLeft);
            case ScrollPanel::OVF_BOTTOM:   return (mVertices[3] > bTopVia(bounds)->mTop);
            case ScrollPanel::OVF_TOP:      return (mVertices[1] < bBotVia(bounds)->mBottom);
            default: {

                LOGF(LOG_FORMAT(" - Unknown scroll overflow: %d"), __PRETTY_FUNCTION__, __LINE__, static_cast<int>(ovf));
                assert(NULL);
                break;
            }
        }
        return false;
    }

    void renderScroll(bool unused) const;

};

} // namespace

// Cast(s)
#define panelVia(p) static_cast<eng::Panel*>(p)

#endif // LIBENG_ENABLE_SCROLLING
#endif // __cplusplus
#endif // LIBENG_PANEL_H_
