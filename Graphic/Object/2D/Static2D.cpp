#include <libeng/Graphic/Object/2D/Static2D.h>
#include <cstring>

namespace eng {

//////
Static2D::Static2D(bool manage) : Panel2D(manage) {

    LOGV(LIBENG_LOG_STATIC, 0, LOG_FORMAT(" - m:%s"), __PRETTY_FUNCTION__, __LINE__, (manage)? "true":"false");

    mTexCoords = new float[8];
    std::memset(mTexCoords, 0, sizeof(float) * 8);
}
Static2D::~Static2D() { LOGV(LIBENG_LOG_STATIC, 0, LOG_FORMAT(), __PRETTY_FUNCTION__, __LINE__); }

void Static2D::setVertices(short x0, short y0, short x2, short y2) {

    LOGV(LIBENG_LOG_STATIC, 0, LOG_FORMAT(), __PRETTY_FUNCTION__, __LINE__);
    positionVertices(x0, y0, x2, y2);
}
void Static2D::render(bool resetUniform) const {

#ifdef DEBUG
    LOGV(LIBENG_LOG_STATIC, (*mLog % 100), LOG_FORMAT(" - u:%s"), __PRETTY_FUNCTION__, __LINE__,
            (resetUniform)? "true":"false");
#endif
    if (resetUniform)
        transform();
    Panel2D::render(false);
}

} // namespace
