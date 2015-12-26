#include "Panel.h"

#ifdef LIBENG_ENABLE_SCROLLING
#include <libeng/Global.h>
#include <cstring>

namespace eng {

//////
Panel::Panel(bool manage) : Static2D(manage), ScrollPanel(!manage) { // As default texture management is done in 'Scroll' class

    LOGV(LIBENG_LOG_SCROLLING, 0, LOG_FORMAT(" - m:%s"), __PRETTY_FUNCTION__, __LINE__, (manage)? "true":"false");
    ScrollPanel::setUnitVertices(mVertices);
}
Panel::~Panel() { LOGV(LIBENG_LOG_SCROLLING, 0, LOG_FORMAT(), __PRETTY_FUNCTION__, __LINE__); }

void Panel::startScroll(unsigned char textureId, bool horizontal, unsigned char rank, unsigned char count) {

#ifdef DEBUG
    LOGV(LIBENG_LOG_SCROLLING, 0, LOG_FORMAT(" - t:%d; h:%s; r:%d; c:%d"), __PRETTY_FUNCTION__, __LINE__, textureId,
            (horizontal)? "Horizontal":"Vertical", rank, count);
    if (mScrollStarted)
        assert(mOrientation != horizontal); // Cannot start a scroll of scrolls that are oriented in same direction
    assert(rank < count);
#endif
    // Position the panel according its rank
    count = count >> 1;
    if (rank != count)
        (horizontal)? scrollX(static_cast<float>((rank - count) * UNIT_SIZE)):
                scrollY(static_cast<float>((rank - count) * UNIT_SIZE));
    //else // Rank #1 stay in middle position

    if (mScrollStarted)
        return;

    std::memcpy(mTexCoords, FULL_TEXCOORD_BUFFER, sizeof(float) * 8);
    mOrientation = horizontal;
    mRank = rank;

    mScrollStarted = true;
}

void Panel::renderScroll(bool unused) const {

#ifdef DEBUG
    LOGV(LIBENG_LOG_SCROLLING, (*mLog % 100), LOG_FORMAT(), __PRETTY_FUNCTION__, __LINE__);
#endif
    //transform(); // Called in 'Scroll::renderScroll' method
    Panel2D::render(false);
}

} // namespace

#endif // LIBENG_ENABLE_SCROLLING
