#ifndef LIBENG_SCROLLPANEL_H_
#define LIBENG_SCROLLPANEL_H_
#if defined(__cplusplus)

#include <libeng/Global.h>
#ifdef LIBENG_ENABLE_SCROLLING

#include <libeng/Graphic/Bounds/Bounds.h>
#include <cstring>

namespace eng {

static const unsigned char UNIT_SIZE = 2; // -1 to 1 == 2
static const float UNIT_VERTEX_BUFFER[] = { -1.f, 1.f, -1.f, -1.f, 1.f, -1.f, 1.f, 1.f };

class Game2D;
class Scroll;

//////
class ScrollPanel {

    friend class Scroll;

public:
    virtual float getScrollLeft() const = 0;
    virtual float getScrollRight() const = 0;
    virtual float getScrollTop() const = 0;
    virtual float getScrollBottom() const = 0;

    typedef enum {

        OVF_LEFT = 0,
        OVF_RIGHT,
        OVF_TOP,
        OVF_BOTTOM

    } Overflow; // Scrolling overflow

    static void setUnitVertices(float* vertices) {

        vertices[0] = UNIT_VERTEX_BUFFER[0];
        vertices[1] = UNIT_VERTEX_BUFFER[1];
        vertices[2] = UNIT_VERTEX_BUFFER[2];
        vertices[3] = UNIT_VERTEX_BUFFER[3];
        vertices[4] = UNIT_VERTEX_BUFFER[4];
        vertices[5] = UNIT_VERTEX_BUFFER[5];
        vertices[6] = UNIT_VERTEX_BUFFER[6];
        vertices[7] = UNIT_VERTEX_BUFFER[7];
    }
    virtual void copy(const ScrollPanel &src) = 0;

protected:
    unsigned char mRank;

    bool mOrientation; // TRUE: Horizontal; FALSE: Vertical

    bool mScrollStarted; // Only used for a scroll of scrolls
    bool mScrollTexMan; // TRUE: Manage texture; FALSE: Each panel should manage its texture

public:
    ScrollPanel(bool manage) : mOrientation(false), mRank(0), mScrollStarted(false), mScrollTexMan(manage) { }
    virtual ~ScrollPanel() { }

    inline unsigned char getRank() const { return mRank; }

    //////
    virtual void initScroll(const Game2D* game) = 0;
    virtual void pauseScroll() = 0;
    virtual void resetScroll(unsigned char count = 3, unsigned char rank = 0) = 0;
    virtual void resumeScroll(unsigned char textureId) = 0;
protected:
    virtual void startScroll(unsigned char texture) = 0;

public:
    virtual void startScroll(unsigned char textureId, bool horizontal, unsigned char rank = 0,
            unsigned char count = 3) = 0;

    virtual void scrollX(float x, bool texSwap = false) = 0;
    virtual void scrollY(float y, bool texSwap = false) = 0;
    virtual bool checkCollision(const Bounds* bounds, Overflow ovf) = 0;

    virtual void renderScroll(bool resetUniform) const = 0;

};

} // namespace

#endif // LIBENG_ENABLE_SCROLLING
#endif // __cplusplus
#endif // LIBENG_SCROLLPANEL_H_
