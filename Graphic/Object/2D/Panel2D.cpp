#include "Panel2D.h"
#include <cstring>

#ifndef __ANDROID__
#include "OpenGLES/ES2/gl.h"
#endif

namespace eng {

//////
Panel2D::Panel2D(bool manage) : Object2D(manage) {

    LOGV(LIBENG_LOG_PANEL, 0, LOG_FORMAT(" - m:%s"), __PRETTY_FUNCTION__, __LINE__, (manage)? "true":"false");

    mVertices = new float[8];
    std::memset(mVertices, 0, sizeof(float) * 8);
}
Panel2D::~Panel2D() {

    LOGV(LIBENG_LOG_PANEL, 0, LOG_FORMAT(), __PRETTY_FUNCTION__, __LINE__);
    delete [] mVertices;
}

void Panel2D::positionVertices(short x0, short y0, short x2, short y2) {

    LOGV(LIBENG_LOG_PANEL, 0, LOG_FORMAT(" - x0:%d; y0:%d; x2:%d; y2:%d"), __PRETTY_FUNCTION__, __LINE__, x0, y0,
            x2, y2);

    mVertices[0] = mScreen->left + (x0 * mScreen->ratioW);
    mVertices[1] = mScreen->bottom + (y0 * mScreen->ratioH);
    mVertices[2] = mVertices[0];
    mVertices[3] = mScreen->bottom + (y2 * mScreen->ratioH);
    mVertices[4] = mScreen->left + (x2 * mScreen->ratioW);
    mVertices[5] = mVertices[3];
    mVertices[6] = mVertices[4];
    mVertices[7] = mVertices[1];
}

void Panel2D::render(bool unused) const {

#ifdef DEBUG
    LOGV(LIBENG_LOG_PANEL, (*mLog % 100), LOG_FORMAT(), __PRETTY_FUNCTION__, __LINE__);
#endif
    Object2D::render(false);
    glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_BYTE, PANEL_INDICE_BUFFER);
}

//////
void Back2D::setTexCoords(const float coords[8]) {

    LOGV(LIBENG_LOG_PANEL, 0, LOG_FORMAT(" - c:%p"), __PRETTY_FUNCTION__, __LINE__, coords);
    if (isFullCoordsBuffer())
        mTexCoords = new float[8];

    mTexCoords[0] = coords[0];
    mTexCoords[1] = coords[1];
    mTexCoords[2] = coords[2];
    mTexCoords[3] = coords[3];
    mTexCoords[4] = coords[4];
    mTexCoords[5] = coords[5];
    mTexCoords[6] = coords[6];
    mTexCoords[7] = coords[7];
}

void Back2D::start(unsigned char texture) {

    LOGV(LIBENG_LOG_PANEL, 0, LOG_FORMAT(" - t:%d"), __PRETTY_FUNCTION__, __LINE__, texture);
    Panel2D::start(texture);

    initScreenVertices();
}
void Back2D::start(unsigned char red, unsigned char green, unsigned char blue) {

    LOGV(LIBENG_LOG_PANEL, 0, LOG_FORMAT(" - r:%d; g:%d; b:%d"), __PRETTY_FUNCTION__, __LINE__, red, green, blue);
    Panel2D::start(red, green, blue);

    initScreenVertices();
}

void Back2D::render(bool resetUniform) const {

#ifdef DEBUG
    LOGV(LIBENG_LOG_PANEL, (*mLog % 100), LOG_FORMAT(" - u:%s"), __PRETTY_FUNCTION__, __LINE__,
            (resetUniform)? "true":"false");
#endif
    if (resetUniform)
        transform();
    Panel2D::render(false);
}

} // namespace
